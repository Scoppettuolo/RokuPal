<?php

/**
 * @file
 * RokuPal / Drupal 6: secure no-op for the old TYPO3 Phar stream wrapper.
 *
 * The original SA-CORE-2019-002 implementation required the entire
 * misc/typo3/ tree (phar-stream-wrapper + drupal-security + polyfill).
 * That tree has been intentionally removed from this fork and will not
 * be restored.
 *
 * Behaviour:
 * - If the native "phar" stream wrapper is registered, unregister it.
 *   This closes the original remote-code-execution vector on PHP < 8
 *   without needing the TYPO3 replacement classes.
 * - Never try to load missing files or instantiate TYPO3\* classes.
 * - Safe to call multiple times (bootstrap, tests, etc.).
 */

/**
 * Registers (or rather disables) the phar stream wrapper.
 *
 * Kept under the original function name so that bootstrap.inc continues
 * to work without further patches.
 */
function file_register_phar_wrapper() {
  // Already handled or never present → nothing to do.
  if (!in_array('phar', stream_get_wrappers(), TRUE)) {
    return;
  }

  // Remove the insecure built-in wrapper. We deliberately do NOT
  // register a replacement because the TYPO3 sources are gone.
  // Sites that truly need phar:// support on modern PHP can re-register
  // it themselves via hook_stream_wrappers() or a custom include.
  @stream_wrapper_unregister('phar');
}
