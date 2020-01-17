#ifndef RAGEDB_COMBINATION_H
#define RAGEDB_COMBINATION_H

#include <cxxabi.h>

typedef struct Ids {
  uint32_t node_id;
  uint32_t rel_id;
} ids;

static void ids_init(ids* obj) {
    obj->node_id = 0;
    obj->rel_id = 0;
}

static void ids_init_set(ids *obj, const ids src) {
    obj->node_id = src.node_id;
    obj->rel_id = src.rel_id;
}

static void ids_set(ids *obj, const ids src) {
    obj->node_id = src.node_id;
    obj->rel_id = src.rel_id;
}

static void ids_clear(ids *obj) {

}

typedef struct Combination {
  uint32_t node_id;
  uint32_t rel_id;
  float weight1;
  float weight2;
} combination;

static void combination_init(combination* obj) {
    obj->node_id = 0;
    obj->rel_id = 0;
    obj->weight1 = -1.0f;
    obj->weight2 = -1.0f;
}

static void combination_init_set(combination *obj, const combination src) {
    obj->node_id = src.node_id;
    obj->rel_id = src.rel_id;
    obj->weight1 = src.weight1;
    obj->weight2 = src.weight2;
}

static void combination_set(combination *obj, const combination src) {
    obj->node_id = src.node_id;
    obj->rel_id = src.rel_id;
    obj->weight1 = src.weight1;
    obj->weight2 = src.weight2;
}

static void combination_clear(combination *obj) {

}

#endif //RAGEDB_COMBINATION_H
