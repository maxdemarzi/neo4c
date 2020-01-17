#ifndef NEO4C_RELATIONSHIP_H
#define NEO4C_RELATIONSHIP_H

#include "include/m-string.h"
#include "include/cJSON.h"

typedef struct rel {
  uint32_t id;
  uint32_t starting_node_id;
  uint32_t ending_node_id;
  string_t type;
  cJSON* properties;
} relationship;


static void relationship_init(relationship *obj)
{
    obj->id = -1;
    obj->starting_node_id = -1;
    obj->ending_node_id = -1;
    string_init(obj->type);
    obj->properties = NULL;
}

static void relationship_init_set(relationship *obj, const relationship src)
{
    obj->id = src.id;
    obj->starting_node_id = src.starting_node_id;
    obj->ending_node_id = src.ending_node_id;
    string_init_set(obj->type, src.type);
    if (src.properties)
        obj->properties = cJSON_Duplicate(src.properties, true);
    else
        obj->properties = NULL;
}

static void relationship_set(relationship *obj, const relationship src)
{
    obj->id = src.id;
    obj->starting_node_id = src.starting_node_id;
    obj->ending_node_id = src.ending_node_id;
    string_set(obj->type, src.type);
    if (obj->properties)
        cJSON_Delete(obj->properties);
    if (src.properties)
        obj->properties = cJSON_Duplicate(src.properties, true);
    else
        obj->properties = NULL;
}

static void relationship_clear(relationship *obj)
{
    string_clear(obj->type);
    if (obj->properties)
        cJSON_Delete(obj->properties);
}


static void relationship_set_id(relationship * obj, int32_t id) {
    obj->id = id;
}

static void relationship_set_properties(relationship * obj, cJSON * properties) {
    if (obj->properties)
        cJSON_Delete(obj->properties);

    obj->properties = cJSON_Duplicate(properties, true);
}

static void relationship_set_nodes_and_type(relationship * obj, int32_t in, int32_t out, string_t type) {
    obj->starting_node_id = in;
    obj->ending_node_id = out;
    string_set(obj->type, type);
}

#endif //NEO4C_RELATIONSHIP_H
