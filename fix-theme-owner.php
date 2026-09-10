<?php
/**
 * Re-register single theme from themes/rokupal. DELETE after use.
 */
chdir(dirname(__FILE__));
error_reporting(E_ALL);
ini_set('display_errors', 1);

require_once './includes/bootstrap.inc';
drupal_bootstrap(DRUPAL_BOOTSTRAP_DATABASE);

header('Content-Type: text/plain; charset=utf-8');
echo "RokuPal register\n";

if (!is_file('themes/rokupal/rokupal.info')) {
  echo "FATAL: themes/rokupal/rokupal.info missing\n";
  exit;
}
if (!is_file('themes/rokupal/page.tpl.php')) {
  echo "FATAL: themes/rokupal/page.tpl.php missing\n";
  exit;
}
if (!is_file('themes/engines/phptemplate/phptemplate.engine')) {
  echo "WARN: engine missing\n";
}

db_query("DELETE FROM {system} WHERE type = 'theme' AND name = 'rokupal'");

$info = array(
  'name' => 'RokuPal',
  'description' => 'RokuPal',
  'core' => '6.x',
  'engine' => 'phptemplate',
  'stylesheets' => array('all' => array('style.css' => 'themes/rokupal/style.css')),
  'scripts' => array(),
  'regions' => array(
    'left' => 'Left sidebar',
    'right' => 'Right sidebar',
    'content' => 'Content',
    'header' => 'Header',
    'footer' => 'Footer',
  ),
  'features' => array('logo' => 1, 'name' => 1, 'slogan' => 1, 'mission' => 1, 'favicon' => 1, 'primary_links' => 1, 'secondary_links' => 1),
);

db_query(
  "INSERT INTO {system} (filename, name, type, owner, status, throttle, bootstrap, schema_version, weight, info)
   VALUES ('%s', 'rokupal', 'theme', '%s', 1, 0, 0, -1, 0, '%s')",
  'themes/rokupal/rokupal.info',
  'themes/engines/phptemplate/phptemplate.engine',
  serialize($info)
);

variable_set('theme_default', 'rokupal');
variable_set('theme_rokupal_settings', array(
  'toggle_logo' => 1,
  'default_logo' => 1,
  'logo_path' => '',
  'toggle_name' => 1,
  'toggle_slogan' => 1,
  'toggle_mission' => 1,
  'toggle_favicon' => 1,
  'default_favicon' => 1,
  'mission' => '',
));

foreach (array('cache', 'cache_menu', 'cache_page', 'cache_block') as $t) {
  @db_query("DELETE FROM {" . $t . "}");
}
@db_query("DELETE FROM {cache} WHERE cid LIKE 'theme%%'");

echo "OK theme_default=rokupal\n";
echo "logo: " . (is_file('themes/rokupal/logo.png') ? 'yes' : 'no') . "\n";
echo "Delete this file and open the site.\n";
