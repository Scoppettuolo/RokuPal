<?php
/**
 * @file
 * RokuPal 1.0.0 front controller.
 */
/**
 * @file
 * The PHP page that serves all page requests on a RokuPal installation.
 *
 * The routines here dispatch control to the appropriate handler, which then
 * prints the appropriate page.
 *
 * All Drupal code is released under the GNU General Public License.
 * See COPYRIGHT.txt and LICENSE.txt.
 */

require_once './includes/bootstrap.inc';
// Surface fatals during development of this modernized fork (disable in production).
if (!ini_get('display_errors')) {
  // Keep server default; watchdog still records PHP errors.
}
drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);

$return = menu_execute_active_handler();

// Menu status constants are integers; page content is a string.
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
  // Print any value (including an empty string) except NULL or undefined.
  print theme('page', $return);
}
else {
  // A page callback must not leave a successful but empty HTTP response.
  // This protects against incomplete legacy callbacks and prevents blank pages.
  drupal_not_found();
}

drupal_page_footer();
