<?php
/**
 * @file
 * RokuPal Menus Manager.
 */

function rokupal_core_menus_page($menu_name = NULL) {
  drupal_add_css(drupal_get_path('module', 'rokupal_core') . '/css/menus-admin.css');
  $token = function_exists('drupal_get_token') ? drupal_get_token('rokupal_menus') : '';

  if ($_SERVER['REQUEST_METHOD'] === 'POST' && !empty($_POST['rokupal_menu_op'])) {
    if (!empty($_POST['token']) && function_exists('drupal_valid_token') && !drupal_valid_token($_POST['token'], 'rokupal_menus')) {
      drupal_set_message(t('Invalid token'), 'error');
      drupal_goto('admin/build/menu');
    }
    $op = $_POST['rokupal_menu_op'];

    if ($op === 'add_link') {
      $mname = preg_replace('/[^a-z0-9\-_]/', '', $_POST['menu_name']);
      $title = trim($_POST['link_title']);
      $path = trim($_POST['link_path']);
      $weight = (int) $_POST['weight'];
      if ($title !== '' && $path !== '' && $mname !== '') {
        $path = preg_replace('#^/#', '', $path);
        $external = preg_match('#^https?://#i', $path) ? 1 : 0;
        $options = serialize(array());
        db_query(
          "INSERT INTO {menu_links} (menu_name, link_path, router_path, link_title, options, module, hidden, external, has_children, expanded, weight, depth, customized, p1, p2, p3, p4, p5, p6, p7, p8, p9) VALUES ('%s', '%s', '%s', '%s', '%s', 'menu', 0, %d, 0, 0, %d, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0)",
          $mname, $path, $external ? '' : $path, $title, $options, $external, $weight
        );
        $mlid = function_exists('db_last_insert_id') ? db_last_insert_id('menu_links', 'mlid') : 0;
        if ($mlid) {
          db_query("UPDATE {menu_links} SET p1 = %d WHERE mlid = %d", $mlid, $mlid);
        }
        rokupal_core_menu_cache_clear();
        drupal_set_message(t('Link added.'));
      }
      drupal_goto('admin/build/menu-customize/' . $mname);
    }

    if ($op === 'delete_link' && !empty($_POST['mlid'])) {
      $mlid = (int) $_POST['mlid'];
      $mname = db_result(db_query("SELECT menu_name FROM {menu_links} WHERE mlid = %d", $mlid));
      db_query("DELETE FROM {menu_links} WHERE mlid = %d", $mlid);
      rokupal_core_menu_cache_clear();
      drupal_set_message(t('Link deleted.'));
      drupal_goto($mname ? 'admin/build/menu-customize/' . $mname : 'admin/build/menu');
    }

    if ($op === 'toggle_link' && !empty($_POST['mlid'])) {
      $mlid = (int) $_POST['mlid'];
      $hidden = (int) db_result(db_query("SELECT hidden FROM {menu_links} WHERE mlid = %d", $mlid));
      db_query("UPDATE {menu_links} SET hidden = %d, customized = 1 WHERE mlid = %d", $hidden ? 0 : 1, $mlid);
      rokupal_core_menu_cache_clear();
      $mname = db_result(db_query("SELECT menu_name FROM {menu_links} WHERE mlid = %d", $mlid));
      drupal_goto($mname ? 'admin/build/menu-customize/' . $mname : 'admin/build/menu');
    }

    if ($op === 'save_weights' && !empty($_POST['weights']) && is_array($_POST['weights'])) {
      foreach ($_POST['weights'] as $mlid => $w) {
        db_query("UPDATE {menu_links} SET weight = %d, customized = 1 WHERE mlid = %d", (int) $w, (int) $mlid);
      }
      rokupal_core_menu_cache_clear();
      drupal_set_message(t('Order saved.'));
      $mname = isset($_POST['menu_name']) ? preg_replace('/[^a-z0-9\-_]/', '', $_POST['menu_name']) : '';
      drupal_goto($mname ? 'admin/build/menu-customize/' . $mname : 'admin/build/menu');
    }
  }

  if ($menu_name) {
    return rokupal_core_menu_customize_page($menu_name, $token);
  }

  drupal_set_title(t('Menus'));
  $menus = function_exists('menu_get_menus') ? menu_get_menus() : array();
  if (empty($menus)) {
    $menus = array(
      'navigation' => 'Navigation',
      'primary-links' => 'Primary links',
      'secondary-links' => 'Secondary links',
    );
  }

  $out = '<div class="rp-menu-wrap"><p>' . t('Choose a menu to manage its links.') . '</p><div class="rp-menu-list">';
  foreach ($menus as $name => $title) {
    $count = (int) db_result(db_query("SELECT COUNT(*) FROM {menu_links} WHERE menu_name = '%s'", $name));
    $out .= '<article class="rp-menu-card"><h2>' . check_plain($title) . ' <code>' . check_plain($name) . '</code></h2>';
    $out .= '<p>' . format_plural($count, '1 link', '@count links') . '</p>';
    $out .= '<a class="rp-btn rp-btn-primary" href="' . check_url(url('admin/build/menu-customize/' . $name)) . '">' . t('Edit links') . '</a></article>';
  }
  $out .= '</div></div>';
  return $out;
}

function rokupal_core_menu_customize_page($menu_name, $token) {
  $menus = function_exists('menu_get_menus') ? menu_get_menus() : array();
  $title = isset($menus[$menu_name]) ? $menus[$menu_name] : $menu_name;
  drupal_set_title(t('Menu: @t', array('@t' => $title)));

  $result = db_query("SELECT mlid, link_title, link_path, weight, hidden, external FROM {menu_links} WHERE menu_name = '%s' ORDER BY weight ASC, link_title ASC", $menu_name);
  $rows = array();
  while ($r = db_fetch_object($result)) {
    $rows[] = $r;
  }

  $out = '<div class="rp-menu-wrap">';
  $out .= '<p><a href="' . check_url(url('admin/build/menu')) . '">← ' . t('All menus') . '</a></p>';

  $out .= '<section class="rp-menu-add"><h2>' . t('Add link') . '</h2>';
  $out .= '<form method="post" action="' . check_url(url('admin/build/menu-customize/' . $menu_name, array('absolute' => FALSE))) . '">';
  $out .= '<input type="hidden" name="rokupal_menu_op" value="add_link" />';
  $out .= '<input type="hidden" name="menu_name" value="' . check_plain($menu_name) . '" />';
  if ($token) {
    $out .= '<input type="hidden" name="token" value="' . check_plain($token) . '" />';
  }
  $out .= '<label>' . t('Title') . ' <input type="text" name="link_title" required="required" /></label> ';
  $out .= '<label>' . t('Path or URL') . ' <input type="text" name="link_path" placeholder="blog or https://..." required="required" size="40" /></label> ';
  $out .= '<label>' . t('Weight') . ' <input type="number" name="weight" value="0" class="rp-w" /></label> ';
  $out .= '<button type="submit" class="rp-btn rp-btn-primary">' . t('Add') . '</button></form></section>';

  $out .= '<section class="rp-menu-links"><h2>' . t('Links') . '</h2>';
  if (!$rows) {
    $out .= '<p>' . t('No links yet.') . '</p>';
  }
  else {
    $out .= '<form method="post" action="' . check_url(url('admin/build/menu-customize/' . $menu_name, array('absolute' => FALSE))) . '">';
    $out .= '<input type="hidden" name="rokupal_menu_op" value="save_weights" />';
    $out .= '<input type="hidden" name="menu_name" value="' . check_plain($menu_name) . '" />';
    if ($token) {
      $out .= '<input type="hidden" name="token" value="' . check_plain($token) . '" />';
    }
    $out .= '<table class="rp-menu-table"><thead><tr><th>' . t('Title') . '</th><th>' . t('Path') . '</th><th>' . t('Weight') . '</th><th></th></tr></thead><tbody>';
    foreach ($rows as $r) {
      $out .= '<tr' . ($r->hidden ? ' class="is-hidden"' : '') . '>';
      $out .= '<td>' . check_plain($r->link_title) . ($r->hidden ? ' <em>(' . t('hidden') . ')</em>' : '') . '</td>';
      $out .= '<td><code>' . check_plain($r->link_path) . '</code></td>';
      $out .= '<td><input type="number" name="weights[' . (int) $r->mlid . ']" value="' . (int) $r->weight . '" class="rp-w" /></td>';
      $out .= '<td>—</td></tr>';
    }
    $out .= '</tbody></table>';
    $out .= '<p><button type="submit" class="rp-btn rp-btn-primary">' . t('Save order') . '</button></p></form>';

    $out .= '<div class="rp-menu-row-actions">';
    foreach ($rows as $r) {
      $out .= '<div class="rp-menu-row-act"><strong>' . check_plain($r->link_title) . '</strong> ';
      $out .= '<form method="post" action="' . check_url(url('admin/build/menu-customize/' . $menu_name, array('absolute' => FALSE))) . '" class="inline">';
      $out .= '<input type="hidden" name="rokupal_menu_op" value="toggle_link" /><input type="hidden" name="mlid" value="' . (int) $r->mlid . '" />';
      if ($token) {
        $out .= '<input type="hidden" name="token" value="' . check_plain($token) . '" />';
      }
      $out .= '<button type="submit" class="rp-btn rp-btn-mute">' . ($r->hidden ? t('Show') : t('Hide')) . '</button></form> ';
      $out .= '<form method="post" action="' . check_url(url('admin/build/menu-customize/' . $menu_name, array('absolute' => FALSE))) . '" class="inline" onsubmit="return confirm(\'Delete this link?\');">';
      $out .= '<input type="hidden" name="rokupal_menu_op" value="delete_link" /><input type="hidden" name="mlid" value="' . (int) $r->mlid . '" />';
      if ($token) {
        $out .= '<input type="hidden" name="token" value="' . check_plain($token) . '" />';
      }
      $out .= '<button type="submit" class="rp-btn rp-btn-danger">' . t('Delete') . '</button></form></div>';
    }
    $out .= '</div>';
  }
  $out .= '</section></div>';
  return $out;
}

function rokupal_core_menu_cache_clear() {
  if (function_exists('menu_cache_clear_all')) {
    menu_cache_clear_all();
  }
  else {
    cache_clear_all('*', 'cache_menu', TRUE);
  }
  cache_clear_all();
}
