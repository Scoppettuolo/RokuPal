<?php
/**
 * @file
 * Legacy mysql_* installer probe — always unavailable on PHP 7/8.
 *
 * Only mysql_is_available() lives here. Do NOT declare db_connect() or other
 * database API functions; those belong to database.mysql.inc / database.mysqli.inc
 * and must not be redeclared when the installer includes every install.*.inc.
 */

/**
 * The mysql_* extension was removed in PHP 7.0.
 *
 * @return bool
 *   Always FALSE on modern PHP.
 */
function mysql_is_available() {
  return FALSE;
}

/**
 * Connection test for the obsolete driver (never succeeds).
 */
function drupal_test_mysql($url, &$success) {
  $success = array();
  drupal_set_message(st('The legacy <code>mysql://</code> driver is not available on PHP 7 or 8. Choose <strong>mysqli</strong> in the form (or use <code>mysqli://</code> in settings.php).'), 'error');
  return FALSE;
}
