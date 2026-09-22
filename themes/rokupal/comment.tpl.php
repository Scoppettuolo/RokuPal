<?php
/**
 * Comment template — RokuPal.
 */
?>
<div class="comment<?php if (isset($comment->status) && $comment->status == COMMENT_NOT_PUBLISHED) print ' comment-unpublished'; ?>">
  <div class="comment-header">
    <?php if (!empty($title)): ?><h3 class="comment-title"><?php print $title; ?></h3><?php endif; ?>
    <div class="comment-meta">
      <?php if (!empty($author)): ?><span class="comment-author"><?php print $author; ?></span><?php endif; ?>
      <?php if (!empty($created)): ?><span class="comment-date"> · <?php print $created; ?></span><?php endif; ?>
    </div>
  </div>
  <div class="comment-content content">
    <?php print $content; ?>
  </div>
  <?php if (!empty($links)): ?>
    <div class="comment-links"><?php print $links; ?></div>
  <?php endif; ?>
</div>
