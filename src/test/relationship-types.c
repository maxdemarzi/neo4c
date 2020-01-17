#include <stdlib.h>
#include "ctest.h"
#include "../graph.h"

CTEST_DATA(relationship_type_test) {
  Graph* graph;
};

CTEST_SETUP(relationship_type_test) {
    data->graph = malloc(sizeof(Graph));
    ragedb_clear(data->graph);
    long long int node1Id = ragedb_node_add_empty(data->graph, "Node", "one");
    long long int node2Id = ragedb_node_add_empty(data->graph, "Node", "two");
}

CTEST_TEARDOWN(relationship_type_test) {
    ragedb_close(data->graph);
}

CTEST2(relationship_type_test, shouldGetRelationshipTypes) {
    long long int rel1Id = ragedb_relationship_add_empty(data->graph, "FOLLOWS", "Node", "one", "Node", "two");
    char* types[ragedb_relationship_types_get_count(data->graph)];
    ragedb_relationship_types_get(data->graph, types);
    ASSERT_STR("FOLLOWS", types[0]);
}

CTEST2(relationship_type_test, shouldGetRelationshipTypesCount) {
    long long int rel1Id = ragedb_relationship_add_empty(data->graph, "FOLLOWS", "Node", "one", "Node", "two");
    long long int rel2Id = ragedb_relationship_add_empty(data->graph, "FOLLOWS", "Node", "two", "Node", "one");
    long long int count = ragedb_relationship_types_get_count(data->graph);
    ASSERT_EQUAL(1, count);
}

CTEST2(relationship_type_test, shouldGetRelationshipTypeCount) {
    long long int rel1Id = ragedb_relationship_add_empty(data->graph, "FOLLOWS", "Node", "one", "Node", "two");
    long long int rel2Id = ragedb_relationship_add_empty(data->graph, "FOLLOWS", "Node", "two", "Node", "one");
    long long int count = ragedb_relationship_type_get_count(data->graph, "FOLLOWS");
    ASSERT_EQUAL(2, count);
}