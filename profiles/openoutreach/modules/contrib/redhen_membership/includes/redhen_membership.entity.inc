<?php
/**
 * @file
 * Redhen Group entity classes
 */

/**
 * The class used for group entities.
 */
class RedhenMembership extends Entity {

  public
    $note = '',
    $name = '',
    $entity_type,
    $entity_id,
    $membership_id = NULL,
    $author_uid = NULL,
    $start_date = NULL,
    $expire_date = NULL,
    $membership_state = REDHEN_MEMBERSHIP_ACTIVE,
    $type = '',
    $created = '',
    $updated = '';

  public function __construct(array $values = array()) {
    global $user;

    parent::__construct($values, 'redhen_membership');

    // New membership. is_new might not be set so check for id
    if (!$this->membership_id) {
      $this->author_uid = $user->uid;
      $this->membership_state = REDHEN_MEMBERSHIP_ACTIVE;
      $this->created = REQUEST_TIME;
    }

  }

  /**
   * Override buildContent() to add contact properties.
   */
  public function buildContent($view_mode = 'full', $langcode = NULL) {

    $wrapper = entity_metadata_wrapper('redhen_membership', $this);
    $entity = $wrapper->entity->value();
    
    $entity_uri = entity_uri($entity->entityType(), $entity);
    $content['entity'] = array(
      '#theme' => 'redhen_property_field',
      '#label' => t('Applies to'),
      '#items' => array(
        array(
          '#markup' => l(entity_label($entity->entityType(), $entity), $entity_uri['path'])
        ),
      ),
      '#classes' => 'field field-label-inline clearfix',
    );
    if ($this->note != '') {
      $content['note'] = array(
        '#theme' => 'redhen_property_field',
        '#label' => t('Note'),
        '#items' => array(
          array(
            '#markup' => check_plain($this->note)
          ),
        ),
        '#classes' => 'field field-label-inline clearfix',
      );
    }

    if ($this->start_date) {
      $content['start_date'] = array(
        '#theme' => 'redhen_property_field',
        '#label' => t('Start Date'),
        '#items' => array(
          array(
            '#markup' => date('m/d/Y', $this->start_date)
          ),
        ),
        '#classes' => 'field field-label-inline clearfix',
      );
    }
    if ($this->expire_date) {
      $content['expire_date'] = array(
        '#theme' => 'redhen_property_field',
        '#label' => t('Expiration Date'),
        '#items' => array(
          array(
            '#markup' => date('m/d/Y', $this->expire_date)
          ),
        ),
        '#classes' => 'field field-label-inline clearfix',
      );
    }

    $membership_states = redhen_membership_state_options();
    $content['membership_state'] = array(
      '#theme' => 'redhen_property_field',
      '#label' => t('State'),
      '#items' => array(
        array(
          '#markup' => $membership_states[$this->membership_state],
        ),
      ),
      '#classes' => 'field field-label-inline clearfix',
    );

    return entity_get_controller($this->entityType)
      ->buildContent($this, $view_mode, $langcode, $content);
  }

  /**
   * Set the redhen_state for a membership
   *
   * @param $state
   *  The REDHEN_STATE_* value to use
   */
  public function setState($state) {
    $this->membership_state = $state;
    $this->save();
  }

  /**
   * Override label() to add membership label.
   */
  public function defaultLabel() {
    $type = redhen_membership_get_types($this->type);
    $wrapper = entity_metadata_wrapper('redhen_membership', $this);
    $entity = $wrapper->entity->value();
    if ($entity) {
      return t('@type membership for @title', array(
        '@type' => entity_label('redhen_membership_type', $type),
        '@title' => entity_label($wrapper->entity_type->value(), $entity),
        ));
    }
  }

  /**
   * Specifify URI
   */
  protected function defaultUri() {
    $wrapper = entity_metadata_wrapper('redhen_membership', $this);
    $entity = $wrapper->entity->value();
    $entity_uri = entity_uri($this->entity_type, $entity);
    return array('path' => $entity_uri['path'] . '/membership/' . $this->identifier());
  }

}

/**
 * Controls metadata for Redhen memberships
 *
 * Redhen utilizes custom property attributes to determine if a property
 * should be available as a filter on the listing page. These attributes are
 *   filter: TRUE to add as a filter.
 *   field_type: textfield, select, etc.
 *   options: array of options for select field types.

 */
class RedhenMembershipMetadataController extends EntityDefaultMetadataController {
  public function entityPropertyInfo() {
    $info = parent::entityPropertyInfo();
    $properties = &$info[$this->type]['properties'];


    $properties['membership_id'] = array(
      'label' => t("Membership ID"),
      'type' => 'integer',
      'description' => t("The unique ID of the membership."),
      'schema field' => 'membership_id',
      'setter callback' => 'entity_property_verbatim_set',
    );

    $properties['entity_type'] = array(
      'label' => t("Host entity type"),
      'type' => 'token',
      'description' => t("The entity type of the host entity."),
      'required' => TRUE,
      'schema field' => 'entity_type',
      'setter callback' => 'entity_property_verbatim_set',
    );

    $properties['entity_id'] = array(
      'label' => t("Host entity ID"),
      'type' => 'integer',
      'description' => t("The entity ID of the host entity."),
      'required' => TRUE,
      'schema field' => 'entity_id',
      'setter callback' => 'entity_property_verbatim_set',
    );

    $properties['start_date'] = array(
      'label' => t("Start Date"),
      'description' => t("The date the membership begins."),
      'type' => 'date',
      'field_type' => 'textfield',
      'schema field' => 'start_date',
      'filter' => TRUE,
      'setter callback' => 'entity_property_verbatim_set',
    );
    $properties['expire_date'] = array(
      'label' => t("Expiration Date"),
      'description' => t("The date the membership expires."),
      'type' => 'date',
      'field_type' => 'textfield',
      'schema field' => 'expire_date',
      'filter' => TRUE,
      'setter callback' => 'entity_property_verbatim_set',
    );
    $properties['created'] = array(
      'label' => t("Created"),
      'description' => t("The date the membership was created."),
      'type' => 'date',
      'schema field' => 'created',
      'setter callback' => 'entity_property_verbatim_set',
    );
    $properties['updated'] = array(
      'label' => t("Updated"),
      'description' => t("The date the membership was updated."),
      'type' => 'date',
      'schema field' => 'updated',
      'setter callback' => 'entity_property_verbatim_set',
    );
    $properties['membership_state'] = array(
      'label' => t('State'),
      'filter' => TRUE,
      'type' => 'integer',
      'field_type' => 'list',
      'required' => TRUE,
      'schema field' => 'membership_state',
      'setter callback' => 'entity_property_verbatim_set',
      'options list' => 'redhen_membership_state_options',
    );

    // Entities
    $properties['author'] = array(
      'label' => t("Author entity"),
      'type' => 'user',
      'description' => t("The author of the membership record."),
      'schema field' => 'author_uid',
    );

    $properties['entity'] = array(
      'label' => t("Host entity"),
      'type' => 'entity',
      'schema field' => 'entity',
      'description' => t("The host entity."),
      'getter callback' => 'redhen_membership_property_host_get',
    );

    return $info;
  }
}

  /**
   * RedhenMembership Views Controller class.
   */
class RedhenMembershipViewsController extends EntityDefaultViewsController {

  /**
   * Add extra fields to views_data().
   */
  public function views_data() {
    $data = parent::views_data();

    $entity_info = entity_get_info();

    foreach ($entity_info as $entity_type => $info) {
      if ($entity_type == 'redhen_org' || $entity_type == 'redhen_contact') {
        // entity to membership.
        $data[$info['base table']]['redhen_membership_rel'] = array(
          'group' => t('Membership'),
          'title' => t('@entity to Membership',
            array('@entity' => drupal_ucfirst($info['label']))),
          'help' => t('The Membership associated with the @entity entity.',
            array('@entity' => drupal_ucfirst($info['label']))),
          'relationship' => array(
            'entity' => $entity_type,
            'handler' => 'redhen_membership_handler_relationship',
            'label' => t('@entity being the Membership',
              array('@entity' => $entity_type)),
            'base' => 'redhen_membership',
            'base field' => 'entity_id',
            'relationship field' => $info['entity keys']['id'],
          ),
        );

        // entity to membership.
        $data[$info['base table']]['redhen_membership_active_member_type'] = array(
          'group' => t('Membership'),
          'title' => t('@entity active membership type(s)',
            array('@entity' => drupal_ucfirst($info['label']))),
          'help' => t('The active membership type(s) of the @entity entity.',
            array('@entity' => drupal_ucfirst($info['label']))),
          'field' => array(
            'handler' => 'redhen_membership_handler_field_active_member_type',
          ),
        );

        // Membership to entity.
        $data['redhen_membership']['redhen_membership_related_' . $entity_type] = array(
          'group' => t('Membership'),
          'title' => t('Membership to @entity',
            array('@entity' => drupal_ucfirst($info['label']))),
          'help' => t('The @entity entity that is associated with the Membership.',
            array('@entity' => $info['label'])),
          'relationship' => array(
            'handler' => 'views_handler_relationship',
            'label' => t('@entity from Membership',
              array('@entity' => $entity_type)),
            'base' => $info['base table'],
            'base field' => $info['entity keys']['id'],
            'relationship field' => 'entity_id',
          ),
        );
      }
    }

    return $data;
  }
}

/**
 * Callback to get $membership->entity.
 *
 * @see redhen_membership_entity_property_info().
 */
function redhen_membership_property_host_get(RedhenMembership $membership, array $options, $property_name, $entity_type) {
  return entity_load_single($membership->entity_type, $membership->entity_id);
}


