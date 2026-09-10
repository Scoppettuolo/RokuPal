<?php

/**
 * @file
 * Database interface code for MySQL database servers using the mysqli client libraries. mysqli is included in PHP 5 by default and allows developers to use the advanced features of MySQL 4.1.x, 5.0.x and beyond.
 */

 // Maintainers of this file should consult:
 // http://www.php.net/manual/en/ref.mysqli.php

/**
 * @ingroup database
 * @{
 */

// Include functions shared between mysql and mysqli.
require_once './includes/database.mysql-common.inc';

/**
 * Report database status.
 */
function db_status_report($phase) {
  $t = get_t();

  $version = db_version();

  $form['mysql'] = array(
    'title' => $t('MySQL database'),
    'value' => ($phase == 'runtime') ? l($version, 'admin/reports/status/sql') : $version,
  );

  if (version_compare($version, DRUPAL_MINIMUM_MYSQL) < 0) {
    $form['mysql']['severity'] = REQUIREMENT_ERROR;
    $form['mysql']['description'] = $t('Your MySQL Server is too old. Drupal requires at least MySQL %version.', array('%version' => DRUPAL_MINIMUM_MYSQL));
  }

  return $form;
}

/**
 * Returns the version of the database server currently in use.
 *
 * @return Database server version
 */
function db_version() {
  global $active_db;
  // No live connection (failed login, early bootstrap, installer) — never fatal.
  if (empty($active_db) || !($active_db instanceof mysqli)) {
    return '0.0.0';
  }
  $info = @mysqli_get_server_info($active_db);
  if (!is_string($info) || $info === '') {
    return '0.0.0';
  }
  $parts = explode('-', $info);
  return $parts[0];
}

/**
 * Initialise a database connection.
 *
 * Note that mysqli does not support persistent connections.
 */
function db_connect($url) {
  // Check if MySQLi support is present in PHP
  if (!function_exists('mysqli_init') && !extension_loaded('mysqli')) {
    _db_error_page('Unable to use the MySQLi database because the MySQLi extension for PHP is not installed. Check your <code>php.ini</code> to see how you can enable it.');
  }

  $url = parse_url($url);

  // Decode urlencoded information in the db connection string
  $url['user'] = urldecode($url['user']);
  // Test if database URL has a password.
  $url['pass'] = isset($url['pass']) ? urldecode($url['pass']) : '';
  $url['host'] = urldecode($url['host']);
  $url['path'] = urldecode($url['path']);
  if (!isset($url['port'])) {
    $url['port'] = NULL;
  }

  $connection = mysqli_init();
  @mysqli_real_connect($connection, $url['host'], $url['user'], $url['pass'], substr($url['path'], 1), $url['port'], NULL, MYSQLI_CLIENT_FOUND_ROWS);

  if (mysqli_connect_errno() > 0) {
    _db_error_page(mysqli_connect_error());
  }

  // Prefer utf8mb4 when the server supports it (emoji / full Unicode).
  // Fall back to utf8 for older MySQL / existing schemas.
  $charset = 'utf8';
  if (defined('MYSQLI_SET_CHARSET_NAME')) {
    // Probe server version for utf8mb4 support (MySQL 5.5.3+ / MariaDB 5.5+).
    $ver = mysqli_get_server_info($connection);
    if (preg_match('/^(\d+\.\d+\.\d+)/', $ver, $m) && version_compare($m[1], '5.5.3', '>=')) {
      $charset = 'utf8mb4';
    }
  }
  if (!empty($GLOBALS['db_collation'])) {
    mysqli_query($connection, 'SET NAMES ' . $charset . ' COLLATE ' . $GLOBALS['db_collation']);
  }
  else {
    mysqli_query($connection, 'SET NAMES ' . $charset);
  }
  // Also set connection charset via API when possible.
  if (function_exists('mysqli_set_charset')) {
    @mysqli_set_charset($connection, $charset);
  }

  // MySQL 8 has ONLY_FULL_GROUP_BY mode enabled by default. There are too many queries in Drupal 6 that do not
  // list all columns in GROUP BY clause. So we need to turn off this mode after connecting.
  list ($version) = explode('-', mysqli_get_server_info($connection));
  if (version_compare($version, '8.0.0', '>=')){
    mysqli_query($connection, 'SET sql_mode=(SELECT REPLACE(@@sql_mode,\'ONLY_FULL_GROUP_BY\',\'\'));');
  }

  return $connection;
}

/**
 * Helper function for db_query().
 */
function _db_query($query, $debug = 0) {
  global $active_db, $queries, $user;
  static $dev_query = NULL;

  if ($dev_query === NULL) {
    $dev_query = (bool) variable_get('dev_query', 0);
  }

  if ($dev_query) {
    $timer = microtime(TRUE);
    // If devel.module query logging is enabled, prepend a comment with the username and calling function
    // to the SQL string. This is useful when running mysql's SHOW PROCESSLIST to learn what exact
    // code is issueing the slow query.
    $bt = debug_backtrace(DEBUG_BACKTRACE_IGNORE_ARGS, 4);
    // t() may not be available yet so we don't wrap 'Anonymous'
    $name = (!empty($user->uid) && isset($user->name)) ? $user->name : variable_get('anonymous', 'Anonymous');
    // str_replace() to prevent SQL injection via username or anonymous name.
    $name = str_replace(array('*', '/'), '', (string) $name);
    $fn = isset($bt[2]['function']) ? $bt[2]['function'] : 'unknown';
    $query = '/* '. $name .' : '. $fn .' */ '. $query;
  }

  $result = mysqli_query($active_db, $query);

  if ($dev_query) {
    $fn = isset($bt[2]['function']) ? $bt[2]['function'] : 'unknown';
    $query = $fn ."\n". $query;
    $queries[] = array($query, microtime(TRUE) - $timer);
  }

  if ($debug) {
    print '<p>query: '. $query .'<br />error:'. mysqli_error($active_db) .'</p>';
  }

  $errno = mysqli_errno($active_db);
  if (!$errno) {
    return $result;
  }
  else {
    // Expected race-conditions / optimistic inserts (locks + search index).
    // Do not flood the screen with warnings for these specific cases.
    if ($errno == 1062 && (strpos($query, '{semaphore}') !== FALSE || strpos($query, '{search_') !== FALSE)) {
      return FALSE;
    }
    // Indicate to drupal_error_handler that this is a database error.
    $GLOBALS['drupal_db_error'] = TRUE;
    trigger_error(check_plain(mysqli_error($active_db) ."\nquery: ". $query), E_USER_WARNING);
    return FALSE;
  }
}

/**
 * Fetch one result row from the previous query as an object.
 *
 * @param $result
 *   A database query result resource, as returned from db_query().
 * @return
 *   An object representing the next row of the result, or FALSE. The attributes
 *   of this object are the table fields selected by the query.
 */
function db_fetch_object($result) {
  if ($result) {
    $object = mysqli_fetch_object($result);
    return isset($object) ? $object : FALSE;
  }
}

/**
 * Fetch one result row from the previous query as an array.
 *
 * @param $result
 *   A database query result resource, as returned from db_query().
 * @return
 *   An associative array representing the next row of the result, or FALSE.
 *   The keys of this object are the names of the table fields selected by the
 *   query, and the values are the field values for this result row.
 */
function db_fetch_array($result) {
  if ($result) {
    $array = mysqli_fetch_array($result, MYSQLI_ASSOC);
    return isset($array) ? $array : FALSE;
  }
}

/**
 * Return an individual result field from the previous query.
 *
 * Only use this function if exactly one field is being selected; otherwise,
 * use db_fetch_object() or db_fetch_array().
 *
 * @param $result
 *   A database query result resource, as returned from db_query().
 * @return
 *   The resulting field or FALSE.
 */
function db_result($result) {
  if ($result && mysqli_num_rows($result) > 0) {
    // The mysqli_fetch_row function has an optional second parameter $row
    // but that can't be used for compatibility with Oracle, DB2, etc.
    $array = mysqli_fetch_row($result);
    // PHP 8+: mysqli_fetch_row can return null
    return (is_array($array) && isset($array[0])) ? $array[0] : FALSE;
  }
  return FALSE;
}

/**
 * Determine whether the previous query caused an error.
 */
function db_error() {
  global $active_db;
  return mysqli_errno($active_db);
}

/**
 * Determine the number of rows changed by the preceding query.
 */
function db_affected_rows() {
  global $active_db; /* mysqli connection resource */
  return mysqli_affected_rows($active_db);
}

/**
 * Runs a limited-range query in the active database.
 *
 * Use this as a substitute for db_query() when a subset of the query is to be
 * returned.
 * User-supplied arguments to the query should be passed in as separate parameters
 * so that they can be properly escaped to avoid SQL injection attacks.
 *
 * @param $query
 *   A string containing an SQL query.
 * @param ...
 *   A variable number of arguments which are substituted into the query
 *   using printf() syntax. The query arguments can be enclosed in one
 *   array instead.
 *   Valid %-modifiers are: %s, %d, %f, %b (binary data, do not enclose
 *   in '') and %%.
 *
 *   NOTE: using this syntax will cast NULL and FALSE values to decimal 0,
 *   and TRUE values to decimal 1.
 *
 * @param $from
 *   The first result row to return.
 * @param $count
 *   The maximum number of result rows to return.
 * @return
 *   A database query result resource, or FALSE if the query was not executed
 *   correctly.
 */
function db_query_range($query) {
  $args = func_get_args();
  $count = array_pop($args);
  $from = array_pop($args);
  array_shift($args);

  $query = db_prefix_tables($query);
  if (isset($args[0]) and is_array($args[0])) { // 'All arguments in one array' syntax
    $args = $args[0];
  }
  _db_query_callback($args, TRUE);
  $query = preg_replace_callback(DB_QUERY_REGEXP, '_db_query_callback', $query);
  $query .= ' LIMIT '. (int)$from .', '. (int)$count;
  return _db_query($query);
}

/**
 * Runs a SELECT query and stores its results in a temporary table.
 *
 * Use this as a substitute for db_query() when the results need to be stored
 * in a temporary table.
 * 
 * User-supplied arguments to the query should be passed in as separate parameters
 * so that they can be properly escaped to avoid SQL injection attacks.
 *
 * Note that if you need to know how many results were returned, you should do
 * a SELECT COUNT(*) on the temporary table afterwards. db_affected_rows() does
 * not give consistent result across different database types in this case.
 *
 * @param $query
 *   A string containing a normal SELECT SQL query.
 * @param ...
 *   A variable number of arguments which are substituted into the query
 *   using printf() syntax. The query arguments can be enclosed in one
 *   array instead.
 *   Valid %-modifiers are: %s, %d, %f, %b (binary data, do not enclose
 *   in '') and %%.
 *
 *   NOTE: using this syntax will cast NULL and FALSE values to decimal 0,
 *   and TRUE values to decimal 1.
 * @param $table
 *   The name of the temporary table to select into. This name will not be
 *   prefixed as there is no risk of collision.
 *
 * @return
 *   A database query result resource, or FALSE if the query was not executed
 *   correctly.
 */
function db_query_temporary($query) {
  $args = func_get_args();
  $tablename = array_pop($args);
  array_shift($args);

  $query = preg_replace('/^SELECT/i', 'CREATE TEMPORARY TABLE '. $tablename .' Engine=HEAP SELECT', db_prefix_tables($query));
  if (isset($args[0]) and is_array($args[0])) { // 'All arguments in one array' syntax
    $args = $args[0];
  }
  _db_query_callback($args, TRUE);
  $query = preg_replace_callback(DB_QUERY_REGEXP, '_db_query_callback', $query);
  return _db_query($query);
}

/**
 * Returns a properly formatted Binary Large Object value.
 *
 * @param $data
 *   Data to encode.
 * @return
 *  Encoded data.
 */
function db_encode_blob($data) {
  global $active_db;
  return "'". mysqli_real_escape_string($active_db, $data) ."'";
}

/**
 * Returns text from a Binary Large OBject value.
 *
 * @param $data
 *   Data to decode.
 * @return
 *  Decoded data.
 */
function db_decode_blob($data) {
  return $data;
}

/**
 * Prepare user input for use in a database query, preventing SQL injection attacks.
 */
function db_escape_string($text) {
  global $active_db;
  return mysqli_real_escape_string($active_db, $text);
}

/**
 * Lock a table.
 */
function db_lock_table($table) {
  db_query('LOCK TABLES {'. db_escape_table($table) .'} WRITE');
}

/**
 * Unlock all locked tables.
 */
function db_unlock_tables() {
  db_query('UNLOCK TABLES');
}

/**
 * Check if a table exists.
 *
 * @param $table
 *   The name of the table.
 *
 * @return
 *   TRUE if the table exists, and FALSE if the table does not exist.
 */
function db_table_exists($table) {
  return (bool) db_fetch_object(db_query("SHOW TABLES LIKE '{". db_escape_table($table) ."}'"));
}

/**
 * Check if a column exists in the given table.
 *
 * @param $table
 *   The name of the table.
 * @param $column
 *   The name of the column.
 *
 * @return
 *   TRUE if the column exists, and FALSE if the column does not exist.
 */
function db_column_exists($table, $column) {
  return (bool) db_fetch_object(db_query("SHOW COLUMNS FROM {". db_escape_table($table) ."} LIKE '". db_escape_table($column) ."'"));
}

/**
 * @} End of "ingroup database".
 */

