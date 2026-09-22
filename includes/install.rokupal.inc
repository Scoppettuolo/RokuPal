<?php
/**
 * @file
 * RokuPal installer API — used by install.php and install-simple.php.
 * Does not use the multi-task classic multi-step installer state machine.
 */

/**
 * Run a full site install in one go.
 *
 * @param string $profile Always treated as 'rokupal'.
 * @param string $locale
 * @param array $options
 *   - db_url
 *   - admin_name
 *   - admin_mail
 *   - admin_pass
 *   - enable_forum (bool)
 *   - enable_poll (bool)
 * @return array messages (strings)
 */

/**
 * Ensure sites/default is writable and write settings.php.
 * Throws Exception with actionable XAMPP/Windows guidance on failure.
 */
function rokupal_install_write_settings($db_url) {
  $dir = './sites/default';
  $files_dir = './sites/default/files';
  $settings = $dir . '/settings.php';
  $settings_default = $dir . '/default.settings.php';

  foreach (array($dir, $files_dir) as $d) {
    if (!is_dir($d)) {
      $ok = @mkdir($d, 0775, TRUE);
      if (!$ok && !is_dir($d)) {
        throw new Exception(
          "No se pudo crear la carpeta $d.\n\n" .
          "En XAMPP (Windows):\n" .
          "1. Cierra el instalador.\n" .
          "2. En el Explorador, ve a la carpeta del sitio (htdocs\\...).\n" .
          "3. Clic derecho en 'sites' → Propiedades → desmarca 'Solo lectura' → Aplicar a subcarpetas.\n" .
          "4. Asegurate de que exista sites\\default.\n" .
          "5. Vuelve a abrir install.php.\n\n" .
          "En Linux: chmod -R 775 sites"
        );
      }
    }
  }

  if (is_file($settings)) {
    $content = file_get_contents($settings);
    if ($content === FALSE) {
      throw new Exception("No se pudo leer $settings (permiso denegado).");
    }
    if (preg_match('/\$db_url\s*=/', $content)) {
      $content = preg_replace('/\$db_url\s*=\s*[\'"].*?[\'"]\s*;/', "\$db_url = '" . addslashes($db_url) . "';", $content, 1);
    }
    else {
      $content .= "\n\$db_url = '" . addslashes($db_url) . "';\n";
    }
  }
  else {
    $content = is_file($settings_default) ? file_get_contents($settings_default) : "<?php\n";
    if ($content === FALSE) {
      $content = "<?php\n";
    }
    if (!preg_match('/\$db_url\s*=/', $content)) {
      $content .= "\n\$db_url = '" . addslashes($db_url) . "';\n\$db_prefix = '';\n";
    }
    else {
      $content = preg_replace('/\$db_url\s*=\s*[\'"].*?[\'"]\s*;/', "\$db_url = '" . addslashes($db_url) . "';", $content, 1);
    }
    if (strpos($content, 'cookie_domain') === FALSE) {
      $content .= "\n# \$cookie_domain = ''; // leave unset on localhost\n";
    }
  }

  if (is_file($settings) && !is_writable($settings)) {
    @chmod($settings, 0664);
  }
  if (!is_writable($dir)) {
    @chmod($dir, 0775);
  }

  $can_write = (is_file($settings) && is_writable($settings)) || (!is_file($settings) && is_writable($dir));
  if (!$can_write) {
    throw new Exception(
      "Permission denied al escribir sites/default/settings.php.\n\n" .
      "Esto es frecuente en XAMPP (Windows).\n\n" .
      "Solucion rapida:\n" .
      "1. Crea sites\\default si no existe.\n" .
      "2. Crea un archivo vacio settings.php dentro de sites\\default.\n" .
      "3. Clic derecho en settings.php → Propiedades → desmarca Solo lectura.\n" .
      "4. Clic derecho en la carpeta sites → Propiedades → desmarca Solo lectura → Aplicar.\n" .
      "5. Recarga install.php.\n\n" .
      "Si prefieres, pega manualmente en settings.php el contenido que el instalador intento escribir (incluye \$db_url)."
    );
  }

  $written = @file_put_contents($settings, $content);
  if ($written === FALSE) {
    throw new Exception(
      "file_put_contents fallo en $settings. " .
      "Revisa permisos de escritura en sites/default (XAMPP: quita Solo lectura)."
    );
  }

  return $settings;
}

function install_run_simple($profile, $locale, $options = array()) {
  $messages = array();
  $profile = 'rokupal';

  $db_url = isset($options['db_url']) ? $options['db_url'] : '';
  $db_driver = isset($options['db_driver']) ? $options['db_driver'] : 'sqlite';
  if (strpos($db_url, '://') === FALSE) {
    $db_url = $db_driver . '://' . ltrim($db_url, '/');
  }
  $admin_name = isset($options['admin_name']) ? $options['admin_name'] : 'admin';
  $admin_mail = isset($options['admin_mail']) ? $options['admin_mail'] : 'admin@example.com';
  $admin_pass = isset($options['admin_pass']) ? $options['admin_pass'] : '';
  $enable_forum = !empty($options['enable_forum']);
  $enable_poll = !empty($options['enable_poll']);
  // Blog always on for RokuPal
  $enable_blog = TRUE;

  if ($db_url === '' || $admin_pass === '' || $admin_name === '') {
    throw new Exception('Database URL, admin name and password are required.');
  }

  // --- settings.php (permisos-aware, XAMPP/Windows friendly) ---
  $settings = rokupal_install_write_settings($db_url);
  $messages[] = 'Wrote ' . $settings;

  // Load settings into global
  global $db_url, $db_prefix, $db_type, $base_url;
  include $settings;

  // Core includes — module.inc MUST load before drupal_install_system().
  require_once './includes/bootstrap.inc';
  require_once './includes/file.inc';
  require_once './includes/common.inc';
  require_once './includes/database.inc';
  require_once './includes/module.inc';
  require_once './includes/session.inc';
  require_once './includes/install.inc';

  // Database driver (after $db_url is known)
  if (strpos($db_url, 'sqlite') === 0) {
    if (is_file('./includes/database.sqlite.inc')) {
      require_once './includes/database.sqlite.inc';
    }
    else {
      throw new Exception('Missing includes/database.sqlite.inc');
    }
  }
  elseif (strpos($db_url, 'mysqli') === 0 || strpos($db_url, 'mysql://') === 0) {
    if (is_file('./includes/database.mysqli.inc')) {
      require_once './includes/database.mysqli.inc';
    }
    elseif (is_file('./includes/database.mysql.inc')) {
      require_once './includes/database.mysql.inc';
    }
    else {
      throw new Exception('Missing MySQL database driver includes');
    }
  }
  elseif (strpos($db_url, 'pgsql') === 0 && is_file('./includes/database.pgsql.inc')) {
    require_once './includes/database.pgsql.inc';
  }

  // Connect
  db_set_active();
  $messages[] = 'Database connection OK';

  // Detect system table without fatal warnings on empty SQLite DB
  $has_system = FALSE;
  if (function_exists('db_table_exists')) {
    // Suppress SQLite "no such table" noise during first install
    $prev = error_reporting(E_ALL & ~E_WARNING);
    try {
      $has_system = (bool) db_table_exists('system');
    }
    catch (Exception $e) {
      $has_system = FALSE;
    }
    error_reporting($prev);
  }

  if (!$has_system) {
    _rokupal_install_system_tables();
    $messages[] = 'System module installed';
  }
  else {
    if (function_exists('drupal_bootstrap')) {
      drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
    }
    $messages[] = 'System already present — bootstrapped full';
  }

  // Ensure full API after system install
  if (!function_exists('module_enable') && function_exists('drupal_bootstrap')) {
    drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
  }

  // Load profile
  $profile_file = './profiles/rokupal/rokupal.profile';
  if (!is_file($profile_file)) {
    throw new Exception('Missing profiles/rokupal/rokupal.profile');
  }
  require_once $profile_file;

  $modules = array('filter', 'node', 'user', 'block', 'comment', 'taxonomy', 'menu', 'dblog', 'search', 'help');
  if (function_exists('rokupal_profile_modules')) {
    $modules = array_unique(array_merge($modules, rokupal_profile_modules()));
  }
  // Optional presets
  if ($enable_forum) {
    $modules[] = 'rokupal_forum';
  }
  if ($enable_poll) {
    $modules[] = 'rokupal_poll';
  }
  // Always blog + core
  $modules[] = 'rokupal_core';
  $modules[] = 'rokupal_blog';
  $modules[] = 'rokupal_markdown';
  $modules[] = 'color';
  $modules[] = 'profile';
  $modules = array_unique($modules);

  // Install + FORCE enable each module (status=1, schema>=0)
  foreach ($modules as $module) {
    if ($module === 'system') {
      continue;
    }
    // Seed path for cold install
    $guess = './modules/' . $module . '/' . $module . '.module';
    if (is_file($guess) && function_exists('drupal_get_filename')) {
      drupal_get_filename('module', $module, 'modules/' . $module . '/' . $module . '.module');
    }
    if (!drupal_get_filename('module', $module) && !is_file($guess)) {
      $messages[] = "Skip missing module: $module";
      continue;
    }
    if (function_exists('drupal_install_modules')) {
      drupal_install_modules(array($module));
    }
    else {
      _rokupal_install_one_module($module);
    }
    // Hard guarantee: enabled in {system}
    $fn = drupal_get_filename('module', $module);
    if (!$fn && is_file($guess)) {
      $fn = 'modules/' . $module . '/' . $module . '.module';
    }
    if ($fn) {
      $exists = db_result(db_query("SELECT name FROM {system} WHERE type = 'module' AND name = '%s'", $module));
      if ($exists) {
        db_query("UPDATE {system} SET status = 1, throttle = 0, filename = '%s' WHERE type = 'module' AND name = '%s'", $fn, $module);
        $sv = (int) db_result(db_query("SELECT schema_version FROM {system} WHERE type = 'module' AND name = '%s'", $module));
        if ($sv < 0) {
          db_query("UPDATE {system} SET schema_version = 0 WHERE type = 'module' AND name = '%s'", $module);
        }
      }
      else {
        db_query(
          "INSERT INTO {system} (filename, name, type, owner, status, throttle, bootstrap, schema_version, weight) VALUES ('%s', '%s', 'module', '', 1, 0, 0, 0, 0)",
          $fn, $module
        );
      }
    }
    $messages[] = "Installed+enabled module: $module";
  }

  module_rebuild_cache();
  if (function_exists('module_list')) {
    module_list(TRUE, FALSE);
  }
  if (function_exists('module_implements')) {
    module_implements('', FALSE, TRUE);
  }
  drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);

  // Site variables
  variable_set('site_name', 'RokuPal');
  variable_set('site_mail', $admin_mail);
  variable_set('install_profile', 'rokupal');
  variable_set('install_task', 'done');
  variable_set('theme_default', 'rokupal');
  // Frontpage set after modules verified below

  // Admin user (uid 1)
  $account = user_load(array('uid' => 1));
  if ($account && $account->uid == 1) {
    $edit = array(
      'name' => $admin_name,
      'mail' => $admin_mail,
      'pass' => $admin_pass,
      'status' => 1,
      'roles' => array(DRUPAL_AUTHENTICATED_RID => 'authenticated user'),
    );
    // roles for uid 1 usually include admin
    user_save($account, $edit);
    $messages[] = 'Admin account updated (uid 1)';
  }
  else {
    // Create if missing (unusual)
    $edit = array(
      'name' => $admin_name,
      'mail' => $admin_mail,
      'pass' => $admin_pass,
      'status' => 1,
    );
    $account = user_save('', $edit);
    $messages[] = 'Admin account created';
  }

  // Login as admin and keep session
  global $user;
  $account = user_load(array('name' => $admin_name, 'status' => 1));
  if ($account) {
    $user = $account;
    $user->roles = array();
    $user->roles[DRUPAL_AUTHENTICATED_RID] = 'authenticated user';
    $r = db_query("SELECT r.rid, r.name FROM {role} r INNER JOIN {users_roles} ur ON ur.rid = r.rid WHERE ur.uid = %d", $user->uid);
    while ($role = db_fetch_object($r)) {
      $user->roles[$role->rid] = $role->name;
    }
    $finalize = array();
    if (function_exists('user_authenticate_finalize')) {
      user_authenticate_finalize($finalize);
    }
    elseif (function_exists('sess_regenerate')) {
      sess_regenerate();
    }
    $messages[] = 'Logged in as ' . $admin_name;
  }

  // Profile tasks (theme, blocks, frontpage)
  if (function_exists('rokupal_profile_tasks')) {
    $task = 'profile';
    rokupal_profile_tasks($task, base_path());
    $messages[] = 'RokuPal profile tasks executed';
  }

  // Blog ensure again (blocks/frontpage)
  if (function_exists('module_load_include')) {
    module_load_include('install', 'rokupal_blog');
  }
  if (function_exists('rokupal_blog_ensure_type')) {
    rokupal_blog_ensure_type();
    rokupal_blog_ensure_vocab();
    rokupal_blog_setup_blocks();
  }

  // Frontpage: only use /blog if blog module is actually enabled
  $blog_on = (bool) db_result(db_query("SELECT status FROM {system} WHERE type = 'module' AND name = 'rokupal_blog'"));
  if ($blog_on) {
    variable_set('site_frontpage', 'blog');
  }
  else {
    variable_set('site_frontpage', 'node');
  }

  if (is_file('./modules/rokupal_markdown/rokupal_markdown.module')) {
    include_once './modules/rokupal_markdown/rokupal_markdown.module';
  }
  if (function_exists('rokupal_markdown_ensure_format')) {
    $md = rokupal_markdown_ensure_format(TRUE);
    variable_set('filter_default_format', $md);
    $messages[] = 'Markdown default format=' . $md;
  }

  menu_rebuild();
  cache_clear_all();
  // Clear menu cache tables explicitly
  if (db_table_exists('cache_menu')) {
    @db_query("DELETE FROM {cache_menu}");
  }
  $messages[] = 'Menu rebuilt, caches cleared';
  $messages[] = 'Frontpage=' . variable_get('site_frontpage', 'node');
  $messages[] = 'DONE';

  return $messages;
}

/**
 * Minimal single-module install if drupal_install_modules unavailable.
 */
function _rokupal_install_one_module($module) {
  $filename = drupal_get_filename('module', $module);
  if (!$filename) {
    return;
  }
  // Ensure system row exists before install hooks.
  $exists = db_result(db_query("SELECT name FROM {system} WHERE type = 'module' AND name = '%s'", $module));
  if (!$exists) {
    db_query(
      "INSERT INTO {system} (filename, name, type, owner, status, throttle, bootstrap, schema_version, weight, info)
       VALUES ('%s', '%s', 'module', '', 0, 0, 0, -1, 0, '%s')",
      $filename,
      $module,
      serialize(array('name' => $module, 'core' => '6.x', 'package' => 'RokuPal'))
    );
  }
  else {
    db_query("UPDATE {system} SET filename = '%s' WHERE type = 'module' AND name = '%s'", $filename, $module);
  }

  drupal_load('module', $module);
  module_load_install($module);
  // Only run install if never installed (schema -1).
  $sv = drupal_get_installed_schema_version($module);
  if ($sv == -1 || $sv === FALSE || $sv === NULL) {
    module_invoke($module, 'install');
    $versions = function_exists('drupal_get_schema_versions') ? drupal_get_schema_versions($module) : array();
    $version = $versions ? max($versions) : 0;
    drupal_set_installed_schema_version($module, $version);
  }
  // Enable: status = 1
  db_query("UPDATE {system} SET status = 1, filename = '%s' WHERE type = 'module' AND name = '%s'", $filename, $module);
  module_invoke($module, 'enable');
}


/**
 * Install system tables without relying on DB-backed drupal_get_filename().
 * Fixes: undefined module_invoke() and "no such table: system" on first install.
 */
function _rokupal_install_system_tables() {
  global $conf;
  if (!is_array($conf)) {
    $conf = array();
  }

  // Required includes for cold install (order matters).
  $files = array(
    './includes/module.inc',
    './includes/common.inc',
    './includes/theme.inc',
    './includes/file.inc',
    './includes/lock-install.inc', // stub locks — no semaphore table yet
  );
  foreach ($files as $f) {
    if (is_file($f) && !in_array(realpath($f), get_included_files(), TRUE)) {
      // get_included_files realpath compare can fail on Windows; just require_once
    }
    if (is_file($f)) {
      require_once $f;
    }
  }
  // Do NOT load lock.inc here — bootstrap will load it later and would redeclare.
  // lock-install.inc provides stubs protected by function_exists.

  if (!is_file('./modules/system/system.install')) {
    throw new Exception('Missing modules/system/system.install');
  }
  require_once './modules/system/system.install';
  if (is_file('./modules/system/system.module')) {
    require_once './modules/system/system.module';
  }

  if (!function_exists('system_install')) {
    throw new Exception('system_install() missing');
  }
  if (!function_exists('drupal_install_schema')) {
    throw new Exception('drupal_install_schema() missing — common.inc not loaded');
  }

  // Seed module paths so drupal_get_filename works before {system} exists.
  $seed = array(
    'system' => 'modules/system/system.module',
    'filter' => 'modules/filter/filter.module',
    'block' => 'modules/block/block.module',
    'user' => 'modules/user/user.module',
    'node' => 'modules/node/node.module',
    'comment' => 'modules/comment/comment.module',
    'taxonomy' => 'modules/taxonomy/taxonomy.module',
  );
  foreach ($seed as $mod => $path) {
    if (is_file('./' . $path) && function_exists('drupal_get_filename')) {
      drupal_get_filename('module', $mod, $path);
    }
  }

  // system_install(): schemas → variables → optional theme scan → users/roles
  system_install();

  $system_version = 0;
  if (function_exists('drupal_get_schema_versions')) {
    $versions = drupal_get_schema_versions('system');
    if (is_array($versions) && $versions) {
      $system_version = max($versions);
    }
  }
  if (defined('SCHEMA_INSTALLED') && !$system_version) {
    $system_version = SCHEMA_INSTALLED;
  }

  $filename = 'modules/system/system.module';
  $exists = FALSE;
  $prev = error_reporting(E_ALL & ~E_WARNING);
  try {
    $exists = (bool) db_result(db_query("SELECT name FROM {system} WHERE name = 'system' AND type = 'module'"));
  }
  catch (Exception $e) {
    $exists = FALSE;
  }
  error_reporting($prev);

  if (!$exists) {
    db_query(
      "INSERT INTO {system} (filename, name, type, owner, status, throttle, bootstrap, schema_version, weight) VALUES ('%s', '%s', '%s', '%s', %d, %d, %d, %d, %d)",
      $filename, 'system', 'module', '', 1, 0, 0, $system_version, 0
    );
  }

  if (function_exists('drupal_bootstrap')) {
    drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
  }
  if (function_exists('module_rebuild_cache')) {
    module_rebuild_cache();
  }
  // Register themes now that bootstrap + locks are available
  if (function_exists('system_theme_data')) {
    try {
      system_theme_data();
    }
    catch (Exception $e) {
      // non-fatal
    }
  }
}
