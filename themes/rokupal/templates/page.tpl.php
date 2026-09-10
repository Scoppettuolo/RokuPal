<?php
/**
 * RokuPal page — logo always rendered.
 */
?><!DOCTYPE html>
<html lang="<?php print isset($language->language) ? $language->language : 'en'; ?>">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title><?php print $head_title; ?></title>
  <?php print $head; ?>
  <?php print $styles; ?>
  <?php print $scripts; ?>
</head>
<body class="<?php print !empty($body_classes) ? $body_classes : 'rokupal'; ?>">
  <div class="rp-page">
    <header class="rp-header" role="banner">
      <div class="rp-header-inner">
        <div class="rp-brand">
          <a class="rp-logo" href="<?php print $front_page; ?>" rel="home" title="RokuPal">
            <?php if (!empty($logo)): ?>
              <img src="<?php print $logo; ?>" alt="RokuPal" width="200" height="56" />
            <?php else: ?>
              <span class="rp-logo-fallback" aria-hidden="true">
                <svg xmlns="http://www.w3.org/2000/svg" width="200" height="56" viewBox="0 0 200 56">
                  <rect width="200" height="56" rx="6" fill="#1a1a2e"/>
                  <rect width="8" height="56" fill="#3b5675"/>
                  <text x="20" y="36" font-family="system-ui,sans-serif" font-size="24" font-weight="700" fill="#7ec8ff">RokuPal</text>
                </svg>
              </span>
            <?php endif; ?>
          </a>
          <?php if (!empty($site_name)): ?>
            <a class="rp-site-name" href="<?php print $front_page; ?>" rel="home"><?php print $site_name; ?></a>
          <?php endif; ?>
          <?php if (!empty($site_slogan)): ?>
            <span class="rp-slogan"><?php print $site_slogan; ?></span>
          <?php endif; ?>
        </div>
        <?php if (!empty($primary_links) || !empty($secondary_links)): ?>
          <nav class="rp-nav" role="navigation">
            <?php if (!empty($primary_links)) print theme('links', $primary_links, array('class' => 'rp-links rp-primary')); ?>
            <?php if (!empty($secondary_links)) print theme('links', $secondary_links, array('class' => 'rp-links rp-secondary')); ?>
          </nav>
        <?php endif; ?>
        <?php if (!empty($header)): ?><div class="rp-header-region"><?php print $header; ?></div><?php endif; ?>
      </div>
    </header>

    <?php if (!empty($breadcrumb)): ?><div class="rp-breadcrumb"><?php print $breadcrumb; ?></div><?php endif; ?>
    <?php if (!empty($highlighted)): ?><div class="rp-highlighted"><?php print $highlighted; ?></div><?php endif; ?>

    <div class="rp-main<?php if (!empty($has_sidebar_first)) print ' has-first'; ?><?php if (!empty($has_sidebar_second)) print ' has-second'; ?>">
      <?php if (!empty($left)): ?><aside class="rp-sidebar rp-sidebar-first"><?php print $left; ?></aside><?php endif; ?>
      <main class="rp-content" role="main">
        <?php if (!empty($mission)): ?><div class="rp-mission"><?php print $mission; ?></div><?php endif; ?>
        <?php if (!empty($title)): ?><h1 class="rp-title"><?php print $title; ?></h1><?php endif; ?>
        <?php if (!empty($tabs)): ?><div class="rp-tabs"><?php print $tabs; ?></div><?php endif; ?>
        <?php if (!empty($help)): ?><div class="rp-help"><?php print $help; ?></div><?php endif; ?>
        <?php if (!empty($messages)) print $messages; ?>
        <div class="rp-content-body"><?php print $content; ?></div>
      </main>
      <?php if (!empty($right)): ?><aside class="rp-sidebar rp-sidebar-second"><?php print $right; ?></aside><?php endif; ?>
    </div>

    <footer class="rp-footer">
      <?php if (!empty($footer)): ?><div class="rp-footer-region"><?php print $footer; ?></div><?php endif; ?>
      <?php if (!empty($footer_message)): ?><div class="rp-footer-message"><?php print $footer_message; ?></div><?php endif; ?>
      <div class="rp-footer-brand">RokuPal</div>
    </footer>
  </div>
  <?php print $closure; ?>
</body>
</html>
