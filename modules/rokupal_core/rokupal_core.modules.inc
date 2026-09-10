<?php
/**
 * @file
 * RokuPal Modules Manager — modern enable / disable / uninstall UI.
 */

/**
 * Required modules that cannot be disabled or uninstalled.
 */
function rokupal_core_modules_required() {
  $req = array('system', 'user', 'node', 'filter', 'block');
  if (function_exists('drupal_required_modules')) {
    $req = array_unique(array_merge($req, drupal_required_modules()));
  }
  return $req;
}

/**
 * Main page: list + actions.
 *
 * @param string $view
 *   '' or 'list' = all modules; 'uninstall' = only disabled with schema installed.
 */
function rokupal_core_modules_page($view = '') {
  drupal_add_css(drupal_get_path('module', 'rokupal_core') . '/css/modules-admin.css');
  drupal_set_title(t('Modules'));

  // Handle POST actions before render
  if (!empty($_POST['rokupal_mod_op']) && !empty($_POST['module']) && $_SERVER['REQUEST_METHOD'] === 'POST') {
    if (function_exists('drupal_valid_token') && !empty($_POST['token'])) {
      if (!drupal_valid_token($_POST['token'], 'rokupal_modules')) {
        drupal_set_message(t('Invalid form token. Try again.'), 'error');
        drupal_goto('admin/build/modules');
      }
    }
    $name = preg_replace('/[^a-z0-9_]/', '', $_POST['module']);
    $op = $_POST['rokupal_mod_op'];
    include_once './includes/install.inc';

    if ($op === 'enable') {
      rokupal_core_module_enable($name);
    }
    elseif ($op === 'disable') {
      rokupal_core_module_disable($name);
    }
    elseif ($op === 'uninstall') {
      rokupal_core_module_uninstall($name);
    }
    drupal_goto('admin/build/modules');
  }

  $files = module_rebuild_cache();
  $required = rokupal_core_modules_required();
  $token = function_exists('drupal_get_token') ? drupal_get_token('rokupal_modules') : '';

  // Group by package
  $groups = array();
  foreach ($files as $name => $file) {
    if (empty($file->info) || !empty($file->info['hidden'])) {
      continue;
    }
    $pkg = !empty($file->info['package']) ? $file->info['package'] : 'Other';
    $groups[$pkg][$name] = $file;
  }
  ksort($groups);

  $out = '';
  $out .= '<div class="rp-mod-wrap">';
  $out .= '<p class="rp-mod-intro">' . t('Activate, deactivate or uninstall modules. Uninstall removes the module’s tables and configuration.') . '</p>';
  $out .= '<div class="rp-mod-legend">';
  $out .= '<span class="rp-badge rp-on">' . t('Active') . '</span> ';
  $out .= '<span class="rp-badge rp-off">' . t('Inactive') . '</span> ';
  $out .= '<span class="rp-badge rp-req">' . t('Required') . '</span>';
  $out .= '</div>';

  foreach ($groups as $pkg => $mods) {
    $out .= '<section class="rp-mod-group">';
    $out .= '<h2 class="rp-mod-pkg">' . check_plain($pkg) . '</h2>';
    $out .= '<div class="rp-mod-list">';

    uasort($mods, function ($a, $b) {
      return strcasecmp($a->info['name'], $b->info['name']);
    });

    foreach ($mods as $name => $file) {
      $enabled = !empty($file->status);
      $schema = isset($file->schema_version) ? (int) $file->schema_version : -1;
      $is_req = in_array($name, $required, TRUE);
      $label = !empty($file->info['name']) ? $file->info['name'] : $name;
      $desc = !empty($file->info['description']) ? $file->info['description'] : '';
      $ver = !empty($file->info['version']) ? $file->info['version'] : '';

      $out .= '<article class="rp-mod-card' . ($enabled ? ' is-on' : ' is-off') . ($is_req ? ' is-req' : '') . '">';
      $out .= '<div class="rp-mod-main">';
      $out .= '<div class="rp-mod-title">' . check_plain($label);
      $out .= ' <code class="rp-mod-machine">' . check_plain($name) . '</code>';
      if ($ver) {
        $out .= ' <span class="rp-mod-ver">' . check_plain($ver) . '</span>';
      }
      $out .= '</div>';
      if ($desc) {
        $out .= '<p class="rp-mod-desc">' . check_plain($desc) . '</p>';
      }
      $out .= '<div class="rp-mod-meta">';
      if ($enabled) {
        $out .= '<span class="rp-badge rp-on">' . t('Active') . '</span>';
      }
      else {
        $out .= '<span class="rp-badge rp-off">' . t('Inactive') . '</span>';
      }
      if ($is_req) {
        $out .= ' <span class="rp-badge rp-req">' . t('Required') . '</span>';
      }
      $out .= ' <span class="rp-schema">schema ' . $schema . '</span>';
      $out .= '</div>';
      $out .= '</div>';

      $out .= '<div class="rp-mod-actions">';
      if ($enabled) {
        if (!$is_req) {
          $out .= rokupal_core_mod_btn($name, 'disable', t('Disable'), 'rp-btn-mute', $token);
          $out .= rokupal_core_mod_btn($name, 'uninstall', t('Uninstall'), 'rp-btn-danger', $token, TRUE);
        }
        else {
          $out .= '<span class="rp-mod-locked">' . t('Cannot disable') . '</span>';
        }
      }
      else {
        $out .= rokupal_core_mod_btn($name, 'enable', t('Enable'), 'rp-btn-primary', $token);
        // Uninstall only if was installed before (schema >= 0)
        if ($schema >= 0 && !$is_req) {
          $out .= rokupal_core_mod_btn($name, 'uninstall', t('Uninstall data'), 'rp-btn-danger', $token, TRUE);
        }
      }
      $out .= '</div>';
      $out .= '</article>';
    }
    $out .= '</div></section>';
  }

  $out .= '</div>';
  return $out;
}

function rokupal_core_mod_btn($module, $op, $label, $class, $token, $confirm = FALSE) {
  $html = '<form method="post" action="' . check_url(url('admin/build/modules')) . '" class="rp-mod-form"';
  if ($confirm) {
    $html .= ' onsubmit="return confirm(\'' . check_plain(t('Uninstall %m and delete its data?', array('%m' => $module))) . '\');"';
  }
  $html .= '>';
  $html .= '<input type="hidden" name="module" value="' . check_plain($module) . '" />';
  $html .= '<input type="hidden" name="rokupal_mod_op" value="' . check_plain($op) . '" />';
  if ($token) {
    $html .= '<input type="hidden" name="token" value="' . check_plain($token) . '" />';
  }
  $html .= '<button type="submit" class="rp-btn ' . check_plain($class) . '">' . check_plain($label) . '</button>';
  $html .= '</form>';
  return $html;
}

/**
 * Enable module: status=1, install if schema -1, invoke enable.
 */
function rokupal_core_module_enable($name) {
  $required = rokupal_core_modules_required();
  $file = drupal_get_filename('module', $name);
  if (!$file) {
    // Try scan
    $files = module_rebuild_cache();
    if (empty($files[$name])) {
      drupal_set_message(t('Module %m not found.', array('%m' => $name)), 'error');
      return;
    }
    $file = $files[$name]->filename;
  }

  $row = db_fetch_object(db_query("SELECT name, status, schema_version, filename FROM {system} WHERE type = 'module' AND name = '%s'", $name));
  if (!$row) {
    db_query(
      "INSERT INTO {system} (filename, name, type, owner, status, throttle, bootstrap, schema_version, weight) VALUES ('%s', '%s', 'module', '', 0, 0, 0, %d, 0)",
      $file, $name, defined('SCHEMA_UNINSTALLED') ? SCHEMA_UNINSTALLED : -1
    );
    $row = (object) array('status' => 0, 'schema_version' => -1, 'filename' => $file);
  }

  $sv = (int) $row->schema_version;
  if ($sv < 0) {
    drupal_load('module', $name);
    module_load_install($name);
    if (function_exists($name . '_install')) {
      module_invoke($name, 'install');
    }
    if (function_exists('drupal_set_installed_schema_version')) {
      $versions = function_exists('drupal_get_schema_versions') ? drupal_get_schema_versions($name) : array();
      $ver = $versions ? max($versions) : 0;
      drupal_set_installed_schema_version($name, $ver);
    }
    else {
      db_query("UPDATE {system} SET schema_version = 0 WHERE type = 'module' AND name = '%s'", $name);
    }
  }

  db_query("UPDATE {system} SET status = 1, throttle = 0, filename = '%s' WHERE type = 'module' AND name = '%s'", $file, $name);
  module_list(TRUE, FALSE);
  module_implements('', FALSE, TRUE);
  drupal_load('module', $name);
  if (function_exists($name . '_enable')) {
    module_invoke($name, 'enable');
  }
  menu_rebuild();
  cache_clear_all();
  drupal_set_message(t('Enabled %m.', array('%m' => $name)));
}

/**
 * Disable module: status=0, invoke disable. Required modules blocked.
 */
function rokupal_core_module_disable($name) {
  if (in_array($name, rokupal_core_modules_required(), TRUE)) {
    drupal_set_message(t('%m is required and cannot be disabled.', array('%m' => $name)), 'error');
    return;
  }
  if (!module_exists($name)) {
    // Force status 0 anyway
    db_query("UPDATE {system} SET status = 0, throttle = 0 WHERE type = 'module' AND name = '%s'", $name);
    drupal_set_message(t('Disabled %m.', array('%m' => $name)));
    return;
  }
  module_load_install($name);
  if (function_exists($name . '_disable')) {
    module_invoke($name, 'disable');
  }
  db_query("UPDATE {system} SET status = 0, throttle = 0 WHERE type = 'module' AND name = '%s'", $name);
  module_list(TRUE, FALSE);
  module_implements('', FALSE, TRUE);
  menu_rebuild();
  cache_clear_all();
  drupal_set_message(t('Disabled %m.', array('%m' => $name)));
}

/**
 * Uninstall: disable + hook_uninstall + schema -1 + remove data.
 */
function rokupal_core_module_uninstall($name) {
  if (in_array($name, rokupal_core_modules_required(), TRUE)) {
    drupal_set_message(t('%m is required and cannot be uninstalled.', array('%m' => $name)), 'error');
    return;
  }
  include_once './includes/install.inc';

  // Disable first
  if (module_exists($name)) {
    module_load_install($name);
    if (function_exists($name . '_disable')) {
      module_invoke($name, 'disable');
    }
    db_query("UPDATE {system} SET status = 0, throttle = 0 WHERE type = 'module' AND name = '%s'", $name);
    module_list(TRUE, FALSE);
  }

  drupal_load('module', $name);
  module_load_install($name);
  if (function_exists('drupal_uninstall_module')) {
    drupal_uninstall_module($name);
  }
  else {
    if (function_exists($name . '_uninstall')) {
      module_invoke($name, 'uninstall');
    }
    if (function_exists('drupal_uninstall_schema')) {
      drupal_uninstall_schema($name);
    }
  }

  $un = defined('SCHEMA_UNINSTALLED') ? SCHEMA_UNINSTALLED : -1;
  db_query("UPDATE {system} SET status = 0, schema_version = %d WHERE type = 'module' AND name = '%s'", $un, $name);

  module_list(TRUE, FALSE);
  module_implements('', FALSE, TRUE);
  menu_rebuild();
  cache_clear_all();
  drupal_set_message(t('Uninstalled %m and removed its data.', array('%m' => $name)));
}
