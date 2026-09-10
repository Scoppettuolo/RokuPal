<?php
/**
 * RokuPal block.
 */
?>
<section id="block-<?php print $block->module . '-' . $block->delta; ?>" class="rp-block block-<?php print $block->module; ?>">
  <?php if (!empty($block->subject)): ?>
    <h2 class="rp-block-title"><?php print $block->subject; ?></h2>
  <?php endif; ?>
  <div class="rp-block-body">
    <?php print $block->content; ?></div>
</section>
