<?php
/**
 * Lightweight sitemap.xml for RokuPal (blog posts + pages + forum topics).
 * Cache file: sites/default/files/sitemap-cache.xml (1 hour).
 */
define('DRUPAL_ROOT', getcwd());
require_once './includes/bootstrap.inc';
drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);

$cache_dir = conf_path() . '/files';
$cache_file = $cache_dir . '/sitemap-cache.xml';
$ttl = 3600;

if (is_file($cache_file) && (time() - filemtime($cache_file)) < $ttl) {
  header('Content-Type: application/xml; charset=utf-8');
  header('X-RokuPal-Sitemap: cache');
  readfile($cache_file);
  exit;
}

$base = url('', array('absolute' => TRUE));
$urls = array();
$urls[] = array('loc' => $base, 'priority' => '1.0');

if (module_exists('rokupal_blog')) {
  $urls[] = array('loc' => url('blog', array('absolute' => TRUE)), 'priority' => '0.9');
}
if (module_exists('rokupal_forum')) {
  $urls[] = array('loc' => url('forum', array('absolute' => TRUE)), 'priority' => '0.8');
}

$result = db_query_range("SELECT n.nid, n.changed FROM {node} n WHERE n.status = 1 AND n.type IN ('blog_post', 'page', 'forum_topic') ORDER BY n.changed DESC", 0, 500);
while ($row = db_fetch_object($result)) {
  $urls[] = array(
    'loc' => url('node/' . $row->nid, array('absolute' => TRUE)),
    'lastmod' => gmdate('Y-m-d', (int) $row->changed),
    'priority' => '0.6',
  );
}

$xml = '<?xml version="1.0" encoding="UTF-8"?>' . "\n";
$xml .= '<urlset xmlns="http://www.sitemaps.org/schemas/sitemap/0.9">' . "\n";
foreach ($urls as $u) {
  $xml .= "  <url>\n    <loc>" . htmlspecialchars($u['loc'], ENT_XML1, 'UTF-8') . "</loc>\n";
  if (!empty($u['lastmod'])) {
    $xml .= "    <lastmod>" . $u['lastmod'] . "</lastmod>\n";
  }
  if (!empty($u['priority'])) {
    $xml .= "    <priority>" . $u['priority'] . "</priority>\n";
  }
  $xml .= "  </url>\n";
}
$xml .= '</urlset>';

if (is_dir($cache_dir) && is_writable($cache_dir)) {
  @file_put_contents($cache_file, $xml);
}

header('Content-Type: application/xml; charset=utf-8');
header('X-RokuPal-Sitemap: generated');
echo $xml;
