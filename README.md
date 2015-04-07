# drupal_better_references
Provides variables and functions for a better usability of the entityreference module

# How it works
The modules checks if the page has a connected node. If so, it checks for all fields prefixed with region_ . 
These fields should be entity_reference fields which contain references to other nodes. If so, the modules loads these nodes and puts them in the content_region variable which is made available in the page template.

You can also load entity_reference fields manually by using the better_references_get_node_list($node, $field_name) function. Just put in the node and the name of the reference field.
The function will return a list of nodes which can be put into the better_references_render($parent, $node_list, $parent_list=array()) function to render all nodes automatically.
