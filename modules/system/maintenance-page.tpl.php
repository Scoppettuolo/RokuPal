<?php
/**
 * @file
 * Maintenance / offline page — HTML5, modernized.
 */
?>
<!DOCTYPE html>
<html lang="<?php print $language->language; ?>" dir="<?php print $language->dir; ?>">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title><?php print $head_title; ?></title>
  <?php print $head; ?>
  <?php print $styles; ?>
  <?php print $scripts; ?>
  <style>
    body {
      margin: 0; font-family: system-ui, -apple-system, "Segoe UI", Roboto, Arial, sans-serif;
      font-size: 16px; line-height: 1.6; color: #0f172a; background: #f1f5f9;
    }
    #page { max-width: 720px; margin: 3rem auto; padding: 0 1.25rem; }
    #header { text-align: center; margin-bottom: 2rem; }
    #logo img { max-height: 56px; }
    #site-name { font-size: 1.5rem; margin: 0.5rem 0 0; }
    #site-name a { color: #0f172a; text-decoration: none; font-weight: 700; }
    #site-slogan { color: #64748b; font-size: 0.95rem; }
    #content {
      background: #fff; border: 1px solid #e2e8f0; border-radius: 12px;
      padding: 1.75rem 2rem; box-shadow: 0 4px 16px rgba(15,23,42,0.06);
    }
    h1.title { margin-top: 0; font-size: 1.5rem; }
    #footer { text-align: center; margin-top: 2rem; color: #94a3b8; font-size: 0.875rem; }
  </style>
</head>
<body class="<?php print isset($body_classes) ? $body_classes : ''; ?>">
  <div id="page">
    <header id="header" role="banner">
      <?php if (!empty($logo)): ?>
        <a href="<?php print $base_path; ?>" title="<?php print t('Home'); ?>" rel="home" id="logo">
          <img src="<?php print $logo; ?>" alt="<?php print t('Home'); ?>" />
        </a>
      <?php endif; ?>
      <?php if (!empty($site_name)): ?>
        <h1 id="site-name">
          <a href="<?php print $base_path; ?>" title="<?php print t('Home'); ?>" rel="home"><?php print $site_name; ?></a>
        </h1>
      <?php endif; ?>
      <?php if (!empty($site_slogan)): ?>
        <div id="site-slogan"><?php print $site_slogan; ?></div>
      <?php endif; ?>
    </header>

    <main id="content" role="main">
      <?php if (!empty($title)): ?><h1 class="title" id="page-title"><?php print $title; ?></h1><?php endif; ?>
      <?php if (!empty($messages)): print $messages; endif; ?>
      <?php print $content; ?>
    </main>

    <footer id="footer" role="contentinfo">
      <?php if (!empty($footer_message)): print $footer_message; endif; ?>
      <?php if (!empty($footer)): print $footer; endif; ?>
    </footer>
  </div>
</body>
</html>
