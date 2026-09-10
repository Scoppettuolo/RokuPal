<?php
/**
 * @file
 * MySQLi connection tests for the installer (PHP 8 / modern messaging).
 */

/**
 * Whether the mysqli extension is loadable.
 */
function mysqli_is_available() {
  return function_exists('mysqli_connect');
}

/**
 * Test that we can connect and run basic DDL on the target database.
 *
 * @param string $url
 *   A connection URL (mysqli://user:pass@host:port/dbname).
 * @param array $success
 *   Populated with successful operation names (CONNECT, CREATE, INSERT, UPDATE, DELETE, DROP).
 *
 * @return bool
 *   TRUE if the database is usable for install.
 */
function drupal_test_mysqli($url, &$success) {
  $success = array();

  if (!mysqli_is_available()) {
    drupal_set_message(st('PHP does not have the <strong>mysqli</strong> extension enabled. Enable it in php.ini and restart Apache.'), 'error');
    return FALSE;
  }

  $parts = parse_url($url);
  if ($parts === FALSE || empty($parts['host'])) {
    drupal_set_message(st('Invalid database connection details.'), 'error');
    return FALSE;
  }

  $user = isset($parts['user']) ? urldecode($parts['user']) : '';
  $pass = isset($parts['pass']) ? urldecode($parts['pass']) : '';
  $host = urldecode($parts['host']);
  $port = isset($parts['port']) ? (int) $parts['port'] : NULL;
  $db   = isset($parts['path']) ? ltrim(urldecode($parts['path']), '/') : '';

  if ($db === '') {
    drupal_set_message(st('Enter a database name. Create an empty database first (phpMyAdmin → New).'), 'error');
    return FALSE;
  }

  $connection = mysqli_init();
  if (!$connection) {
    drupal_set_message(st('Could not initialize MySQLi.'), 'error');
    return FALSE;
  }

  $connected = @mysqli_real_connect($connection, $host, $user, $pass, $db, $port ?: NULL, NULL, MYSQLI_CLIENT_FOUND_ROWS);
  $errno = mysqli_connect_errno();
  $errstr = mysqli_connect_error();

  if (!$connected || $errno) {
    // 1045 = access denied, 1049 = unknown database, 2002/2003 = cannot connect
    if ($errno == 1045) {
      drupal_set_message(st('Access denied for user %user@%host. Check username and password. On XAMPP the default is often user <code>root</code> with an empty password.', array('%user' => $user, '%host' => $host)), 'error');
    }
    elseif ($errno == 1049) {
      drupal_set_message(st('Database %db does not exist. Create it empty in phpMyAdmin, then try again.', array('%db' => $db)), 'error');
    }
    elseif ($errno == 2002 || $errno == 2003) {
      drupal_set_message(st('Cannot reach MySQL at %host. Start MySQL in the XAMPP control panel and confirm the host name.', array('%host' => $host)), 'error');
    }
    else {
      drupal_set_message(st('Could not connect to MySQL (%errno): %error', array('%errno' => $errno, '%error' => $errstr)), 'error');
    }
    return FALSE;
  }

  $success[] = 'CONNECT';

  $query = 'CREATE TABLE drupal_install_test (id int NOT NULL)';
  if (!mysqli_query($connection, $query)) {
    drupal_set_message(st('Connected, but cannot create tables as %user. Grant CREATE/INSERT/UPDATE/DELETE/DROP on database %db.', array('%user' => $user, '%db' => $db)), 'error');
    mysqli_close($connection);
    return FALSE;
  }
  $success[] = 'CREATE';

  if (!mysqli_query($connection, "INSERT INTO drupal_install_test (id) VALUES (1)")) {
    drupal_set_message(st('Cannot INSERT into the test table. Check privileges for %user.', array('%user' => $user)), 'error');
    @mysqli_query($connection, 'DROP TABLE drupal_install_test');
    mysqli_close($connection);
    return FALSE;
  }
  $success[] = 'INSERT';

  if (!mysqli_query($connection, 'UPDATE drupal_install_test SET id = 2')) {
    drupal_set_message(st('Cannot UPDATE the test table. Check privileges for %user.', array('%user' => $user)), 'error');
    @mysqli_query($connection, 'DROP TABLE drupal_install_test');
    mysqli_close($connection);
    return FALSE;
  }
  $success[] = 'UPDATE';

  if (!mysqli_query($connection, 'DELETE FROM drupal_install_test')) {
    drupal_set_message(st('Cannot DELETE from the test table. Check privileges for %user.', array('%user' => $user)), 'error');
    @mysqli_query($connection, 'DROP TABLE drupal_install_test');
    mysqli_close($connection);
    return FALSE;
  }
  $success[] = 'DELETE';

  if (!mysqli_query($connection, 'DROP TABLE drupal_install_test')) {
    drupal_set_message(st('Cannot DROP the test table. Check privileges for %user.', array('%user' => $user)), 'error');
    mysqli_close($connection);
    return FALSE;
  }
  $success[] = 'DROP';

  // Prefer utf8mb4 when available.
  if (!@mysqli_query($connection, "SET NAMES 'utf8mb4' COLLATE 'utf8mb4_unicode_ci'")) {
    @mysqli_query($connection, "SET NAMES 'utf8'");
  }

  mysqli_close($connection);
  return TRUE;
}
