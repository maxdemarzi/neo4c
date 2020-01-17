#include <stdlib.h>
#include "ctest.h"
#include "../graph.h"

CTEST_DATA(node_degree_tests) {
  Graph* graph;
};

CTEST_SETUP(node_degree_tests) {
    data->graph = malloc(sizeof(Graph));
    ragedb_clear(data->graph);
    long long int node1Id = ragedb_node_add_empty(data->graph, "Node", "one");
    long long int node2Id = ragedb_node_add_empty(data->graph, "Node", "two");
}

CTEST_TEARDOWN(node_degree_tests) {
    ragedb_close(data->graph);
}

CTEST2(node_degree_tests, shouldGetNodeDegree) {
    ragedb_node_add_empty(data->graph, "Node", "four");
    ragedb_node_add_empty(data->graph, "Node", "five");
    ragedb_node_add_empty(data->graph, "Node", "six");
    ragedb_relationship_add_empty(data->graph, "FRIENDS", "Node", "four", "Node", "five");
    ragedb_relationship_add_empty(data->graph, "ENEMIES", "Node", "five", "Node", "four");
    long long int degree = ragedb_node_get_degree(data->graph, "Node", "four", ALL, NULL);
    ASSERT_EQUAL(2, degree);
}

CTEST2(node_degree_tests, shouldNotGetNodeDegreeNotThere) {
    ragedb_node_add_empty(data->graph, "Node", "four");
    ragedb_node_add_empty(data->graph, "Node", "five");
    ragedb_node_add_empty(data->graph, "Node", "six");
    ragedb_relationship_add_empty(data->graph, "FRIENDS", "Node", "four", "Node", "five");
    ragedb_relationship_add_empty(data->graph, "ENEMIES", "Node", "four", "Node", "five");
    long long int degree = ragedb_node_get_degree(data->graph, "Node", "seven", ALL, NULL);
    ASSERT_EQUAL(-1, degree);
}

CTEST2(node_degree_tests, shouldGetNodeDegreeIncoming) {
    ragedb_node_add_empty(data->graph, "Node", "four");
    ragedb_node_add_empty(data->graph, "Node", "five");
    ragedb_node_add_empty(data->graph, "Node", "six");
    ragedb_relationship_add_empty(data->graph, "FRIENDS", "Node", "four", "Node", "five");
    ragedb_relationship_add_empty(data->graph, "ENEMIES", "Node", "four", "Node", "five");
    long long int degree = ragedb_node_get_degree(data->graph, "Node", "four", IN, NULL);
    ASSERT_EQUAL(0, degree);
}

CTEST2(node_degree_tests, shouldGetNodeDegreeOutgoing) {
    ragedb_node_add_empty(data->graph, "Node", "four");
    ragedb_node_add_empty(data->graph, "Node", "five");
    ragedb_node_add_empty(data->graph, "Node", "six");
    ragedb_relationship_add_empty(data->graph, "FRIENDS", "Node", "four", "Node", "five");
    ragedb_relationship_add_empty(data->graph, "ENEMIES", "Node", "four", "Node", "five");
    long long int degree = ragedb_node_get_degree(data->graph, "Node", "four", OUT, NULL);
    ASSERT_EQUAL(2, degree);
}

CTEST2(node_degree_tests, shouldGetNodeDegreeAllSingleType) {
    ragedb_node_add_empty(data->graph, "Node", "four");
    ragedb_node_add_empty(data->graph, "Node", "five");
    ragedb_node_add_empty(data->graph, "Node", "six");
    ragedb_relationship_add_empty(data->graph, "FRIENDS", "Node", "four", "Node", "five");
    ragedb_relationship_add_empty(data->graph, "ENEMIES", "Node", "four", "Node", "five");

    char* types = "FRIENDS";
    long long int degree = ragedb_node_get_degree(data->graph, "Node", "four", ALL, types);
    ASSERT_EQUAL(1, degree);
}

CTEST2(node_degree_tests, shouldGetNodeDegreeIncomingSingleType) {
    ragedb_node_add_empty(data->graph, "Node", "four");
    ragedb_node_add_empty(data->graph, "Node", "five");
    ragedb_node_add_empty(data->graph, "Node", "six");
    ragedb_relationship_add_empty(data->graph, "FRIENDS", "Node", "four", "Node", "five");
    ragedb_relationship_add_empty(data->graph, "ENEMIES", "Node", "four", "Node", "five");

    char* types = "FRIENDS";
    long long int degree = ragedb_node_get_degree(data->graph, "Node", "four", IN, types);
    ASSERT_EQUAL(0, degree);
}

CTEST2(node_degree_tests, shouldGetNodeDegreeOutgoingSingleType) {
    ragedb_node_add_empty(data->graph, "Node", "four");
    ragedb_node_add_empty(data->graph, "Node", "five");
    ragedb_node_add_empty(data->graph, "Node", "six");
    ragedb_relationship_add_empty(data->graph, "FRIENDS", "Node", "four", "Node", "five");
    ragedb_relationship_add_empty(data->graph, "ENEMIES", "Node", "four", "Node", "five");

    char* types = "FRIENDS";
    long long int degree = ragedb_node_get_degree(data->graph, "Node", "four", OUT, types);
    ASSERT_EQUAL(1, degree);
}

CTEST2(node_degree_tests, shouldGetNodeDegreeMultipleTypes) {
    ragedb_node_add_empty(data->graph, "Node", "four");
    ragedb_node_add_empty(data->graph, "Node", "five");
    ragedb_node_add_empty(data->graph, "Node", "six");
    ragedb_relationship_add_empty(data->graph, "FRIENDS", "Node", "four", "Node", "five");
    ragedb_relationship_add_empty(data->graph, "ENEMIES", "Node", "four", "Node", "five");

    char* types2 = "FRIENDS,ENEMIES";
    long long int degree = ragedb_node_get_degree(data->graph, "Node", "four", ALL, types2);
    ASSERT_EQUAL(2, degree);
}