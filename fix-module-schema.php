<?php
/**
 * One-shot repair: module status display, schemas, filter formats, stale locks.
 * DELETE after running.
 */
chdir(dirname(__FILE__));
require_once './includes/bootstrap.inc';
drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
header('Content-Type: text/plain; charset=utf-8');

echo "=== Module status/schema ===\n";
$result = db_query("SELECT name, status, schema_version, filename FROM {system} WHERE type = 'module' ORDER BY name");
while ($row = db_fetch_object($result)) {
  $line = "{$row->name}: status={$row->status} schema={$row->schema_version}";
  if ((int) $row->status === 1 && (int) $row->schema_version < 0) {
    db_query("UPDATE {system} SET schema_version = 0 WHERE type = 'module' AND name = '%s'", $row->name);
    $line .= " -> schema fixed to 0";
  }
  // Ensure filename exists on disk path style
  echo $line . "\n";
}

echo "\n=== Filter formats roles ===\n";
if (db_table_exists('filter_formats')) {
  $roles = ',' . DRUPAL_ANONYMOUS_RID . ',' . DRUPAL_AUTHENTICATED_RID . ',';
  db_query("UPDATE {filter_formats} SET roles = '%s'", $roles);
  $r = db_query("SELECT format, name, roles FROM {filter_formats}");
  while ($f = db_fetch_object($r)) {
    echo "format {$f->format} {$f->name} roles={$f->roles}\n";
  }
}

if (function_exists('rokupal_markdown_ensure_format')) {
  echo "Markdown format=" . rokupal_markdown_ensure_format(TRUE) . "\n";
}
elseif (module_exists('rokupal_markdown')) {
  module_load_include('module', 'rokupal_markdown', 'rokupal_markdown');
  if (function_exists('rokupal_markdown_ensure_format')) {
    echo "Markdown format=" . rokupal_markdown_ensure_format(TRUE) . "\n";
  }
}

echo "\n=== Clear stale locks ===\n";
if (db_table_exists('semaphore')) {
  db_query("DELETE FROM {semaphore}");
  echo "semaphore table cleared\n";
}

cache_clear_all();
if (function_exists('module_list')) {
  module_list(TRUE, FALSE);
}
if (function_exists('module_rebuild_cache')) {
  // light rebuild
  module_rebuild_cache();
}
echo "\nDone. DELETE fix-module-schema.php\n";
echo "Then open admin/build/modules — enabled modules should show checked.\n";
