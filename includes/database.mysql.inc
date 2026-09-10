<?php
/**
 * @file
 * Legacy mysql_* driver stub — unavailable on PHP 7+.
 *
 * Prefer mysqli://. Functions are guarded so including this file after
 * database.mysqli.inc never fatals with "Cannot redeclare".
 */

if (!function_exists('db_status_report')) {
  function db_status_report($phase) {
    $t = get_t();
    return array(
      'title' => $t('MySQL database (legacy driver)'),
      'value' => $t('Not available'),
      'severity' => REQUIREMENT_ERROR,
      'description' => $t('Use <code>mysqli://</code> in <code>$db_url</code>. The PHP mysql_* extension was removed in PHP 7.0.'),
    );
  }
}

if (!function_exists('db_version')) {
  function db_version() {
    return '0.0.0-unavailable';
  }
}

if (!function_exists('db_connect')) {
  function db_connect($url) {
    _db_error_page('The legacy mysql:// driver is not available on PHP 7/8. Use mysqli:// in $db_url.');
  }
}

if (!function_exists('_db_query')) {
  function _db_query($query, $debug = 0) {
    return FALSE;
  }
}

if (!function_exists('db_query')) {
  function db_query($query) {
    return FALSE;
  }
}

if (!function_exists('db_fetch_object')) {
  function db_fetch_object($result) { return FALSE; }
}
if (!function_exists('db_fetch_array')) {
  function db_fetch_array($result) { return FALSE; }
}
if (!function_exists('db_result')) {
  function db_result($result, $row = 0) { return FALSE; }
}
if (!function_exists('db_error')) {
  function db_error() { return 'mysql driver unavailable'; }
}
if (!function_exists('db_affected_rows')) {
  function db_affected_rows() { return 0; }
}
if (!function_exists('db_query_range')) {
  function db_query_range($query) { return FALSE; }
}
if (!function_exists('db_query_temporary')) {
  function db_query_temporary($query) { return FALSE; }
}
if (!function_exists('db_encode_blob')) {
  function db_encode_blob($data) { return $data; }
}
if (!function_exists('db_decode_blob')) {
  function db_decode_blob($data) { return $data; }
}
if (!function_exists('db_escape_string')) {
  function db_escape_string($text) { return addslashes((string) $text); }
}
if (!function_exists('db_escape_table')) {
  function db_escape_table($name) { return preg_replace('/[^A-Za-z0-9_]+/', '', (string) $name); }
}
if (!function_exists('db_last_insert_id')) {
  function db_last_insert_id($table, $field) { return 0; }
}
