<?php
/**
 * @file
 * RokuPal themes manager.
 */

function rokupal_core_themes_page() {
  drupal_set_title(t('Themes'));
  $token = function_exists('drupal_get_token') ? drupal_get_token('rokupal_themes') : '';

  if ($_SERVER['REQUEST_METHOD'] === 'POST' && !empty($_POST['rokupal_theme_op'])) {
    if (!empty($_POST['token']) && function_exists('drupal_valid_token') && !drupal_valid_token($_POST['token'], 'rokupal_themes')) {
      drupal_set_message(t('Invalid token'), 'error');
      drupal_goto('admin/build/themes');
    }
    $name = preg_replace('/[^a-z0-9_\-]/', '', $_POST['theme']);
    $op = $_POST['rokupal_theme_op'];
    if ($name !== '') {
      if ($op === 'enable') {
        db_query("UPDATE {system} SET status = 1 WHERE type = 'theme' AND name = '%s'", $name);
        drupal_set_message(t('Enabled %t.', array('%t' => $name)));
      }
      elseif ($op === 'disable') {
        $default = variable_get('theme_default', 'rokupal');
        if ($name === $default) {
          drupal_set_message(t('Cannot disable the default theme.'), 'error');
        }
        else {
          db_query("UPDATE {system} SET status = 0 WHERE type = 'theme' AND name = '%s'", $name);
          drupal_set_message(t('Disabled %t.', array('%t' => $name)));
        }
      }
      elseif ($op === 'default') {
        db_query("UPDATE {system} SET status = 1 WHERE type = 'theme' AND name = '%s'", $name);
        variable_set('theme_default', $name);
        drupal_set_message(t('%t is now the default theme.', array('%t' => $name)));
      }
      if (function_exists('list_themes')) {
        list_themes(TRUE);
      }
      cache_clear_all();
    }
    drupal_goto('admin/build/themes');
  }

  // Ensure themes scanned
  if (function_exists('system_theme_data')) {
    @system_theme_data();
  }
  $themes = function_exists('list_themes') ? list_themes(TRUE) : array();
  $default = variable_get('theme_default', 'rokupal');

  $out = '<div class="rp-themes" style="max-width:900px;font-family:system-ui,sans-serif">';
  $out .= '<p>' . t('Choose the default theme. RokuPal is the recommended theme.') . '</p>';
  $out .= '<div style="display:grid;gap:0.75rem">';

  if (empty($themes)) {
    // Fallback single theme
    $themes = array('rokupal' => (object) array('name' => 'rokupal', 'status' => 1, 'info' => array('name' => 'RokuPal')));
  }

  foreach ($themes as $key => $theme) {
    if (!empty($theme->info['hidden'])) {
      continue;
    }
    $label = !empty($theme->info['name']) ? $theme->info['name'] : $key;
    $enabled = !empty($theme->status);
    $is_default = ($key === $default);
    $out .= '<article style="border:1px solid #dde3ea;border-radius:10px;padding:1rem;background:#fff' . ($is_default ? ';border-left:4px solid #2f9e44' : '') . '">';
    $out .= '<h2 style="margin:0 0 0.35rem">' . check_plain($label) . ' <code style="font-size:0.75rem;color:#888">' . check_plain($key) . '</code>';
    if ($is_default) {
      $out .= ' <span style="background:#d3f9d8;color:#2b8a3e;font-size:0.7rem;padding:0.15rem 0.4rem;border-radius:999px;font-weight:700">' . t('DEFAULT') . '</span>';
    }
    $out .= '</h2>';
    if (!empty($theme->info['description'])) {
      $out .= '<p style="color:#555;margin:0.25rem 0">' . check_plain($theme->info['description']) . '</p>';
    }
    $out .= '<div style="display:flex;gap:0.4rem;flex-wrap:wrap;margin-top:0.5rem">';
    if (!$is_default) {
      $out .= '<form method="post" action="' . check_url(url('admin/build/themes')) . '" style="display:inline">';
      $out .= '<input type="hidden" name="rokupal_theme_op" value="default" /><input type="hidden" name="theme" value="' . check_plain($key) . '" />';
      if ($token) {
        $out .= '<input type="hidden" name="token" value="' . check_plain($token) . '" />';
      }
      $out .= '<button type="submit" style="background:#2f9e44;color:#fff;border:0;border-radius:6px;padding:0.35rem 0.7rem;font-weight:600;cursor:pointer">' . t('Set default') . '</button></form>';
    }
    if ($key === 'rokupal') {
      $out .= ' <a href="' . check_url(url('admin/settings/color')) . '" style="padding:0.35rem 0.7rem;background:#e7f5ff;border-radius:6px;text-decoration:none;font-weight:600">' . t('Colors') . '</a>';
      $out .= ' <a href="' . check_url(url('admin/build/themes/settings/rokupal')) . '" style="padding:0.35rem 0.7rem;background:#f1f3f5;border-radius:6px;text-decoration:none;font-weight:600">' . t('Settings') . '</a>';
    }
    $out .= '</div></article>';
  }
  $out .= '</div></div>';
  return $out;
}
