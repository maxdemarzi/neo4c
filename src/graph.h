#ifndef RAGEDB_GRAPH_H
#define RAGEDB_GRAPH_H
#include <stdbool.h>

#include "direction.h"
#include "relationship.h"
#include "combination.h"
#include "include/m-array.h"
#include "include/m-dict.h"
#include "include/m-tuple.h"
#include "include/m-string.h"
#include "include/m-variant.h"
#include "include/cJSON.h"
#include "include/roaring.h"



DICT_DEF2(str_2_long, string_t, long long int)
#define M_OPL_str_2_long_t() DICT_OPLIST(str_2_long, STRING_OPLIST, M_DEFAULT_OPLIST)
DICT_DEF2(node_keys, string_t, str_2_long_t)
//ARRAY_DEF(array_str, string_t)
ARRAY_DEF(array_json, cJSON*)
#define M_OPL_array_json_t() ARRAY_OPLIST(array_json)
ARRAY_DEF(array_relationship, relationship, (INIT(relationship_init M_IPTR),SET(relationship_set M_IPTR),INIT_SET(relationship_init_set M_IPTR),CLEAR(relationship_clear M_IPTR)))
#define M_OPL_array_relationship_t() ARRAY_OPLIST(array_relationship, M_OPL_relationship_t())
ARRAY_DEF(array_long, long long int)
#define M_OPL_array_long_t() ARRAY_OPLIST(array_long)
ARRAY_DEF(array_float, float)
#define M_OPL_array_float_t() ARRAY_OPLIST(array_float)



#define COMBINATION_OPLIST (INIT(combination_init M_IPTR),SET(combination_set M_IPTR),INIT_SET(combination_init_set M_IPTR),CLEAR(combination_clear M_IPTR))
ARRAY_DEF(array_combination, combination, COMBINATION_OPLIST)
#define M_OPL_array_combination_t() ARRAY_OPLIST(array_combination, M_OPL_combination_t())

#define IDS_OPLIST (INIT(ids_init M_IPTR),SET(ids_set M_IPTR),INIT_SET(ids_init_set M_IPTR),CLEAR(ids_clear M_IPTR))
ARRAY_DEF (array_ids, ids, IDS_OPLIST)
#define M_OPL_array_ids_t() ARRAY_OPLIST(array_ids, M_OPL_ids_t())

VARIANT_DEF2(either,
        (id_combo, array_ids_t, ARRAY_OPLIST(array_ids, IDS_OPLIST)),
        (weighted_combo, array_combination_t, ARRAY_OPLIST(array_combination, COMBINATION_OPLIST) ) )

#define M_OPL_either_t() VARIANT_OPLIST(either, ARRAY_OPLIST(array_ids, IDS_OPLIST), ARRAY_OPLIST(array_combination, COMBINATION_OPLIST))

DICT_DEF2(str_2_either, string_t, STRING_OPLIST, either_t,  M_OPL_either_t())
#define M_OPL_str_2_either_t() DICT_OPLIST(str_2_either, STRING_OPLIST, M_OPL_either_t())
#define STR_2_EITHER_DICT_OPLIST DICT_OPLIST(str_2_either, STRING_OPLIST, ARRAY_OPLIST(either_t, M_OPL_either_t()))
ARRAY_DEF(relationships, str_2_either_t, STR_2_EITHER_DICT_OPLIST)
#define M_OPL_array_relationships_t() ARRAY_OPLIST(relationships)

typedef struct {
  node_keys_t node_keys;                  // "Index" to get node id by label:key
  array_json_t nodes;                     // Store of the properties of Nodes
  array_relationship_t relationships;     // Store of the properties of Relationships
  str_2_long_t relationship_counts;       // Keep track of the number of relationship counts by type
  relationships_t outgoing_relationships; // Outgoing relationships of each node
  relationships_t incoming_relationships; // Incoming relationships of each node
  roaring_bitmap_t* deleted_nodes;
  roaring_bitmap_t* deleted_relationships;
  array_long_t empty_list;
  array_ids_t empty_ids;
  array_combination_t empty_combination;
  combination rel_check_combination;
  ids rel_check_ids;

} Graph;

Graph* ragedb_open(const char* filename);
void ragedb_clear(Graph* graph);
void ragedb_close(Graph* graph);

// Relationship Types
long long int ragedb_relationship_types_get_count(Graph* graph);
long long int ragedb_relationship_type_get_count(Graph* graph, char* type);
void ragedb_relationship_types_get(Graph* graph, char** types);

// Nodes
long long int ragedb_node_get_id(Graph* graph, char* label, char* key);
long long int ragedb_node_add_empty(Graph* graph, char* label, char* key);
long long int ragedb_node_add(Graph* graph, char* label, char* key, char* properties);
cJSON ragedb_node_get(Graph* graph, char* label, char* key);
cJSON ragedb_node_get_by_id(Graph* graph, long long int id);
bool ragedb_node_remove(Graph* graph, char* label, char* key);
bool ragedb_node_remove_by_id(Graph* graph, long long int id);

// Properties
char* ragedb_property_get_string(cJSON* json, char* property);
long long int ragedb_property_get_integer(cJSON* json, char* property);
double ragedb_property_get_double(cJSON* json, char* property);
cJSON ragedb_property_get_object(cJSON* json, char* property);

// Node Properties
char* ragedb_node_property_get_string(Graph* graph, char* label, char* key, char* property);
long long int ragedb_node_property_get_integer(Graph* graph, char* label, char* key, char* property);
double ragedb_node_property_get_double(Graph* graph, char* label, char* key, char* property);
cJSON ragedb_node_property_get_object(Graph* json, char* label, char* key, char* property);

bool ragedb_node_property_set_string(Graph* graph, char* label, char* key, char* property, char* value);
bool ragedb_node_property_set_string_by_id(Graph* graph, long long int id, char* property, char* value);
bool ragedb_node_property_set_integer(Graph* graph, char* label, char* key, char* property, long long int value);
bool ragedb_node_property_set_integer_by_id(Graph* graph, long long int id, char* property, long long int value);
bool ragedb_node_property_set_double(Graph* graph, char* label, char* key, char* property, double value);
bool ragedb_node_property_set_double_by_id(Graph* graph, long long int id, char* property, double value);
bool ragedb_node_property_set_object(Graph* graph, char* label, char* key, char* property, char* value);
bool ragedb_node_property_set_object_by_id(Graph* graph, long long int id, char* property, char* value);

bool ragedb_node_property_delete(Graph* graph, char* label, char* key, char* property);
bool ragedb_node_property_delete_by_id(Graph* graph, long long int id, char* property);

bool ragedb_node_properties_set(Graph* graph, char* label, char* key, char* value);
bool ragedb_node_properties_set_by_id(Graph* graph, long long int id, char* value);
bool ragedb_node_properties_delete(Graph* graph, char* label, char* key);
bool ragedb_node_properties_delete_by_id(Graph* graph, long long int id);

// Relationships
long long int ragedb_relationship_add_empty(Graph* graph, char* type, char* label1, char* key1, char* label2, char* key2);
long long int ragedb_relationship_add(Graph* graph, char* type, char* label1, char* key1, char* label2, char* key2, char* properties);
long long int ragedb_relationship_add_with_weights(Graph* graph, char* type, char* label1, char* key1, char* label2, char* key2, char* properties, float weight1, float weight2);
bool ragedb_relationship_remove_by_id(Graph* graph, long long int id);
cJSON ragedb_relationship_get_by_id(Graph* graph, long long int id);

// Relationship Properties
char* ragedb_relationship_property_get_string(Graph* graph, long long int id, char* property);
long long int ragedb_relationship_property_get_integer(Graph* graph, long long int id, char* property);
double ragedb_relationship_property_get_double(Graph* graph, long long int id, char* property);
cJSON ragedb_relationship_property_get_object(Graph* graph, long long int id, char* property);
const char * ragedb_relationship_get_type(Graph* graph, long long int id);
uint32_t ragedb_relationship_get_starting_node_id(Graph* graph, long long int id);
uint32_t ragedb_relationship_get_ending_node_id(Graph* graph, long long int id);

bool ragedb_relationship_property_set_string(Graph* graph, long long int id, char* property, char* value);
bool ragedb_relationship_property_set_integer(Graph* graph, long long int id, char* property, long long int value);
bool ragedb_relationship_property_set_double(Graph* graph, long long int id, char* property, double value);
bool ragedb_relationship_property_set_object(Graph* graph, long long int id, char* property, char* value);

bool ragedb_relationship_property_delete(Graph* graph, long long int id, char* property);

bool ragedb_relationship_properties_set(Graph* graph, long long int id, char* value);
bool ragedb_relationship_properties_delete(Graph* graph, long long int id);

// Node Degree
// Comma delimited types ex. "FRIENDS,ENEMIES"
long long int ragedb_node_get_degree(Graph* graph, char* label, char* key, enum direction direction, char* types);

// Traversing
array_ids_t* ragedb_node_get_outgoing(Graph* graph, char* type, char* label, char* key);
array_ids_t* ragedb_node_get_outgoing_by_id(Graph* graph, char* type, long long int id);
array_ids_t* ragedb_node_get_incoming(Graph* graph, char* type, char* label, char* key);
array_ids_t* ragedb_node_get_incoming_by_id(Graph* graph, char* type, long long int id);

array_combination_t* ragedb_node_get_weighted_outgoing(Graph* graph, char* type, char* label, char* key);
array_combination_t* ragedb_node_get_weighted_outgoing_by_id(Graph* graph, char* type, long long int id);
array_combination_t* ragedb_node_get_weighted_incoming(Graph* graph, char* type, char* label, char* key);
array_combination_t* ragedb_node_get_weighted_incoming_by_id(Graph* graph, char* type, long long int id);


// Extras
// This is going to limit us to 4B nodes and 4B relationships
roaring_uint32_iterator_t* ragedb_all_get_node_ids(Graph* graph);
roaring_uint32_iterator_t* ragedb_all_get_node_ids_with_label(Graph* graph, char* label);
roaring_uint32_iterator_t* ragedb_all_get_relationship_ids(Graph* graph);
roaring_uint32_iterator_t* ragedb_all_get_relationship_ids_with_type(Graph* graph, char* type);

#endif //RAGEDB_GRAPH_H
