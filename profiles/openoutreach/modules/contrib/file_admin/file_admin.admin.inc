<?php

/**
 * @file
 * Administrative interface for file type configuration.
 */

/**
 * Provide an administrative settings form for file types.
 */
function file_admin_file_type_settings_form($form, &$form_state, $file_type) {
  $form = array();
  $form['additional_settings'] = array(
    '#type' => 'vertical_tabs',
    '#attached' => array(
      'js' => array(drupal_get_path('module', 'file_admin') . '/file_admin.js'),
    ),
  );
  $form['submission'] = array(
    '#type' => 'fieldset',
    '#title' => t('Submission form settings'),
    '#collapsible' => TRUE,
    '#group' => 'additional_settings',
  );
  $form['submission']['help']  = array(
    '#type' => 'textarea',
    '#title' => t('Explanation or submission guidelines'),
    '#default_value' => variable_get('file_admin_help_' . $file_type, ''),
    '#description' => t('This text will be displayed at the top of the page when creating or editing content of this type.'),
  );
  $form['workflow'] = array(
    '#type' => 'fieldset',
    '#title' => t('Publishing options'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#group' => 'additional_settings',
  );
  $form['workflow']['file_options'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Default options'),
    '#default_value' => variable_get('file_admin_file_options_' . $file_type, array('published')),
    '#options' => array(
      'published' => t('Published'),
      'promote' => t('Promoted'),
      'sticky' => t('Sticky at top of lists'),
    ),
    '#description' => t('Users with the <em>Administer files</em> permission will be able to override these options.'),
  );
  $form['file_type'] = array(
    '#type' => 'value',
    '#value' => $file_type,
  );
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save file type settings'),
    '#weight' => 40,
  );

  return $form;
}

/**
 * Submit handler for file_admin_file_type_settings_form.
 */
function file_admin_file_type_settings_form_submit($form, &$form_state) {
  $type = $form_state['values']['file_type'];
  foreach (array('help', 'file_options') as $variable) {
    variable_set('file_admin_' . $variable . '_' . $type, $form_state['values'][$variable]);
  }
}

