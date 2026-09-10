<?php
/**
 * RokuPal comment — defensive variables (PHP 8 safe).
 */
$author  = isset($author) ? $author : '';
$created = isset($created) ? $created : (isset($comment->timestamp) ? format_date($comment->timestamp, 'small') : '');
$content = isset($content) ? $content : '';
$links   = isset($links) ? $links : '';
$new     = isset($new) ? $new : '';
?>
<article class="rp-comment<?php if (!empty($comment->new)) print ' is-new'; ?>">
  <header class="rp-comment-header">
    <?php if ($author): ?><span class="rp-comment-author"><?php print $author; ?></span><?php endif; ?>
    <?php if ($created): ?><span class="rp-comment-date"> · <?php print $created; ?></span><?php endif; ?>
    <?php if ($new): ?><span class="rp-new"><?php print $new; ?></span><?php endif; ?>
  </header>
  <div class="rp-comment-body"><?php print $content; ?></div>
  <?php if ($links): ?><div class="rp-comment-links"><?php print $links; ?></div><?php endif; ?>
</article>
