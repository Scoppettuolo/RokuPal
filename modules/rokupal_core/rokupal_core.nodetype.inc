<?php
/**
 * @file
 * RokuPal content-type settings — full options (workflow + comments + upload).
 */

function rokupal_core_nodetype_page($type_obj = NULL) {
  if (is_object($type_obj) && !empty($type_obj->type)) {
    $type = $type_obj->type;
  }
  elseif (is_string($type_obj) && $type_obj !== '') {
    $type = str_replace('-', '_', $type_obj);
  }
  else {
    $type = str_replace('-', '_', arg(3));
  }
  $info = node_get_types('type', $type);
  if (!$info) {
    return drupal_not_found();
  }

  drupal_set_title(t('Edit @name', array('@name' => $info->name)));
  drupal_add_css(drupal_get_path('module', 'rokupal_core') . '/css/nodetype-admin.css');
  $token = function_exists('drupal_get_token') ? drupal_get_token('rokupal_nodetype') : '';
  $url_type = str_replace('_', '-', $type);

  if ($_SERVER['REQUEST_METHOD'] === 'POST' && !empty($_POST['rokupal_nt_save'])) {
    if (!empty($_POST['token']) && function_exists('drupal_valid_token') && !drupal_valid_token($_POST['token'], 'rokupal_nodetype')) {
      drupal_set_message(t('Invalid token'), 'error');
      drupal_goto('admin/content/node-type/' . $url_type);
    }

    $name = trim(isset($_POST['name']) ? $_POST['name'] : '');
    $info->name = $name !== '' ? $name : $info->name;
    $info->description = isset($_POST['description']) ? $_POST['description'] : '';
    $info->help = isset($_POST['help']) ? $_POST['help'] : '';
    $info->title_label = isset($_POST['title_label']) ? trim($_POST['title_label']) : 'Title';
    $info->body_label = isset($_POST['body_label']) ? trim($_POST['body_label']) : 'Body';
    $info->has_title = ($info->title_label !== '');
    $info->has_body = ($info->body_label !== '');
    $info->min_word_count = isset($_POST['min_word_count']) ? (int) $_POST['min_word_count'] : 0;
    $info->modified = 1;
    node_type_save($info);

    // Workflow
    $opts = array();
    foreach (array('status', 'promote', 'sticky', 'revision') as $o) {
      if (!empty($_POST['opt_' . $o])) {
        $opts[] = $o;
      }
    }
    variable_set('node_options_' . $type, $opts);

    // Comments (full set)
    if (module_exists('comment')) {
      if (isset($_POST['comment'])) {
        variable_set('comment_' . $type, (int) $_POST['comment']);
      }
      if (isset($_POST['comment_default_mode'])) {
        variable_set('comment_default_mode_' . $type, (int) $_POST['comment_default_mode']);
      }
      if (isset($_POST['comment_default_order'])) {
        variable_set('comment_default_order_' . $type, (int) $_POST['comment_default_order']);
      }
      if (isset($_POST['comment_default_per_page'])) {
        variable_set('comment_default_per_page_' . $type, max(1, (int) $_POST['comment_default_per_page']));
      }
      if (isset($_POST['comment_controls'])) {
        variable_set('comment_controls_' . $type, (int) $_POST['comment_controls']);
      }
      if (isset($_POST['comment_anonymous'])) {
        variable_set('comment_anonymous_' . $type, (int) $_POST['comment_anonymous']);
      }
      if (isset($_POST['comment_subject_field'])) {
        variable_set('comment_subject_field_' . $type, (int) $_POST['comment_subject_field']);
      }
      if (isset($_POST['comment_preview'])) {
        variable_set('comment_preview_' . $type, (int) $_POST['comment_preview']);
      }
      if (isset($_POST['comment_form_location'])) {
        variable_set('comment_form_location_' . $type, (int) $_POST['comment_form_location']);
      }
    }

    // Upload attachments
    if (module_exists('upload') && isset($_POST['upload'])) {
      variable_set('upload_' . $type, (int) $_POST['upload']);
    }

    node_types_rebuild();
    cache_clear_all();
    drupal_set_message(t('Content type %n saved.', array('%n' => $info->name)));
    drupal_goto('admin/content/node-type/' . $url_type);
  }

  // --- Load current values ---
  $opts = variable_get('node_options_' . $type, array('status', 'promote'));
  if (!is_array($opts)) {
    $opts = array();
  }
  $opt = array_flip($opts);

  $out = '<div class="rp-nt">';
  $out .= '<div class="rp-nt-header"><h2>' . check_plain($info->name) . '</h2><p>' . t('Machine name: @t', array('@t' => $type)) . '</p></div>';
  $out .= '<form method="post" action="' . check_url(url('admin/content/node-type/' . $url_type)) . '">';
  $out .= '<input type="hidden" name="rokupal_nt_save" value="1" />';
  if ($token) {
    $out .= '<input type="hidden" name="token" value="' . check_plain($token) . '" />';
  }

  // Identification
  $out .= '<fieldset><legend>' . t('Identification') . '</legend>';
  $out .= '<p><label>' . t('Name') . '<br /><input type="text" name="name" value="' . check_plain($info->name) . '" size="40" required="required" /></label></p>';
  $out .= '<p><label>' . t('Type (machine name)') . '<br /><code>' . check_plain($type) . '</code></label></p>';
  $out .= '<p><label>' . t('Description') . '<br /><textarea name="description" rows="3" cols="60">' . check_plain($info->description) . '</textarea></label></p>';
  $out .= '</fieldset>';

  // Submission
  $out .= '<fieldset><legend>' . t('Submission form settings') . '</legend>';
  $out .= '<p><label>' . t('Title field label') . '<br /><input type="text" name="title_label" value="' . check_plain($info->title_label) . '" /></label></p>';
  $out .= '<p><label>' . t('Body field label') . '<br /><input type="text" name="body_label" value="' . check_plain($info->body_label) . '" /></label></p>';
  $out .= '<p><label>' . t('Minimum number of words') . '<br /><input type="number" name="min_word_count" value="' . (int) $info->min_word_count . '" min="0" /></label></p>';
  $out .= '<p><label>' . t('Explanation or submission guidelines') . '<br /><textarea name="help" rows="3" cols="60">' . check_plain($info->help) . '</textarea></label></p>';
  $out .= '</fieldset>';

  // Workflow
  $out .= '<fieldset><legend>' . t('Workflow settings') . '</legend>';
  $out .= '<p>' . t('Default options for new content of this type:') . '</p>';
  $out .= '<p class="rp-check"><label><input type="checkbox" name="opt_status" value="1"' . (isset($opt['status']) ? ' checked="checked"' : '') . ' /> ' . t('Published') . '</label></p>';
  $out .= '<p class="rp-check"><label><input type="checkbox" name="opt_promote" value="1"' . (isset($opt['promote']) ? ' checked="checked"' : '') . ' /> ' . t('Promoted to front page') . '</label></p>';
  $out .= '<p class="rp-check"><label><input type="checkbox" name="opt_sticky" value="1"' . (isset($opt['sticky']) ? ' checked="checked"' : '') . ' /> ' . t('Sticky at top of lists') . '</label></p>';
  $out .= '<p class="rp-check"><label><input type="checkbox" name="opt_revision" value="1"' . (isset($opt['revision']) ? ' checked="checked"' : '') . ' /> ' . t('Create new revision') . '</label></p>';
  $out .= '</fieldset>';

  // Comments — full original set
  if (module_exists('comment')) {
    $out .= '<fieldset><legend>' . t('Comment settings') . '</legend>';

    $c = (int) variable_get('comment_' . $type, defined('COMMENT_NODE_READ_WRITE') ? COMMENT_NODE_READ_WRITE : 2);
    $out .= '<p><label><strong>' . t('Default comment setting') . '</strong><br /><select name="comment">';
    $out .= rokupal_core_nt_opts(array(0 => t('Disabled'), 1 => t('Read only'), 2 => t('Read/Write')), $c);
    $out .= '</select></label></p>';

    $mode = (int) variable_get('comment_default_mode_' . $type, defined('COMMENT_MODE_THREADED_EXPANDED') ? COMMENT_MODE_THREADED_EXPANDED : 4);
    $modes = function_exists('_comment_get_modes') ? _comment_get_modes() : array(
      1 => t('Flat list - collapsed'),
      2 => t('Flat list - expanded'),
      3 => t('Threaded list - collapsed'),
      4 => t('Threaded list - expanded'),
    );
    $out .= '<p><label><strong>' . t('Default display mode') . '</strong><br /><select name="comment_default_mode">';
    $out .= rokupal_core_nt_opts($modes, $mode);
    $out .= '</select></label></p>';

    $order = (int) variable_get('comment_default_order_' . $type, defined('COMMENT_ORDER_NEWEST_FIRST') ? COMMENT_ORDER_NEWEST_FIRST : 1);
    $orders = function_exists('_comment_get_orders') ? _comment_get_orders() : array(
      1 => t('Date - newest first'),
      2 => t('Date - oldest first'),
    );
    $out .= '<p><label><strong>' . t('Default display order') . '</strong><br /><select name="comment_default_order">';
    $out .= rokupal_core_nt_opts($orders, $order);
    $out .= '</select></label></p>';

    $per = (int) variable_get('comment_default_per_page_' . $type, 50);
    $per_opts = function_exists('_comment_per_page') ? _comment_per_page() : array(10 => 10, 30 => 30, 50 => 50, 70 => 70, 90 => 90, 150 => 150, 200 => 200, 250 => 250, 300 => 300);
    $out .= '<p><label><strong>' . t('Default comments per page') . '</strong><br /><select name="comment_default_per_page">';
    $out .= rokupal_core_nt_opts($per_opts, $per);
    $out .= '</select></label></p>';

    $controls = (int) variable_get('comment_controls_' . $type, defined('COMMENT_CONTROLS_HIDDEN') ? COMMENT_CONTROLS_HIDDEN : 3);
    $out .= '<p><label><strong>' . t('Comment controls') . '</strong><br /><select name="comment_controls">';
    $out .= rokupal_core_nt_opts(array(
      0 => t('Display above the comments'),
      1 => t('Display below the comments'),
      2 => t('Display above and below the comments'),
      3 => t('Do not display'),
    ), $controls);
    $out .= '</select></label></p>';

    $anon = (int) variable_get('comment_anonymous_' . $type, defined('COMMENT_ANONYMOUS_MAYNOT_CONTACT') ? COMMENT_ANONYMOUS_MAYNOT_CONTACT : 0);
    $out .= '<p><label><strong>' . t('Anonymous commenting') . '</strong><br /><select name="comment_anonymous">';
    $out .= rokupal_core_nt_opts(array(
      0 => t('Anonymous posters may not enter their contact information'),
      1 => t('Anonymous posters may leave their contact information'),
      2 => t('Anonymous posters must leave their contact information'),
    ), $anon);
    $out .= '</select></label></p>';

    $subj = (int) variable_get('comment_subject_field_' . $type, 1);
    $out .= '<p><label><strong>' . t('Comment subject field') . '</strong><br /><select name="comment_subject_field">';
    $out .= rokupal_core_nt_opts(array(0 => t('Disabled'), 1 => t('Enabled')), $subj);
    $out .= '</select></label></p>';

    $prev = (int) variable_get('comment_preview_' . $type, defined('COMMENT_PREVIEW_REQUIRED') ? COMMENT_PREVIEW_REQUIRED : 1);
    $out .= '<p><label><strong>' . t('Preview comment') . '</strong><br /><select name="comment_preview">';
    $out .= rokupal_core_nt_opts(array(0 => t('Optional'), 1 => t('Required')), $prev);
    $out .= '</select></label></p>';

    $loc = (int) variable_get('comment_form_location_' . $type, defined('COMMENT_FORM_SEPARATE_PAGE') ? COMMENT_FORM_SEPARATE_PAGE : 0);
    $out .= '<p><label><strong>' . t('Location of comment submission form') . '</strong><br /><select name="comment_form_location">';
    $out .= rokupal_core_nt_opts(array(
      0 => t('Display on separate page'),
      1 => t('Display below post or comments'),
    ), $loc);
    $out .= '</select></label></p>';

    $out .= '</fieldset>';
  }

  // File attachments
  if (module_exists('upload')) {
    $up = (int) variable_get('upload_' . $type, 1);
    $out .= '<fieldset><legend>' . t('File attachments') . '</legend>';
    $out .= '<p><label><strong>' . t('Attachments') . '</strong><br /><select name="upload">';
    $out .= rokupal_core_nt_opts(array(0 => t('Disabled'), 1 => t('Enabled')), $up);
    $out .= '</select></label></p>';
    $out .= '</fieldset>';
  }

  $out .= '<div class="rp-nt-actions"><button type="submit">' . t('Save content type') . '</button></div>';
  $out .= '</form></div>';
  return $out;
}

function rokupal_core_nt_opts($options, $selected) {
  $html = '';
  foreach ($options as $k => $label) {
    $sel = ((string) $k === (string) $selected) ? ' selected="selected"' : '';
    $html .= '<option value="' . check_plain($k) . '"' . $sel . '>' . check_plain($label) . '</option>';
  }
  return $html;
}
