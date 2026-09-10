<?php
/**
 * @file
 * RokuPal Blocks Manager — regions + HTML5 drag-and-drop.
 */

function rokupal_core_blocks_page() {
  // Refresh theme .info regions (new regions in rokupal.info)
  if (function_exists('list_themes')) {
    list_themes(TRUE);
  }
  if (function_exists('system_theme_data')) {
    @system_theme_data();
  }

  $path = drupal_get_path('module', 'rokupal_core');
  drupal_add_css($path . '/css/blocks-admin.css');
  drupal_add_js($path . '/js/blocks-dnd.js');
  drupal_set_title(t('Blocks'));

  $theme = variable_get('theme_default', 'rokupal');
  $regions = system_region_list($theme);
  if (empty($regions) || !is_array($regions)) {
    $regions = array(
      'header' => t('Header'),
      'content' => t('Content'),
      'left' => t('Left sidebar'),
      'right' => t('Right sidebar'),
      'footer' => t('Footer'),
    );
  }
  $regions[''] = t('Disabled');

  $token = function_exists('drupal_get_token') ? drupal_get_token('rokupal_blocks') : '';

  // Save layout from DnD (JSON) or single form
  if ($_SERVER['REQUEST_METHOD'] === 'POST' && !empty($_POST['rokupal_block_op'])) {
    if (!empty($_POST['token']) && function_exists('drupal_valid_token') && !drupal_valid_token($_POST['token'], 'rokupal_blocks')) {
      drupal_set_message(t('Invalid token'), 'error');
      drupal_goto('admin/build/block');
    }
    $op = $_POST['rokupal_block_op'];
    if ($op === 'save_layout' && !empty($_POST['layout_json'])) {
      $layout = json_decode($_POST['layout_json'], TRUE);
      if (is_array($layout)) {
        foreach ($layout as $region => $items) {
          if (!is_array($items)) {
            continue;
          }
          if ($region !== '' && !array_key_exists($region, $regions)) {
            continue;
          }
          $weight = 0;
          foreach ($items as $id) {
            if (!is_string($id) || strpos($id, ':') === FALSE) {
              continue;
            }
            list($module, $delta) = explode(':', $id, 2);
            $module = preg_replace('/[^a-z0-9_]/', '', $module);
            $status = ($region === '') ? 0 : 1;
            rokupal_core_block_place($theme, $module, $delta, $region, $weight, $status);
            $weight++;
          }
        }
        cache_clear_all();
        drupal_set_message(t('Block layout saved.'));
      }
    }
    elseif ($op === 'save_region' && !empty($_POST['module'])) {
      $module = preg_replace('/[^a-z0-9_]/', '', $_POST['module']);
      $delta = isset($_POST['delta']) ? $_POST['delta'] : '';
      $region = isset($_POST['region']) ? $_POST['region'] : '';
      if ($region !== '' && !array_key_exists($region, $regions)) {
        $region = '';
      }
      $weight = isset($_POST['weight']) ? (int) $_POST['weight'] : 0;
      $status = ($region === '') ? 0 : 1;
      rokupal_core_block_place($theme, $module, $delta, $region, $weight, $status);
      cache_clear_all();
      drupal_set_message(t('Block saved.'));
    }
    drupal_goto('admin/build/block');
  }

  $blocks = array();
  if (function_exists('module_implements')) {
    foreach (module_implements('block') as $module) {
      $list = module_invoke($module, 'block', 'list');
      if (!is_array($list)) {
        continue;
      }
      foreach ($list as $delta => $info) {
        $info = (array) $info;
        $key = $module . ':' . $delta;
        $blocks[$key] = array(
          'module' => $module,
          'delta' => (string) $delta,
          'info' => isset($info['info']) ? $info['info'] : $key,
        );
      }
    }
  }

  $placed = array();
  $result = db_query("SELECT * FROM {blocks} WHERE theme = '%s'", $theme);
  while ($row = db_fetch_object($result)) {
    $placed[$row->module . ':' . $row->delta] = $row;
  }

  $by_region = array();
  foreach ($regions as $rid => $label) {
    $by_region[$rid] = array();
  }
  foreach ($blocks as $key => $b) {
    $region = '';
    $weight = 0;
    if (isset($placed[$key])) {
      $region = $placed[$key]->region;
      $weight = (int) $placed[$key]->weight;
      if (!(int) $placed[$key]->status) {
        $region = '';
      }
    }
    if (!isset($by_region[$region])) {
      $region = '';
    }
    $b['region'] = $region;
    $b['weight'] = $weight;
    $by_region[$region][] = $b;
  }
  foreach ($by_region as &$list) {
    usort($list, function ($a, $b) { return $a['weight'] - $b['weight']; });
  }
  unset($list);

  $out = '<div class="rp-blk-wrap">';
  $out .= '<p class="rp-blk-intro">' . t('Drag blocks between regions, then click <strong>Save layout</strong>. Theme: @t', array('@t' => $theme)) . '</p>';
  $out .= '<form method="post" action="' . check_url(url('admin/build/block')) . '" id="rp-blk-form">';
  $out .= '<input type="hidden" name="rokupal_block_op" value="save_layout" />';
  $out .= '<input type="hidden" name="layout_json" id="rp-layout-json" value="" />';
  if ($token) {
    $out .= '<input type="hidden" name="token" value="' . check_plain($token) . '" />';
  }
  $out .= '<p><button type="submit" class="rp-btn rp-btn-primary" id="rp-blk-save">' . t('Save layout') . '</button></p>';
  $out .= '<div class="rp-blk-grid">';

  foreach ($regions as $rid => $label) {
    $out .= '<section class="rp-blk-region' . ($rid === '' ? ' is-disabled' : '') . '" data-region="' . check_plain($rid) . '">';
    $out .= '<h2>' . check_plain($label) . '</h2>';
    $out .= '<div class="rp-blk-drop" data-region="' . check_plain($rid) . '">';
    foreach ($by_region[$rid] as $b) {
      $id = $b['module'] . ':' . $b['delta'];
      $out .= '<article class="rp-blk-card" draggable="true" data-id="' . check_plain($id) . '">';
      $out .= '<div class="rp-blk-handle" title="' . t('Drag') . '">⋮⋮</div>';
      $out .= '<div class="rp-blk-title">' . check_plain($b['info']) . '</div>';
      $out .= '<div class="rp-blk-id"><code>' . check_plain($id) . '</code></div>';
      $out .= '</article>';
    }
    $out .= '</div></section>';
  }
  $out .= '</div>';
  $out .= '<p><button type="submit" class="rp-btn rp-btn-primary">' . t('Save layout') . '</button></p>';
  $out .= '</form></div>';
  return $out;
}

function rokupal_core_block_place($theme, $module, $delta, $region, $weight, $status) {
  $exists = db_result(db_query("SELECT module FROM {blocks} WHERE module = '%s' AND delta = '%s' AND theme = '%s'", $module, $delta, $theme));
  if ($exists) {
    db_query("UPDATE {blocks} SET region = '%s', status = %d, weight = %d WHERE module = '%s' AND delta = '%s' AND theme = '%s'",
      $region, $status, $weight, $module, $delta, $theme);
  }
  else {
    db_query("INSERT INTO {blocks} (module, delta, theme, status, weight, region, custom, throttle, visibility, pages, title) VALUES ('%s', '%s', '%s', %d, %d, '%s', 0, 0, 0, '', '')",
      $module, $delta, $theme, $status, $weight, $region);
  }
}
