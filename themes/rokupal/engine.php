<?php
/**
 * RokuPal engine body.
 */
function phptemplate_init($template) {
  $dir = dirname($template->filename);
  $file = $dir . DIRECTORY_SEPARATOR . 'template.php';
  if (is_file($file)) {
    include_once $file;
  }
}

function phptemplate_theme($existing, $type, $theme, $path) {
  $templates = array();
  if (function_exists('drupal_find_theme_functions')) {
    $templates = drupal_find_theme_functions($existing, array('phptemplate', $theme));
  }
  // Always discover templates from themes/rokupal
  $paths = array_unique(array_filter(array($path, 'themes/rokupal', dirname($path))));
  if (function_exists('drupal_find_theme_templates')) {
    foreach ($paths as $p) {
      if ($p && is_dir($p)) {
        $templates += drupal_find_theme_templates($existing, '.tpl.php', $p);
      }
    }
  }
  return $templates;
}
