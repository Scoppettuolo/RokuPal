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

function rokupal_preprocess_page(&$vars) {
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
  if ($uid && function_exists('profile_user_avatar')) {
    $avatar = profile_user_avatar($uid);
  }
  if ($uid) {
    $link = l($name, 'user/' . $uid);
  }
  else {
    $link = check_plain($name);
  }
  if ($avatar) {
    return '<span class="rp-user">' . $avatar . ' ' . $link . '</span>';
  }
  return $link;
}
