<?php
// Modern theme: CSS variables palette (Color module).
$info = array(
  'css_variables' => TRUE,
  'fields' => array(
    'base' => t('Background'),
    'link' => t('Links'),
    'text' => t('Text'),
    'header' => t('Header'),
    'accent' => t('Accent'),
  ),
  'schemes' => array(
    'default' => array(
      'title' => t('Default'),
      'colors' => array(
        'base' => '#0f172a',
        'link' => '#0ea5e9',
        'text' => '#e2e8f0',
        'header' => '#1e293b',
        'accent' => '#38bdf8',
      ),
    ),
    'light' => array(
      'title' => t('Light'),
      'colors' => array(
        'base' => '#f8fafc',
        'link' => '#0284c7',
        'text' => '#0f172a',
        'header' => '#e2e8f0',
        'accent' => '#0ea5e9',
      ),
    ),
  ),
  'css_map' => array(
    'base' => '--color-bg',
    'link' => '--color-link',
    'text' => '--color-text',
    'header' => '--color-header',
    'accent' => '--color-accent',
  ),
);
