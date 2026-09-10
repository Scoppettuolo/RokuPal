<?php
/**
 * RokuPal page — flexible regions + logo.
 */
?><!DOCTYPE html>
<html lang="<?php print isset($language->language) ? $language->language : 'en'; ?>">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title><?php print $head_title; ?></title>
  <?php print $head; ?>
  <?php print $styles; ?>
  <?php if (!empty($rokupal_color_css)): ?>
  <style type="text/css" id="rokupal-color-vars"><?php print $rokupal_color_css; ?></style>
  <?php endif; ?>
  <?php print $scripts; ?>
</head>
<body class="<?php print !empty($body_classes) ? $body_classes : 'rokupal'; ?>">
  <div class="rp-page">
    <header class="rp-header" role="banner">
      <div class="rp-header-inner">
        <div class="rp-brand">
          <a class="rp-logo" href="<?php print $front_page; ?>" rel="home" title="RokuPal">
            <?php if (!empty($logo)): ?>
              <img src="<?php print $logo; ?>" alt="RokuPal" class="rp-logo-img" />
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
            <?php if (!empty($primary_links)): ?><?php print theme('links', $primary_links, array('class' => 'rp-primary')); ?><?php endif; ?>
            <?php if (!empty($secondary_links)): ?><?php print theme('links', $secondary_links, array('class' => 'rp-secondary')); ?><?php endif; ?>
          </nav>
        <?php endif; ?>
      </div>
      <?php if (!empty($header)): ?><div class="rp-region-header"><?php print $header; ?></div><?php endif; ?>
      <?php if (!empty($header_first) || !empty($header_second)): ?>
        <div class="rp-header-columns">
          <?php if (!empty($header_first)): ?><div class="rp-header-col"><?php print $header_first; ?></div><?php endif; ?>
          <?php if (!empty($header_second)): ?><div class="rp-header-col"><?php print $header_second; ?></div><?php endif; ?>
        </div>
      <?php endif; ?>
    </header>

    <?php if (!empty($highlighted)): ?>
      <div class="rp-highlighted"><?php print $highlighted; ?></div>
    <?php endif; ?>

    <?php if (!empty($breadcrumb)): ?>
      <div class="rp-breadcrumb"><?php print $breadcrumb; ?></div>
    <?php endif; ?>

    <div class="rp-main<?php if (!empty($has_sidebar_first)) print ' has-first'; ?><?php if (!empty($has_sidebar_second)) print ' has-second'; ?>">
      <?php if (!empty($left)): ?>
        <aside class="rp-sidebar rp-sidebar-first" role="complementary"><?php print $left; ?></aside>
      <?php endif; ?>

      <main class="rp-content" role="main">
        <?php if (!empty($tabs)): ?><div class="rp-tabs"><?php print $tabs; ?></div><?php endif; ?>
        <?php if (!empty($messages)): ?><?php print $messages; ?><?php endif; ?>
        <?php if (!empty($help)): ?><div class="rp-help"><?php print $help; ?></div><?php endif; ?>
        <?php if (!empty($content_top)): ?><div class="rp-content-top"><?php print $content_top; ?></div><?php endif; ?>
        <?php if (!empty($title)): ?><h1 class="rp-title"><?php print $title; ?></h1><?php endif; ?>
        <?php print $content; ?>
        <?php if (!empty($content_bottom)): ?><div class="rp-content-bottom"><?php print $content_bottom; ?></div><?php endif; ?>
      </main>

      <?php if (!empty($right)): ?>
        <aside class="rp-sidebar rp-sidebar-second" role="complementary"><?php print $right; ?></aside>
      <?php endif; ?>
    </div>

    <?php if (!empty($footer_top)): ?>
      <div class="rp-footer-top"><?php print $footer_top; ?></div>
    <?php endif; ?>

    <footer class="rp-footer" role="contentinfo">
      <?php if (!empty($footer_first) || !empty($footer_second) || !empty($footer_third)): ?>
        <div class="rp-footer-columns">
          <?php if (!empty($footer_first)): ?><div class="rp-footer-col"><?php print $footer_first; ?></div><?php endif; ?>
          <?php if (!empty($footer_second)): ?><div class="rp-footer-col"><?php print $footer_second; ?></div><?php endif; ?>
          <?php if (!empty($footer_third)): ?><div class="rp-footer-col"><?php print $footer_third; ?></div><?php endif; ?>
        </div>
      <?php endif; ?>
      <?php if (!empty($footer)): ?><div class="rp-footer-main"><?php print $footer; ?></div><?php endif; ?>
      <div class="rp-footer-meta">
        <a href="https://github.com/Scoppettuolo" rel="noopener">GitHub</a>
        ·
        <a href="https://katnya.blogspot.com/" rel="noopener">Katnya</a>
        · RokuPal 1.0
      </div>
    </footer>
  </div>
</body>
</html>
