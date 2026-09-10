<?php

/**
 * @file
 * Theming for maintenance pages.
 */

/**
 * Sets up the theming system for site installs, updates and when the site is
 * in off-line mode. It also applies when the database is unavailable.
 *
 * Minnelli is always used for the initial install and update operations. In
 * other cases, "settings.php" must have a "maintenance_theme" key set for the
 * $conf variable in order to change the maintenance theme.
 */
function _drupal_maintenance_theme() {
  global $theme, $theme_key;

  // If $theme is already set, assume the others are set too, and do nothing.
  if (isset($theme)) {
    return;
  }

  require_once './includes/path.inc';
  require_once './includes/theme.inc';
  require_once './includes/common.inc';
  require_once './includes/unicode.inc';
  require_once './includes/file.inc';
  require_once './includes/module.inc';
  require_once './includes/database.inc';
  unicode_check();

  // Install and update pages are treated differently to prevent theming overrides.
  if (defined('MAINTENANCE_MODE') && (MAINTENANCE_MODE == 'install' || MAINTENANCE_MODE == 'update')) {
    $theme = 'minnelli';
  }
  else {
    if (!db_is_active()) {
      // Because we are operating in a crippled environment, we need to
      // bootstrap just enough to allow hook invocations to work.
      $module_list['system']['filename'] = 'modules/system/system.module';
      $module_list['filter']['filename'] = 'modules/filter/filter.module';
      module_list(TRUE, FALSE, FALSE, $module_list);
      drupal_load('module', 'system');
      drupal_load('module', 'filter');
    }

    $theme = variable_get('maintenance_theme', 'minnelli');
  }

  $themes = list_themes();

  // Store the identifier for retrieving theme settings with.
  $theme_key = $theme;

  // Find all our ancestor themes and put them in an array.
  $base_theme = array();
  $ancestor = $theme;
  while ($ancestor && isset($themes[$ancestor]->base_theme)) {
    $base_theme[] = $new_base_theme = $themes[$themes[$ancestor]->base_theme];
    $ancestor = $themes[$ancestor]->base_theme;
  }
  _init_theme($themes[$theme], array_reverse($base_theme), '_theme_load_offline_registry');

  // These are usually added from system_init() -except maintenance.css.
  // When the database is inactive it's not called so we add it here.
  drupal_add_css(drupal_get_path('module', 'system') .'/defaults.css', 'module');
  drupal_add_css(drupal_get_path('module', 'system') .'/system.css', 'module');
  drupal_add_css(drupal_get_path('module', 'system') .'/system-menus.css', 'module');
  drupal_add_css(drupal_get_path('module', 'system') .'/maintenance.css', 'module');
}

/**
 * This builds the registry when the site needs to bypass any database calls.
 */
function _theme_load_offline_registry($theme, $base_theme = NULL, $theme_engine = NULL) {
  $registry = _theme_build_registry($theme, $base_theme, $theme_engine);
  _theme_set_registry($registry);
}

/**
 * Return a themed list of maintenance tasks to perform.
 *
 * @ingroup themeable
 */
function theme_task_list($items, $active = NULL) {
  $done = isset($items[$active]) || $active == NULL;
  $output = '<ol class="task-list">';
  foreach ($items as $k => $item) {
    if ($active == $k) {
      $class = 'active';
      $done = false;
    }
    else {
      $class = $done ? 'done' : '';
    }
    $output .= '<li class="'. $class .'">'. $item .'</li>';
  }
  $output .= '</ol>';
  return $output;
}

/**
 * Generate a themed installation page.
 *
 * Note: this function is not themeable.
 *
 * @param $content
 *   The page content to show.
 */
function theme_install_page($content) {
  drupal_set_header('Content-Type: text/html; charset=utf-8');
  // A broken form callback must not make the page body non-renderable.
  if (!is_scalar($content)) {
    $content = '';
  }
  $content = (string) $content;

  // Collect messages before rendering.
  $messages_html = '';
  $messages = drupal_set_message();
  if (isset($messages['error'])) {
    $title = count($messages['error']) > 1
      ? st('Fix the following to continue')
      : st('Fix the following to continue');
    $messages_html .= '<div class="messages error"><strong>'. $title .'</strong>'. theme('status_messages', 'error') .'<p>'. st('<a href="!url">Try again</a> after correcting the values.', array('!url' => check_url(request_uri()))) .'</p></div>';
  }
  if (isset($messages['warning'])) {
    $messages_html .= '<div class="messages warning">'. theme('status_messages', 'warning') .'</div>';
  }
  if (isset($messages['status'])) {
    $messages_html .= '<div class="messages status">'. theme('status_messages', 'status') .'</div>';
  }

  $title = drupal_get_title();
  $site = function_exists('drupal_install_profile_name') ? drupal_install_profile_name() : 'Drupal';
  $css_path = base_path() . 'misc/install-modern.css';
  // Task list is printed via install_task_list() into the sidebar region of the
  // old template; capture it from $content if already embedded, or from a global.
  $steps = isset($GLOBALS['install_steps_html']) ? $GLOBALS['install_steps_html'] : '';

  $html = '<!DOCTYPE html>'."
";
  $html .= '<html lang="en"><head><meta charset="utf-8">';
  $html .= '<meta name="viewport" content="width=device-width, initial-scale=1">';
  $html .= '<title>'. check_plain($title ? $title : st('Install')) .' · '. check_plain($site) .'</title>';
  $html .= '<link rel="stylesheet" href="'. check_url($css_path) .'">';
  $html .= '</head><body class="install-modern"><div class="install-shell">';
  $html .= '<header class="install-brand"><h1>'. check_plain($site) .'</h1>';
  $html .= '<p>'. st('Modern installer · PHP 8 ready') .'</p></header>';
  $html .= '<main class="install-card">';
  if ($steps) {
    $html .= $steps;
  }
  if ($title) {
    $html .= '<h2 class="page-title">'. check_plain($title) .'</h2>';
  }
  $html .= $messages_html;
  $html .= $content;
  $html .= '</main>';
  $html .= '<footer class="install-footer">'. st('Secure connection recommended · settings.php must be writable during install') .'</footer>';
  $html .= '</div></body></html>';
  return $html;
}

/**
 * Generate a themed update page.
 *
 * Note: this function is not themeable.
 *
 * @param $content
 *   The page content to show.
 * @param $show_messages
 *   Whether to output status and error messages.
 *   FALSE can be useful to postpone the messages to a subsequent page.
 */
function theme_update_page($content, $show_messages = TRUE) {
  // Set required headers.
  drupal_set_header('Content-Type: text/html; charset=utf-8');

  // Assign content and show message flag.
  $variables['content'] = $content;
  $variables['show_messages'] = $show_messages;
  // The maintenance preprocess function is recycled here.
  template_preprocess_maintenance_page($variables);

  // Special handling of warning messages.
  $messages = drupal_set_message();
  if (isset($messages['warning'])) {
    $title = count($messages['warning']) > 1 ? 'The following update warnings should be carefully reviewed before continuing' : 'The following update warning should be carefully reviewed before continuing';
    $variables['messages'] .= '<h4>'. $title .':</h4>';
    $variables['messages'] .= theme('status_messages', 'warning');
  }

  // This was called as a theme hook (not template), so we need to
  // fix path_to_theme() for the template, to point at the actual
  // theme rather than system module as owner of the hook.
  global $theme_path;
  $theme_path = 'themes/garland';

  return theme_render_template('themes/garland/maintenance-page.tpl.php', $variables);
}

/**
 * The variables generated here is a mirror of template_preprocess_page().
 * This preprocessor will run it's course when theme_maintenance_page() is
 * invoked. It is also used in theme_install_page() and theme_update_page() to
 * keep all the variables consistent.
 *
 * An alternate template file of "maintenance-page-offline.tpl.php" can be
 * used when the database is offline to hide errors and completely replace the
 * content.
 *
 * The $variables array contains the following arguments:
 * - $content
 * - $show_blocks
 *
 * @see maintenance-page.tpl.php
 */
function template_preprocess_maintenance_page(&$variables) {
  // Add favicon
  if (theme_get_setting('toggle_favicon')) {
    drupal_set_html_head('<link rel="shortcut icon" href="'. check_url(theme_get_setting('favicon')) .'" type="image/x-icon" />');
  }

  global $theme;
  // Retrieve the theme data to list all available regions.
  $theme_data = _system_theme_data();
  $regions = $theme_data[$theme]->info['regions'];

  // Get all region content set with drupal_set_content().
  foreach (array_keys($regions) as $region) {
    // Assign region to a region variable.
    $region_content = drupal_get_content($region);
    isset($variables[$region]) ? $variables[$region] .= $region_content : $variables[$region] = $region_content;
  }

  // Setup layout variable.
  $variables['layout'] = 'none';
  if (!empty($variables['left'])) {
    $variables['layout'] = 'left';
  }
  if (!empty($variables['right'])) {
    $variables['layout'] = ($variables['layout'] == 'left') ? 'both' : 'right';
  }

  // Construct page title
  if (drupal_get_title()) {
    $head_title = array(strip_tags(drupal_get_title()), variable_get('site_name', 'Drupal'));
  }
  else {
    $head_title = array(variable_get('site_name', 'Drupal'));
    if (variable_get('site_slogan', '')) {
      $head_title[] = variable_get('site_slogan', '');
    }
  }
  $variables['head_title']        = implode(' | ', $head_title);
  $variables['base_path']         = base_path();
  $variables['breadcrumb']        = '';
  $variables['feed_icons']        = '';
  $variables['footer_message']    = filter_xss_admin(variable_get('site_footer', FALSE));
  $variables['head']              = drupal_get_html_head();
  $variables['help']              = '';
  $variables['language']          = $GLOBALS['language'];
  $variables['language']->dir     = $GLOBALS['language']->direction ? 'rtl' : 'ltr';
  $variables['logo']              = theme_get_setting('logo');
  $variables['messages']          = $variables['show_messages'] ? theme('status_messages') : '';
  $variables['mission']           = '';
  $variables['primary_links']     = array();
  $variables['secondary_links']   = array();
  $variables['search_box']        = '';
  $variables['site_name']         = (theme_get_setting('toggle_name') ? variable_get('site_name', 'Drupal') : '');
  $variables['site_slogan']       = (theme_get_setting('toggle_slogan') ? variable_get('site_slogan', '') : '');
  $variables['css']               = drupal_add_css();
  $variables['styles']            = drupal_get_css();
  $variables['scripts']           = drupal_get_js();
  $variables['tabs']              = '';
  $variables['title']             = drupal_get_title();
  $variables['closure']           = '';

  // Compile a list of classes that are going to be applied to the body element.
  $body_classes = array();
  $body_classes[] = 'in-maintenance';
  if (isset($variables['db_is_active']) && !$variables['db_is_active']) {
    $body_classes[] = 'db-offline';
  }
  if ($variables['layout'] == 'both') {
    $body_classes[] = 'two-sidebars';
  }
  elseif ($variables['layout'] == 'none') {
    $body_classes[] = 'no-sidebars';
  }
  else {
    $body_classes[] = 'one-sidebar sidebar-'. $variables['layout'];
  }
  $variables['body_classes'] = implode(' ', $body_classes);

  // Dead databases will show error messages so supplying this template will
  // allow themers to override the page and the content completely.
  if (isset($variables['db_is_active']) && !$variables['db_is_active']) {
    $variables['template_file'] = 'maintenance-page-offline';
  }
}
