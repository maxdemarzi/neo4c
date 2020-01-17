#include <stdlib.h>
#include "ctest.h"
#include "../graph.h"

CTEST_DATA(relationship_type_test) {
  Graph* graph;
};

CTEST_SETUP(relationship_type_test) {
    data->graph = malloc(sizeof(Graph));
    neo4c_clear(data->graph);
    long long int node1Id = neo4c_node_add_empty(data->graph, "Node", "one");
    long long int node2Id = neo4c_node_add_empty(data->graph, "Node", "two");
}

CTEST_TEARDOWN(relationship_type_test) {
    neo4c_close(data->graph);
}

CTEST2(relationship_type_test, shouldGetRelationshipTypes) {
    long long int rel1Id = neo4c_relationship_add_empty(data->graph, "FOLLOWS", "Node", "one", "Node", "two");
    char* types[neo4c_relationship_types_get_count(data->graph)];
    neo4c_relationship_types_get(data->graph, types);
    ASSERT_STR("FOLLOWS", types[0]);
}

CTEST2(relationship_type_test, shouldGetRelationshipTypesCount) {
    long long int rel1Id = neo4c_relationship_add_empty(data->graph, "FOLLOWS", "Node", "one", "Node", "two");
    long long int rel2Id = neo4c_relationship_add_empty(data->graph, "FOLLOWS", "Node", "two", "Node", "one");
    long long int count = neo4c_relationship_types_get_count(data->graph);
    ASSERT_EQUAL(1, count);
}

CTEST2(relationship_type_test, shouldGetRelationshipTypeCount) {
    long long int rel1Id = neo4c_relationship_add_empty(data->graph, "FOLLOWS", "Node", "one", "Node", "two");
    long long int rel2Id = neo4c_relationship_add_empty(data->graph, "FOLLOWS", "Node", "two", "Node", "one");
    long long int count = neo4c_relationship_type_get_count(data->graph, "FOLLOWS");
    ASSERT_EQUAL(2, count);
}