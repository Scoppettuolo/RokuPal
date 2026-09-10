<?php
/**
 * @file
 * RokuPal install profile — blog by default, admin stays logged in.
 */

function rokupal_profile_modules() {
  // Core modules + blog only (forum/poll optional later).
  return array(
    'comment',
    'help',
    'menu',
    'taxonomy',
    'dblog',
    'search',
    'rokupal_core',
    'rokupal_blog',
  );
}

function rokupal_profile_details() {
  return array(
    'name' => 'RokuPal',
    'description' => 'RokuPal: tema propio + blog estilo WordPress por defecto (bloques y sidebar). Forum y Poll se activan aparte si se necesitan.',
  );
}

function rokupal_profile_task_list() {
  return array();
}

/**
 * After site configure form: theme, frontpage=blog, blocks, keep admin session.
 */
function rokupal_profile_tasks(&$task, $url) {
  // Content types minimal
  if (!node_get_types('type', 'page')) {
    $page = (object) array(
      'type' => 'page',
      'name' => st('Page'),
      'module' => 'node',
      'description' => st('Static page.'),
      'custom' => TRUE,
      'modified' => TRUE,
      'locked' => FALSE,
      'has_title' => 1,
      'title_label' => st('Title'),
      'has_body' => 1,
      'body_label' => st('Body'),
      'min_word_count' => 0,
    );
    node_type_save($page);
    variable_set('node_options_page', array('status'));
    variable_set('comment_page', 0);
  }

  // Theme: only RokuPal
  if (is_file('themes/rokupal/rokupal.info')) {
    db_query("DELETE FROM {system} WHERE type = 'theme' AND name = 'rokupal'");
    $info = array(
      'name' => 'RokuPal',
      'description' => 'RokuPal',
      'core' => '6.x',
      'engine' => 'phptemplate',
      'stylesheets' => array('all' => array('style.css' => 'themes/rokupal/style.css')),
      'regions' => array(
        'left' => 'Left sidebar',
        'right' => 'Right sidebar',
        'content' => 'Content',
        'header' => 'Header',
        'footer' => 'Footer',
      ),
      'features' => array(
        'logo' => 1, 'name' => 1, 'slogan' => 1, 'mission' => 1,
        'search' => 1, 'favicon' => 1, 'primary_links' => 1, 'secondary_links' => 1,
      ),
    );
    $owner = is_file('themes/engines/phptemplate/phptemplate.engine')
      ? 'themes/engines/phptemplate/phptemplate.engine'
      : 'themes/rokupal/rokupal.info';
    db_query(
      "INSERT INTO {system} (filename, name, type, owner, status, throttle, bootstrap, schema_version, weight, info)
       VALUES ('%s', 'rokupal', 'theme', '%s', 1, 0, 0, -1, 0, '%s')",
      'themes/rokupal/rokupal.info',
      $owner,
      serialize($info)
    );
    variable_set('theme_default', 'rokupal');
    variable_set('theme_rokupal_settings', array(
      'toggle_logo' => 1,
      'default_logo' => 1,
      'logo_path' => '',
      'toggle_name' => 1,
      'toggle_slogan' => 1,
      'toggle_mission' => 0,
      'toggle_favicon' => 1,
      'default_favicon' => 1,
      'mission' => '',
    ));
  }

  // Front page = blog (no "Welcome to your site")
  variable_set('site_frontpage', 'blog');

  // Ensure blog structure + blocks (module already enabled via profile_modules)
  if (function_exists('module_load_include')) {
    module_load_include('install', 'rokupal_blog');
  }
  if (function_exists('rokupal_blog_ensure_type')) {
    rokupal_blog_ensure_type();
    rokupal_blog_ensure_vocab();
    rokupal_blog_setup_blocks();
  }

  // Hide default Drupal welcome by never using empty front node list for anon
  variable_set('rokupal_hide_welcome', 1);

  menu_rebuild();
  cache_clear_all();

  // Keep installer logged in as uid 1.
  global $user;
  $account = user_load(array('uid' => 1));
  if ($account && !empty($account->uid)) {
    $user = $account;
    $edit = array();
    // Rebuild roles
    $user->roles = array();
    $user->roles[2] = 'authenticated user'; // DRUPAL_AUTHENTICATED_RID = 2
    $r = db_query("SELECT r.rid, r.name FROM {role} r INNER JOIN {users_roles} ur ON ur.rid = r.rid WHERE ur.uid = %d", 1);
    while ($role = db_fetch_object($r)) {
      $user->roles[$role->rid] = $role->name;
    }
    if (function_exists('user_authenticate_finalize')) {
      user_authenticate_finalize($edit);
    }
    else {
      if (function_exists('sess_regenerate')) {
        sess_regenerate();
      }
      $user->sid = session_id();
    }
  }
}

/**
 * Alter configure form defaults.
 */
function rokupal_form_alter(&$form, $form_state, $form_id) {
  if ($form_id == 'install_configure') {
    if (isset($form['site_information']['site_name'])) {
      $form['site_information']['site_name']['#default_value'] = 'RokuPal';
    }
  }
}
