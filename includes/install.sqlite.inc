<?php
/** SQLite installer tester for the PDO-backed driver. */
function sqlite_is_available() {
  return extension_loaded('pdo_sqlite');
}
function drupal_test_sqlite($url, &$success) {
  if (!sqlite_is_available()) {
    $success['CONNECT'] = FALSE;
    return FALSE;
  }
  $raw = substr((string) $url, 9);
  $path = urldecode($raw);
  if ($path === '') $path = './sites/default/files/.ht.sqlite';
  if (strpos($path, '/') !== 0) $path = './' . ltrim($path, './');
  $dir = dirname($path);
  if (!is_dir($dir) && !@mkdir($dir, 0775, TRUE) && !is_dir($dir)) {
    $success['CONNECT'] = FALSE;
    return FALSE;
  }
  if (!is_writable($dir)) {
    $success['CONNECT'] = FALSE;
    return FALSE;
  }
  try {
    $pdo = new PDO('sqlite:' . $path, NULL, NULL, array(PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION));
    $pdo->query('SELECT 1');
    $success['CONNECT'] = TRUE;
    return TRUE;
  }
  catch (Exception $e) {
    $success['CONNECT'] = FALSE;
    return FALSE;
  }
}
?>
