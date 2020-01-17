#include <float.h>
#include "graph.h"

//Graph
Graph* ragedb_open(const char* filename) {
    Graph*graph = malloc(sizeof(Graph));

    return graph;
}

void ragedb_clear(Graph* graph) {
    node_keys_init(graph->node_keys);
    array_json_init(graph->nodes);
    array_relationship_init(graph->relationships);
    str_2_long_init(graph->relationship_counts);
    relationships_init(graph->outgoing_relationships);
    relationships_init(graph->incoming_relationships);
    graph->deleted_nodes = roaring_bitmap_create();
    graph->deleted_relationships = roaring_bitmap_create();
    array_long_init(graph->empty_list);
    array_ids_init(graph->empty_ids);
    array_combination_init(graph->empty_combination);
    combination_init(&graph->rel_check_combination);
    ids_init(&graph->rel_check_ids);
}

// Relationship Types
long long int ragedb_relationship_types_get_count(Graph* graph) {
    long long int size = str_2_long_size(graph->relationship_counts);
    return size;
}

long long int ragedb_relationship_type_get_count(Graph* graph, char* type) {
    // Initialize Type
    string_t type_string;
    string_init(type_string);
    string_set_str(type_string, type);
    long long int* size = str_2_long_get(graph->relationship_counts, type_string);
    return *size;
}

void ragedb_relationship_types_get(Graph* graph, char** types) {
    int count = 0;
    str_2_long_it_t it;

    for (str_2_long_it(it, graph->relationship_counts)     /* Start iterator on first element */
            ; !str_2_long_end_p(it)      /* Until the end is not reached */
            ; str_2_long_next(it)) {     /* Set the iterator to the next element*/
        str_2_long_cref(it);
        struct str_2_long_pair_s pair = *str_2_long_ref(it);
        types[count++] = (char*) pair.key; /* Get a reference to the underlying */
    }
}

//Nodes
long long int ragedb_node_get_id(Graph* graph, char* label, char* key) {
    // Initialize Label and Key
    string_t label_string;
    string_init(label_string);
    string_set_str(label_string, label);

    string_t key_string;
    string_init(key_string);
    string_set_str(key_string, key);

    str_2_long_t* node_key = node_keys_get(graph->node_keys, label_string);
    if (node_key==NULL) {
        return -1;
    }
    else {
        long long int* nodeId = str_2_long_get(*node_key, key_string);
        if (nodeId==NULL) {
            return -1;
        }
        else {
            return *nodeId;
        }
    }
}

long long int ragedb_node_add_empty(Graph* graph, char* label, char* key) {
   return ragedb_node_add(graph, label, key, "");
}

long long int ragedb_node_add(Graph* graph, char* label, char* key, char* properties) {
    // Initialize Label and Key
    string_t label_string;
    string_init(label_string);
    string_set_str(label_string, label);

    string_t key_string;
    string_init(key_string);
    string_set_str(key_string, key);

    // Check if Label already exists
    if (node_keys_get(graph->node_keys, label_string)==NULL) {
        str_2_long_t node_key;
        str_2_long_init(node_key);
        node_keys_set_at(graph->node_keys, label_string, node_key);
    }
    str_2_long_t* node_key = node_keys_get(graph->node_keys, label_string);

    // Check if key already exists
    long long int nodeId;
    if (str_2_long_get(*node_key, key_string)!=NULL) {
        return -1;
    }
    else {

        cJSON* json = cJSON_Parse(properties);

        if (json==NULL) {
            json = cJSON_CreateObject();
        }
        // Set Metadata properties
        cJSON_AddItemToObject(json, "~label", cJSON_CreateString(label));
        cJSON_AddItemToObject(json, "~key", cJSON_CreateString(key));

        // If we have deleted nodes, fill in the space by adding the new node here
        if (roaring_bitmap_is_empty(graph->deleted_nodes)) {
            nodeId = (long long int) array_json_size(graph->nodes);
            cJSON_AddItemToObject(json, "~id", cJSON_CreateNumber((double) nodeId));
            array_json_push_back(graph->nodes, json);
            // Initialize Relationship pairs
            str_2_either_t outgoing;
            str_2_either_init(outgoing);
            relationships_push_back(graph->outgoing_relationships, outgoing);
            str_2_either_t incoming;
            str_2_either_init(incoming);
            relationships_push_back(graph->incoming_relationships, incoming);
            str_2_long_set_at(*node_key, key_string, nodeId);
        } else {
            roaring_uint32_iterator_t *  i = roaring_create_iterator(graph->deleted_nodes);
            while(i->has_value) {
                nodeId = i->current_value;
                break;
            }
            cJSON_AddItemToObject(json, "~id", cJSON_CreateNumber((double) nodeId));
            array_json_push_at(graph->nodes, nodeId, json);
            // Initialize Relationship pairs
            str_2_either_t outgoing;
            str_2_either_init(outgoing);
            relationships_set_at(graph->outgoing_relationships, nodeId, outgoing);
            str_2_either_t incoming;
            str_2_either_init(incoming);
            relationships_set_at(graph->incoming_relationships, nodeId, incoming);
            str_2_long_set_at(*node_key, key_string, nodeId);
            // Remove created nodeId from deleted nodes
            roaring_bitmap_remove(graph->deleted_nodes, nodeId);
        }
        return nodeId;
    }

}

cJSON ragedb_node_get(Graph* graph, char* label, char* key) {
    long long int nodeId = ragedb_node_get_id(graph, label, key);
    if (nodeId==-1) {
        return *cJSON_CreateNull();
    }
    else {
        return ragedb_node_get_by_id(graph, nodeId);
    }
}

cJSON ragedb_node_get_by_id(Graph* graph, long long int id) {
    return **array_json_get_at(graph->nodes, id);
}

cJSON ragedb_relationship_get_by_id(Graph* graph, long long int id) {
    return *array_relationship_get_at(graph->relationships, id)->properties;
}

bool ragedb_node_remove(Graph* graph, char* label, char* key) {
    long long int nodeId = ragedb_node_get_id(graph, label, key);
    if (nodeId==-1) {
        return false;
    }
    else {
        return ragedb_node_remove_by_id(graph, nodeId);
    }
}
bool ragedb_node_remove_by_id(Graph* graph, long long int id) {
    cJSON* properties = *array_json_get_at(graph->nodes, id);

    char* label = ragedb_property_get_string(properties, "~label");
    char* key = ragedb_property_get_string(properties, "~key");

    // Initialize Label and Key
    string_t label_string;
    string_init(label_string);
    string_set_str(label_string, label);

    string_t key_string;
    string_init(key_string);
    string_set_str(key_string, key);

    str_2_long_t* node_key = node_keys_get(graph->node_keys, label_string);
    str_2_long_erase(*node_key, key_string);

    cJSON_Delete(properties);

    roaring_bitmap_add(graph->deleted_nodes, id);

//    combination* rel_check_combination = malloc(sizeof(combination));
//    combination_init(rel_check_combination);
//    ids* rel_check_ids = malloc(sizeof(ids));
//    ids_init(rel_check_ids);

    // Remove Outgoing Relationships
    str_2_either_t* relationships_map = relationships_get(graph->outgoing_relationships, id);
    str_2_either_it_t it;

    for (str_2_either_it(it, *relationships_map); !str_2_either_end_p(it); str_2_either_next(it)) {
        struct str_2_either_pair_s* current = str_2_either_ref(it);
        // If it's unweighted
        if (either_id_combo_p(current->value)) {
            array_ids_t* id_combo = either_get_id_combo(current->value);
            size_t length = array_ids_size(*id_combo);
            for M_EACH(combo, *id_combo, array_ids_t) {
                        // Add the relationship to be recycled
                        roaring_bitmap_add(graph->deleted_relationships, combo->rel_id);

                        // Clear the relationship properties
                        cJSON* relationship_properties = array_relationship_get_at(graph->relationships,
                                combo->rel_id)->properties;
                        cJSON_Delete(relationship_properties);

                        // Remove relationship from other node
                        str_2_either_t* incoming_relationships = relationships_get(graph->incoming_relationships,
                                combo->node_id);
                        array_ids_t* existing = either_get_id_combo(*str_2_either_get(*incoming_relationships, current->key));
                        size_t existing_length = array_ids_size(*existing);

                        // Find the position of the relationship in the other node's relationship array
                        int position = 0;
                        for (int j = 0; j<existing_length; j++) {
                            ids* maybe = array_ids_get_at(*existing, j);
                            if (maybe->rel_id==combo->rel_id) {
                                position = j;
                                break;
                            }
                        }
                        array_ids_pop_at(&graph->rel_check_ids, *existing, position);
                    }

            // Update the relationship type counts
            long long int* count = str_2_long_get(graph->relationship_counts, current->key);
            (*count) -= length;
            str_2_long_set_at(graph->relationship_counts, current->key, *count);

        } else {
            array_combination_t* weighted_combo = either_get_weighted_combo(current->value);
            size_t length = array_combination_size(*weighted_combo);
            for M_EACH(combo, *weighted_combo, array_combination_t) {
                            // Add the relationship to be recycled
                            roaring_bitmap_add(graph->deleted_relationships, combo->rel_id);

                            // Clear the relationship properties
                            cJSON* relationship_properties = array_relationship_get_at(graph->relationships,
                                    combo->rel_id)->properties;
                            cJSON_Delete(relationship_properties);

                            // Remove relationship from other node
                            str_2_either_t* incoming_relationships = relationships_get(graph->incoming_relationships,
                                    combo->node_id);
                            array_combination_t* existing = either_get_weighted_combo(
                                    *str_2_either_get(*incoming_relationships, current->key));
                            size_t existing_length = array_combination_size(*existing);

                            // Find the position of the relationship in the other node's relationship array
                            int position = 0;
                            for (int j = 0; j<existing_length; j++) {
                                combination* maybe = array_combination_get_at(*existing, j);
                                if (maybe->rel_id==combo->rel_id) {
                                    position = j;
                                    break;
                                }
                            }
                            array_combination_pop_at(&graph->rel_check_combination, *existing, position);
                        }

            // Update the relationship type counts
            long long int* count = str_2_long_get(graph->relationship_counts, current->key);
            (*count) -= length;
            str_2_long_set_at(graph->relationship_counts, current->key, *count);
        }
    }

    // Remove Incoming Relationships
    relationships_map = relationships_get(graph->incoming_relationships, id);
    for (str_2_either_it(it, *relationships_map); !str_2_either_end_p(it); str_2_either_next(it)) {
        struct str_2_either_pair_s* current = str_2_either_ref(it);
        // If it's unweighted
        if (either_id_combo_p(current->value)) {
            array_ids_t* id_combo = either_get_id_combo(current->value);
            size_t length = array_ids_size(*id_combo);
            for M_EACH(combo, *id_combo, array_ids_t) {
                            // Add the relationship to be recycled
                            roaring_bitmap_add(graph->deleted_relationships, combo->rel_id);

                            // Clear the relationship properties
                            cJSON* relationship_properties = array_relationship_get_at(graph->relationships,
                                    combo->rel_id)->properties;
                            cJSON_Delete(relationship_properties);

                            // Remove relationship from other node
                            str_2_either_t* relationships = relationships_get(graph->outgoing_relationships,
                                    combo->node_id);

                            array_ids_t* existing = either_get_id_combo(*str_2_either_get(*relationships, current->key));

                            size_t existing_length = array_ids_size(*existing);

                            // Find the position of the relationship in the other node's relationship array
                            int position = 0;
                            for (int j = 0; j<existing_length; j++) {
                                ids* maybe = array_ids_get_at(*existing, j);
                                if (maybe->rel_id==combo->rel_id) {
                                    position = j;
                                    break;
                                }
                            }
                            array_ids_pop_at(&graph->rel_check_ids, *existing, position);
                        }

                // Update the relationship type counts
                long long int* count = str_2_long_get(graph->relationship_counts, current->key);
                (*count) -= length;
                str_2_long_set_at(graph->relationship_counts, current->key, *count);

        } else {
            array_combination_t* weighted_combo = either_get_weighted_combo(current->value);
            size_t length = array_combination_size(*weighted_combo);
            for M_EACH(combo, *weighted_combo, array_combination_t) {
                            // Add the relationship to be recycled
                            roaring_bitmap_add(graph->deleted_relationships, combo->rel_id);

                            // Clear the relationship properties
                            cJSON* relationship_properties = array_relationship_get_at(graph->relationships,
                                    combo->rel_id)->properties;
                            cJSON_Delete(relationship_properties);

                            // Remove relationship from other node
                            str_2_either_t* relationships = relationships_get(graph->outgoing_relationships,
                                    combo->node_id);
                            array_combination_t* existing = either_get_weighted_combo(*str_2_either_get(*relationships, current->key));
                            size_t existing_length = array_combination_size(*existing);

                            // Find the position of the relationship in the other node's relationship array
                            int position = 0;
                            for (int j = 0; j<existing_length; j++) {
                                combination* maybe = array_combination_get_at(*existing, j);
                                if (maybe->rel_id==combo->rel_id) {
                                    position = j;
                                    break;
                                }
                            }
                            array_combination_pop_at(&graph->rel_check_combination, *existing, position);
                        }

            // Update the relationship type counts
            long long int* count = str_2_long_get(graph->relationship_counts, current->key);
            (*count) -= length;
            str_2_long_set_at(graph->relationship_counts, current->key, *count);

        }
//        free(rel_check_combination);
//        free(rel_check_ids);
    }

    return true;
}

long long int ragedb_node_get_degree(Graph* graph, char* label, char* key, enum direction direction, char* types) {
    long long int nodeId = ragedb_node_get_id(graph, label, key);
    if (nodeId==-1) {
        return -1;
    }

    long long int count = 0;

    // If they want all the relationship types
    if (types==NULL) {
        if (direction!=IN) {
            str_2_either_t* relationships = relationships_get(graph->outgoing_relationships, nodeId);
            str_2_either_it_t it;
            for (str_2_either_it(it, *relationships); !str_2_either_end_p(it); str_2_either_next(it)) {
                struct str_2_either_pair_s* current = str_2_either_ref(it);
                  if (either_id_combo_p(current->value)) {
                    count += array_ids_size(*either_get_id_combo(current->value));
                 } else {
                      count += array_combination_size(*either_get_weighted_combo(current->value));
                  }
            }
        }
        if (direction!=OUT) {
            str_2_either_t* relationships = relationships_get(graph->incoming_relationships, nodeId);
            str_2_either_it_t it;
            for (str_2_either_it(it, *relationships); !str_2_either_end_p(it); str_2_either_next(it)) {
                struct str_2_either_pair_s* current = str_2_either_ref(it);
                if (either_id_combo_p(current->value)) {
                    count += array_ids_size(*either_get_id_combo(current->value));
                } else {
                    count += array_combination_size(*either_get_weighted_combo(current->value));
                }
            }
        }
    }
    else {
        // If they want specific relationship types
        // Initialize Type
        string_t type_string;
        string_init(type_string);

        char delimiter[] = ",";
        int init_size = strlen(types);
        char copy[init_size];
        strcpy(copy, types);

        char *ptr = strtok(copy, delimiter);
        while(ptr != NULL) {
            string_set_str(type_string, ptr);
            ptr = strtok(NULL, delimiter);

            if (direction!=IN) {
                str_2_either_t* relationships = relationships_get(graph->outgoing_relationships, nodeId);
                either_t* value = str_2_either_get_at(*relationships, type_string);
                if (!either_empty_p(*value)) {
                    if (either_id_combo_p(*value)) {
                        count += array_ids_size(*either_get_id_combo(*value));
                    }
                    else {
                        count += array_combination_size(*either_get_weighted_combo(*value));
                    }
                }
            }
            if (direction!=OUT) {
                str_2_either_t* relationships = relationships_get(graph->incoming_relationships, nodeId);
                either_t* value = str_2_either_get_at(*relationships, type_string);
                if (!either_empty_p(*value)) {
                    if (either_id_combo_p(*value)) {
                        count += array_ids_size(*either_get_id_combo(*value));
                    }
                    else {
                        count += array_combination_size(*either_get_weighted_combo(*value));
                    }
                }
            }
        }
    }
    return count;
}

long long int ragedb_relationship_add_empty(Graph* graph, char* type, char* label1, char* key1, char* label2, char* key2) {
    return ragedb_relationship_add(graph, type, label1, key1, label2, key2, "");
}

long long int ragedb_relationship_add(Graph* graph, char* type, char* label1, char* key1, char* label2, char* key2, char* properties) {
    // Get the node ids
    long long int node1Id = ragedb_node_get_id(graph, label1, key1);
    long long int node2Id = ragedb_node_get_id(graph, label2, key2);
    if (node1Id==-1 || node2Id==-1) {
        return -1;
    }

    // Initialize Type
    string_t type_string;
    string_init(type_string);
    string_set_str(type_string, type);

    // increase the total amount of relationships of that type
    if (str_2_long_get(graph->relationship_counts, type_string)==NULL) {
        str_2_long_set_at(graph->relationship_counts, type_string, 1);
    }
    else {
        long long int* count = str_2_long_get(graph->relationship_counts, type_string);
        (*count) += 1;
        str_2_long_set_at(graph->relationship_counts, type_string, *count);
    }

    // If we have deleted relationships, fill in the space by adding the new relationship here
    long long int relId;

    // Set the properties of the relationship
    cJSON* json = cJSON_Parse(properties);

    if (json==NULL) {
        json = cJSON_CreateObject();
    }

    relationship rel;
    relationship_init(&rel);
    relationship_set_nodes_and_type(&rel, node1Id, node2Id, type_string);
    relationship_set_properties(&rel, json);

    if (roaring_bitmap_is_empty(graph->deleted_relationships)) {
        relId = (long long int) array_relationship_size(graph->relationships);
        relationship_set_id(&rel, relId);
        array_relationship_push_back(graph->relationships, rel);

    } else {
        roaring_uint32_iterator_t *  i = roaring_create_iterator(graph->deleted_relationships);
        while(i->has_value) {
            relId = i->current_value;
            break;
        }
        relationship_set_id(&rel, relId);
        array_relationship_push_at(graph->relationships, relId, rel);
        roaring_bitmap_remove(graph->deleted_nodes, relId);
    }

    // Add the relationship pointers to both sides
    str_2_either_t* relationships = relationships_get(graph->outgoing_relationships, node1Id);
    ids combo;
    combo.node_id = node2Id;
    combo.rel_id = relId;
    if (str_2_either_get(*relationships, type_string)==NULL) {
        array_ids_t next;
        array_ids_init(next);
        array_ids_push_back(next, combo);
        either_t either;
        either_init(either);
        either_set_id_combo(either, next);
        str_2_either_set_at(*relationships, type_string, either);
    }
    else {
        array_ids_t* existing = either_get_id_combo(*str_2_either_get(*relationships, type_string));
        array_ids_push_back(*existing, combo);
    }

    relationships = relationships_get(graph->incoming_relationships, node2Id);
    ids reverse_combo;
    reverse_combo.node_id = node1Id;
    reverse_combo.rel_id = relId;
    if (str_2_either_get(*relationships, type_string)==NULL) {
        array_ids_t next;
        array_ids_init(next);
        array_ids_push_back(next, reverse_combo);
        either_t either;
        either_init(either);
        either_set_id_combo(either, next);
        str_2_either_set_at(*relationships, type_string, either);
    }
    else {
        array_ids_t* existing = either_get_id_combo(*str_2_either_get(*relationships, type_string));
        array_ids_push_back(*existing, reverse_combo);
    }
    return relId;
}

long long int ragedb_relationship_add_with_weights(Graph* graph, char* type, char* label1, char* key1, char* label2, char* key2, char* properties, float weight1, float weight2) {
    // Get the node ids
    long long int node1Id = ragedb_node_get_id(graph, label1, key1);
    long long int node2Id = ragedb_node_get_id(graph, label2, key2);
    if (node1Id==-1 || node2Id==-1) {
        return -1;
    }

    // Initialize Type
    string_t type_string;
    string_init(type_string);
    string_set_str(type_string, type);

    // increase the total amount of relationships of that type
    if (str_2_long_get(graph->relationship_counts, type_string)==NULL) {
        str_2_long_set_at(graph->relationship_counts, type_string, 1);
    }
    else {
        long long int* count = str_2_long_get(graph->relationship_counts, type_string);
        (*count) += 1;
        str_2_long_set_at(graph->relationship_counts, type_string, *count);
    }

    // If we have deleted relationships, fill in the space by adding the new relationship here
    long long int relId;

    // Set the properties of the relationship
    cJSON* json = cJSON_Parse(properties);

    if (json==NULL) {
        json = cJSON_CreateObject();
    }

    relationship rel;
    relationship_init(&rel);
    relationship_set_nodes_and_type(&rel, node1Id, node2Id, type_string);
    relationship_set_properties(&rel, json);

    if (roaring_bitmap_is_empty(graph->deleted_relationships)) {
        relId = (long long int) array_relationship_size(graph->relationships);
        relationship_set_id(&rel, relId);
        array_relationship_push_back(graph->relationships, rel);

    } else {
        roaring_uint32_iterator_t *  i = roaring_create_iterator(graph->deleted_relationships);
        while(i->has_value) {
            relId = i->current_value;
            break;
        }
        relationship_set_id(&rel, relId);
        array_relationship_push_at(graph->relationships, relId, rel);
        roaring_bitmap_remove(graph->deleted_nodes, relId);
    }

    // Add the relationship pointers to both sides
    str_2_either_t* relationships = relationships_get(graph->outgoing_relationships, node1Id);
    combination combo;
    combo.node_id = node2Id;
    combo.rel_id = relId;
    combo.weight1 = weight1;
    combo.weight2 = weight2;
    if (str_2_either_get(*relationships, type_string)==NULL) {
        array_combination_t next;
        array_combination_init(next);
        array_combination_push_back(next, combo);
        either_t either;
        either_init(either);
        either_set_weighted_combo(either, next);
        str_2_either_set_at(*relationships, type_string, either);
    }
    else {
        array_combination_t* existing = either_get_weighted_combo(*str_2_either_get(*relationships, type_string));
        array_combination_push_back(*existing, combo);
    }

    relationships = relationships_get(graph->incoming_relationships, node2Id);
    combination reverse_combo;
    reverse_combo.node_id = node1Id;
    reverse_combo.rel_id = relId;
    reverse_combo.weight1 = weight1;
    reverse_combo.weight2 = weight2;
    if (str_2_either_get(*relationships, type_string)==NULL) {
        array_combination_t next;
        array_combination_init(next);
        array_combination_push_back(next, reverse_combo);
        either_t either;
        either_init(either);
        either_set_weighted_combo(either, next);
        str_2_either_set_at(*relationships, type_string, either);
    }
    else {
        array_combination_t* existing = either_get_weighted_combo(*str_2_either_get(*relationships, type_string));
        array_combination_push_back(*existing, reverse_combo);
    }
    return relId;
}

bool ragedb_relationship_remove_by_id(Graph* graph, long long int id) {
    // Get the type, starting and ending node ids
    relationship *rel = array_relationship_get_at(graph->relationships, id);

    // Add to deleted relationships bitmap
    roaring_bitmap_add(graph->deleted_relationships, id);

    // Remove relationship from Node 1
    str_2_either_t* relationships_map = relationships_get(graph->outgoing_relationships, rel->starting_node_id);
    size_t length;
    if (either_id_combo_p(*str_2_either_get(*relationships_map, rel->type))) {
        array_ids_t* existing = either_get_id_combo(*str_2_either_get(*relationships_map, rel->type));
        length = array_ids_size(*existing);

        // Find the position of the relationship
        int position = 0;
        for (int j=0; j<length; j++) {
            ids* maybe = array_ids_get_at(*existing, j);
            if (maybe->rel_id == id) {
                position = j;
                break;
            }
        }

        // Remove the node and relationship ids
        array_ids_pop_at(&graph->rel_check_ids, *existing, position);

        // Remove relationship from Node 2
        relationships_map = relationships_get(graph->incoming_relationships, rel->ending_node_id);
        existing = either_get_id_combo(*str_2_either_get(*relationships_map, rel->type));

        length = array_ids_size(*existing);

        // Find the position of the relationship
        position = 0;
        for (int j=0; j<length; j++) {
            ids* maybe = array_ids_get_at(*existing, j);
            if (maybe->rel_id == id) {
                position = j;
                break;
            }
        }

        // Remove the node and relationship ids
        array_ids_pop_at(&graph->rel_check_ids, *existing, position);

    } else {
        array_combination_t* existing = either_get_weighted_combo(*str_2_either_get(*relationships_map, rel->type));
        length = array_combination_size(*existing);

        // Find the position of the relationship
        int position = 0;
        for (int j=0; j<length; j++) {
            combination* maybe = array_combination_get_at(*existing, j);
            if (maybe->rel_id == id) {
                position = j;
                break;
            }
        }

        // Remove the node and relationship ids
        array_combination_pop_at(&graph->rel_check_combination, *existing, position);

        // Remove relationship from Node 2
        relationships_map = relationships_get(graph->incoming_relationships, rel->ending_node_id);
        existing = either_get_weighted_combo(*str_2_either_get(*relationships_map, rel->type));

        length = array_combination_size(*existing);

        // Find the position of the relationship
        position = 0;
        for (int j=0; j<length; j++) {
            combination* maybe = array_combination_get_at(*existing, j);
            if (maybe->rel_id == id) {
                position = j;
                break;
            }
        }

        // Remove the node and relationship ids
        array_combination_pop_at(&graph->rel_check_combination, *existing, position);
    }


    // Update the relationship type counts
    long long int* count = str_2_long_get(graph->relationship_counts, rel->type);
    (*count) -= 1;
    str_2_long_set_at(graph->relationship_counts, rel->type, *count);

    // Clear the relationship
    relationship_clear(rel);

    return true;
}

//array_long_t* ragedb_node_get_outgoing_node_ids(Graph* graph, char* type, char* label, char* key) {
//    // Get the node ids
//    long long int nodeId = ragedb_node_get_id(graph, label, key);
//    if (nodeId==-1) {
//        return &graph->empty_list;
//    }
//    else {
//        return ragedb_node_get_outgoing_node_ids_by_id(graph, type, nodeId);
//    }
//}
//
//array_long_t* ragedb_node_get_outgoing_node_ids_by_id(Graph* graph, char* type, long long int id) {
//    // If node id is greater than number of nodes
//    long long int max_id = array_json_size(graph->nodes);
//    if (id > max_id) {
//        return &graph->empty_list;
//    }
//    // If node id has been deleted
//    if (roaring_bitmap_contains(graph->deleted_nodes, id)) {
//        return &graph->empty_list;
//    }
//
//    // Initialize Type
//    string_t type_string;
//    string_init(type_string);
//    string_set_str(type_string, type);
//
//    str_2_either_t* relationships = relationships_get(graph->outgoing_relationships, id);
//    array_combination_t* existing = str_2_either_get(*relationships, type_string);
//    string_clear(type_string);
//    if (existing==NULL) {
//        return &graph->empty_list;
//    }
//    array_long_t node_ids;
//    array_long_init(node_ids);
//    for M_EACH(combo, *existing, array_combination_t) {
//                    array_long_push_back(node_ids, combo->node_id);
//                }
//
//    return node_ids;
//}
//
//array_long_t* ragedb_node_get_incoming_node_ids(Graph* graph, char* type, char* label, char* key) {
//    // Get the node id
//    long long int nodeId = ragedb_node_get_id(graph, label, key);
//    // If not found return a empty list.
//    if (nodeId==-1) {
//        return &graph->empty_list;
//    }
//    else {
//        // Otherwise use node id to get list of node ids
//        return ragedb_node_get_incoming_node_ids_by_id(graph, type, nodeId);
//    }
//}
//
//array_long_t* ragedb_node_get_incoming_node_ids_by_id(Graph* graph, char* type, long long int id) {
//    // If node id is greater than number of nodes
//    long long int max_id = array_json_size(graph->nodes);
//    if (id > max_id) {
//        return &graph->empty_list;
//    }
//    // If node id has been deleted
//    if (roaring_bitmap_contains(graph->deleted_nodes, id)) {
//        return &graph->empty_list;
//    }
//
//    // Initialize Type
//    string_t type_string;
//    string_init(type_string);
//    string_set_str(type_string, type);
//
//    str_2_either_t* relationships = relationships_get(graph->incoming_relationships, id);
//    array_combination_t * existing = str_2_either_get(*relationships, type_string);
//    string_clear(type_string);
//    if (existing==NULL) {
//        return &graph->empty_list;
//    }
//
//    array_long_t node_ids;
//    array_long_init(node_ids);
//    for M_EACH(combo, *existing, array_combination_t) {
//        array_long_push_back(node_ids, combo->node_id);
//    }
//
//    return node_ids;
//}
//
//array_long_t* ragedb_node_get_outgoing_relationship_ids(Graph* graph, char* type, char* label, char* key) {
//    // Get the node id
//    long long int nodeId = ragedb_node_get_id(graph, label, key);
//    if (nodeId==-1) {
//        return &graph->empty_list;
//    }
//    else {
//        // Otherwise use node id to get list of relationship ids
//        return ragedb_node_get_outgoing_relationship_ids_by_id(graph, type, nodeId);
//    }
//}
//
//array_long_t* ragedb_node_get_outgoing_relationship_ids_by_id(Graph* graph, char* type, long long int id) {
//    // If node id is greater than number of nodes
//    long long int max_id = array_json_size(graph->nodes);
//    if (id > max_id) {
//        return &graph->empty_list;
//    }
//    // If node id has been deleted
//    if (roaring_bitmap_contains(graph->deleted_nodes, id)) {
//        return &graph->empty_list;
//    }
//
//    // Initialize Type
//    string_t type_string;
//    string_init(type_string);
//    string_set_str(type_string, type);
//
//    str_2_either_t* relationships = relationships_get(graph->outgoing_relationships, id);
//    array_combination_t * existing = str_2_either_get(*relationships, type_string);
//    string_clear(type_string);
//    if (existing==NULL) {
//        return &graph->empty_list;
//    }
//
//    array_long_t node_ids;
//    array_long_init(node_ids);
//    for M_EACH(combo, *existing, array_combination_t) {
//                    array_long_push_back(node_ids, combo->node_id);
//                }
//
//    return node_ids;
//}
//
//array_long_t* ragedb_node_get_incoming_relationship_ids(Graph* graph, char* type, char* label, char* key) {
//    // Get the node id
//    long long int nodeId = ragedb_node_get_id(graph, label, key);
//    // If not found return a empty list.
//    if (nodeId==-1) {
//        return &graph->empty_list;
//    }
//    else {
//        // Otherwise use node id to get list of relationship ids
//        return ragedb_node_get_incoming_relationship_ids_by_id(graph, type, nodeId);
//    }
//}
//
//array_long_t* ragedb_node_get_incoming_relationship_ids_by_id(Graph* graph, char* type, long long int id) {
//    // If node id is greater than number of nodes
//    long long int max_id = array_json_size(graph->nodes);
//    if (id > max_id) {
//        return &graph->empty_list;
//    }
//    // If node id has been deleted
//    if (roaring_bitmap_contains(graph->deleted_nodes, id)) {
//        return &graph->empty_list;
//    }
//
//    // Initialize Type
//    string_t type_string;
//    string_init(type_string);
//    string_set_str(type_string, type);
//
//    str_2_either_t* relationships = relationships_get(graph->incoming_relationships, id);
//    array_combination_t * existing = str_2_either_get(*relationships, type_string);
//    string_clear(type_string);
//    if (existing==NULL) {
//        return &graph->empty_list;
//    }
//
//    array_long_t node_ids;
//    array_long_init(node_ids);
//    for M_EACH(combo, *existing, array_combination_t) {
//                    array_long_push_back(node_ids, combo->rel_id);
//                }
//
//    return node_ids;
//}

array_ids_t* ragedb_node_get_outgoing(Graph* graph, char* type, char* label, char* key) {
    // Get the node id
    long long int nodeId = ragedb_node_get_id(graph, label, key);
    if (nodeId==-1) {
        return &graph->empty_ids;
    }
    else {
        // Otherwise use node id to get list of node and relationship ids
        return ragedb_node_get_outgoing_by_id(graph, type, nodeId);
    }
}

array_ids_t* ragedb_node_get_outgoing_by_id(Graph* graph, char* type, long long int id) {
    // If node id is greater than number of nodes
    long long int max_id = array_json_size(graph->nodes);
    if (id > max_id) {
        return &graph->empty_ids;
    }
    // If node id has been deleted
    if (roaring_bitmap_contains(graph->deleted_nodes, id)) {
        return &graph->empty_ids;
    }

    // Initialize Type
    string_t type_string;
    string_init(type_string);
    string_set_str(type_string, type);

    str_2_either_t* relationships = relationships_get(graph->outgoing_relationships, id);
    either_t* existing = str_2_either_get(*relationships, type_string);
    string_clear(type_string);
    if (existing==NULL) {
        return &graph->empty_ids;
    }
    return either_get_id_combo(*existing);
}

array_ids_t* ragedb_node_get_incoming(Graph* graph, char* type, char* label, char* key) {
    // Get the node id
    long long int nodeId = ragedb_node_get_id(graph, label, key);
    // If not found return a empty list.
    if (nodeId==-1) {
        return &graph->empty_ids;
    }
    else {
        // Otherwise use node id to get list of node and relationship ids
        return ragedb_node_get_incoming_by_id(graph, type, nodeId);
    }
}

array_ids_t* ragedb_node_get_incoming_by_id(Graph* graph, char* type, long long int id) {
    // If node id is greater than number of nodes
    long long int max_id = array_json_size(graph->nodes);
    if (id > max_id) {
        return &graph->empty_ids;
    }
    // If node id has been deleted
    if (roaring_bitmap_contains(graph->deleted_nodes, id)) {
        return &graph->empty_ids;
    }

    // Initialize Type
    string_t type_string;
    string_init(type_string);
    string_set_str(type_string, type);

    str_2_either_t* relationships = relationships_get(graph->incoming_relationships, id);
    either_t* existing = str_2_either_get(*relationships, type_string);
    string_clear(type_string);
    if (existing==NULL) {
        return &graph->empty_ids;
    }
    return either_get_id_combo(*existing);
}

array_combination_t* ragedb_node_get_weighted_outgoing(Graph* graph, char* type, char* label, char* key) {
    // Get the node id
    long long int nodeId = ragedb_node_get_id(graph, label, key);
    if (nodeId==-1) {
        return &graph->empty_combination;
    }
    else {
        // Otherwise use node id to get list of node and relationship ids
        return ragedb_node_get_weighted_outgoing_by_id(graph, type, nodeId);
    }
}

array_combination_t* ragedb_node_get_weighted_outgoing_by_id(Graph* graph, char* type, long long int id) {
    // If node id is greater than number of nodes
    long long int max_id = array_json_size(graph->nodes);
    if (id > max_id) {
        return &graph->empty_combination;
    }
    // If node id has been deleted
    if (roaring_bitmap_contains(graph->deleted_nodes, id)) {
        return &graph->empty_combination;
    }

    // Initialize Type
    string_t type_string;
    string_init(type_string);
    string_set_str(type_string, type);

    str_2_either_t* relationships = relationships_get(graph->outgoing_relationships, id);
    either_t* existing = str_2_either_get(*relationships, type_string);
    string_clear(type_string);
    if (existing==NULL) {
        return &graph->empty_combination;
    }
    return either_get_weighted_combo(*existing);
}

array_combination_t* ragedb_node_get_weighted_incoming(Graph* graph, char* type, char* label, char* key) {
    // Get the node id
    long long int nodeId = ragedb_node_get_id(graph, label, key);
    // If not found return a empty list.
    if (nodeId==-1) {
        return &graph->empty_combination;
    }
    else {
        // Otherwise use node id to get list of node and relationship ids
        return ragedb_node_get_weighted_incoming_by_id(graph, type, nodeId);
    }
}

array_combination_t* ragedb_node_get_weighted_incoming_by_id(Graph* graph, char* type, long long int id) {
    // If node id is greater than number of nodes
    long long int max_id = array_json_size(graph->nodes);
    if (id > max_id) {
        return &graph->empty_combination;
    }
    // If node id has been deleted
    if (roaring_bitmap_contains(graph->deleted_nodes, id)) {
        return &graph->empty_combination;
    }

    // Initialize Type
    string_t type_string;
    string_init(type_string);
    string_set_str(type_string, type);

    str_2_either_t* relationships = relationships_get(graph->incoming_relationships, id);
    either_t* existing = str_2_either_get(*relationships, type_string);
    string_clear(type_string);
    if (existing==NULL) {
        return &graph->empty_combination;
    }
    return either_get_weighted_combo(*existing);
}

// Node Properties
char* ragedb_node_property_get_string(Graph* graph, char* label, char* key, char* property) {
    long long int nodeId = ragedb_node_get_id(graph, label, key);
    if (nodeId==-1) {
        return NULL;
    }
    else {
        cJSON json = ragedb_node_get_by_id(graph, nodeId);
        return ragedb_property_get_string(&json, property);
    }
}

long long int ragedb_node_property_get_integer(Graph* graph, char* label, char* key, char* property) {
    long long int nodeId = ragedb_node_get_id(graph, label, key);
    if (nodeId==-1) {
        return INT_MIN;
    }
    else {
        cJSON json = ragedb_node_get_by_id(graph, nodeId);
        return ragedb_property_get_integer(&json, property);
    }
}

double ragedb_node_property_get_double(Graph* graph, char* label, char* key, char* property) {
    long long int nodeId = ragedb_node_get_id(graph, label, key);
    if (nodeId==-1) {
        return DBL_MIN;
    }
    else {
        cJSON json = ragedb_node_get_by_id(graph, nodeId);
        return ragedb_property_get_double(&json, property);
    }
}

cJSON ragedb_node_property_get_object(Graph* graph, char* label, char* key, char* property) {
    long long int nodeId = ragedb_node_get_id(graph, label, key);
    if (nodeId==-1) {
        cJSON *null = cJSON_CreateObject();
        return *null;
    }
    else {
        cJSON json = ragedb_node_get_by_id(graph, nodeId);
        return ragedb_property_get_object(&json, property);
    }
}

bool ragedb_node_property_set_string(Graph* graph, char* label, char* key, char* property, char* value) {
    long long int nodeId = ragedb_node_get_id(graph, label, key);
    if (nodeId==-1) {
        return false;
    } else {
        return ragedb_node_property_set_string_by_id(graph, nodeId, property, value);
    }
}

bool ragedb_node_property_set_string_by_id(Graph* graph, long long int id, char* property, char* value) {
    // If trying to change metadata
    if (strncmp(property, "~", 1) == 0) {
        return false;
    }
    cJSON* properties = *array_json_get_at(graph->nodes, id);
    cJSON *string = cJSON_CreateString(value);
    cJSON_ReplaceItemInObject(properties, property, string);
    return true;
}

bool ragedb_node_property_set_integer(Graph* graph, char* label, char* key, char* property, long long int value) {
    long long int nodeId = ragedb_node_get_id(graph, label, key);
    if (nodeId==-1) {
        return false;
    } else {
        return ragedb_node_property_set_integer_by_id(graph, nodeId, property, value);
    }
}

bool ragedb_node_property_set_integer_by_id(Graph* graph, long long int id, char* property, long long int value) {
    // If trying to change metadata
    if (strncmp(property, "~", 1) == 0) {
        return false;
    }
    cJSON* properties = *array_json_get_at(graph->nodes, id);
    cJSON *json = cJSON_CreateNumber(value);
    cJSON_ReplaceItemInObject(properties, property, json);
    return true;
}

bool ragedb_node_property_set_double(Graph* graph, char* label, char* key, char* property, double value) {
    long long int nodeId = ragedb_node_get_id(graph, label, key);
    if (nodeId==-1) {
        return false;
    } else {
        return ragedb_node_property_set_double_by_id(graph, nodeId, property, value);
    }
}

bool ragedb_node_property_set_double_by_id(Graph* graph, long long int id, char* property, double value) {
    // If trying to change metadata
    if (strncmp(property, "~", 1) == 0) {
        return false;
    }
    cJSON* properties = *array_json_get_at(graph->nodes, id);
    cJSON *json = cJSON_CreateNumber(value);
    cJSON_ReplaceItemInObject(properties, property, json);
    return true;
}

bool ragedb_node_property_set_object(Graph* graph, char* label, char* key, char* property, char* value) {
    long long int nodeId = ragedb_node_get_id(graph, label, key);
    if (nodeId==-1) {
        return false;
    } else {
        return ragedb_node_property_set_object_by_id(graph, nodeId, property, value);
    }
}

bool ragedb_node_property_set_object_by_id(Graph* graph, long long int id, char* property, char* value) {
    // If trying to change metadata
    if (strncmp(property, "~", 1) == 0) {
        return false;
    }
    cJSON* properties = *array_json_get_at(graph->nodes, id);
    cJSON *json = cJSON_Parse(value);
    cJSON_ReplaceItemInObject(properties, property, json);
    return true;
}

bool ragedb_node_property_delete(Graph* graph, char* label, char* key, char* property) {
    long long int nodeId = ragedb_node_get_id(graph, label, key);
    if (nodeId==-1) {
        return false;
    } else {
        return ragedb_node_property_delete_by_id(graph, nodeId, property);
    }
}

bool ragedb_node_property_delete_by_id(Graph* graph, long long int id, char* property) {
    // If trying to change metadata
    if (strncmp(property, "~", 1) == 0) {
        return false;
    }
    cJSON* properties = *array_json_get_at(graph->nodes, id);
    cJSON_DeleteItemFromObject(properties, property);
    return true;
}

bool ragedb_node_properties_set(Graph* graph, char* label, char* key, char* value) {
    long long int nodeId = ragedb_node_get_id(graph, label, key);
    if (nodeId==-1) {
        return false;
    } else {
        return ragedb_node_properties_set_by_id(graph, nodeId, value);
    }
}

bool ragedb_node_properties_set_by_id(Graph* graph, long long int id, char* value) {
    cJSON* properties = *array_json_get_at(graph->nodes, id);
    const cJSON* label = cJSON_GetObjectItem(properties, "~label");
    const cJSON* key = cJSON_GetObjectItem(properties, "~key");

    cJSON *json = cJSON_Parse(value);

    // Set Metadata properties
    cJSON_AddItemToObject(json, "~label", cJSON_CreateString(label->valuestring));
    cJSON_AddItemToObject(json, "~key", cJSON_CreateString(key->valuestring));

    cJSON_Delete(properties);
    array_json_set_at(graph->nodes, id, json);
    return true;
}

bool ragedb_node_properties_delete(Graph* graph, char* label, char* key) {
    long long int nodeId = ragedb_node_get_id(graph, label, key);
    if (nodeId==-1) {
        return false;
    } else {
        return ragedb_node_properties_delete_by_id(graph, nodeId);
    }
}

bool ragedb_node_properties_delete_by_id(Graph* graph, long long int id) {
    cJSON* properties = *array_json_get_at(graph->nodes, id);
    const cJSON* label = cJSON_GetObjectItem(properties, "~label");
    const cJSON* key = cJSON_GetObjectItem(properties, "~key");

    cJSON *json = cJSON_CreateObject();
    // Set Metadata properties
    cJSON_AddItemToObject(json, "~label", cJSON_CreateString(label->valuestring));
    cJSON_AddItemToObject(json, "~key", cJSON_CreateString(key->valuestring));

    cJSON_Delete(properties);
    array_json_set_at(graph->nodes, id, json);
    return true;
}

// Relationship Properties
char* ragedb_relationship_property_get_string(Graph* graph, long long int id, char* property) {
    // If relationship id is greater than number of relationships
    long long int max_id = array_relationship_size(graph->relationships);
    if (id > max_id) {
        return NULL;
    }
    // If relationship id has been deleted
    if (roaring_bitmap_contains(graph->deleted_relationships, id)) {
        return NULL;
    }

    cJSON json = ragedb_relationship_get_by_id(graph, id);
    return ragedb_property_get_string(&json, property);
}

long long int ragedb_relationship_property_get_integer(Graph* graph, long long int id, char* property) {
    // If relationship id is greater than number of relationships
    long long int max_id = array_relationship_size(graph->relationships);
    if (id > max_id) {
        return INT_MIN;
    }
    // If relationship id has been deleted
    if (roaring_bitmap_contains(graph->deleted_relationships, id)) {
        return INT_MIN;
    }

    cJSON json = ragedb_relationship_get_by_id(graph, id);
    return ragedb_property_get_integer(&json, property);
}

double ragedb_relationship_property_get_double(Graph* graph, long long int id, char* property) {
    // If relationship id is greater than number of relationships
    long long int max_id = array_relationship_size(graph->relationships);
    if (id > max_id) {
        return DBL_MIN;
    }
    // If relationship id has been deleted
    if (roaring_bitmap_contains(graph->deleted_relationships, id)) {
        return DBL_MIN;
    }

    cJSON json = ragedb_relationship_get_by_id(graph, id);
    return ragedb_property_get_double(&json, property);
}

cJSON ragedb_relationship_property_get_object(Graph* graph, long long int id, char* property) {
    // If relationship id is greater than number of relationships
    long long int max_id = array_relationship_size(graph->relationships);
    if (id > max_id) {
        cJSON *null = cJSON_CreateObject();
        return *null;
    }
    // If relationship id has been deleted
    if (roaring_bitmap_contains(graph->deleted_relationships, id)) {
        cJSON *null = cJSON_CreateObject();
        return *null;
    }
    cJSON json = ragedb_relationship_get_by_id(graph, id);
    return ragedb_property_get_object(&json, property);
}

const char * ragedb_relationship_get_type(Graph* graph, long long int id) {
    // If relationship id is greater than number of relationships
    long long int max_id = array_relationship_size(graph->relationships);
    if (id > max_id) {
        return NULL;
    }
    // If relationship id has been deleted
    if (roaring_bitmap_contains(graph->deleted_relationships, id)) {
        return NULL;
    }
    return string_get_cstr(array_relationship_get_at(graph->relationships, id)->type);
}

uint32_t ragedb_relationship_get_starting_node_id(Graph* graph, long long int id) {
    // If relationship id is greater than number of relationships
    long long int max_id = array_relationship_size(graph->relationships);
    if (id > max_id) {
        return -1;
    }
    // If relationship id has been deleted
    if (roaring_bitmap_contains(graph->deleted_relationships, id)) {
        return -1;
    }
    return array_relationship_get_at(graph->relationships, id)->starting_node_id;
}

uint32_t ragedb_relationship_get_ending_node_id(Graph* graph, long long int id){
    // If relationship id is greater than number of relationships
    long long int max_id = array_relationship_size(graph->relationships);
    if (id > max_id) {
        return -1;
    }
    // If relationship id has been deleted
    if (roaring_bitmap_contains(graph->deleted_relationships, id)) {
        return -1;
    }

    return array_relationship_get_at(graph->relationships, id)->ending_node_id;
}


bool ragedb_relationship_property_set_string(Graph* graph, long long int id, char* property, char* value) {
    // If relationship id is greater than number of relationships
    long long int max_id = array_relationship_size(graph->relationships);
    if (id > max_id) {
        return false;
    }
    // If relationship id has been deleted
    if (roaring_bitmap_contains(graph->deleted_relationships, id)) {
        return false;
    }
    // If trying to change metadata
    if (strncmp(property, "~", 1) == 0) {
        return false;
    }
    cJSON* properties = array_relationship_get_at(graph->relationships, id)->properties;
    cJSON *json = cJSON_CreateString(value);
    cJSON_ReplaceItemInObject(properties, property, json);
    return true;
}

bool ragedb_relationship_property_set_integer(Graph* graph, long long int id, char* property, long long int value) {
    // If relationship id is greater than number of relationships
    long long int max_id = array_relationship_size(graph->relationships);
    if (id > max_id) {
        return false;
    }
    // If relationship id has been deleted
    if (roaring_bitmap_contains(graph->deleted_relationships, id)) {
        return false;
    }
    // If trying to change metadata
    if (strncmp(property, "~", 1) == 0) {
        return false;
    }
    cJSON* properties = array_relationship_get_at(graph->relationships, id)->properties;
    cJSON *json = cJSON_CreateNumber(value);
    cJSON_ReplaceItemInObject(properties, property, json);
    return true;
}

bool ragedb_relationship_property_set_double(Graph* graph, long long int id, char* property, double value) {
    // If relationship id is greater than number of relationships
    long long int max_id = array_relationship_size(graph->relationships);
    if (id > max_id) {
        return false;
    }
    // If relationship id has been deleted
    if (roaring_bitmap_contains(graph->deleted_relationships, id)) {
        return false;
    }
    // If trying to change metadata
    if (strncmp(property, "~", 1) == 0) {
        return false;
    }
    cJSON* properties = array_relationship_get_at(graph->relationships, id)->properties;
    cJSON *json = cJSON_CreateNumber(value);
    cJSON_ReplaceItemInObject(properties, property, json);
    return true;
}

bool ragedb_relationship_property_set_object(Graph* graph, long long int id, char* property, char* value) {
    // If relationship id is greater than number of relationships
    long long int max_id = array_relationship_size(graph->relationships);
    if (id > max_id) {
        return false;
    }
    // If relationship id has been deleted
    if (roaring_bitmap_contains(graph->deleted_relationships, id)) {
        return false;
    }
    // If trying to change metadata
    if (strncmp(property, "~", 1) == 0) {
        return false;
    }
    cJSON* properties = array_relationship_get_at(graph->relationships, id)->properties;
    cJSON *json = cJSON_Parse(value);
    cJSON_ReplaceItemInObject(properties, property, json);
    return true;
}

bool ragedb_relationship_property_delete(Graph* graph, long long int id, char* property) {
    // If relationship id is greater than number of relationships
    long long int max_id = array_relationship_size(graph->relationships);
    if (id > max_id) {
        return false;
    }
    // If relationship id has been deleted
    if (roaring_bitmap_contains(graph->deleted_relationships, id)) {
        return false;
    }
    // If trying to change metadata
    if (strncmp(property, "~", 1) == 0) {
        return false;
    }
    cJSON* properties = array_relationship_get_at(graph->relationships, id)->properties;
    cJSON_DeleteItemFromObject(properties, property);
    return true;
}

bool ragedb_relationship_properties_set(Graph* graph, long long int id, char* value) {
    // If relationship id is greater than number of relationships
    long long int max_id = array_relationship_size(graph->relationships);
    if (id > max_id) {
        return false;
    }
    // If relationship id has been deleted
    if (roaring_bitmap_contains(graph->deleted_relationships, id)) {
        return false;
    }
    // Set properties

    relationship_set_properties(array_relationship_get_at(graph->relationships, id), cJSON_Parse(value));

    return true;
}

bool ragedb_relationship_properties_delete(Graph* graph, long long int id) {
    // If relationship id is greater than number of relationships
    long long int max_id = array_relationship_size(graph->relationships);
    if (id > max_id) {
        return false;
    }
    // If relationship id has been deleted
    if (roaring_bitmap_contains(graph->deleted_relationships, id)) {
        return false;
    }
    // Get Metadata properties
    cJSON* properties = array_relationship_get_at(graph->relationships, id)->properties;
    cJSON_Delete(properties);
    array_relationship_get_at(graph->relationships, id)->properties = cJSON_CreateObject();
    return true;
}

// JSON properties
cJSON ragedb_property_get_object(cJSON* json, char* property) {
    const cJSON* value = cJSON_GetObjectItem(json, property);
    if (value==NULL) {
        cJSON* null = cJSON_CreateObject();
        return *null;
    }
    return *value;
}

char* ragedb_property_get_string(cJSON* json, char* property) {
    const cJSON* value = cJSON_GetObjectItem(json, property);
    if(value == NULL) {
        return NULL;
    }
    return value->valuestring;
}

long long int ragedb_property_get_integer(cJSON* json, char* property) {
    const cJSON* value = cJSON_GetObjectItem(json, property);
    if(value == NULL) {
        return INT_MIN;
    }
    return value->valueint;
}

double ragedb_property_get_double(cJSON* json, char* property) {
    const cJSON* value = cJSON_GetObjectItem(json, property);
    if(value == NULL) {
        return DBL_MIN;
    }
    return value->valuedouble;
}

//Extra
roaring_uint32_iterator_t* ragedb_all_get_node_ids(Graph* graph) {
    // create a new empty bitmap
    roaring_bitmap_t *r1 = roaring_bitmap_create();
    // Get the max node id
    long long int max_id = array_json_size(graph->nodes);
    // Fill the bitmap from 0 to max node id
    roaring_bitmap_add_range(r1, 0, max_id);
    // Remove the deleted node ids
    roaring_uint32_iterator_t *  deleted = roaring_create_iterator(graph->deleted_nodes);
    while(deleted->has_value) {
        roaring_bitmap_remove(r1, deleted->current_value);
        roaring_advance_uint32_iterator(deleted);
    }
    roaring_uint32_iterator_t *  iterator = roaring_create_iterator(r1);
    return iterator;
}

roaring_uint32_iterator_t* ragedb_all_get_node_ids_with_label(Graph* graph, char* label){
    // Initialize Label
    string_t label_string;
    string_init(label_string);
    string_set_str(label_string, label);

    // create a new empty bitmap
    roaring_bitmap_t *r1 = roaring_bitmap_create();

    // Add the node ids of the label
    str_2_long_t* node_key = node_keys_get(graph->node_keys, label_string);
    str_2_long_it_t pair_it ;
    for(str_2_long_it(pair_it, *node_key); !str_2_long_end_p(pair_it); str_2_long_next(pair_it)) {
        struct str_2_long_pair_s* current = str_2_long_ref(pair_it);
        roaring_bitmap_add(r1, current->value);
    }

    roaring_uint32_iterator_t *  iterator = roaring_create_iterator(r1);
    return iterator;
}

roaring_uint32_iterator_t* ragedb_all_get_relationship_ids(Graph* graph) {
    // create a new empty bitmap
    roaring_bitmap_t *r1 = roaring_bitmap_create();
    // Get the max relationship id
    long long int max_id = array_relationship_size(graph->relationships);
    // Fill the bitmap from 0 to max relationship id
    roaring_bitmap_add_range(r1, 0, max_id);
    // Remove the deleted relationship ids
    roaring_uint32_iterator_t *  deleted = roaring_create_iterator(graph->deleted_relationships);
    while(deleted->has_value) {
        roaring_bitmap_remove(r1, deleted->current_value);
        roaring_advance_uint32_iterator(deleted);
    }
    roaring_uint32_iterator_t *  iterator = roaring_create_iterator(r1);
    return iterator;
}

roaring_uint32_iterator_t* ragedb_all_get_relationship_ids_with_type(Graph* graph, char* type){
    // Initialize Type
    string_t type_string;
    string_init(type_string);
    string_set_str(type_string, type);

    // create a new empty bitmap
    roaring_bitmap_t *r1 = roaring_bitmap_create();

    for M_EACH(relationships, graph->outgoing_relationships, array_relationships_t) {
                    either_t* list = str_2_either_get_at(*relationships, type_string);
                    if (!either_empty_p(*list)) {
                        if (either_id_combo_p(*list)) {
                            for M_EACH(combo, *either_get_id_combo(*list), array_ids_t) {
                                            roaring_bitmap_add(r1, combo->rel_id);
                                        }
                        }
                        else {
                            for M_EACH(combo, *either_get_weighted_combo(*list), array_combination_t) {
                                            roaring_bitmap_add(r1, combo->rel_id);
                                        }

                        }
                    }
    }

    roaring_uint32_iterator_t *  iterator = roaring_create_iterator(r1);
    return iterator;
}

void ragedb_close(Graph* graph) {
    roaring_bitmap_free(graph->deleted_nodes);
    roaring_bitmap_free(graph->deleted_relationships);
    free(graph);
}


