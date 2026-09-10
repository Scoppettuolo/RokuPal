<?php

/**
 * @file
 * Handles incoming requests to fire off regularly-scheduled tasks (cron jobs).
 *
 * Modernized lightly for PHP 8+ environments:
 * - Clearer bootstrap
 * - Optional silent mode via ?quiet=1
 * - No extra dependencies (keeps Drupal 6 light)
 */

include_once './includes/bootstrap.inc';
drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);

// Allow quiet runs (useful for external schedulers / Task Scheduler / cron)
$quiet = !empty($_GET['quiet']);

if (!$quiet) {
  // Minimal feedback when hit from a browser
  header('Content-Type: text/plain; charset=utf-8');
}

// Run all module cron hooks
drupal_cron_run();

if (!$quiet) {
  print "Cron ran successfully at " . date('c') . "\n";
}
