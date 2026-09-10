<?php
/**
 * Fix schema_version + reinstall marker tables so Uninstall works after Disable.
 * DELETE this file after running.
 */
chdir(dirname(__FILE__));
error_reporting(E_ALL);
ini_set('display_errors', 1);
require_once './includes/bootstrap.inc';
drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
include_once './includes/install.inc';
header('Content-Type: text/plain; charset=utf-8');

$modules = array('rokupal_core', 'rokupal_blog', 'rokupal_forum', 'rokupal_poll');
foreach ($modules as $m) {
  $filename = drupal_get_filename('module', $m);
  if (!$filename) {
    echo "$m: NOT ON DISK\n";
    continue;
  }
  $row = db_fetch_object(db_query("SELECT status, schema_version FROM {system} WHERE type = 'module' AND name = '%s'", $m));
  if (!$row) {
    echo "$m: no system row — skip\n";
    continue;
  }
  // Force schema >= 0 if module is or was installed (status 1 or schema issues)
  if ((int) $row->schema_version < 0) {
    module_load_install($m);
    if (function_exists($m . '_schema')) {
      $schema = module_invoke($m, 'schema');
      if (!empty($schema)) {
        foreach (array_keys($schema) as $table) {
          if (!db_table_exists($table)) {
            drupal_install_schema($m);
            break;
          }
        }
      }
    }
    drupal_set_installed_schema_version($m, 0);
    echo "$m: schema_version set to 0 (was {$row->schema_version})\n";
  }
  else {
    echo "$m: status={$row->status} schema={$row->schema_version} OK\n";
  }
  db_query("UPDATE {system} SET filename = '%s' WHERE type = 'module' AND name = '%s'", $filename, $m);
}

// Restore theme + frontpage + blocks if blog on
if (module_exists('rokupal_blog')) {
  variable_set('theme_default', 'rokupal');
  variable_set('site_frontpage', 'blog');
  module_load_include('install', 'rokupal_blog');
  if (function_exists('rokupal_blog_setup_blocks')) {
    rokupal_blog_setup_blocks();
    echo "blog blocks restored\n";
  }
}
menu_rebuild();
cache_clear_all();
echo "\nUninstall works ONLY after: Modules tab → uncheck module → Save.\n";
echo "Then open Uninstall tab.\n";
echo "Delete fix-modules.php\n";
