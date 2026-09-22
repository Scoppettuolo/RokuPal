<?php
/**
 * RokuPal health check — open in browser: /health.php
 * Safe to leave in place on private/dev installs; delete on public production if desired.
 */
header('Content-Type: text/html; charset=utf-8');
// Optional: create sites/default/health.key with a secret string; then open health.php?key=SECRET
$health_key_file = __DIR__ . '/sites/default/health.key';
if (is_file($health_key_file)) {
  $need = trim(file_get_contents($health_key_file));
  $got = isset($_GET['key']) ? (string) $_GET['key'] : '';
  if ($need !== '' && !hash_equals($need, $got)) {
    header('HTTP/1.1 403 Forbidden');
    header('Content-Type: text/plain; charset=utf-8');
    echo "Forbidden. Provide ?key= from sites/default/health.key\n";
    exit;
  }
}

error_reporting(E_ALL);
ini_set('display_errors', '0');
ini_set('log_errors', '1');

function h($s) {
  return htmlspecialchars((string) $s, ENT_QUOTES, 'UTF-8');
}

$checks = array();
$checks['PHP version'] = array(version_compare(PHP_VERSION, '7.4.0', '>='), PHP_VERSION . ' (need >= 7.4)');
$checks['bootstrap.inc'] = array(is_file(__DIR__ . '/includes/bootstrap.inc'), '');
$checks['index.php'] = array(is_file(__DIR__ . '/index.php'), '');
$checks['sites/default'] = array(is_dir(__DIR__ . '/sites/default'), '');
$checks['sites/default writable'] = array(is_writable(__DIR__ . '/sites/default'), 'needed during install');
$checks['settings.php'] = array(is_file(__DIR__ . '/sites/default/settings.php'), 'created by installer');
$sp = __DIR__ . '/sites/default/settings.php';
$checks['settings.php readable'] = array(is_file($sp) && is_readable($sp), '');
$checks['files/'] = array(is_dir(__DIR__ . '/sites/default/files') || is_dir(__DIR__ . '/files'), 'uploads / avatars');
$checks['mysqli'] = array(function_exists('mysqli_connect'), 'MySQL/MariaDB');
$checks['pdo_sqlite / sqlite3'] = array(extension_loaded('pdo_sqlite') || extension_loaded('sqlite3'), 'SQLite option');
$checks['password_hash'] = array(function_exists('password_hash'), 'modern passwords');
$checks['hash_hmac'] = array(function_exists('hash_hmac'), 'tokens');
$checks['hash_equals'] = array(function_exists('hash_equals'), 'timing-safe compare');
$checks['json'] = array(function_exists('json_encode'), '');
$checks['mbstring'] = array(function_exists('mb_strlen'), 'recommended');
$checks['gd'] = array(function_exists('imagecreatetruecolor'), 'images / color');

$rokupal_mods = array(
  'rokupal_core', 'rokupal_site', 'rokupal_admin_bar', 'rokupal_comments',
  'rokupal_blog', 'rokupal_markdown', 'rokupal_forum', 'rokupal_poll',
  'rokupal_modules_ui', 'rokupal_blocks_ui', 'rokupal_menus_ui',
  'rokupal_nodetype_ui', 'rokupal_themes_ui', 'rokupal_formats',
);
foreach ($rokupal_mods as $m) {
  $checks['module files: ' . $m] = array(
    is_file(__DIR__ . '/modules/' . $m . '/' . $m . '.module') || is_file(__DIR__ . '/modules/' . $m . '/' . $m . '.info'),
    ''
  );
}
$checks['theme rokupal'] = array(is_file(__DIR__ . '/themes/rokupal/rokupal.info'), '');
$checks['theme style.css'] = array(is_file(__DIR__ . '/themes/rokupal/style.css'), '');

$fail = 0;
foreach ($checks as $ok_msg) {
  if (empty($ok_msg[0])) {
    $fail++;
  }
}

echo '<!DOCTYPE html><html><head><meta charset="utf-8" /><title>RokuPal health</title>';
echo '<style>
body{font-family:system-ui,sans-serif;max-width:720px;margin:2rem auto;padding:0 1rem;background:#f4f6f8;color:#1a1d23}
h1{font-size:1.35rem} .ok{color:#0a7} .bad{color:#c00;font-weight:600}
table{width:100%;border-collapse:collapse;background:#fff;border-radius:8px;overflow:hidden;box-shadow:0 1px 4px rgba(0,0,0,.08)}
td,th{padding:.5rem .75rem;border-bottom:1px solid #e5e9ef;text-align:left;font-size:.9rem}
.meta{color:#667;font-size:.85rem;margin:1rem 0}
code{background:#eef;padding:.1rem .3rem;border-radius:3px}
</style></head><body>';
echo '<h1>RokuPal health check</h1>';
echo '<p class="meta">PHP ' . h(PHP_VERSION) . ' · ' . h(PHP_OS) . ' · SAPI ' . h(php_sapi_name()) . '</p>';
echo '<p class="meta">' . ($fail ? '<span class="bad">' . (int) $fail . ' issue(s)</span>' : '<span class="ok">All listed checks passed</span>') . '</p>';
echo '<table><tr><th>Check</th><th>Status</th><th>Notes</th></tr>';
foreach ($checks as $label => $pair) {
  $ok = !empty($pair[0]);
  $note = $pair[1];
  echo '<tr><td>' . h($label) . '</td><td class="' . ($ok ? 'ok' : 'bad') . '">' . ($ok ? 'OK' : 'FAIL') . '</td><td>' . h($note) . '</td></tr>';
}
echo '</table>';

echo '<h2>Bootstrap</h2><pre style="background:#fff;padding:1rem;border-radius:8px;overflow:auto">';
try {
  require_once __DIR__ . '/includes/bootstrap.inc';
  echo "bootstrap.inc loaded\n";
  if (function_exists('conf_path')) {
    echo 'conf_path: ' . h(conf_path(FALSE)) . "\n";
  }
}
catch (Throwable $e) {
  echo 'FAIL: ' . h($e->getMessage()) . "\n" . h($e->getFile()) . ':' . $e->getLine() . "\n";
}
echo '</pre>';

echo '<h2>settings.php</h2><pre style="background:#fff;padding:1rem;border-radius:8px;overflow:auto">';
if (is_file($sp)) {
  $c = file_get_contents($sp);
  if (preg_match('/\$db_url\s*=\s*[\'\"]([^\'\"]+)/', $c, $m)) {
    $url = $m[1];
    $url = preg_replace('#://([^:]+):([^@]+)@#', '://$1:***@', $url);
    echo 'db_url: ' . h($url) . "\n";
  }
  else {
    echo "db_url: (not found or custom)\n";
  }
  echo 'size: ' . filesize($sp) . " bytes\n";
}
else {
  echo "settings.php not created yet (run install.php)\n";
}
echo '</pre>';

echo '<p class="meta">Delete or protect <code>health.php</code> on public production servers.</p>';
echo '</body></html>';
