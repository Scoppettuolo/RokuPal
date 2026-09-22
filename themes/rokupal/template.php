<?php
/**
 * @file
 * RokuPal theme helpers. Single location: themes/rokupal/ (never sites/all).
 */

function rokupal_theme_dir() {
  return str_replace('\\', '/', dirname(__FILE__));
}

/**
 * Relative path from RokuPal root. Prefer themes/rokupal only.
 */
function rokupal_theme_path() {
  static $path = NULL;
  if ($path !== NULL) {
    return $path;
  }
  // Canonical location for this fork — do not use sites/all/themes/rokupal.
  if (is_file('themes/rokupal/style.css')) {
    $path = 'themes/rokupal';
    return $path;
  }
  // Fallback if installed under sites/all (legacy only).
  if (function_exists('drupal_get_path')) {
    $p = drupal_get_path('theme', 'rokupal');
    if ($p && is_file($p . '/style.css')) {
      $path = $p;
      return $path;
    }
  }
  $path = 'themes/rokupal';
  return $path;
}

function rokupal_url($relative) {
  $base = function_exists('base_path') ? base_path() : '/';
  return $base . rokupal_theme_path() . '/' . ltrim(str_replace('\\', '/', $relative), '/');
}


/**
 * Ensure theme CSS is always registered (subdirectory + cache safe).
 */
function rokupal_ensure_css() {
  static $done = FALSE;
  if ($done) {
    return;
  }
  $done = TRUE;
  // Disable aggregation once (not every request write).
  if (function_exists('variable_get') && !variable_get('rokupal_agg_off', 0)) {
    if (variable_get('preprocess_css', 0) || variable_get('preprocess_js', 0)) {
      variable_set('preprocess_css', 0);
      variable_set('preprocess_js', 0);
    }
    variable_set('rokupal_agg_off', 1);
  }
  if (!function_exists('drupal_add_css')) {
    return;
  }
  $base = rokupal_theme_path();
  // Critical CSS always.
  $files = array(
    $base . '/style.css',
    $base . '/layout.css',
    $base . '/css/base.css',
    $base . '/css/layout.css',
    $base . '/css/components.css',
  );
  // Lazy: forum CSS only on forum paths (or when forum module listing nodes).
  $need_forum = (arg(0) === 'forum');
  if ($need_forum) {
    $files[] = $base . '/css/forum.css';
  }
  foreach ($files as $f) {
    if (is_file($f)) {
      drupal_add_css($f, 'theme');
    }
  }
  if (is_file($base . '/js/rokupal.js') && function_exists('drupal_add_js')) {
    drupal_add_js($base . '/js/rokupal.js', 'theme', 'footer');
  }
}

function rokupal_preprocess_page(&$vars) {
  rokupal_ensure_css();

  // Search box in header when Search module is on.
  $vars['search_box'] = '';
  if (function_exists('module_exists') && module_exists('search') && user_access('search content') && function_exists('drupal_get_form')) {
    $vars['search_box'] = drupal_get_form('search_theme_form');
  }

  // Body classes for layout/CSS hooks.
  $classes = array('rokupal');
  if (!empty($vars['rokupal_admin_bar'])) {
    $classes[] = 'has-rp-admin-bar';
  }
  if (!empty($vars['left'])) {
    $classes[] = 'with-sidebar-first';
  }
  if (!empty($vars['right'])) {
    $classes[] = 'with-sidebar-second';
  }
  if (!empty($vars['is_front'])) {
    $classes[] = 'front';
  }
  else {
    $classes[] = 'not-front';
  }
  $vars['body_classes'] = implode(' ', $classes);


  // Lightweight admin bar (rokupal_core)
  $vars['rokupal_admin_bar'] = '';
  if (function_exists('rokupal_admin_bar_render')) {
    $vars['rokupal_admin_bar'] = rokupal_admin_bar_render();
  }
  elseif (function_exists('rokupal_core_admin_bar_render')) {
    $vars['rokupal_admin_bar'] = rokupal_core_admin_bar_render();
  }

  $vars['rokupal_color_css'] = '';
  // Live color palette (do not rely only on module preprocess order)
  if (function_exists('color_build_css')) {
    $css = color_build_css();
    if ($css !== '') {
      // MUST print after style.css or :root defaults override palette
      $vars['rokupal_color_css'] = $css;
    }
  }
  elseif (function_exists('variable_get')) {
    $p = variable_get('color_rokupal_palette', array());
    if (is_array($p) && $p) {
      $css = ":root{
";
      $map = array(
        'bg' => '--rp-bg', 'surface' => '--rp-surface', 'text' => '--rp-text', 'muted' => '--rp-muted',
        'header' => '--rp-brand', 'header_text' => '--rp-header-text', 'header_link' => '--rp-header-link',
        'footer' => '--rp-footer-bg', 'footer_text' => '--rp-footer-text', 'accent' => '--rp-accent',
        'link' => '--rp-link', 'post_bg' => '--rp-post-bg', 'post_border' => '--rp-post-border',
        'sidebar_bg' => '--rp-sidebar-bg', 'border' => '--rp-border',
      );
      foreach ($map as $k => $var) {
        if (!empty($p[$k])) {
          $css .= $var . ':' . $p[$k] . ";
";
        }
      }
      $css .= "}
";
      $vars['rokupal_color_css'] = $css;
      $tag = '<style type="text/css" id="rokupal-color-vars">' . $css . '</style>';
      $vars['head'] = (isset($vars['head']) ? $vars['head'] : '') . $tag;
    }
  }

  $path = rokupal_theme_path();

  if (function_exists('drupal_add_css')) {
    drupal_add_css($path . '/style.css', 'theme', 'all', FALSE);
  $color_css = file_directory_path() . '/css/rokupal-color.css';
  if (is_file($color_css)) {
    drupal_add_css($color_css, 'module', 'all', FALSE);
  }
  }
  if (function_exists('drupal_get_css')) {
    $vars['styles'] = drupal_get_css();
  }

  // Logo only from themes/rokupal/logo.png
  $logo_fs = rokupal_theme_dir() . '/logo.png';
  if (!is_file($logo_fs) && is_file('themes/rokupal/logo.png')) {
    $logo_fs = getcwd() . '/themes/rokupal/logo.png';
  }
  if (is_file($logo_fs) || is_file('themes/rokupal/logo.png')) {
    $vars['logo'] = rokupal_url('logo.png');
  }
  else {
    $vars['logo'] = '';
  }

  $classes = array('rokupal');
  $classes[] = !empty($vars['is_front']) ? 'is-front' : 'not-front';
  $classes[] = !empty($vars['logged_in']) ? 'is-logged-in' : 'is-anonymous';
  if (!empty($vars['is_admin'])) {
    $classes[] = 'is-admin';
  }
  $vars['body_classes'] = implode(' ', $classes);

  // Region aliases: sidebar_first/second -> left/right
  if (empty($vars['left']) && !empty($vars['sidebar_first'])) {
    $vars['left'] = $vars['sidebar_first'];
  }
  if (empty($vars['right']) && !empty($vars['sidebar_second'])) {
    $vars['right'] = $vars['sidebar_second'];
  }
  // Merge both if both set
  if (!empty($vars['sidebar_first']) && !empty($vars['left']) && $vars['left'] !== $vars['sidebar_first']) {
    $vars['left'] = $vars['sidebar_first'] . $vars['left'];
  }
  if (!empty($vars['sidebar_second']) && !empty($vars['right']) && $vars['right'] !== $vars['sidebar_second']) {
    $vars['right'] = $vars['sidebar_second'] . $vars['right'];
  }
  $vars['has_sidebar_first'] = !empty($vars['left']);
  $vars['has_sidebar_second'] = !empty($vars['right']);
}

function phptemplate_preprocess_page(&$vars) {
  rokupal_preprocess_page($vars);
}


function rokupal_preprocess_comment(&$vars) {
  if (empty($vars['created']) && !empty($vars['comment']->timestamp)) {
    $vars['created'] = format_date($vars['comment']->timestamp, 'small');
  }
  if (empty($vars['author']) && !empty($vars['comment']->uid)) {
    $vars['author'] = theme('username', $vars['comment']);
  }
}


/**
 * Username with optional external avatar.
 */
function phptemplate_username($object) {
  $name = !empty($object->name) ? $object->name : t('Anonymous');
  $uid = isset($object->uid) ? (int) $object->uid : 0;
  $avatar = '';

  // Core user picture (user.module).
  if ($uid && function_exists('variable_get') && variable_get('user_pictures', 0)) {
    $account = is_object($object) ? $object : NULL;
    if ($account && empty($account->picture) && function_exists('user_load')) {
      $full = user_load($uid);
      if ($full) {
        $account = $full;
      }
    }
    if (!empty($account->picture) && function_exists('theme')) {
      // theme_user_picture expects a user object.
      $avatar = theme('user_picture', $account);
    }
  }

  if ($uid) {
    $link = l($name, 'user/' . $uid, array('attributes' => array('class' => 'rp-username')));
  }
  else {
    $link = '<span class="rp-username">' . check_plain($name) . '</span>';
  }
  if ($avatar) {
    return '<span class="rp-user rp-user-with-pic">' . $avatar . ' ' . $link . '</span>';
  }
  return '<span class="rp-user">' . $link . '</span>';
}


/**
 * Compact search: no label, short button.
 */
function rokupal_preprocess_search_theme_form(&$vars) {
  if (isset($vars['form']['search_theme_form']['#title'])) {
    $vars['form']['search_theme_form']['#title'] = '';
    $vars['form']['search_theme_form']['#title_display'] = 'invisible';
  }
  // D6 structure: form keys vary; also strip via CSS.
  if (isset($vars['form']['submit']['#value'])) {
    $vars['form']['submit']['#value'] = t('Go');
  }
}


/**
 * Theme override: compact header search (input + button, no label).
 * D6 default stacks label / field / submit on three lines.
 */
function rokupal_search_theme_form($form) {
  // Hide title permanently in render array.
  if (isset($form['search_theme_form'])) {
    unset($form['search_theme_form']['#title']);
    $form['search_theme_form']['#title'] = '';
  }
  // Build minimal markup — avoid theme('form_element') which wraps label.
  $output = '<div id="search" class="container-inline rp-search-inline">';
  if (isset($form['search_theme_form'])) {
    $output .= drupal_render($form['search_theme_form']);
  }
  if (isset($form['submit'])) {
    $form['submit']['#value'] = t('Search');
    $output .= drupal_render($form['submit']);
  }
  // Render any remaining (token, etc.) hidden.
  $output .= drupal_render($form);
  $output .= '</div>';
  return $output;
}
