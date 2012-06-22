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
 * Determine whether a feature is being recreated.
 */
function openoutreach_is_recreating($feature = NULL) {
  // Test for Drush usage.
  if (function_exists('drush_get_command') && $command = drush_get_command()) {
    switch($command['command']) {
      case 'features-update-all':
        return TRUE;
      case 'features-update':
        // If a specific feature was requested, test for it. If not, return
        // true for any feature recreation.
        return is_null($feature) || in_array($feature, $command['arguments']);
    }
  }

  // Test for admin UI usage.
  $feature = is_null($feature) ? arg(3) : $feature;
  if ($_GET['q'] == "admin/structure/features/{$feature}/recreate") {
    return TRUE;
  }
  return FALSE;
}

/**
 * Implements hook_apps_servers_info().
 */
function openoutreach_apps_servers_info() {
  $profile = variable_get('install_profile', 'standard');
  $info =  drupal_parse_info_file(drupal_get_path('profile', $profile) . '/' . $profile . '.info');
  return array(
    'debut' => array(
      'title' => 'debut',
      'description' => t('Debut apps'),
      'manifest' => 'http://appserver.openoutreach.org/app/query',
      'profile' => $profile,
      'profile_version' => isset($info['version']) ? $info['version'] : '7.x-1.x',
      'server_name' => $_SERVER['SERVER_NAME'],
      'server_ip' => $_SERVER['SERVER_ADDR'],
    ),
  );
}

