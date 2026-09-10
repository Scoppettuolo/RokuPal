<?php
/**
 * Disable color module if it fatals bootstrap. DELETE after use.
 */
chdir(dirname(__FILE__));
error_reporting(E_ALL);
ini_set('display_errors', 1);
# Minimal DB bootstrap without full hooks
require_once './includes/bootstrap.inc';
# Only database so we can UPDATE system without loading color_init
drupal_bootstrap(DRUPAL_BOOTSTRAP_DATABASE);
db_query("UPDATE {system} SET status = 0 WHERE type = 'module' AND name = 'color'");
echo "color module DISABLED (status=0).\n";
echo "Copy modules/color/ from the zip, then re-enable in admin if desired.\n";
echo "Delete fix-color.php\n";
