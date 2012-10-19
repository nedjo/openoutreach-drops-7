<?php

/**
 * @file
 * Installation profile for the Open Outreach distribution.
 */

// Include only when in install mode. MAINTENANCE_MODE is defined in
// install.php and in drush_core_site_install().
if (defined('MAINTENANCE_MODE') && MAINTENANCE_MODE == 'install') {
  include_once('openoutreach.install.inc');
}

/**
 * Implements hook_block_info().
 */
function openoutreach_block_info() {
  $blocks = array();
  $blocks['powered-by'] = array(
    'info' => t('Powered by Open Outreach'),
    'weight' => '10',
    'cache' => DRUPAL_NO_CACHE,
  );
  return $blocks;
}

/**
 * Implements hook_block_view().
 *
 * Display the powered by Open Outreach block.
 */
function openoutreach_block_view() {
  global $user;
  $admin_rid = variable_get('user_admin_role');

  $block = array();
  $block['subject'] = NULL;
  $content = '<span class="powered-by">' . t('Powered by <a href="!poweredby">Open Outreach</a>.', array('!poweredby' => 'http://openoutreach.org'));

  // If this is an admin role, show documentation links.
  if (isset($user->roles[$admin_rid])) {
    $content .= ' ' . t('Get started with <a href="!docs">user documentation</a> and <a href="!screencasts">screencasts</a>.', array('!docs' => 'http://openoutreach.org/section/using-open-outreach', '!screencasts' => 'http://openoutreach.org/screencasts'));
  }
  $content .= '</span>';
  $block['content'] = $content;
  return $block;
}

/**
 * Implements hook_modules_installed().
 *
 * Unset distracting messages at install time.
 */
function openoutreach_modules_enabled($modules) {
  if (defined('MAINTENANCE_MODE') && MAINTENANCE_MODE == 'install' && array_intersect($modules, array('captcha', 'date_api'))) {
    drupal_get_messages('status');
    drupal_get_messages('warning');
  }
}

/**
 * Implements hook_modules_installed().
 *
 * Add custom taxonomy terms to the event_type vocabulary if it is created.
 */
function openoutreach_entity_insert($entity, $type) {
  if ($type == 'taxonomy_vocabulary') {
    switch ($entity->machine_name) {
      // Add custom contact and organization types for the vocabularies created
      // by debut_redhen.
      case 'contact_type':
        $names = array('Staff', 'Volunteer', 'Media', 'Funder');
        break;
      case 'org_type':
        $names = array('Nonprofit', 'Foundation', 'Government', 'Business');
        break;
      // Add custom event types for the vocabulary created by debut_event.
      case 'event_type':
        $names = array('Conference', 'Meeting', 'Workshop');
        break;
      default:
        $names = array();
    }
    foreach ($names as $name) {
      $term = new StdClass();
      $term->name = $name;
      $term->vid = $entity->vid;
      $term->vocabulary_machine_name = $entity->machine_name;
      taxonomy_term_save($term);
    }
  }
}

/**
 * Implements hook_admin_menu_output_build().
 *
 * Add links to the admin_menu shortcuts menu.
 */
function openoutreach_admin_menu_output_build(&$content) {
  $content['shortcuts']['shortcuts']['admin-structure-taxonomy'] = array(
    '#title' => t('Add terms'),
    '#href' => 'admin/structure/taxonomy',
    '#access' => user_access('administer taxonomy'),
  );
  $content['shortcuts']['shortcuts']['user'] = array(
    '#title' => t('My account'),
    '#href' => 'user',
  );
}

/**
 * Implements hook_apps_servers_info().
 */
function openoutreach_apps_servers_info() {
  $profile = variable_get('install_profile', 'standard');
  $info =  drupal_parse_info_file(drupal_get_path('profile', $profile) . '/' . $profile . '.info');
  $return = array(
    'debut' => array(
      'title' => 'debut',
      'description' => t('Debut apps'),
      'manifest' => 'http://appserver.openoutreach.org/app/query',
      'profile' => $profile,
      'profile_version' => isset($info['version']) ? $info['version'] : '7.x-1.x',
    ),
  );

  if (isset($_SERVER['SERVER_NAME'])) {
    $return['debut']['server_name'] = $_SERVER['SERVER_NAME'];
  }
 
  if (isset($_SERVER['SERVER_ADDR'])) {
    $return['debut']['server_ip'] = $_SERVER['SERVER_ADDR'];
  }

  return $return;
}

