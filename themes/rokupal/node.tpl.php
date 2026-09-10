<?php
/**
 * RokuPal node card.
 */
?>
<article id="node-<?php print $node->nid; ?>" class="rp-node node-<?php print $node->type; ?><?php if ($sticky) print ' is-sticky'; ?><?php if (!$status) print ' is-unpublished'; ?>">
  <?php if ($page == 0): ?>
    <h2 class="rp-node-title"><a href="<?php print $node_url; ?>"><?php print $title; ?></a></h2>
  <?php endif; ?>
  <?php if ($submitted): ?>
    <div class="rp-meta"><?php print $submitted; ?></div>
  <?php endif; ?>
  <div class="rp-node-content">
    <?php print $content; ?>
  </div>
  <?php if ($links): ?>
    <div class="rp-node-links"><?php print $links; ?></div>
  <?php endif; ?>
</article>
