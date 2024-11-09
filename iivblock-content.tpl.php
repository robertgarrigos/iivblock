<?php

/**
 * @file
 * Default theme implementation to display IIV Block content.
 *
 * Available variables:
 * - $image: Block image.
 * - $img_url: Image url (for background-image).
 * - $content: Block content.
 * - $wrapper_classes: Block content wrapper class(es).
 *
 * @see template_preprocess_block()
 */
?>
<?php if ($image_position == 'left' || $image_position == 'right'): ?>
  <div class="row <?php print $wrapper_classes; ?>">
    <?php if ($image_position == 'left'): ?>
      <div class="col-md-6 iivblock-image first <?php print $image_transition; ?>">
        <?php print $image ?>
      </div>
      <div class="col-md-6 iivblock-body second  <?php print $content_transition; ?>">
        <?php print $content ?>
      </div>
    <?php else: ?>
      <div class="col-md-6 iivblock-body first <?php print $content_transition; ?>">
        <?php print $content ?>
      </div>
      <div class="col-md-6 iivblock-image second <?php print $image_transition; ?>">
        <?php print $image ?>
      </div>
    <?php endif; ?>
    <script>
      iivblock_observer.observe(document.querySelector(".iivblock-inner.<?php print $machine_name; ?>"));
    </script>
  </div>
<?php else: ?>
  <div class="row <?php print $wrapper_classes; ?>">
    <div class="col">
      <?php if ($image_position == 'up'): ?>
        <div class="row">
          <div class="col d-flex justify-content-center iivblock-image first <?php print $image_transition; ?>">
            <?php print $image ?>
          </div>
        </div>
        <div class="row">
          <div class="col d-flex justify-content-center iivblock-body second  <?php print $content_transition; ?>">
            <?php print $content ?>
          </div>
        </div>
      <?php else: ?>
        <div class="row">
          <div class="col d-flex justify-content-center iivblock-body first <?php print $content_transition; ?>">
            <?php print $content ?>
          </div>
        </div>
        <div class="row">
          <div class="col d-flex justify-content-center iivblock-image second <?php print $image_transition; ?>">
            <?php print $image ?>
          </div>
        </div>
      <?php endif; ?>
    </div>
    <script>
      iivblock_observer.observe(document.querySelector(".iivblock-inner.<?php print $machine_name; ?>"));
    </script>
  </div>
<?php endif; ?>
