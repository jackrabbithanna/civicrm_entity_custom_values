<?php

/**
 * @file
 * CiviCRM Entity Custom Values module, Entity related classes.
 */

/**
 * Entity controller for civicrm_custom_value entities.
 */
class CiviCRMCustomValueEntityController extends EntityAPIController {

  protected function getEntityCustomValues($entity_id, $entity_table = 'Contact'){
    if(!civicrm_initialize(TRUE)){
      return array();
    }
    $values = array();
    $values['entity_id'] = $entity_id;
    $values['entity_table'] = $entity_table;
    $values['type'] = strtolower($entity_table);

    $result = civicrm_api3('CustomValue', 'get', array(
        'sequential' => 0,
        'entity_id' => $entity_id,
        'entity_table' => $entity_table,
    ));

    $custom_group_query = new EntityFieldQuery();
    $custom_groups = $custom_group_query->entityCondition('entity_type', 'civicrm_custom_group')
        ->execute();
    if(isset($custom_groups['civicrm_custom_group'])){
      $groups = entity_load('civicrm_custom_group', array_keys($custom_groups['civicrm_custom_group']));
    }

    if(!$result['is_error'] && $result['count']) {
      foreach($result['values'] as $key => $value){
        if(!isset($value['entity_table'])){
          $custom_field = entity_load_single('civicrm_custom_field', $key);
          $custom_group_id = $custom_field->custom_group_id;
          $values['groups'][$custom_group_id]['custom_group_id'] = $custom_group_id;
          $values['groups'][$custom_group_id]['name'] = $groups[$custom_group_id]->name;
          $values['groups'][$custom_group_id]['title'] = $groups[$custom_group_id]->title;

          foreach($value as $prop => $prop_value) {
            if(is_numeric($prop)){
              $values['groups'][$custom_group_id]['fields'][$key]['values'][$prop] = $prop_value;

              $values[$groups[$custom_group_id]->table_name . '_id'] = 1;
              $values['custom_'.$key.'_'.$prop] = $prop_value;
            }
            else {
              $values['groups'][$custom_group_id]['fields'][$key][$prop] = $prop_value;
            }
          }
          $values['groups'][$custom_group_id]['fields'][$key]['api_key'] = 'custom_' . $key;
        }
      }


      return $values;
    }
    else {
      return array();
    }
  }


  public function __construct($entityType) {
    $this->entityType = $entityType;
    $this->entityInfo = entity_get_info($entityType);
    $this->entityCache = array();
    $this->hookLoadArguments = array();
    $this->idKey = $this->entityInfo['entity keys']['id'];

    $this->revisionKey = FALSE;

    // Check if the entity type supports static caching of loaded entities.
    $this->cache = FALSE;
    $this->defaultRevisionKey = 'default_revision';

  }

  /**
   * Retrieves civicrm_custom_values.
   *
   * @return The results in a Traversable object.
   */
  public function query($ids, $conditions, $revision_id = FALSE) {
    if (!isset($conditions['entity_id'])) {
      return array();
    }

    $entity_id = $conditions['entity_id'];
    if(!isset($conditions['entity_table'])){
      $entity_table = 'Contact';
    }
    else {
      $entity_table = $conditions['entity_table'];
    }

    $entities = array();

    $entity = new $this->entityInfo['entity class']($this->getEntityCustomValues($entity_id, $entity_table));
    $entities[$entity_id] = $entity;

    return $entities;
  }

  public function load($ids = array(), $conditions = array()) {
    if (!isset($conditions['entity_id'])) {
      return array();
    }

    $entity_id = $conditions['entity_id'];
    if(!isset($conditions['entity_table'])){
      $entity_table = 'Contact';
    }
    else {
      $entity_table = $conditions['entity_table'];
    }

    $entity = new $this->entityInfo['entity class']($this->getEntityCustomValues($entity_id, $entity_table));
    $entities[$entity_id] = $entity;

    return $entities;
  }


  public function create(array $values = array()) {
    // @todo: Test and fix.
    $entity = new $this->entityInfo['entity class']();
    foreach ($values as $name => $value) {
      $entity->$name = $value;
    }
    $entity->is_new = TRUE;
    if(!isset($values)){

    }
    return $entity;
  }

  public function save($entity, DatabaseTransaction $transaction = NULL) {
    // @todo: Implement creating. Test and fix.

    $this->invoke($entity->is_new ? 'insert' : 'update', $entity);
    //$this->resetCache($entity->id);
  }

  public function delete($ids, DatabaseTransaction $transaction = NULL) {
    // @todo: Test and fix.

    $entities = array();
    //load the entity

    if (!$entities) {
      // Do nothing, in case invalid.
      return;
    }
    foreach ($entities as $entity) {

      $this->invoke('delete', $entity);
    }
   // $this->resetCache($ids);
  }

  public function buildContent($entity, $view_mode = 'full', $langcode = NULL, $content = array()) {


    return parent::buildContent($entity, $view_mode, $langcode, $content);
  }
}

/**
 * The civicrm_custom_value entity.
 */
class CiviCRMCustomValueEntity extends Entity {

  protected $entityType = 'civicrm_custom_value';

  protected function setUp() {
    $this->entityInfo = entity_get_info($this->entityType);
    $this->idKey = $this->entityInfo['entity keys']['id'];
    $this->nameKey = isset($this->entityInfo['entity keys']['name']) ? $this->entityInfo['entity keys']['name'] : $this->idKey;
    $this->statusKey = empty($info['entity keys']['status']) ? 'status' : $info['entity keys']['status'];
  }

  public function __construct($values) {
    parent::__construct($values, 'civicrm_custom_value');
  }


  /**
   * Returns info about the properties of this entity, as declared in hook_entity_property_info().
   *
   * @return array
   *   The properties defined for this entity.
   *
   * @see picasa_entity_property_info()
   */
  /*public function properties() {
    $info = entity_get_property_info($this->entityType());
    return $info['properties'];
  }*/


}

class CivicrmEntityCustomValueUIController extends EntityContentUIController {

  public $overviewPagerLimit = 25;

  /**
   * Always use the same civicrm_entity_form
   * @return mixed
   */
  public function hook_forms() {
    $forms = parent::hook_forms();
    foreach ($this->entityInfo['bundles'] as $bundle => $bundle_info) {
      if (isset($bundle_info['admin'])) {
        $form_id = (!isset($bundle) || $bundle == $this->entityType) ? $this->entityType . '_form' : $this->entityType . '_edit_' . $bundle . '_form';
       /* $forms[$form_id] = array(
            'callback' => 'civicrm_entity_custom_values_form',
        );*/
      }
    }
    return $forms;
  }
  public function overviewTable($conditions = array()) {
    /*
        $query = new EntityFieldQuery();
        $query->entityCondition('entity_type', $this->entityType);

        // Add all conditions to query.
        foreach ($conditions as $key => $value) {
          $query->propertyCondition($key, $value);
        }

        if ($this->overviewPagerLimit) {
          $query->pager($this->overviewPagerLimit);
        }

        $results = $query->execute();

        $ids = isset($results[$this->entityType]) ? array_keys($results[$this->entityType]) : array();
        $entities = $ids ? entity_load($this->entityType, $ids) : array();
        ksort($entities);

        $rows = array();
        foreach ($entities as $entity) {
          $rows[] = $this->overviewTableRow($conditions, entity_id($this->entityType, $entity), $entity);
        }

        $render = array(
          '#theme' => 'table',
          '#header' => $this->overviewTableHeaders($conditions, $rows),
          '#rows' => $rows,
          '#empty' => t('None.'),
        );
        return $render;
    */
  }


}
