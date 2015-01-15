<?php


/**
 * Given a node ID returns the nid of the connected node with the current language or the fallback nid
 * @param $nid
 * @return mixed
 */
function better_references_language_get_matching_nid($nid) {
  $translations_available = (module_exists('i18n') && module_exists('translation')) ? true : false;
  if (!$translations_available) return $nid;

  // Try to load a matching translation
  $user_language = i18n_language_context()->language;
  $possible_translations = translation_node_get_translations($nid);
  if ($possible_translations && isset($possible_translations[$user_language])) {
    return $possible_translations[$user_language]->nid;
  }
  //Return the current nid if we found nothing
  return $nid;
}

/**
 * Takes a node and a field name and returns a list of loaded nodes referenced in the field
 * @param $node
 * @param $field_name
 * @return array
 */
function better_references_get_node_list($node, $field_name) {
  $items = field_get_items('node', $node, $field_name);
  if (!$items)
    return array();

  $nodes = array();

  foreach ($items as $item) {
    $item_nid = better_references_language_get_matching_nid($item['target_id']);
    $item = node_load($item_nid);
    //Check if we have a loaded node and push it into the regions array
    if($item)
      array_push($nodes, $item);
  }
  return $nodes;
}

/**
 * Returns the rendered content of the given node_list
 * @param $field
 * @param null $parent
 * @param null $region_list
 */
function better_references_render($parent, $node_list, $parent_list=array()) {
  array_push($parent_list, $parent->type);
  $rendered_nodes = array();
  foreach($node_list as $node) {
    $node->better_references_parent_list = $parent_list;
    $theme_hook_suggestions = better_references_get_theme_hook_suggestions($node, $parent_list);
    if (isset($node->theme_hook_suggestions)) {
      $node->theme_hook_suggestions = array_merge($node->theme_hook_suggestions, $theme_hook_suggestions);
    } else {
      $node->theme_hook_suggestions = $theme_hook_suggestions;
    }
    $node_view = node_view($node);
    $rendered_nodes[] = drupal_render($node_view);
  }
  //Allow alteration of rendered data
  $data = array(
    'rendered_nodes' => $rendered_nodes
  );
  drupal_alter('better_references_nodes_rendered', $data);

  return implode('', $rendered_nodes);
}

/**
 * Returns the rendered content of the given node list as grid
 * @param $parent
 * @param $node_list
 * @param array $parent_list
 * @param array $grid_options
 */
function better_references_render_grid($parent, $node_list, $parent_list=array(), $grid_options=array()) {
  $divider_start = '<'.$grid_options['divider'].' class="'.implode(' ', $grid_options['classes']).'">';
  $divider_end = '</'.$grid_options['divider'].'>';
  $current_width = 0;
  $output = $divider_start;
  foreach($node_list as $node) {
    $content_width = field_get_items('node', $node, 'field_content_width');
    $content_width = $content_width ? $content_width[0]['value'] : $grid_options['max_width'];
    if($content_width+$current_width > $grid_options['max_width']) {
      $output .= $divider_end . $divider_start;
      $current_width = 0;
    }
    $output .= better_references_render($parent, array($node), $parent_list);
    $current_width += $content_width;
  }
  $output .= $divider_end;
  return $output;
}

/**
 * Takes a node and a list of parent types and creates theme_hook_suggestions for the node
 * @param $node
 * @param $parent_list
 * @return array
 */
function better_references_get_theme_hook_suggestions($node, $parent_list) {
  $suggestions = array();
  if ($parent_list) {
    $direct_parent = $parent_list[count($parent_list)-1];
    $suggestions[] = 'node__parent_'.$direct_parent.'__'.$node->type;
    if (count($parent_list > 1)) {
      $suggestion = 'node__';
      foreach($parent_list as $parent) {
        $suggestion .= 'parent_'.$parent.'__';
      }
      $suggestion .= $node->type;
      $suggestions[] = $suggestion;
    }
  }
  return $suggestions;
}