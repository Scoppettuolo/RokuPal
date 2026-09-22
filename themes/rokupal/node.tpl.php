<?php
/**
 * Node template — RokuPal.
 */
?>
<div id="node-<?php print $node->nid; ?>" class="node node-<?php print $node->type; ?><?php if ($sticky) print ' sticky'; ?><?php if (!$status) print ' node-unpublished'; ?>">
  <?php if ($page == 0): ?>
    <h2 class="node-title"><a href="<?php print $node_url; ?>"><?php print $title; ?></a></h2>
  <?php endif; ?>
  <?php if ($submitted): ?>
    <div class="submitted"><?php print $submitted; ?></div>
  <?php endif; ?>
  <div class="node-content content">
    <?php print $content; ?>
  </div>
  <?php if ($links): ?>
    <div class="node-links"><?php print $links; ?></div>
  <?php endif; ?>
</div>
