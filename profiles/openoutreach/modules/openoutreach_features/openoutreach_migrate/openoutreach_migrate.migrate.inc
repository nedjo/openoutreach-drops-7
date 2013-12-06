<?php

/**
 * @file
 *  Base migrations used in OpenOutreach.
 */

/**
 * Base migration for OpenOutreach.
 */
abstract class OpenOutreachMigration extends Migration {

  /**
   * The directory from which CSV files and media assets will be imported.
   *
   * To replace the default imports with customized versions, create a 'custom'
   * directory in the openoutreach_migrate directory and place custom import
   * files there.
   */
  protected $baseImportDirectory;

  /**
   * The name of a CSV file to be imported.
   */
  protected $importFileName;

  /**
   * The type of migration, used as a bundle value where appropriate and also
   * for the base filename of the import CSV file.
   */
  protected $type;

  public function processImport(array $options = array()) {
    parent::processImport($options);
    // Do not force menu rebuilding. Otherwise pathauto will try to rebuild
    // in each node_insert invocation.
    variable_set('menu_rebuild_needed', FALSE);
  }

  /**
   * Suppress distracting install-time messages.
   */
  protected function progressMessage($result) {
    if (!drupal_installation_attempted()) {
      parent::progressMessage($result);
    }
  }

  protected function setProperties($type) {
    $path = drupal_get_path('module', 'openoutreach_migrate');
    $this->baseImportDirectory = (file_exists($path . '/local') ? $path . '/local' : $path . '/import');
    $this->importFileName = $type . '.csv';
    $this->type = $type;
  }

  protected function getImportFilePath() {
    return $this->baseImportDirectory . '/' . $this->importFileName;
  }

}

/**
 * Base migration for nodes in OpenOutreach.
 */
abstract class OpenOutreachMigrationNode extends OpenOutreachMigration {

  /**
   * Whether or not the node type supports a field_tags taxonomy term reference
   * field.
   */
  protected $useTags = TRUE;

  /**
   * The relative path from the $baseImportDirectory to a directory with images
   * for import.
   */
  protected $imageDirectory;

  public function __construct() {
    parent::__construct();
    $this->description = t('Import node content.');

    // Create a map object for tracking the relationships between source rows
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'title' => array(
          'type' => 'varchar',
          'length' => 255,
          'not null' => TRUE,
        ),
      ),
      MigrateDestinationNode::getKeySchema()
    );

    // Create a MigrateSource object.
    $this->source = new MigrateSourceCSV($this->getImportFilePath(), $this->csvcolumns(), array('header_rows' => 1));

    $this->destination = new MigrateDestinationNode($this->type);

    $this->addFieldMapping('uid')->defaultValue(1);
    $this->addFieldMapping('status')->defaultValue(1);
    $this->addFieldMapping('promote', 'promote');
    $this->addFieldMapping('language')->defaultValue(LANGUAGE_NONE);
    $this->addFieldMapping('title', 'title');

    $this->addFieldMapping('field_content_image', 'image');
    $this->addFieldMapping('field_content_image:file_replace')
      ->defaultValue(FILE_EXISTS_REPLACE);
    $this->addFieldMapping('field_content_image:source_dir')
      ->defaultValue($this->getImageDirectoryPath());

    $this->addFieldMapping('body', 'body');
    $this->addFieldMapping('body:format')->defaultValue(module_exists('debut_wysiwyg') ? 'wysiwyg_editor' : 'filtered_html');

    if ($this->useTags) {
      $this->addFieldMapping('field_tags', 'tags')
         ->separator(', ');
      $this->addFieldMapping('field_tags:create_term')
        ->defaultValue(TRUE);
    }

    $this->addFieldMapping('is_new')->defaultValue(TRUE);
  }

  function csvcolumns() {
    $columns[] = array('title', 'Title');
    $columns[] = array('promote', 'Promote');
    $columns[] = array('path', 'Path');
    $columns[] = array('image', 'Image');
    $columns[] = array('image_title', 'Image title');
    $columns[] = array('image_alt', 'Image alt');
    if ($this->useTags) {
      $columns[] = array('tags', 'Tags');
    }
    $columns[] = array('body', 'Body');
    return $columns;
  }

  protected function getImageDirectoryPath() {
    return $this->baseImportDirectory . '/' . $this->imageDirectory;
  }

  function setProperties($type) {
    parent::setProperties($type);
    $this->type = $type;
    $this->imageDirectory = 'images/' . $type;
  }
}

/**
 * Article migration for nodes in OpenOutreach.
 */
class OpenOutreachMigrationNodeArticle extends OpenOutreachMigrationNode {

  public function __construct() {
    parent::setProperties('article');
    parent::__construct();
    $this->description = t('Import article content.');
  }

}

/**
 * Blog migration for nodes in OpenOutreach.
 */
class OpenOutreachMigrationNodeBlog extends OpenOutreachMigrationNode {

  public function __construct() {
    parent::setProperties('blog');
    parent::__construct();
    $this->description = t('Import blog content.');
  }

}

/**
 * Event migration for nodes in OpenOutreach.
 */
class OpenOutreachMigrationNodeEvent extends OpenOutreachMigrationNode {

  public function __construct() {
    parent::setProperties('event');
    parent::__construct();
    $this->description = t('Import event content.');
    $this->addFieldMapping('field_event_date', 'from');
    $this->addFieldMapping('field_event_date:to', 'to');
    $this->addFieldMapping('field_event_date:timezone', 'timezone');
    $this->addFieldMapping('field_event_date:rrule', 'rrule');
  }

  /**
   * Fill in missing date information and transform it into expected formats.
   */
  public function prepareRow($current_row) {
    static $from;

    // If no time was specified, set it.
    if (empty($current_row->from)) {
      if (empty($from)) {
        // Set beginning time to 9 AM on current day.
        $from = strtotime('09:00');
      }
      else {
        // Push two days forward for subsequent usage.
        $from += 172800;
      }
      $current_row->from = $from;
      // Set duration to two hours.
      $current_row->to = $from + 7200;
    }

    // If no timezone was specified, use the default.
    if (empty($current_row->timezone)) {
      $current_row->timezone = date_default_timezone();
    }

  }

  function csvcolumns() {
    $columns = parent::csvcolumns();
    $columns[] = array('from', 'From');
    $columns[] = array('to', 'To');
    $columns[] = array('timezone', 'Time zone');
    $columns[] = array('rrule', 'Repeat rule');
    return $columns;
  }

}

/**
 * Highlighted migration for nodes in OpenOutreach.
 */
class OpenOutreachMigrationNodeHighlighted extends OpenOutreachMigrationNode {

  public function __construct() {
    parent::setProperties('highlighted');
    $this->useTags = FALSE;
    parent::__construct();
    $this->description = t('Import highlighted content.');
    $this->addFieldMapping('field_kicker', 'kicker');
    $this->addFieldMapping('field_link', 'link');
  }

  function csvcolumns() {
    $columns = parent::csvcolumns();
    $columns[] = array('kicker', 'Kicker');
    $columns[] = array('link', 'Link');
    return $columns;
  }

}

/**
 * Section migration for nodes in OpenOutreach.
 */
class OpenOutreachMigrationNodeSection extends OpenOutreachMigrationNode {

  public function __construct() {
    parent::setProperties('section');
    parent::__construct();
    $this->description = t('Import section content.');
  }

  /**
   * Process book hierarchy.
   *
   * Create a new book if no 'parent' is specified. If there is a parent, load
   * it by name and set it as the parent.
   *
   * Based on code by mvc from https://drupal.org/node/763880.
   */
  public function complete($node, $row) {
    // Test if this is an update of an existing node, or an insert of a new
    // node. For simplicity, book hierarchy information is only set when
    // new nodes are inserted, and subsequent updates will only change other
    // fields for these nodes. In case of problems importing book hierarchy
    // information, it will be necessary to roll back and re-insert the nodes.
    $test_node = node_load($node->nid);
    if (!isset($test_node->book)) {
      // STEP 1: based on book_nodeapi(), case 'prepare'
      $node->book = array();
      if (empty($row->parent)) {
        // This is the base node of this book
        $node->book['bid'] = 'new';
      }
      else {
        $nid = db_select('node', 'n')
          ->condition('n.type', 'section')
          ->condition('n.title', $row->parent)
          ->fields('n', array('nid'))
          ->execute()
          ->fetchField();
        $parent_book_node = node_load($nid);
        $parent_book_node_mlid = $parent_book_node->book['mlid'];
        $parent = book_link_load($parent_book_node_mlid);
        if ($parent && $parent['access']) {
          $node->book['bid'] = $parent['bid'];
          $node->book['plid'] = $parent['mlid'];
          $node->book['menu_name'] = $parent['menu_name'];
        }
      }
      // Set defaults.
      $node->book += _book_link_defaults(!empty($node->nid) ? $node->nid : 'new');
      // Find the depth limit for the parent select.
      if (isset($node->book['bid']) && !isset($node->book['parent_depth_limit'])) {
        $node->book['parent_depth_limit'] = _book_parent_depth_limit($node->book);
      }
      // STEP 2: based on book_nodeapi(), case 'presave'
      // Always save a revision for non-administrators.
      if (!empty($node->book['bid']) && !user_access('administer nodes')) {
        $node->revision = 1;
      }
      // Make sure a new node gets a new menu link.
      $node->book['mlid'] = NULL;
      // STEP 3: based on book_nodeapi(), case 'insert'/'update'
      if ($node->book['bid'] == 'new') {
        // This is the base node of this book
        $node->book['bid'] = $node->nid;
      }
      $node->book['nid'] = $node->nid;
      $node->book['menu_name'] = book_menu_name($node->book['bid']);
      _book_update_outline($node);
      // STEP 4: save our work
      node_save($node);
    }
  }

  function csvcolumns() {
    $columns = parent::csvcolumns();
    $columns[] = array('parent', 'Parent');
    return $columns;
  }

}

/**
 * Migration for menu links in OpenOutreach.
 * Adapted from http://www.mezasefi.com/d/en/blogs/nobu/2012-10-02/
 * sample-migration-module-importing-menu-csv.html.
 */
abstract class OpenOutreachMigrationMenuLinks extends OpenOutreachMigration {
  public function __construct() {
    parent::__construct(MigrateGroup::getInstance('OpenOutreachMigrationMenuLinks'));
    $this->description = t('Creates sample menus.');

    $this->source = new MigrateSourceCSV($this->getImportFilePath(), $this->csvcolumns(), array('header_rows' => 1));

    $this->destination = new MigrateDestinationMenuLinks();

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'mlid' => array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'description' => 'ID of destination link',
        ),
      ),
      MigrateDestinationMenuLinks::getKeySchema()
    );

    $this->addFieldMapping('menu_name', 'menu_name');
    $this->addFieldMapping('plid', 'ref_parent')->sourceMigration($this->getMachineName());
    $this->addFieldMapping('link_path', 'path');
    $this->addFieldMapping('router_path')->defaultValue('node/%');
    $this->addFieldMapping('link_title', 'title');
    $this->addFieldMapping('link_description', 'description');
    $this->addFieldMapping('external')->defaultValue('0');
    $this->addFieldMapping('expanded')->defaultValue('0');
    $this->addFieldMapping('weight','weight');
    $this->addFieldMapping('customized')->defaultValue('1');
    $this->addFieldMapping('has_children')->defaultValue('0');
    // Will be overridden automatically.
    $this->addFieldMapping('depth')->defaultValue('1');

    $this->addUnmigratedDestinations(array('module', 'hidden','options','p1', 'p2', 'p3', 'p4', 'p5', 'p6', 'p7', 'p8', 'p9', 'updated'));

  }

  function prepareRow($row) {
    // Convert the alias to the node path.
    $node_path = drupal_lookup_path('source', $row->path);
    if (empty($node_path)) {
      return FALSE;
    }

    $row->path = $node_path;
  }

  /**
   * Creates a stub menu link, for when a child is imported before its parent.
   *
   * See http://drupal.org/node/1403044#comment-5790748.
   *
   * @param $migration
   *  The source migration
   * @return
   *  int $mlid on success
   *  FALSE on failure
   */
  protected function createStub($migration) {
    // If ref_parent is 0, that means it has no parent, so don't create a stub.
    if (!$migration->sourceValues->ref_parent) {
      return FALSE;
    }
    $menu_link = array (
      'menu_name' => $migration->sourceValues->menu_name,
      'link_path' => 'stub-path',
      'router_path' => 'stub-path',
      'link_title' => t('Stub title'),
    );
    $mlid = menu_link_save($menu_link);
    if ($mlid) {
      return array($mlid);
    }
    else {
      return FALSE;
    }
  }

  function csvcolumns() {
    $columns[] = array('mlid', 'Menu link ID');
    $columns[] = array('ref_parent', 'Parent menu link ID');
    $columns[] = array('menu_name', 'Menu name');
    $columns[] = array('path', 'Path');
    $columns[] = array('title', 'Title');
    $columns[] = array('description', 'Description');
    $columns[] = array('weight', 'Weight');
    return $columns;
  }

}

/**
 * Menu link migration for section in OpenOutreach.
 */
class OpenOutreachMigrationMenuLinksSection extends OpenOutreachMigrationMenuLinks {

  public function __construct() {
    parent::setProperties('section_menu');
    parent::__construct();
    // Require the nodes that are referenced.
    $this->dependencies[] = 'OpenOutreachMigrationNodeSection';
  }

}
