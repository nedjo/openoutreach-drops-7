<?php

/**
 * @file
 * Provides functionality to be used at install time.
 */

/**
 * Generate an install task to install subprofile features.
 *
 * Adapted from panopoly_install_tasks().
 */
function openoutreach_install_tasks($install_state) {

  // Kick off the tasks.
  $tasks = array();

  // Summon the power of the Apps module.
  require_once(drupal_get_path('module', 'apps') . '/apps.profile.inc');

  // Set up a task to verify capability to run apps.
  $tasks['openoutreach_apps_verify'] = array(
    'display_name' => t('Verify apps support'),
    'type' => 'form',
  );

  // Set up the openoutreach Apps install task.
  $debut_server = array(
    'machine name' => 'debut',
    'default apps' => array(
      'debut',
      'debut_article',
      'debut_bio',
      'debut_blog',
      'debut_comment',
      'debut_event',
      'debut_highlighted',
      'debut_link',
      'debut_media',
      'debut_section',
      'debut_seo',
      'debut_social',
      'debut_wysiwyg',
    ),
    'required apps' => array(
      'debut',
    ),
  );
  $tasks = $tasks + apps_profile_install_tasks($install_state, $debut_server);

  // Rename one of the default apps tasks. In the case of a non-interactive
  // installation, apps_profile_install_tasks() never defines this task, so we
  // need to make sure we don't accidentally create it when it doesn't exist.
  if (isset($tasks['apps_profile_apps_select_form_debut'])) {
    $tasks['apps_profile_apps_select_form_debut']['display_name'] = t('Install apps for Open Outreach');
  }

  // Enable any selected Open Outreach features.
  $tasks['openoutreach_enable_features'] = array(
    'display' => FALSE,
    'type' => 'batch',
    'run' => (isset($_SESSION['openoutreach_features'])) ? INSTALL_TASK_RUN_IF_NOT_COMPLETED : INSTALL_TASK_SKIP,
    'function' => 'openoutreach_enable_features',
  );

  // Set up the theme selection task.
  $tasks['openoutreach_theme_form'] = array(
    'display_name' => t('Choose a theme'),
    'type' => 'form',
  );

  // Set up the prepare task to close it out.
  $tasks['openoutreach_prepare'] = array(
    'display_name' => t('Prepare site'),
    'type' => 'form',
  );

  return $tasks;
}

/**
 * Implements hook_install_tasks_alter()
 */
function openoutreach_install_tasks_alter(&$tasks, $install_state) {
  // Create a more informative finished page.
  $tasks['install_finished']['function'] = 'openoutreach_finished';
  $tasks['install_finished']['display_name'] = t('Finished!');
  $tasks['install_finished']['type'] = 'form';
}

/**
 * Implements hook_install_configure_form_alter().
 */
function openoutreach_form_install_configure_form_alter(&$form, &$form_state) {
  // Set some reasonable defaults for site configuration form.
  $form['site_information']['site_name']['#default_value'] = 'Open Outreach';
  $form['admin_account']['account']['name']['#default_value'] = 'admin';
  // Don't set the email address to "admin@localhost" as that will fail D7's
  // email address validation.
  if ($_SERVER['HTTP_HOST'] != 'localhost') {
    $form['site_information']['site_mail']['#default_value'] = 'admin@'. $_SERVER['HTTP_HOST'];
    $form['admin_account']['account']['mail']['#default_value'] = 'admin@'. $_SERVER['HTTP_HOST'];
  }
}

/**
 * Implements hook_form_FORM_ID_alter().
 *
 * Adapted from panopoly_apps_profile_apps_select_form_alter().
 *
 * @todo: restore hidden content fieldset when Debut apps support it.
 */
function openoutreach_form_apps_profile_apps_select_form_alter(&$form, $form_state) {
  // For some things there are no need.
  $form['apps_message']['#access'] = FALSE;
  $form['apps_fieldset']['apps']['#title'] = NULL;

  // Debut doesn't yet support default content.
  $form['default_content_fieldset']['#access'] = FALSE;

  // Improve style of apps selection form.
  if (isset($form['apps_fieldset'])) {
    $options = array();
    foreach($_SESSION['apps_manifest'] as $name => $app) {
      if ($name != '#theme') {
        $options[$name] = '<strong>' . $app['name'] . '</strong><p><div class="admin-options"><div class="form-item">' . theme('image', array('path' => $app['logo']['path'], 'height' => '32', 'width' => '32')) . '</div>' . $app['description'] . '</div></p>';
      }
    }
    ksort($options);
    $form['apps_fieldset']['apps']['#options'] = $options;
  }

  // Enable installation of Open Outreach features.
  // Extract features checkboxes from the features admin form.
  module_load_include('inc', 'features', 'features.admin');
  $features_admin_form = features_admin_form(array(), array());
  $features = $features_admin_form['features']['status'];
  // Core will be enabled already.
  unset($features['openoutreach_core']);
  // Only show openoutreach features.
  foreach (array_keys($features) as $name) {
    if (strpos($name, 'openoutreach_') !== 0) {
      unset($features[$name]);
    }
    else {
      // Remove links to the feature, since they won't work here.
      $features[$name]['#title'] = strip_tags($features[$name]['#title']);
      // Default to enabled.
      $features[$name]['#default_value'] = TRUE;
    }
  }
  $form['openoutreach_features'] = array(
    '#type' => 'fieldset',
    '#title' => t('Select Open Outreach features'),
    '#description' => t('Select the features you wish to enable. You will also have a chance to enable or disable features after installing.'),
    '#weight' => -1,
    '#tree' => TRUE,
  ) + $features;

  $form['#submit'][] = 'openoutreach_apps_profile_apps_select_form_submit';
}

/**
 * Submit handler for install_configure_form. Set a variable specifying the
 * features to be installed.
 */
function openoutreach_apps_profile_apps_select_form_submit($form, &$form_state) {
  if ($form_state['values']['op'] == t('Install Apps')) {
    $features = array_keys(array_filter($form_state['values']['openoutreach_features']));
    if (!empty($features)) {
      $_SESSION['openoutreach_features'] = $features;
    }
  }
}

/**
 * Install selected Open Outreach features.
 */
function openoutreach_enable_features(&$install_state) {
  $features = $_SESSION['openoutreach_features'];

  if (!empty($features)) {
    $operations = array();
    foreach ($features as $module) {
      $module_info = system_get_info('module', $module);
      $module_name = isset($module_info['name']) ? $module_info['name'] : $module;
      $operations[] = array('app_profile_enable_module', array($module, $module_name));
    }
    if (!empty($operations)) {
      $batch = array(
        'title' => t('Enable Open Outreach features'),
        'init_message' => t('Enabling features...'),
        'progress_message' => t('Enabling @current out of @total features.'),
        'operations' => $operations,
        'error_message' => t('Feature enabling encountered an error.'),
        'file' => drupal_get_path('module', 'apps') . '/apps.profile.inc',
      );
    }
    return $batch;
  }
}

/**
 * Pseudo implementation of hook_user_default_permissions().
 *
 * @see openoutreach_install_permissions().
 */
function openoutreach_install_user_default_permissions() {
  $permissions = array();

  // Exported permission: access contextual links
  $permissions['access contextual links'] = array(
    'name' => 'access contextual links',
    'roles' => array(
      0 => 'administrator',
      1 => 'editor',
    ),
    'module' => 'contextual',
  );

  // Exported permission: search content
  $permissions['search content'] = array(
    'name' => 'search content',
    'roles' => array(
      0 => 'administrator',
      1 => 'anonymous user',
      2 => 'authenticated user',
    ),
    'module' => 'search',
  );

  // Exported permission: use advanced search
  $permissions['use advanced search'] = array(
    'name' => 'use advanced search',
    'roles' => array(
      0 => 'administrator',
      1 => 'anonymous user',
      2 => 'authenticated user',
    ),
    'module' => 'search',
  );

  // Exported permission: view advanced help index
  $permissions['view advanced help index'] = array(
    'name' => 'view advanced help index',
    'roles' => array(
      0 => 'administrator',
    ),
    'module' => 'advanced_help',
  );

  // Exported permission: view advanced help popup
  $permissions['view advanced help popup'] = array(
    'name' => 'view advanced help popup',
    'roles' => array(
      0 => 'administrator',
    ),
    'module' => 'advanced_help',
  );

  // Exported permission: view advanced help topic
  $permissions['view advanced help topic'] = array(
    'name' => 'view advanced help topic',
    'roles' => array(
      0 => 'administrator',
    ),
    'module' => 'advanced_help',
  );

  return $permissions;
}

/**
 * Install permissions for optional modules.
 *
 * Optional modules - identified by the "recommends" array in
 * openoutreach.info - cannot have their permissions in features; recommended
 * modules are enabled after features permissions are implemented, and in any
 * case such features would break if the optional module was disabled.
 *
 * Take advantage of existing support in the Features module for processing
 * default user permissions. Use a pseudo module name, 'openoutreach_install'.
 */
function openoutreach_install_permissions() {
  features_include();
  module_load_include('inc', 'features', 'features.export');
  user_permission_features_rebuild('openoutreach_install');
}

/**
 * Form to check to see if Apps support is possible.
 */
function openoutreach_apps_verify($form, &$form_state) {
  $form = array();

  $form['#attached']['css'] = array(
    drupal_get_path('profile', 'openoutreach') . '/openoutreach.install.css',
  );

  $form = array();

  $form['opening'] = array(
    '#markup' => '<h1>' . t('Verify Apps support') . '</h1>',
  );

  $profiles = install_find_profiles();
  $is_pantheon = isset($profiles['pantheon']);

  if ($is_pantheon) {
    // Note Pantheon requirements.
    // We can test writability as a proxy for the Pantheon on server
    // development site setting.
    $pantheon_local_enabled = is_writeable(drupal_realpath('sites/all/modules'));
    $form['pantheon'] = array(
      '#title' => t('Pantheon'),
      '#type' => 'fieldset',
      '#description' => theme('image', array('path' => drupal_get_path('profile', 'openoutreach') . '/images/enable-apps-pantheon.png')),
    );
    if (!$pantheon_local_enabled) {
      $form['pantheon']['#description'] .= t('Before installing Open Outreach on Pantheon, you need to enable "On Server Development" to use apps.') . '<br /><br />';
    }
    else {
      $form['pantheon']['#description'] .= t('"On Server Development" is enabled, so you\'re good to go to install Open Outreach on Pantheon using apps.') . '<br /><br />';
    }
    $form['pantheon']['#description'] .= t('After you install your apps, you will need to use the "Commit" button to add them to the version control system.');
    $form['#attached']['css'] = array(
      drupal_get_path('profile', 'openoutreach') . '/openoutreach.install.css',
    );
  }
  else {
    $form['openingtext'] = array(
      '#markup' => '<p>' . t('Apps downloads the code needed to install apps using the same mechanism as the Update module in Drupal core. This functionality depends on certain PHP extensions being enabled on your server. Below is the documentation & verification for the various methods of installing. <strong>Note that you need only <em>one</em> of these methods enabled in order to install apps.</strong>') . '</p>',
      '#weight' => -10,
    );

    // Verify FTP support
    $ftp_installed = extension_loaded('ftp');
    $form['ftp'] = array(
      '#type' => 'fieldset',
      '#title' => t('FTP (!status)', array('!status' => $ftp_installed ? t('Enabled!') : t('Not enabled'))),
      '#description' => '',
      '#collapsible' => !$ftp_installed,
      '#collapsed' => !$ftp_installed,
      '#weight' => $ftp_installed ? 0 : 10,
    );
    if (!$ftp_installed) {
      $form['ftp']['#description'] .= t('Your server does not have the FTP PHP extension. You will need to install it or use an alternative method. See <a href="http://us2.php.net/manual/en/book.ftp.php">http://us2.php.net/manual/en/book.ftp.php</a> for how to install the FTP PHP extension.') . '<br /><br />';
    }
    $form['ftp']['#description'] .= t('To install with FTP, you will need an FTP username and password that has permissions to write to your site directory on your server. Be aware that FTP is not an encrypted protocol and your credentials will be transmitted in the clear.');

    // Verify SSH support
    $ssh_installed = extension_loaded('ssh2');
    $form['ssh'] = array(
      '#type' => 'fieldset',
      '#title' => t('SSH (!status)', array('!status' => $ssh_installed ? t('Enabled!') : t('Not enabled'))),
      '#description' => '',
      '#collapsible' => !$ssh_installed,
      '#collapsed' => !$ssh_installed,
      '#weight' => $ssh_installed ? 1 : 11,
    );
    if (!$ssh_installed) {
      $form['ssh']['#description'] .= t('Your server does not have ssh2 installed. You will need to install it or use an alternative method. See <a href="http://us2.php.net/manual/en/book.ssh2.php">http://us2.php.net/manual/en/book.ssh2.php</a> for how to install the ssh2 php extension.') . '<br /><br />';
    }
    $form['ssh']['#description'] .= t('To install with SSH, you will need a username and password of a user that can SSH into the server and has write permissions to your site directory on your server.');

    // Verify web server write permissions
    $install_permissions = is_writeable(drupal_realpath('sites/all/modules'));
    $vars = array('@install_path' => 'sites/all/modules');
    $form['webserver'] = array(
      '#type' => 'fieldset',
      '#title' => t('Webserver direct install (!status)', array('!status' => $install_permissions ? t('Enabled!') : t('Not enabled'))),
      '#description' => '',
      '#collapsible' => !$install_permissions,
      '#collapsed' => !$install_permissions,
      '#weight' => $install_permissions ? 2 : 12,
    );
    $form['webserver']['#description'] .= $install_permissions
      ? t('You have write permissions to @install_path and so can install using this method.', $vars) . '<br /><br />'
      : t('You do not have sufficient permissions to instal by webserver direct install. In order to assign these permissions, go to the root of your drupal install and type <br/><br/><strong>sudo chmod -R 777 @install_path</strong>', $vars) . '<br /><br />';
    $form['webserver']['#description'] .= t('Be aware that there are security issues with leaving your site in this state.');

    // When altering this form and adding an install option, set this value to
    // TRUE if your option is enabled (available). 
    $form['app_enabled'] = array(
      '#type' => 'value',
      '#value' => $pantheon_local_enabled || $ftp_installed || $ssh_installed || $install_permissions ? TRUE : FALSE,
    );

    $form['#process'] = array('openoutreach_install_verify_process');
  }

  $form['continue'] = array(
    '#type' => 'submit',
    '#value' => t('Continue'),
    '#weight' => 20,
  );

  return $form;
}

/**
 * Process callback for apps_install_verify form.
 *
 * If no options are enabled, show all of them and a message.
 */
function openoutreach_install_verify_process($form) {
  if (!$form['app_enabled']['#value']) {
    foreach (element_children($form) as $key) {
      if (isset($form[$key]['#type']) && $form[$key]['#type'] == 'fieldset') {
        $form[$key]['#collapsible'] = FALSE;
        $form[$key]['#collapsed'] = FALSE;
      }
    }
    drupal_set_message(t('No enabled App install methods found.'), 'warning');
  }
  return $form;
}


/**
 * Form to choose the starting theme from list of available options.
 */
function openoutreach_theme_form($form, &$form_state) {

  // Create list of theme options, minus admin + testing + starter themes.
  $themes = array();
  foreach(system_rebuild_theme_data() as $theme) {
    if (!in_array($theme->name, array('test_theme', 'update_test_basetheme', 'update_test_subtheme', 'block_test_theme', 'stark', 'seven', 'fusion_core', 'fusion_starter', 'fusion_starter_lite', 'garland'))) {
      $themes[$theme->name] = theme('image', array('path' => $theme->info['screenshot'])) . '<strong>' . $theme->info['name'] . '</strong><br><p><em>' . strip_tags($theme->info['description']) . '</em></p><p class="clearfix"></p>';
    }
  }

  // Move Bartik to last.
  $bartik = $themes['bartik'];
  unset($themes['bartik']);
  $themes['bartik'] = $bartik;

  $form['theme'] = array(
    '#title' => t('Starting Theme'),
    '#type' => 'radios',
    '#options' => $themes,
    '#default_value' => 'granada',
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Choose theme'),
  );

  $form['#attached']['css'] = array(
    drupal_get_path('profile', 'openoutreach') . '/openoutreach.install.css',
  );

  return $form;
}

/**
 * Form submit handler to select the theme.
 */
function openoutreach_theme_form_submit($form, &$form_state) {
  // Enable and set the theme of choice.
  $theme = $form_state['input']['theme'];
  theme_enable(array($theme));
  variable_set('theme_default', $theme);

  // Enable modules used by the default theme.
  $info = drupal_parse_info_file(drupal_get_path('theme', $theme) . "/$theme.info");
  $dependencies = array(
    'fusion' => array('fusion_apply_ui'),
    'designkit' => array('designkit'),
  );
  foreach ($dependencies as $key => $modules) {
    if (isset($info[$key])) {
      module_enable($modules);
    }
  }

  // Assign regions for the theme.
  // TODO: how does Panopoly automate this?
  $regions = system_region_list($theme);
  $blocks = array();

  $assignments = array(
    'system' => array(
      'main' => 'content',
      'main-menu' => 'main_menu',
      'help' => 'help',
    ),
  );

  foreach ($assignments as $module => $module_blocks) {
    foreach ($module_blocks as $delta => $region) {
      if (isset($regions[$region])) {
        $blocks[] = array(
          'module' => $module,
          'delta' => $delta,
          'theme' => $theme,
          'status' => 1,
          'weight' => 0,
          'region' => $region,
          'pages' => '',
          'cache' => -1,
        );
      }
    }
  }
  if ($blocks) {
    foreach ($blocks as $record) {
      $query = db_merge('block')
        ->key(array('theme' => $record['theme'], 'module' => $record['module'], 'delta' => $record['delta']))
        ->fields($record)
        ->execute();
    }
  }

  // Flush theme caches so things are right.
  system_rebuild_theme_data();
  drupal_theme_rebuild();
}

/**
 * Form to talk about preparing the site for prime time.
 */
function openoutreach_prepare($form, &$form_state) {
  // Hide some messages from various modules that are just too chatty!
  drupal_get_messages('status');
  drupal_get_messages('warning');

  $form = array();

  $form['opening'] = array(
    '#markup' => '<h1>' . t('Prepare Site'),
  );

  $form['openingtext'] = array(
    '#markup' => '<h2>' . t('Open Outreach now needs to do a bit more to get everything set up.') . '</h2><p>' . t('This step may take a few minutes.') . '</p>',
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Prepare your site'),
  );

  return $form;
}

/**
 * Submit form to prepare site for prime time.
 */
function openoutreach_prepare_submit($form, &$form_state) {
  openoutreach_install_permissions();

  // Flush all caches to ensure that any full bootstraps during the installer
  // do not leave stale cached data, and that any content types or other items
  // registered by the install profile are registered correctly.
  drupal_flush_all_caches();

  // Remember the profile which was used.
  variable_set('install_profile', drupal_get_profile());

  // Install profiles are always loaded last.
  db_update('system')
    ->fields(array('weight' => 1000))
    ->condition('type', 'module')
    ->condition('name', drupal_get_profile())
    ->execute();

  // Cache a fully-built schema.
  drupal_get_schema(NULL, TRUE);

  // Run cron to populate update status tables (if available) so that users
  // will be warned if they've installed an out of date Drupal version.
  // Will also trigger indexing of profile-supplied content or feeds.
  drupal_cron_run();
}

/**
 * Form to finish it all out and send us on our way.
 */
function openoutreach_finished($form, &$form_state) {
  $form = array();

  $form['opening'] = array(
    '#markup' => '<h1>' . t('Finished!') . '</h1>',
  );

  $form['openingtext'] = array(
    '#markup' => '<h2>' . t('Congratulations, you just installed Open Outreach!') . '</h2>',
  );
  
  $form['openoutreach_logo'] = array(
    '#markup' => theme('image', array('path' => drupal_get_path('profile', 'openoutreach') . '/images/openoutreach_logo.png')),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Visit your new site'),
  );

  return $form;
}

/**
 * Submit form to finish it out and send us on our way!
 */
function openoutreach_finished_submit($form, &$form_state) {
  // Once more for good measure.
  drupal_flush_all_caches();

  unset($_SESSION['openoutreach_features']);
  
  // And away we go.
  // $form_state['redirect'] won't work here since we are still in the
  // installer, so use drupal_goto() (for interactive installs only) instead.
  $install_state = $form_state['build_info']['args'][0];
  if ($install_state['interactive']) {
    drupal_goto('<front>');
  }
}
