<?php
/**
 * RokuPal post-install / release verification.
 * CLI:  php scripts/post-install-check.php
 * Web:  /scripts/post-install-check.php  (protect or delete on public prod)
 */
$root = dirname(__DIR__);
$cli = (php_sapi_name() === 'cli');
if (!$cli) {
  header('Content-Type: text/plain; charset=utf-8');
}

function line($ok, $msg) {
  echo ($ok ? '[OK]  ' : '[!!] ') . $msg . "\n";
  return $ok ? 0 : 1;
}

$fails = 0;
echo "RokuPal post-install check\n";
echo str_repeat('=', 48) . "\n";
echo 'PHP ' . PHP_VERSION . ' | ' . PHP_OS . "\n\n";

echo "Environment\n";
$fails += line(version_compare(PHP_VERSION, '7.4.0', '>='), 'PHP >= 7.4');
$fails += line(function_exists('password_hash'), 'password_hash');
$fails += line(function_exists('hash_hmac'), 'hash_hmac');
$fails += line(function_exists('hash_equals'), 'hash_equals');
$fails += line(function_exists('json_encode'), 'json');

echo "\nCore files\n";
$fails += line(is_file($root . '/index.php'), 'index.php');
$fails += line(is_file($root . '/includes/bootstrap.inc'), 'bootstrap.inc');
$fails += line(is_file($root . '/includes/common.inc'), 'common.inc');
$fails += line(is_file($root . '/sites/default/settings.php'), 'settings.php (site installed)');
$fails += line(is_file($root . '/themes/rokupal/rokupal.info'), 'theme rokupal');
$fails += line(is_file($root . '/themes/rokupal/style.css'), 'theme style.css');

echo "\nRokuPal modules (files present)\n";
$mods = array(
  'rokupal_core', 'rokupal_site', 'rokupal_admin_bar', 'rokupal_branding',
  'rokupal_comments', 'rokupal_blog', 'rokupal_markdown', 'rokupal_formats',
  'rokupal_forum', 'rokupal_poll',
  'rokupal_modules_ui', 'rokupal_blocks_ui', 'rokupal_menus_ui',
  'rokupal_nodetype_ui', 'rokupal_themes_ui',
);
foreach ($mods as $m) {
  $ok = is_file($root . '/modules/' . $m . '/' . $m . '.module')
     || is_file($root . '/modules/' . $m . '/' . $m . '.info');
  $fails += line($ok, $m);
}

echo "\nVERSION constant\n";
try {
  // Load system.module defines without full bootstrap if possible
  $sys = $root . '/modules/system/system.module';
  if (is_file($sys)) {
    $src = file_get_contents($sys);
    if (preg_match("/define\(\s*'VERSION'\s*,\s*'([^']+)'/", $src, $m)) {
      echo '  VERSION=' . $m[1] . "\n";
      $fails += line($m[1] !== '6.0' && $m[1] !== '', 'VERSION defined as RokuPal release');
    }
  }
}
catch (Throwable $e) {
  $fails += line(FALSE, $e->getMessage());
}

echo "\nBootstrap\n";
try {
  require_once $root . '/includes/bootstrap.inc';
  $fails += line(TRUE, 'bootstrap.inc loaded');
  if (function_exists('conf_path')) {
    echo '  conf_path=' . conf_path(FALSE) . "\n";
  }
}
catch (Throwable $e) {
  $fails += line(FALSE, 'bootstrap: ' . $e->getMessage());
}

echo "\n" . str_repeat('=', 48) . "\n";
if ($fails) {
  echo "RESULT: $fails issue(s)\n";
  exit(1);
}
echo "RESULT: OK\n";
exit(0);
