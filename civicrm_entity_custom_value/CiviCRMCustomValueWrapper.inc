<?php

class CiviCRMCustomValueWrapper extends EntityDrupalWrapper {

  protected function setUpPropertyInfo(){
    // experiments here, need to figure out better handling of struct type, probably some custom getter and setter functions
    // but for now, some simple properties, on the fly....
    $properties['properties'] = array(
      'entity_table' => array(
        'label' => t('Entity Table'),
        'type' => 'text',
      ),
      'entity_id' => array(
        'label' => t('Entity ID'),
        'type' => 'integer',
      ),
    );
    if(civicrm_initialize(TRUE)) {
      if (isset($this->data->entity_table)) {
        $civicrm_metadata = civicrm_api3($this->data->entity_table, 'getfields');

      } else {
        $civicrm_metadata = civicrm_api3('Contact', 'getfields');

      }
      $fields = $civicrm_metadata['values'];
      foreach ($this->data as $key => $value) {
        if (stristr($key, 'custom')) {
          $key_array = explode('_', $key);
          $custom_id = $key_array[1];
          $custom_value_num = $key_array[2];
          $field_info = civicrm_entity_custom_values_get_field_info('civicrm_' . strtolower($this->data->entity_table), 'custom_' . $custom_id, $fields['custom_' . $custom_id]);
          unset($field_info['custom_' . $custom_id]['schema field']);
          $properties['properties']['custom_' . $custom_id . '_' . $custom_value_num] = $field_info['custom_' . $custom_id];
        }
      }
    }
    return $properties;
  }

  protected function setUp() {
    if(isset($this->data->entity_table)){
      $entity_table = $this->data->entity_table;

      if(isset($this->data->entity_id)) {
        $entity_id = $this->data->entity_id;
        $this->propertyInfo = $this->setUpPropertyInfo() + array('properties' => array());
      }
    }
    else {
      $this->propertyInfo = entity_get_property_info($this->type) + array('properties' => array());
    }
    dsm($this->propertyInfo);
    $info = $this->info + array('property info' => array(), 'bundle' => NULL);
    $this->propertyInfo['properties'] += $info['property info'];
    $this->bundle = $info['bundle'];
    $this->entityInfo = entity_get_info($this->type);
    if (isset($this->bundle)) {
      $this->spotBundleInfo(FALSE);
    }
  }


  /**
   * Returns whether $value is a valid value to set.
   */
  public function validate($value) {
    if (isset($value) && !entity_property_verify_data_type($value, $this->type, $this->propertyInfo)) {
      return FALSE;
    }
    // Only proceed with further checks if this is not a list item. If this is
    // a list item, the checks are performed on the list property level.
    if (isset($this->info['parent']) && $this->info['parent'] instanceof EntityListWrapper) {
      return TRUE;
    }
    if (!isset($value) && !empty($this->info['required'])) {
      // Do not allow NULL values if the property is required.
      return FALSE;
    }
    return !isset($this->info['validation callback']) || call_user_func($this->info['validation callback'], $value, $this->info);
  }
}