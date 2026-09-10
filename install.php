<?php
/**
 * RokuPal install-simple — single form installer (proven path for this fork).
 */
error_reporting(E_ALL);
ini_set('display_errors', 1);

if (!is_file('./includes/bootstrap.inc')) {
  print 'Missing includes/bootstrap.inc';
  exit;
}

require_once './includes/bootstrap.inc';
require_once './includes/install.rokupal.inc';

function rokupal_install_page_shell($inner) {
  header('Content-Type: text/html; charset=utf-8');
  echo '<!DOCTYPE html><html><head><meta charset="utf-8"/><title>RokuPal install</title>';
  echo '<style>
body{font-family:system-ui,sans-serif;max-width:42rem;margin:2rem auto;padding:0 1rem;background:#eef1f5;color:#1a1d23}
.card{background:#fff;padding:1.5rem;border-radius:8px;box-shadow:0 1px 4px rgba(0,0,0,.08)}
h1{margin-top:0} label{display:block;margin:.75rem 0 .25rem;font-weight:600}
input[type=text],input[type=password]{width:100%;max-width:100%;padding:.45rem;border:1px solid #bbb;border-radius:4px;box-sizing:border-box}
button,.btn{margin-top:1rem;padding:.5rem 1.2rem;background:#3b5675;color:#fff;border:0;border-radius:4px;font-weight:600;cursor:pointer;text-decoration:none;display:inline-block}
.ok{background:#d4edda;padding:.75rem;border-radius:4px;margin:.5rem 0}
.err{background:#f8d7da;padding:.75rem;border-radius:4px;margin:.5rem 0}
.muted{color:#555;font-size:.9rem}
ul.log{font-size:.85rem;max-height:12rem;overflow:auto;background:#f7f9fc;padding:0.75rem 1.25rem}
</style></head><body><div class="card">';
  echo $inner;
  echo '</div></body></html>';
}

if (empty($_POST['op'])) {
  $inner = '<h1>RokuPal — instalación</h1>';
  $inner .= '<p class="muted">Instalador del fork (perfil <strong>RokuPal</strong> único). Blog se activa siempre.</p>';
  $inner .= '<form method="post">';
  $inner .= '<label>Base de datos</label>';
  $inner .= '<input type="text" name="db_url" value="sqlite://sites/default/files/.ht.sqlite" />';
  $inner .= '<p class="muted">sqlite://sites/default/files/.ht.sqlite<br>mysqli://user:pass@localhost/rokupal</p>';
  $inner .= '<label>Usuario admin</label><input type="text" name="admin_name" value="admin" />';
  $inner .= '<label>Email admin</label><input type="text" name="admin_mail" value="admin@example.com" />';
  $inner .= '<label>Contraseña admin</label><input type="password" name="admin_pass" value="" required />';
  $inner .= '<label><input type="checkbox" name="enable_forum" value="1" /> Activar Forum (opcional)</label>';
  $inner .= '<label><input type="checkbox" name="enable_poll" value="1" /> Activar Poll (opcional)</label>';
  $inner .= '<button type="submit" name="op" value="install">Instalar RokuPal</button>';
  $inner .= '</form>';
  rokupal_install_page_shell($inner);
  exit;
}

try {
  $msgs = install_run_simple('rokupal', 'en', array(
    'db_url' => trim($_POST['db_url']),
    'db_driver' => isset($_POST['db_driver']) ? trim($_POST['db_driver']) : 'sqlite',
    'admin_name' => trim($_POST['admin_name']),
    'admin_mail' => trim($_POST['admin_mail']),
    'admin_pass' => $_POST['admin_pass'],
    'enable_forum' => !empty($_POST['enable_forum']),
    'enable_poll' => !empty($_POST['enable_poll']),
  ));
  $inner = '<h1>Instalación completa</h1><div class="ok">RokuPal listo. Sesión de admin iniciada.</div>';
  $inner .= '<ul class="log"><li>' . implode('</li><li>', array_map('check_plain', $msgs)) . '</li></ul>';
  $base = rtrim(str_replace('\\', '/', dirname($_SERVER['SCRIPT_NAME'])), '/');
  if ($base === '' || $base === '/') {
    $home = '/';
  }
  else {
    $home = $base . '/';
  }
  $inner .= '<p><a class="btn" href="' . htmlspecialchars($home) . '">Ir al sitio</a> ';
  $inner .= '<a class="btn" href="' . htmlspecialchars($home) . '?q=blog">Ir al blog</a></p>';
  rokupal_install_page_shell($inner);
}
catch (Exception $e) {
  rokupal_install_page_shell('<h1>Error</h1><div class="err">' . htmlspecialchars($e->getMessage()) . '</div><p><a href="install.php">Volver</a></p>');
}
