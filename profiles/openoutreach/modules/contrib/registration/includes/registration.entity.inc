<?php

/**
 * @file
 * Entity hooks and callbacks for registrations.
 */

/**
 * Implements hook_entity_info().
 */
function registration_entity_info() {
  $entities = array(
    'registration' => array(
      'label' => t('Registration'),
      'plural label' => t('Registrations'),
      'controller class' => 'EntityAPIController',
      'entity class' => 'Registration',
      'metadata controller class' => 'RegistrationMetadataController',
      'base table' => 'registration',
      'fieldable' => TRUE,
      'entity keys' => array(
        'id' => 'registration_id',
        'bundle' => 'type',
      ),
      'access callback' => 'registration_access',
      'bundle keys' => array(
        'bundle' => 'name',
      ),
      'bundles' => array(),
      'view modes' => array(
        'full' => array(
          'label' => t('Full Registration'),
          'custom settings' => FALSE,
        ),
      ),
      'uri callback' => 'entity_class_uri',
      'token type' => 'registration',
      'module' => 'registration',
      'label callback' => 'entity_class_label'
    ),
    'registration_type' => array(
      'label' => t('Registration type'),
      'entity class' => 'RegistrationType',
      'controller class' => 'EntityAPIControllerExportable',
      'base table' => 'registration_type',
      'fieldable' => FALSE,
      'bundle of' => 'registration',
      'exportable' => TRUE,
      'entity keys' => array(
        'id' => 'id',
        'name' => 'name',
        'label' => 'label',
      ),
      'access callback' => 'registration_type_access',
      'module' => 'registration',
      // Enable the entity API's admin UI.
      'admin ui' => array(
        'path' => 'admin/structure/registration/registration_types',
        'file' => 'registration_type.admin.inc',
        'file path' => drupal_get_path('module', 'registration') . '/includes',
        'controller class' => 'RegistrationTypeUIController',
      ),
    ),
    'registration_state' => array(
      'label' => t('Registration State'),
      'plural label' => t('Registration states'),
      'controller class' => 'RegistrationStateController',
      'entity class' => 'RegistrationState',
      'base table' => 'registration_state',
      'fieldable' => FALSE,
      'entity keys' => array(
        'id' => 'registration_state_id',
        'label' => 'label',
        'name' => 'name',
      ),
      'bundles' => array(
        'registration_state' => array(
          'label' => 'Registration States',
        ),
      ),
      'admin ui' => array(
        'path' => 'admin/structure/registration/registration_states',
        'file' => 'registration.forms.inc',
        'file path' => drupal_get_path('module', 'registration') . '/includes',
        'controller class' => 'RegistrationStatesUIController',
      ),
      'token type' => 'registration_state',
      'module' => 'registration',
      'access callback' => 'registration_state_access',
      'exportable' => TRUE
    ),
  );

  // Support entity cache module.
  if (module_exists('entitycache')) {
    $return['registration']['field cache'] = FALSE;
    $return['registration']['entity cache'] = TRUE;
  }

  return $entities;
}

/**
 * Implements hook_entity_info_alter().
 */
function registration_entity_info_alter(&$entity_info) {
  // @todo: we're testing to ensure the schema exists; needed because running gui
  // install profile was hitting this BEFORE the schema was installed.
  if (drupal_get_schema('registration')) {
    // We are adding the info about the registration types via a hook to avoid a
    // recursion issue as loading the model types requires the entity info as well.
    foreach (registration_get_types() as $type => $info) {
      $entity_info['registration']['bundles'][$type] = array(
        'label' => $info->label,
        'admin' => array(
          'path' => 'admin/structure/registration/registration_types/manage/%registration_type',
          'real path' => 'admin/structure/registration/registration_types/manage/' . $type,
          'bundle argument' => 5,
          'access arguments' => array('administer registration types'),
        ),
      );
    }
  }
}

/**
 * Main class for Registration entities.
 */
class Registration extends Entity {

  public
    $registration_id,
    $entity_id,
    $entity_type,
    $entity,
    $mail,
    $anon_mail = NULL,
    $user_uid = NULL,
    $user,
    $count,
    $author,
    $author_uid,
    $state,
    $created,
    $updated;

  public function __construct(array $values = array(), $entityType = NULL) {
    parent::__construct($values, $entityType);

    // Automatically load related entities.
    $this->author = $this->author();
    $this->entity = $this->entity();
    $this->user = $this->user();
    $this->state = $this->state();

    $this->mail = $this->mail();
  }

  /**
   * Specifies the default label, which is picked up by label() by default.
   */
  protected function defaultLabel() {
    if ($this->entity) {
      return t('Registration for @title', array(
          '@title' => entity_label($this->entity_type, $this->entity)
        )
      );
    }
    return '';
  }

  /**
   * Load host entity of registration.
   *
   * @return object|bool
   *   An entity, or FALSE if the entity does not exist.
   */
  protected function entity() {
    return (($this->entity_id && !empty($this->entity_type)) ? entity_load_single($this->entity_type, $this->entity_id) : FALSE);
  }

  /**
   * Load author user account of registration.
   *
   * @return object|bool
   *   User entity, or FALSE if the user does not exist.
   */
  protected function author() {
    return ($this->author_uid) ? user_load($this->author_uid) : FALSE;
  }

  /**
   * Load registered user account of registration.
   *
   * @return object|bool
   *   User entity, or FALSE if the user does not exist.
   */
  protected function user() {
    return ($this->user_uid) ? user_load($this->user_uid) : FALSE;
  }

  /**
   * Load State of registration.
   *
   * @return object|bool
   *   State entity, or FALSE if the state does not exist.
   */
  protected function state() {
    return (!empty($this->state) ? entity_load_single('registration_state', $this->state) : FALSE);
  }

  /**
   * Build content for Registration.
   *
   * @return render array
   */
  public function buildContent($view_mode = 'full', $langcode = NULL) {
    $build = parent::buildContent($view_mode, $langcode);

    $host_entity_type_info = entity_get_info($this->entity_type);
    $host_entity = entity_load_single($this->entity_type, $this->entity_id);
    list(, , $host_entity_bundle) = entity_extract_ids($this->entity_type, $host_entity);

    $host_label = entity_label($this->entity_type, $host_entity);
    $host_uri = entity_uri($this->entity_type, $host_entity);

    // Mail
    $build['mail'] = array(
      '#prefix' => '<div class="field registration-mail"><div class="field-label">' . t('Email Address') . '</div>',
      '#markup' => $this->mail,
      '#suffix' => '</div>',
    );

    // Link to host entity
    $host_entity_link_label = (isset($host_entity_type_info['bundles'][$host_entity_bundle]['label'])) ? '<div class="field-label">' . $host_entity_type_info['bundles'][$host_entity_bundle]['label'] . '</div>' : '';
    $build['host_entity_link'] = array(
      '#prefix' => '<div class="field registration-entity-link">' . $host_entity_link_label,
      '#markup' => l($host_label, $host_uri['path']),
      '#suffix' => '</div>',
    );

    // Date created
    $build['created'] = array(
      '#prefix' => '<div class="field registration-created"><div class="field-label">' . t('Created') . '</div>',
      '#markup' => format_date($this->created),
      '#suffix' => '</div>',
    );

    // Date updated
    $build['updated'] = array(
      '#prefix' => '<div class="field registration-updated"><div class="field-label">' . t('Updated') . '</div>',
      '#markup' => format_date($this->updated),
      '#suffix' => '</div>',
    );

    // Slots used
    $build['slots'] = array(
      '#prefix' => '<div class="field registration-slots"><div class="field-label">' . t('Slots Used') . '</div>',
      '#markup' => $this->count,
      '#suffix' => '</div>',
    );

    // Registration authored by
    if ($this->author) {
      $author_uri = user_uri($this->author);
    }

    $build['author'] = array(
      '#prefix' => '<div class="field registration-author"><div class="field-label">' . t('Author') . '</div>',
      '#markup' => (isset($author_uri['path']) ? l($this->author->name, $author_uri['path']) : t('Anonymous')),
      '#suffix' => '</div>',
    );

    // Registration belongs to
    if ($this->user) {
      $account_uri = user_uri($this->user);
    }

    $build['user'] = array(
      '#prefix' => '<div class="field registration-user"><div class="field-label">' . t('User') . '</div>',
      '#markup' => (isset($account_uri['path']) ? l($this->user->name, $account_uri['path']) : t('Anonymous')),
      '#suffix' => '</div>',
    );

    $build['state'] = array(
      '#prefix' => '<div class="field registration-state"><div class="field-label">' . t('State') . '</div>',
      '#markup' => ($this->state) ? filter_xss_admin(entity_label('registration_state', $this->state)) : '',
      '#suffix' => '</div>',
    );

    return $build;
  }

  /**
   * Save registration.
   *
   * @return mixed
   */
  public function save() {
    // set a default state if not provided
    if (!$this->state || $this->state == -1) {
      $default_state = registration_get_default_state();
      if ($default_state) {
        $this->state = $default_state->internalIdentifier();
      }
    }
    $this->updated = REQUEST_TIME;
    if (!$this->registration_id) {
      $this->created = REQUEST_TIME;
    }
    return parent::save();
  }

  /**
   * Specify URI
   */
  protected function defaultUri() {
    return array('path' => 'registration/' . $this->internalIdentifier());
  }

  /**
   * Return email address to use with this registration.
   *
   * @return string
   */
  protected function mail() {
    return ($this->user) ? $this->user->mail : $this->anon_mail;
  }

  /**
   * Determine registrant type relative to a given account
   *
   * @param $account
   *   A Drupal user
   *
   * @return string
   *   Can be me, user, or anon.
   */
  public function registrant_type($account) {
    if (!empty($account)) {
      if ($account->uid && $account->uid === $this->user_uid) {
        return REGISTRATION_REGISTRANT_TYPE_ME;
      }
      elseif ($this->user_uid) {
        return REGISTRATION_REGISTRANT_TYPE_USER;
      }
      elseif (!empty($this->anon_mail)) {
        return REGISTRATION_REGISTRANT_TYPE_ANON;
      }
    }
  }
}

/**
 * @see hook_entity_property_info()
 */
class RegistrationMetadataController extends EntityDefaultMetadataController {
  public function entityPropertyInfo() {
    $info = parent::entityPropertyInfo();
    $properties = &$info[$this->type]['properties'];

    $properties['created'] = array(
      'label' => t("Created"),
      'description' => t("The date the registration was created."),
      'type' => 'date',
      'schema field' => 'created',
    );
    $properties['updated'] = array(
      'label' => t("Updated"),
      'description' => t("The date the registration was updated."),
      'type' => 'date',
      'schema field' => 'updated',
    );
    $properties['entity'] = array(
      'label' => t('Entity'),
      'type' => 'entity',
      'schema field' => 'entity_id',
      'description' => t('The entity the registration belongs to.'),
      'getter callback' => 'registration_get_properties',
    );
    $properties['author'] = array(
      'label' => t("Author"),
      'type' => 'user',
      'description' => t("The author of the registration."),
      'schema field' => 'author_uid'
    );
    $properties['user'] = array(
      'label' => t("User"),
      'type' => 'user',
      'description' => t("The user associated with the registration."),
      'schema field' => 'user_uid'
    );

    return $info;
  }
}

/**
 * Required by RegistrationMetadataController for Views integration.
 *
 * @param $entity
 * @param array $options
 * @param $name
 * @param $type
 *
 * @return EntityMetadataWrapper
 */
function registration_get_properties($entity, array $options, $name, $type) {
  switch ($name) {
    case 'entity':
      return entity_metadata_wrapper($entity->entity_type, $entity->entity);
      break;
  }
}

/**
 * Loads a registration by ID.
 */
function registration_load($registration_id) {
  if (empty($registration_id)) {
    return FALSE;
  }

  $registrations = registration_load_multiple(array($registration_id), array());
  return $registrations ? reset($registrations) : FALSE;
}

/**
 * Loads multiple registrations by ID or based on a set of matching conditions.
 *
 * @see entity_load()
 *
 * @param $registration_ids
 * @param $conditions
 *   An array of conditions on the {registration} table in the form
 *     'field' => $value.
 * @param $reset
 *   Whether to reset the internal registration loading cache.
 *
 * @return
 *   An array of registration objects indexed by registration_id.
 */
function registration_load_multiple($registration_ids = array(), $conditions = array(), $reset = FALSE) {
  if (empty($registration_ids) && empty($conditions)) {
    return array();
  }

  return entity_load('registration', $registration_ids, $conditions, $reset);
}

/**
 * Deletes multiple registrations by ID.
 *
 * @param $registration_ids
 *   An array of registration IDs to delete.
 *
 * @return
 *   TRUE on success, FALSE otherwise.
 */
function registration_delete_multiple($registration_ids) {
  return entity_get_controller('registration')->delete($registration_ids);
}

/**
 * Saves a registration.
 *
 * @param $registration
 *   The full registration object to save.
 *
 * @return
 *   If the record insert or update failed, returns FALSE. If it succeeded,
 *   returns SAVED_NEW or SAVED_UPDATED, depending on the operation performed.
 */
function registration_save(Registration $registration) {
  return $registration->save();
}

/**
 * Access callback: Entity API for Registration entities.
 *
 * Checks if a user has permission to execute an operation on a registration
 * entity.
 *
 * Implements hook_registration_access(). Modules may return a boolean value,
 * or NULL if they do not care one way or the other.
 *
 * @param string $op
 *   Operation user wishes to perform.
 * @param object $registration
 *   (optional) A Fully loaded registration object.
 * @param object $account
 *   (optional) An user object, or omit for logged in user.
 * @param string $entity_type
 *   (optional) Entity type of $registration. Usually 'registration'.
 *
 * @return bool
 *
 * @see registration_entity_info()
 */
function registration_access($op, $registration = NULL, $account = NULL, $entity_type) {
  $account = isset($account) ? $account : $GLOBALS['user'];

  // Check environment for Registration.
  switch ($op) {
    case 'update':
      $people = registration_access_people($registration);
      $registrant_type = $registration->registrant_type($account);
      if (!isset($registrant_type) && !isset($people[$registrant_type])) {
        return FALSE;
      }
    case 'create':
      if (!count(registration_access_people($registration))) {
        return FALSE;
      }
      break;
  }


  // First grant access to the entity for the specified operation if no other
  // module denies it and at least one other module says to grant access.
  $access_results = module_invoke_all('registration_access', $op, $registration, $account, $entity_type);
  if (in_array(FALSE, $access_results, TRUE)) {
    return FALSE;
  }
  elseif (in_array(TRUE, $access_results, TRUE)) {
    return TRUE;
  }

  // bypass further access checks if user can administer registration
  if (user_access('administer registration', $account)) {
    return TRUE;
  }

  $account_own = ($registration->author && ($registration->author->uid == $account->uid));

  // Fall back to assigned permissions
  $type = $registration->type;
  switch ($op) {
    case 'view':
      return ($account_own && user_access("view own $type registration", $account)) || user_access("view $type registration", $account);
    case 'update':
      return ($account_own && user_access("update own $type registration", $account)) || user_access("update any $type registration", $account);
    case 'create':
      return (user_access("create $type registration", $account));
    case 'delete':
      return ($account_own && user_access("delete own $type registration", $account)) || user_access("delete any $type registration", $account);
  }
}

/**
 * The class used for model type entities
 */
class RegistrationType extends Entity {

  public $name;
  public $label;
  public $locked;

  public function __construct($values = array()) {
    parent::__construct($values, 'registration_type');
  }
}

/**
 * Registration Type UI controller.
 */
class RegistrationTypeUIController extends EntityDefaultUIController {

  /**
   * Overrides hook_menu() defaults.
   */
  public function hook_menu() {
    $items = parent::hook_menu();
    $items[$this->path]['description'] = t('Manage registration entity types, including adding and removing fields and the display of fields.');
    return $items;
  }
}

/**
 * Gets an array of all registration types, keyed by the name.
 *
 * @param $name
 *   If set, the type with the given name is returned.
 */
function registration_get_types($name = NULL) {
  $types = entity_load_multiple_by_name('registration_type', isset($name) ? array($name) : FALSE);
  return isset($name) ? reset($types) : $types;
}

/**
 * Menu argument loader; Load a registration type by string.
 *
 * @param $type
 *   The machine-readable name of a registration type to load.
 *
 * @return
 *   A registration type array or FALSE if $type does not exist.
 */
function registration_type_load($type) {
  return registration_get_types($type);
}

/**
 * Access callback for the entity API.
 */
function registration_type_access($op, $entity = NULL, $account = NULL, $entity_type = NULL) {
  return user_access('administer registration types', $account);
}

/**
 * Saves a model type to the db.
 */
function registration_type_save(RegistrationType $type) {
  $type->save();
}

/**
 * Deletes a model type from the db.
 */
function registration_type_delete(RegistrationType $type) {
  $type->delete();
}

/**
 * The class used for registration state entities
 */
class RegistrationState extends Entity {

  public $name, $label, $description, $default_state,
    $active, $show_on_form, $weight;

  public function __construct($values = array()) {
    parent::__construct($values, 'registration_state');
  }

}

/**
 * The controller class used for registration state entities
 */
class RegistrationStateController extends EntityAPIControllerExportable {

  public function save($entity, DatabaseTransaction $transaction = NULL) {
    parent::save($entity, $transaction);

    if ($entity->default_state == 1) {
      $query = db_update('registration_state')
        ->fields(array('default_state' => 0))
        ->condition('registration_state_id', $entity->registration_state_id, '<>');
      $query->execute();
    }
  }

}


/**
 * UI Controller for Registration states.
 */
class RegistrationStatesUIController extends EntityDefaultUIController {

  public function overviewForm($form, &$form_state) {
    return drupal_get_form('registration_states_overview_form');
  }

}

/**
 * Access callback for the entity API.
 */
function registration_state_access($op, $entity = NULL, $account = NULL, $entity_type = NULL) {
  return user_access('administer registration states', $account);
}
