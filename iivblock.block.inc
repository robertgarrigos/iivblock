<?php

/**
 * BlockIIV extends Block.
 *
 * This class allows us to create blocks with image and text.
 */
class BlockIIV extends Block {

  /**
   * Constructor for an IIV Block.
   */
  public function __construct($plugin_name, array $data) {
    parent::__construct($plugin_name, $data);

    $this->settings += array(
      'label' => '',
      'machine_name' => '',
    );
  }

  /**
   * Sets title text on draggable block panel in Layout builder.
   */
  public function getAdminTitle() {
    return !empty($this->settings['label']) ? check_plain($this->settings['label']) : t('IIV Block');
  }

  /**
   * Returns the rendered content of this block.
   *
   * @return string
   *   Rendered markup of an IIV Block content.
   */
  public function getContent() {
    $elements = array(
      'content' => '',
      'image' => '',
      'img_url' => '',
      'wrapper_classes' => 'iivblock-inner ' . $this->settings['machine_name'],
      'image_position' => '',
      'image_transition' => isset($this->settings['image_transition']) ? $this->settings['image_transition'] : 'normal',
      'content_transition' => isset($this->settings['content_transition']) ? $this->settings['content_transition'] : 'normal',
      'machine_name' => $this->settings['machine_name'],
    );

    $block = config_get('iivblock.block.' . $this->settings['machine_name']);
    if (!empty($block['content'])) {
      $content = token_replace($block['content']);
      $elements['content'] = check_markup($content, $block['format'], '', TRUE);
    }
    if (!empty($block['image'])) {
      $markup = '';
      $image_style = FALSE;
      if (!empty($block['image_style'])) {
        $image_style = image_style_load($block['image_style']);
      }
      $file = file_load($block['image']);
      if (is_a($file, 'File')) {
        if ($block['image_align'] == 'parallax') {
          if ($image_style) {
            $elements['img_url'] = image_style_url($image_style['name'], $file->uri);
          }
          else {
            $elements['img_url'] = file_create_url($file->uri);
          }
        }
        else {
          $variables = array(
            'style_name' => 'thumbnail',
            'uri' => $file->uri,
            'width' => isset($file->width) ? $file->width : NULL,
            'height' => isset($file->height) ? $file->height : NULL,
            'alt' => $block['alt'],
            'attributes' => array(),
          );
          if (!empty($block['img_title'])) {
            $variables['attributes']['title'] = $block['img_title'];
          }
          if ($image_style) {
            if ($file->filemime != 'image/svg+xml') {
              $variables['style_name'] = $image_style['name'];
              $markup = theme('image_style', $variables);
            }
            else {
              $width = $this->extractEffectWidth($image_style);
              if ($width) {
                $variables['width'] = $width;
              }
              $markup = theme('image', $variables);
            }
          }
          else {
            $markup = theme('image', $variables);
          }
        }
      }
      // Wrap image with a link tag.
      if (!empty($block['link']) && $block['image_align'] != 'parallax') {
        $href = '';
        if (valid_url($block['link']) || valid_url($block['link'], TRUE)) {
          $href = $block['link'];
        }
        elseif (preg_match('/.+ \[(\d+)\]$/', $block['link'], $matches)) {
          // Get alias from nid if input came via autocomplete.
          $href = url('node/' . $matches[1]);
        }
        $attributes = array();
        $attributes['href'] = $href;
        if (!empty($block['link_target'])) {
          $attributes['target'] = $block['link_target'];
        }
        if ($block['nofollow']) {
          $attributes['rel'] = 'nofollow';
        }
        $markup = '<a' . backdrop_attributes($attributes) . '>' . $markup . '</a>';
      }
      $elements['image'] = $markup;
    }
    $elements['image_position'] = $block['image_position'];
    $elements['image_transition'] = $block['image_transition'];
    $elements['content_transition'] = $block['content_transition'];
    return theme('iivblock_content', $elements);
  }

  /**
   * Builds the block's configuration form.
   */
  public function form(&$form, &$form_state) {
    parent::form($form, $form_state);
    if ($this->is_new) {
      $args = arg();
      if (isset($args[8])) {
        $block_info = explode(':', $args[8]);
        if (isset($block_info[1]) && $block_info[1] != 'default') {
          // Copy original data (image/text).
          $block = config_get('iivblock.block.' . $block_info[1]);
          // Force new machine name.
          $this->settings['machine_name'] = '';
          $this->settings['title'] = $block['title'];
        }
      }
    }
    else {
      // Edit existing block, no clone.
      $block = config_get('iivblock.block.' . $this->settings['machine_name']);
    }
    $form['label'] = array(
      '#type' => 'textfield',
      '#title' => t('Admin label'),
      '#default_value' => isset($this->settings['label']) ? $this->settings['label'] : 'IIV Block',
      '#maxlength' => 64,
      '#description' => t('This text is used only in administrative interfaces. It will not be shown to site visitors.'),
      '#required' => TRUE,
      '#id' => 'iivblock-label',
      '#weight' => -1,
    );
    $form['machine_name'] = array(
      '#type' => 'machine_name',
      '#default_value' => $this->settings['machine_name'],
      '#maxlength' => 128,
      '#machine_name' => array(
        'exists' => 'iivblock_load',
        'source' => array('label'),
      ),
      '#description' => t('A unique machine-readable name for this Block. It must only contain lowercase letters, numbers, and underscores.'),
      '#disabled' => !empty($this->settings['machine_name']),
    );

    $form['title_display']['title_display']['#default_value'] = LAYOUT_TITLE_CUSTOM;
    $form['title_display']['title_display']['#access'] = FALSE;
    $form['title_display']['title']['#default_value'] = $this->settings['title'];
    $form['title_display']['title']['#description'] = t('Leave empty for no block title.');
    // Image.
    $extensions = 'jpg jpeg png gif svg';
    if (config_get('system.core', 'image_toolkit') == 'gd' && defined('IMAGETYPE_WEBP')) {
      $gd_info = gd_info();
      if (isset($gd_info['WebP Support']) && $gd_info['WebP Support'] == TRUE) {
        $extensions .= ' webp';
      }
    }
    $upload_validators = array('file_validate_extensions' => array($extensions));
    $config = config_get('iivblock.settings');
    if (!empty($config['iivblock_max_file_size'])) {
      $max_bytes = parse_size($config['iivblock_max_file_size']);
      $upload_validators['file_validate_size'] = array($max_bytes);
    }
    if (!empty($config['iivblock_max_dimensions'])) {
      $upload_validators['file_validate_image_resolution'] = array($config['iivblock_max_dimensions'], '');
    }
    $upload_description = theme('file_upload_help', array(
      'upload_validators' => $upload_validators,
    ));
    $upload_location = 'public://';
    if (!empty($config['iivblock_image_path'])) {
      $upload_location .= $config['iivblock_image_path'];
    }
    $form['image'] = array(
      '#type' => 'managed_file',
      '#title' => t('Image'),
      '#description' => $upload_description,
      '#default_value' => isset($block['image']) ? $block['image'] : '',
      '#upload_location' => $upload_location,
      '#upload_validators' => $upload_validators,
    );

    // Additional image settings and attributes.
    $form['extended'] = array(
      '#type' => 'fieldset',
      '#title' => t('Additional image settings'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    $options = array();
    foreach (image_styles() as $key => $preset) {
      $options[$key] = $preset['name'];
    }
    if (!empty($options)) {
      $form['extended']['image_style'] = array(
        '#type' => 'select',
        '#title' => t('Image style'),
        '#options' => array('' => t('<none>')) + $options,
        '#default_value' => isset($block['image_style']) ? $block['image_style'] : '',
      );
    }

    $form['extended']['alt'] = array(
      '#type' => 'textfield',
      '#title' => t('Alternate text'),
      '#description' => t('This text will be used by screen readers, search engines, or when the image cannot be loaded.'),
      '#default_value' => isset($block['alt']) ? $block['alt'] : '',
    );
    $form['extended']['img_title'] = array(
      '#type' => 'textfield',
      '#title' => t('Title'),
      '#description' => t('The title is used as a tool tip when the user hovers the mouse over the image.'),
      '#default_value' => isset($block['img_title']) ? $block['img_title'] : '',
    );
    $form['extended']['link'] = array(
      '#type' => 'textfield',
      '#title' => t('Link'),
      '#description' => t('Leave empty for no link. Start typing to get suggestions, or paste a full url.'),
      '#default_value' => isset($block['link']) ? $block['link'] : '',
      '#autocomplete_path' => 'node/autocomplete',
    );
    $form['extended']['link_target'] = array(
      '#type' => 'select',
      '#title' => t('Link target'),
      '#options' => array(
        '' => 'none',
        '_blank' => '_blank',
        '_parent' => '_parent',
        '_self' => '_self',
        '_top' => '_top',
      ),
      '#default_value' => isset($block['link_target']) ? $block['link_target'] : '',
    );
    $form['extended']['nofollow'] = array(
      '#type' => 'checkbox',
      '#title' => t('NoFollow option'),
      '#description' => t('Mark this item if you want add rel=nofollow to your link.'),
      '#default_value' => !empty($block['nofollow']) ? $block['nofollow'] : FALSE,
    );
    $form['extended2'] = array(
      '#type' => 'fieldset',
      '#title' => t('Image configuration'),
      '#collapsible' => FALSE,
      '#collapsed' => FALSE,
    );
    $form['extended2']['image_position'] = array(
      '#type' => 'select',
      '#title' => t('Image position'),
      '#options' => array(
        'left' => t('Left'),
        'right' => t('Right'),
      ),
      '#default_value' => isset($block['image_position']) ? $block['image_position'] : '',
    );
    $form['extended2']['image_transition'] = array(
      '#type' => 'select',
      '#title' => t('Image transition'),
      '#options' => array(
        'normal' => t('Normal'),
        'fadeIn' => t('fadeIn'),
        'slideInLeft' => t('slideInLeft'),
        'slideInRight' => t('slideInRight'),
        'slideInBottom' => t('slideInBottom'),
        'slideInTop' => t('slideInTop'),
        'swingInBottom' => t('swingInBottom'),
        'swingInTop' => t('swingInTop'),
      ),
      '#default_value' => isset($block['image_transition']) ? $block['image_transition'] : '',
    );
    $form['content'] = array(
      '#type' => 'text_format',
      '#title' => t('Block content'),
      '#default_value' => isset($block['content']) ? $block['content'] : '',
      '#format' => isset($block['format']) ? $block['format'] : NULL,
      '#editor_uploads' => TRUE,
      '#rows' => 5,
    );
    $form['extended3'] = array(
      '#type' => 'fieldset',
      '#title' => t('Content configuration'),
      '#collapsible' => FALSE,
      '#collapsed' => FALSE,
    );
    $form['extended3']['content_transition'] = array(
      '#type' => 'select',
      '#title' => t('Content transition'),
      '#options' => array(
        'normal' => t('Normal'),
        'fadeIn' => t('fadeIn'),
        'slideInLeft' => t('slideInLeft'),
        'slideInRight' => t('slideInRight'),
        'slideInBottom' => t('slideInBottom'),
        'slideInTop' => t('slideInTop'),
        'swingInBottom' => t('swingInBottom'),
        'swingInTop' => t('swingInTop'),
      ),
      '#default_value' => isset($block['content_transition']) ? $block['content_transition'] : '',
    );

  }

  /**
   * Validates the form settings.
   */
  public function formValidate($form, &$form_state) {
    parent::formValidate($form, $form_state);

    // Check that one of title, text, or image, is provided.
    if (empty($form_state['values']['image']) && empty($form_state['values']['title']) && empty($form_state['values']['content']['value'])) {
      form_error($form['image'], t('An image, a Display title, or Block content must be provided.'));
    }
  }

  /**
   * Submit handler to save the form settings.
   */
  public function formSubmit($form, &$form_state) {
    parent::formSubmit($form, $form_state);
    $this->settings['label'] = $form_state['values']['label'];
    $this->settings['machine_name'] = $form_state['values']['machine_name'];

    // Additional content goes to a different config.
    $block = array();
    $block['parent'] = $this->delta;
    $block['label'] = $form_state['values']['label'];
    $block['machine_name'] = $form_state['values']['machine_name'];
    $block['title'] = $form_state['values']['title'];
    $block['content'] = $form_state['values']['content']['value'];
    $block['format'] = $form_state['values']['content']['format'];

    // Record file usage inside block body.
    $file_usages = db_select('file_usage', 'f')
      ->fields('f', array('fid'))
      ->condition('module', 'iivblock')
      ->condition('type', $block['machine_name'])
      ->execute()
      ->fetchAllAssoc('fid', PDO::FETCH_ASSOC);
    $content_fids = filter_parse_file_fids($block['content']);
    $files = file_load_multiple($content_fids);
    $status_permanent = FILE_STATUS_PERMANENT;
    foreach ($files as $fid => $file) {
      if ($file->status !== "$status_permanent") {
        // Using the machine_name for type is a bit dirty,
        // and so is using the file fid as object fid.
        file_usage_add($file, 'iivblock', $block['machine_name'], $fid);
      }
      else {
        if (!array_key_exists($fid, $file_usages)) {
          file_usage_add($file, 'iivblock', $block['machine_name'], $fid);
        }
      }
    }
    foreach ($file_usages as $old_fid => $value) {
      if (!in_array($old_fid, $content_fids)) {
        $file = file_load($old_fid);
        if ($file) {
          file_usage_delete($file, 'iivblock', $block['machine_name'], $old_fid);
        }
      }
    }

    if (!empty($form_state['values']['image'])) {
      $current_file = file_load($form_state['values']['image']);
      if (!$this->is_new) {
        // Edit an existing block.
        if ($form_state['values']['image'] != $form['image']['#default_value']) {
          // Set file usage on new file, also marks permanent.
          file_usage_add($current_file, 'iivblock', 'block', $current_file->fid);
          if (!empty($form['image']['#default_value'])) {
            // Remove usage from old (replaced) file.
            $old_file = file_load($form['image']['#default_value']);
            if ($old_file) {
              file_usage_delete($old_file, 'iivblock');
            }
          }
        }
      }
      else {
        // New block or copy of an existing block, increase file usage.
        file_usage_add($current_file, 'iivblock', 'block', $current_file->fid);
      }
    }
    else {
      if (!empty($form['image']['#default_value'])) {
        // Old image was removed, no new one added.
        $old_file = file_load($form['image']['#default_value']);
        if ($old_file) {
          file_usage_delete($old_file, 'iivblock');
        }
      }
    }
    $block['image'] = $form_state['values']['image'];
    // Extended attributes.
    $block['alt'] = $form_state['values']['extended']['alt'];
    $block['img_title'] = $form_state['values']['extended']['img_title'];
    $block['link'] = $form_state['values']['extended']['link'];
    $block['link_target'] = $form_state['values']['extended']['link_target'];
    $block['nofollow'] = $form_state['values']['extended']['nofollow'];
    $block['image_style'] = $form_state['values']['extended']['image_style'];
    $block['image_align'] = $form_state['values']['extended']['image_align'];

    $block['image_position'] = $form_state['values']['extended2']['image_position'];
    $block['image_transition'] = $form_state['values']['extended2']['image_transition'];
    $block['content_transition'] = $form_state['values']['extended3']['content_transition'];

    $config = config('iivblock.block.' . $block['machine_name']);
    $config->setData($block);
    $config->save();
  }

  /**
   * Try to extract a width value from an image style.
   *
   * Image styles do not necessarily scale or crop, so a width value may exist
   * or not.
   *
   * @param array $image_style
   *   The image style definitions.
   *
   * @return
   *   Number (width in pixels) or FALSE if the style doesn't resize.
   */
  protected function extractEffectWidth($image_style) {
    $style_widths = &backdrop_static(__METHOD__, array());
    $style_name = $image_style['name'];

    if (!isset($style_widths[$style_name])) {
      $width = FALSE;
      foreach ($image_style['effects'] as $index => $values) {
        if (!empty($values['data']['width'])) {
          // The last width value wins.
          $width = $values['data']['width'];
        }
      }
      $style_widths[$style_name] = $width;
    }

    return $style_widths[$style_name];
  }

}
