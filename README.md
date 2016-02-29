This module is for development of the CustomValue api entity, exposed as a Drupal Entity. 

I'm experimenting with a radical approach, you'll notice that I've placed a copy the Entity API module, which I'm modifying slightly...when I get it set what I want, then I'll propose a patch to the maintainer. 

So what's radical about it?

The CustomValue CiviCRM entity is not tied to any one particular database table.
There are no unique entity ids
The fields on each entity type will be different, and even on contacts there can be different configurations (different groups can attach to Individual, Household, Organization contact types). 
This means if I want to have one CustomValue entity type that can handle all possible custom field configurations for any civicrm entity, then I need to be able to set metadata on a PER ENTITY BASIS, that's per entity instance...

Is this possible?

The answer is yes, I think so. 

The reason the Entity API module needs to be patched, is to allow for developers to set a 'wrapper class' property in the hook_entity_info() implementation. 
See issue: https://www.drupal.org/node/2057429

I've also needed to update 
function entity_property_verify_data_type($data, $type) to accept property info to be passed to the function instead of coming from entity_get_property_info()

Upon examination of the code, you'll notice a class 
class CiviCRMCustomValueWrapper extends EntityDrupalWrapper

and here is where some of the "fun" is....

and stuff like this works: 
$custom = entity_load('civicrm_custom_value', array(), array('entity_id' => 52, 'entity_table' => 'Contact'));
dsm($custom);
$wrapper = entity_metadata_wrapper('civicrm_custom_value', $custom[52]);

dsm($wrapper->custom_10_1->value());

Entity data structure still a work in progress...but loading works, and saving and deleting won't be a problem...

Working right now to get this working with Drupal forms, then Rules, then we'll see what can or needs be done about rules...

