<?php
/**
 * @file
 * Stub locks for install only. Never redeclare if lock.inc already loaded.
 */

if (!function_exists('lock_init')) {
  function lock_init() {
  }
}

if (!function_exists('lock_acquire')) {
  function lock_acquire($name, $timeout = 30.0) {
    return TRUE;
  }
}

if (!function_exists('lock_may_be_available')) {
  function lock_may_be_available($name) {
    return TRUE;
  }
}

if (!function_exists('lock_wait')) {
  function lock_wait($name, $delay = 30) {
    return FALSE;
  }
}

if (!function_exists('lock_release')) {
  function lock_release($name) {
  }
}

if (!function_exists('lock_release_all')) {
  function lock_release_all($lock_id = NULL) {
  }
}
