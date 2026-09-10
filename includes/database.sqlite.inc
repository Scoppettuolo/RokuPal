<?php
/**
 * SQLite driver for the modernized Drupal 6 compatibility layer.
 *
 * The legacy database API is preserved at the boundary while the connection
 * uses PDO, exceptions are converted into Drupal errors, and results remain
 * lazy PDO statements to reduce memory consumption.
 */

if (!extension_loaded('pdo_sqlite')) {
  _db_error_page('SQLite support requires the PDO SQLite extension (pdo_sqlite).');
}

if (!function_exists('db_escape_table')) {
  function db_escape_table($string) { return preg_replace('/[^A-Za-z0-9_]+/', '', (string) $string); }
}

function db_status_report($phase = 'runtime') {
  $t = function_exists('get_t') ? get_t() : function ($s) { return $s; };
  return array(
    'title' => $t('SQLite database'),
    'value' => $t('Available'),
    'severity' => REQUIREMENT_OK,
    'description' => $t('SQLite is provided through PDO and is the default database for new installations.'),
  );
}

function db_version() {
  global $active_db;
  static $version;
  if ($version !== NULL) return $version;
  if (!$active_db instanceof PDO) return 'unknown';
  return $version = (string) $active_db->query('SELECT sqlite_version()')->fetchColumn();
}

function _sqlite_path_from_url($url) {
  $raw = is_array($url) ? (isset($url['default']) ? $url['default'] : reset($url)) : $url;
  $raw = (string) $raw;
  if (strpos($raw, "\0") !== FALSE) {
    _db_error_page('SQLite database path contains an invalid null byte.');
  }
  if (strpos($raw, 'sqlite://') === 0) {
    $path = substr($raw, 9);
    if (strpos($path, '/') !== 0) $path = './' . $path;
    return urldecode($path);
  }
  return urldecode($raw);
}

function db_connect($url) {
  $path = _sqlite_path_from_url($url);
  if ($path === '' || $path === '/') {
    _db_error_page('SQLite database path is empty.');
  }
  $directory = dirname($path);
  if (!is_dir($directory) && !@mkdir($directory, 0775, TRUE) && !is_dir($directory)) {
    _db_error_page('SQLite database directory cannot be created: ' . $directory);
  }
  try {
    $pdo = new PDO('sqlite:' . $path, NULL, NULL, array(
      PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION,
      PDO::ATTR_DEFAULT_FETCH_MODE => PDO::FETCH_ASSOC,
      PDO::ATTR_EMULATE_PREPARES => TRUE,
    ));
    $pdo->exec('PRAGMA foreign_keys = ON');
    $pdo->exec('PRAGMA busy_timeout = 5000');
    $pdo->exec('PRAGMA journal_mode = WAL');
    $pdo->exec('PRAGMA synchronous = NORMAL');
    return $pdo;
  }
  catch (Exception $e) {
    _db_error_page($e->getMessage());
  }
}

function _sqlite_expand_query($query, $args) {
  global $db_prefix;
  if (count($args) === 1 && is_array($args[0])) $args = $args[0];
  $prefix = is_array($db_prefix) ? (isset($db_prefix['default']) ? $db_prefix['default'] : '') : (string) $db_prefix;
  $prefix = preg_replace('/[^A-Za-z0-9_]+/', '', (string) $prefix);
  $query = preg_replace('/\{([A-Za-z0-9_]+)\}/', $prefix . '$1', $query);
  if (!$args) return $query;
  _db_query_callback($args, TRUE);
  return preg_replace_callback(DB_QUERY_REGEXP, '_db_query_callback', $query);
}

if (!function_exists('db_query')) {
  function db_query($query) {
    $args = func_get_args();
    array_shift($args);
    return _db_query(_sqlite_expand_query($query, $args));
  }
}

function _sqlite_install_query($query) {
  // Profile/module seed data is replayed when the installer resumes after a
  // timeout or refresh. SQLite must ignore only duplicate INSERTs in that
  // maintenance flow; normal site writes must still report constraint errors.
  if (defined('MAINTENANCE_MODE') && MAINTENANCE_MODE === 'install' && preg_match('/^\\s*INSERT\\s+INTO\\s+/i', $query)) {
    return preg_replace('/^\\s*INSERT\\s+INTO\\s+/i', 'INSERT OR IGNORE INTO ', $query, 1);
  }
  return $query;
}

function _db_query($query, $debug = 0) {
  global $active_db, $queries;
  $started = microtime(TRUE);
  $query = _sqlite_install_query($query);
  // SQLite has no TRUNCATE TABLE. DELETE preserves Drupal's cache-clearing
  // semantics and works with prefixed tables after expansion.
  $query = preg_replace('/^\s*TRUNCATE\s+TABLE\s+/i', 'DELETE FROM ', $query);
  try {
    $result = $active_db->query($query);
  }
  catch (Exception $e) {
    if ($debug || ini_get('display_errors')) {
      trigger_error(check_plain($e->getMessage()) . "\nquery: " . $query, E_USER_WARNING);
    }
    return FALSE;
  }
  if (isset($queries)) {
    $queries[] = array($query, microtime(TRUE) - $started, 'default');
  }
  return $result;
}

function db_fetch_object($result) {
  if (!$result instanceof PDOStatement) return FALSE;
  $row = $result->fetch(PDO::FETCH_ASSOC);
  return $row === FALSE ? FALSE : (object) $row;
}
function db_fetch_array($result) {
  if (!$result instanceof PDOStatement) return FALSE;
  $row = $result->fetch(PDO::FETCH_ASSOC);
  return $row === FALSE ? FALSE : $row;
}
function db_result($result, $row = 0) {
  if (!$result instanceof PDOStatement) return FALSE;
  if ($row > 0) $result->fetch(PDO::FETCH_ASSOC, PDO::FETCH_ORI_ABS, $row);
  $value = $result->fetchColumn(0);
  return $value === FALSE ? FALSE : $value;
}
function db_error() {
  global $active_db;
  return $active_db instanceof PDO ? implode(' ', $active_db->errorInfo()) : 'SQLite connection unavailable';
}
function db_affected_rows() {
  global $active_db;
  if (!$active_db instanceof PDO) return 0;
  return (int) $active_db->query('SELECT changes()')->fetchColumn();
}
function db_last_insert_id($table, $field) {
  global $active_db, $db_prefix;
  if (!($active_db instanceof PDO)) return 0;

  // SQLite/PDO versions differ in how reliably lastInsertId() reports an
  // AUTOINCREMENT value. Try the connection rowid and sqlite_sequence before
  // returning zero; node and comment hooks need the ID immediately.
  $id = (int) $active_db->lastInsertId();
  if ($id <= 0) {
    $id = (int) $active_db->query('SELECT last_insert_rowid()')->fetchColumn();
  }
  if ($id <= 0) {
    $prefix = is_array($db_prefix) ? (isset($db_prefix['default']) ? $db_prefix['default'] : '') : (string) $db_prefix;
    $sequence_table = db_escape_table($prefix . 'sqlite_sequence');
    $table_name = db_escape_table($prefix . $table);
    try {
      $stmt = $active_db->prepare('SELECT seq FROM "' . $sequence_table . '" WHERE name = :name');
      $stmt->execute(array(':name' => $table_name));
      $id = (int) $stmt->fetchColumn();
    }
    catch (Exception $e) {
      // Tables without AUTOINCREMENT have no sqlite_sequence row.
      $id = 0;
    }
  }
  return $id;
}
function db_query_range($query, $from, $count) {
  $args = func_get_args();
  array_shift($args);
  $count = array_pop($args);
  $from = array_pop($args);
  return _db_query(_sqlite_expand_query($query, $args) . ' LIMIT ' . (int) $count . ' OFFSET ' . (int) $from);
}
function db_query_temporary($query) {
  return _db_query(preg_replace('/^\s*SELECT/i', 'CREATE TEMP TABLE ' . db_escape_table('drupal_tmp_' . mt_rand()) . ' AS SELECT', $query, 1));
}
function db_encode_blob($data) {
  // SQLite PDO string quoting is not binary-safe for gzip payloads containing
  // NUL bytes. Hex literals preserve the complete BLOB without truncation.
  return "X'" . bin2hex((string) $data) . "'";
}
function db_decode_blob($data) { return $data; }
function db_escape_string($text) {
  global $active_db;
  if (is_array($text)) $text = implode(',', $text);
  if ($active_db instanceof PDO) {
    $quoted = $active_db->quote((string) $text, PDO::PARAM_STR);
    return substr($quoted, 1, -1);
  }
  return addslashes((string) $text);
}
function db_lock_table($table) { return TRUE; }
function db_unlock_tables() { return TRUE; }
function db_table_exists($table) {
  static $cache = array();
  $table = db_escape_table($table);
  if (array_key_exists($table, $cache)) return $cache[$table];
  return $cache[$table] = (bool) db_result(db_query("SELECT COUNT(*) FROM sqlite_master WHERE type = 'table' AND name = '" . $table . "'"));
}
function db_column_exists($table, $column) {
  static $cache = array();
  $table = db_escape_table($table);
  $column = db_escape_table($column);
  $key = $table . ':' . $column;
  if (array_key_exists($key, $cache)) return $cache[$key];
  $result = db_query("PRAGMA table_info(`$table`)");
  while ($row = db_fetch_array($result)) {
    if ($row['name'] === $column) return $cache[$key] = TRUE;
  }
  return $cache[$key] = FALSE;
}
function db_check_setup() { return TRUE; }

function db_type_map() {
  static $map;
  if ($map !== NULL) return $map;
  return $map = array(
    'varchar:normal' => 'VARCHAR', 'char:normal' => 'CHAR',
    'text:tiny' => 'TEXT', 'text:small' => 'TEXT', 'text:medium' => 'TEXT', 'text:normal' => 'TEXT',
    'text:big' => 'TEXT', 'blob:normal' => 'BLOB', 'int:small' => 'INTEGER',
    'int:normal' => 'INTEGER', 'int:medium' => 'INTEGER', 'int:big' => 'INTEGER',
    'serial:normal' => 'INTEGER', 'float:normal' => 'REAL', 'numeric:normal' => 'NUMERIC',
    'datetime:normal' => 'TEXT',
  );
}

function db_create_table_sql($name, $table) {
  $fields = array();
  foreach ($table['fields'] as $field_name => $spec) {
    $type = isset($spec['sqlite_type']) ? $spec['sqlite_type'] : (db_type_map()[$spec['type'] . ':' . (isset($spec['size']) ? $spec['size'] : 'normal')] ?? 'TEXT');
    $field = '"' . db_escape_table($field_name) . '" ' . $type;
    if ($spec['type'] === 'serial') $field .= ' PRIMARY KEY AUTOINCREMENT';
    elseif (!empty($spec['not null'])) $field .= ' NOT NULL';
    if (isset($spec['default'])) $field .= ' DEFAULT ' . (is_string($spec['default']) ? "'" . str_replace("'", "''", $spec['default']) . "'" : (int) $spec['default']);
    $fields[] = $field;
  }
  if (!empty($table['primary key'])) {
    $primary = array(); foreach ($table['primary key'] as $key) $primary[] = '"' . db_escape_table(is_array($key) ? $key[0] : $key) . '"';
    $has_serial = FALSE; foreach ($table['fields'] as $spec) if ($spec['type'] === 'serial') $has_serial = TRUE;
    if (!$has_serial) $fields[] = 'PRIMARY KEY (' . implode(', ', $primary) . ')';
  }
  // Schema installation can be resumed after a timeout or browser refresh.
  // SQLite must treat already-created objects as a no-op in that case.
  $sql = 'CREATE TABLE IF NOT EXISTS "' . db_escape_table($name) . '" (' . implode(', ', $fields) . ')';
  $statements = array($sql);
  if (!empty($table['unique keys'])) foreach ($table['unique keys'] as $key => $cols) $statements[] = 'CREATE UNIQUE INDEX IF NOT EXISTS "' . db_escape_table($name . '_' . $key) . '" ON "' . db_escape_table($name) . '" (' . implode(', ', array_map(function($c) { return '"' . db_escape_table(is_array($c) ? $c[0] : $c) . '"'; }, $cols)) . ')';
  if (!empty($table['indexes'])) foreach ($table['indexes'] as $key => $cols) $statements[] = 'CREATE INDEX IF NOT EXISTS "' . db_escape_table($name . '_' . $key) . '" ON "' . db_escape_table($name) . '" (' . implode(', ', array_map(function($c) { return '"' . db_escape_table(is_array($c) ? $c[0] : $c) . '"'; }, $cols)) . ')';
  return $statements;
}


