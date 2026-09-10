<?php

/**
 * @file
 * User session handling functions.
 */

function sess_open($save_path, $session_name) {
  return TRUE;
}

function sess_close() {
  return TRUE;
}

/**
 * Reads an entire session from the database (internal use only).
 *
 * Also initializes the $user object for the user associated with the session.
 * This function is registered with session_set_save_handler() to support
 * database-backed sessions. It is called on every page load when PHP sets
 * up the $_SESSION superglobal.
 *
 * This function is an internal function and must not be called directly.
 * Doing so may result in logging out the current user, corrupting session data
 * or other unexpected behavior. Session data must always be accessed via the
 * $_SESSION superglobal.
 *
 * @param $key
 *   The session ID of the session to retrieve.
 *
 * @return
 *   The user's session, or an empty string if no session exists.
 */
function sess_read($key) {
  global $user;
  static $roles_cache = array();

  // Write/Close run after object destruction since PHP 5.0.5 — close early.
  register_shutdown_function('session_write_close');

  $key = (string) $key;
  // First-time visitors and clients without cookies (crawlers).
  if ($key === '' || !isset($_COOKIE[session_name()])) {
    $user = drupal_anonymous_user();
    return '';
  }

  // Active session row joined with users. Keep a digest of the serialized
  // payload so unchanged requests can avoid taking a database write lock.
  $user = db_fetch_object(db_query("SELECT u.*, s.* FROM {users} u INNER JOIN {sessions} s ON u.uid = s.uid WHERE s.sid = '%s'", $key));
  if (is_object($user) && isset($user->session)) {
    $user->_drupal_session_hash = sha1((string) $user->session);
  }

  // Authenticated and active account.
  if (is_object($user) && isset($user->uid) && $user->uid > 0 && isset($user->status) && $user->status == 1) {
    // This is done to unserialize the data member of $user
    $user = drupal_unpack($user);

    // Add roles element to $user. Keep the result static for the request so
    // repeated bootstrap reads do not issue the same role query again.
    if (isset($roles_cache[$user->uid])) {
      $user->roles = $roles_cache[$user->uid];
    }
    else {
      $user->roles = array();
      $user->roles[DRUPAL_AUTHENTICATED_RID] = 'authenticated user';
      $result = db_query("SELECT r.rid, r.name FROM {role} r INNER JOIN {users_roles} ur ON ur.rid = r.rid WHERE ur.uid = %d", $user->uid);
      while ($role = db_fetch_object($result)) {
        $user->roles[$role->rid] = $role->name;
      }
      $roles_cache[$user->uid] = $user->roles;
    }
  }
  // We didn't find the client's record (session has expired), or they are
  // blocked, or they are an anonymous user.
  else {
    $session = isset($user->session) ? $user->session : '';
    $user = drupal_anonymous_user($session);
  }

  return $user->session;
}

/**
 * Writes an entire session to the database (internal use only).
 *
 * This function is registered with session_set_save_handler() to support
 * database-backed sessions.
 *
 * This function is an internal function and must not be called directly.
 * Doing so may result in corrupted session data or other unexpected behavior.
 * Session data must always be accessed via the $_SESSION superglobal.
 *
 * @param $key
 *   The session ID of the session to write to.
 * @param $value
 *   Session data to write as a serialized string.
 *
 * @return
 *   Always returns TRUE.
 */
function sess_write($key, $value) {
  global $user;
  static $last_write = array();

  // If saving of session data is disabled or if the client doesn't have a session,
  // and one isn't being created ($value), do nothing. This keeps crawlers out of
  // the session table. This reduces memory and server load, and gives more useful
  // statistics. We can't eliminate anonymous session table rows without breaking
  // the throttle module and the "Who's Online" block.
  if (!session_save_session() || ($user->uid == 0 && empty($_COOKIE[session_name()]) && empty($value))) {
    return TRUE;
  }

  $uid = isset($user->uid) ? (int) $user->uid : 0;
  $cache = isset($user->cache) ? $user->cache : 0;
  $session_hash = sha1((string) $value);
  $session_interval = max(30, (int) variable_get('session_write_interval', 180));

  // Avoid a database write lock when the serialized payload is unchanged and
  // the session was touched recently. Changed payloads are always persisted.
  if (isset($user->_drupal_session_hash) && $user->_drupal_session_hash === $session_hash && isset($user->timestamp) && time() - (int) $user->timestamp < $session_interval) {
    return TRUE;
  }

  // Skip identical session writes within the same request lifecycle patterns.
  // Compare against what we last successfully wrote for this sid.
  $sig = $uid . ':' . $cache . ':' . $value;
  if (isset($last_write[$key]) && $last_write[$key] === $sig) {
    return TRUE;
  }

  db_query("UPDATE {sessions} SET uid = %d, cache = %d, hostname = '%s', session = '%s', timestamp = %d WHERE sid = '%s'", $uid, $cache, ip_address(), $value, time(), $key);
  if (db_affected_rows()) {
    $last_write[$key] = $sig;
    if (isset($user->_drupal_session_hash)) {
      $user->_drupal_session_hash = $session_hash;
    }
    // Last access time is updated no more frequently than once every 180 seconds.
    // This reduces contention in the users table.
    if ($uid && (!isset($user->access) || time() - (int) $user->access > $session_interval)) {
      $now = time();
      db_query("UPDATE {users} SET access = %d WHERE uid = %d", $now, $uid);
      $user->access = $now;
    }
  }
  else {
    // If this query fails, another parallel request probably got here first.
    // In that case, any session data generated in this request is discarded.
    @db_query("INSERT INTO {sessions} (sid, uid, cache, hostname, session, timestamp) VALUES ('%s', %d, %d, '%s', '%s', %d)", $key, $uid, $cache, ip_address(), $value, time());
    $last_write[$key] = $sig;
  }

  return TRUE;
}

/**
 * Called when an anonymous user becomes authenticated or vice-versa.
 */
function sess_regenerate() {
  global $user;
  $old_session_id = session_id();

  // We code around http://bugs.php.net/bug.php?id=32802 by destroying
  // the session cookie by setting expiration in the past (a negative
  // value).  This issue only arises in PHP versions before 4.4.0,
  // regardless of the Drupal configuration.
  // TODO: remove this when we require at least PHP 4.4.0
  if (isset($_COOKIE[session_name()])) {
    setcookie(session_name(), '', time() - 42000, '/');
  }

  // On PHP7, sess_read($key) is being called with the new session key but not
  // on PHP5. This failsafe stores the initial user object, which on PHP5 should
  // be just the same, and restore it later on.
  $account = $user;
  session_regenerate_id();
  $user = $account;

  db_query("UPDATE {sessions} SET sid = '%s' WHERE sid = '%s'", session_id(), $old_session_id);
}

/**
 * Counts how many users have sessions. Can count either anonymous sessions or authenticated sessions.
 *
 * @param int $timestamp
 *   A Unix timestamp representing a point of time in the past.
 *   The default is 0, which counts all existing sessions.
 * @param boolean $anonymous
 *   TRUE counts only anonymous users.
 *   FALSE counts only authenticated users.
 * @return  int
 *   The number of users with sessions.
 */
function sess_count($timestamp = 0, $anonymous = true) {
  $query = $anonymous ? ' AND uid = 0' : ' AND uid > 0';
  return db_result(db_query('SELECT COUNT(sid) AS count FROM {sessions} WHERE timestamp >= %d'. $query, $timestamp));
}

/**
 * Called by PHP session handling with the PHP session ID to end a user's session.
 *
 * @param  string $sid
 *   the session id
 */
function sess_destroy_sid($sid) {
  db_query("DELETE FROM {sessions} WHERE sid = '%s'", $sid);
  return TRUE;
}

/**
 * End a specific user's session
 *
 * @param  string $uid
 *   the user id
 */
function sess_destroy_uid($uid) {
  db_query('DELETE FROM {sessions} WHERE uid = %d', $uid);
  return TRUE;
}

function sess_gc($lifetime) {
  // Be sure to adjust 'php_value session.gc_maxlifetime' to a large enough
  // value. For example, if you want user sessions to stay in your database
  // for three weeks before deleting them, you need to set gc_maxlifetime
  // to '1814400'. At that value, only after a user doesn't log in after
  // three weeks (1814400 seconds) will his/her session be removed.
  db_query("DELETE FROM {sessions} WHERE timestamp < %d", time() - $lifetime);

  return TRUE;
}

/**
 * Determine whether to save session data of the current request.
 *
 * This function allows the caller to temporarily disable writing of session data,
 * should the request end while performing potentially dangerous operations, such as
 * manipulating the global $user object.  See http://drupal.org/node/218104 for usage
 *
 * @param $status
 *   Disables writing of session data when FALSE, (re-)enables writing when TRUE.
 * @return
 *   FALSE if writing session data has been disabled. Otherwise, TRUE.
 */
function session_save_session($status = NULL) {
  static $save_session = TRUE;
  if (isset($status)) {
    $save_session = $status;
  }
  return ($save_session);
}
