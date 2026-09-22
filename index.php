<?php
/**
 * @file
 * RokuPal front controller.
 */

# Show errors until the site is stable (XAMPP / shared hosting diagnosis).
# Comment these three lines in production if you prefer logs only.
error_reporting(E_ALL);
ini_set('display_errors', '1');
ini_set('display_startup_errors', '1');

require_once './includes/bootstrap.inc';
drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);

$return = menu_execute_active_handler();

if (is_int($return)) {
  switch ($return) {
    case MENU_NOT_FOUND:
      drupal_not_found();
      break;
    case MENU_ACCESS_DENIED:
      drupal_access_denied();
      break;
    case MENU_SITE_OFFLINE:
      drupal_site_offline();
      break;
  }
}
elseif (isset($return)) {
  print theme('page', $return);
}
else {
  drupal_not_found();
}

drupal_page_footer();
