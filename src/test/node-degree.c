#include <stdlib.h>
#include "ctest.h"
#include "../graph.h"

CTEST_DATA(node_degree_tests) {
  Graph* graph;
};

CTEST_SETUP(node_degree_tests) {
    data->graph = malloc(sizeof(Graph));
    neo4c_clear(data->graph);
    long long int node1Id = neo4c_node_add_empty(data->graph, "Node", "one");
    long long int node2Id = neo4c_node_add_empty(data->graph, "Node", "two");
}

CTEST_TEARDOWN(node_degree_tests) {
    neo4c_close(data->graph);
}

CTEST2(node_degree_tests, shouldGetNodeDegree) {
    neo4c_node_add_empty(data->graph, "Node", "four");
    neo4c_node_add_empty(data->graph, "Node", "five");
    neo4c_node_add_empty(data->graph, "Node", "six");
    neo4c_relationship_add_empty(data->graph, "FRIENDS", "Node", "four", "Node", "five");
    neo4c_relationship_add_empty(data->graph, "ENEMIES", "Node", "five", "Node", "four");
    long long int degree = neo4c_node_get_degree(data->graph, "Node", "four", ALL, NULL);
    ASSERT_EQUAL(2, degree);
}

CTEST2(node_degree_tests, shouldNotGetNodeDegreeNotThere) {
    neo4c_node_add_empty(data->graph, "Node", "four");
    neo4c_node_add_empty(data->graph, "Node", "five");
    neo4c_node_add_empty(data->graph, "Node", "six");
    neo4c_relationship_add_empty(data->graph, "FRIENDS", "Node", "four", "Node", "five");
    neo4c_relationship_add_empty(data->graph, "ENEMIES", "Node", "four", "Node", "five");
    long long int degree = neo4c_node_get_degree(data->graph, "Node", "seven", ALL, NULL);
    ASSERT_EQUAL(-1, degree);
}

CTEST2(node_degree_tests, shouldGetNodeDegreeIncoming) {
    neo4c_node_add_empty(data->graph, "Node", "four");
    neo4c_node_add_empty(data->graph, "Node", "five");
    neo4c_node_add_empty(data->graph, "Node", "six");
    neo4c_relationship_add_empty(data->graph, "FRIENDS", "Node", "four", "Node", "five");
    neo4c_relationship_add_empty(data->graph, "ENEMIES", "Node", "four", "Node", "five");
    long long int degree = neo4c_node_get_degree(data->graph, "Node", "four", IN, NULL);
    ASSERT_EQUAL(0, degree);
}

CTEST2(node_degree_tests, shouldGetNodeDegreeOutgoing) {
    neo4c_node_add_empty(data->graph, "Node", "four");
    neo4c_node_add_empty(data->graph, "Node", "five");
    neo4c_node_add_empty(data->graph, "Node", "six");
    neo4c_relationship_add_empty(data->graph, "FRIENDS", "Node", "four", "Node", "five");
    neo4c_relationship_add_empty(data->graph, "ENEMIES", "Node", "four", "Node", "five");
    long long int degree = neo4c_node_get_degree(data->graph, "Node", "four", OUT, NULL);
    ASSERT_EQUAL(2, degree);
}

CTEST2(node_degree_tests, shouldGetNodeDegreeAllSingleType) {
    neo4c_node_add_empty(data->graph, "Node", "four");
    neo4c_node_add_empty(data->graph, "Node", "five");
    neo4c_node_add_empty(data->graph, "Node", "six");
    neo4c_relationship_add_empty(data->graph, "FRIENDS", "Node", "four", "Node", "five");
    neo4c_relationship_add_empty(data->graph, "ENEMIES", "Node", "four", "Node", "five");

    char* types = "FRIENDS";
    long long int degree = neo4c_node_get_degree(data->graph, "Node", "four", ALL, types);
    ASSERT_EQUAL(1, degree);
}

CTEST2(node_degree_tests, shouldGetNodeDegreeIncomingSingleType) {
    neo4c_node_add_empty(data->graph, "Node", "four");
    neo4c_node_add_empty(data->graph, "Node", "five");
    neo4c_node_add_empty(data->graph, "Node", "six");
    neo4c_relationship_add_empty(data->graph, "FRIENDS", "Node", "four", "Node", "five");
    neo4c_relationship_add_empty(data->graph, "ENEMIES", "Node", "four", "Node", "five");

    char* types = "FRIENDS";
    long long int degree = neo4c_node_get_degree(data->graph, "Node", "four", IN, types);
    ASSERT_EQUAL(0, degree);
}

CTEST2(node_degree_tests, shouldGetNodeDegreeOutgoingSingleType) {
    neo4c_node_add_empty(data->graph, "Node", "four");
    neo4c_node_add_empty(data->graph, "Node", "five");
    neo4c_node_add_empty(data->graph, "Node", "six");
    neo4c_relationship_add_empty(data->graph, "FRIENDS", "Node", "four", "Node", "five");
    neo4c_relationship_add_empty(data->graph, "ENEMIES", "Node", "four", "Node", "five");

    char* types = "FRIENDS";
    long long int degree = neo4c_node_get_degree(data->graph, "Node", "four", OUT, types);
    ASSERT_EQUAL(1, degree);
}

CTEST2(node_degree_tests, shouldGetNodeDegreeMultipleTypes) {
    neo4c_node_add_empty(data->graph, "Node", "four");
    neo4c_node_add_empty(data->graph, "Node", "five");
    neo4c_node_add_empty(data->graph, "Node", "six");
    neo4c_relationship_add_empty(data->graph, "FRIENDS", "Node", "four", "Node", "five");
    neo4c_relationship_add_empty(data->graph, "ENEMIES", "Node", "four", "Node", "five");

    char* types2 = "FRIENDS,ENEMIES";
    long long int degree = neo4c_node_get_degree(data->graph, "Node", "four", ALL, types2);
    ASSERT_EQUAL(2, degree);
}