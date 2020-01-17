#include <stdlib.h>
#include "ctest.h"
#include "../graph.h"

CTEST_DATA(traversing_tests) {
  Graph* graph;
};

CTEST_SETUP(traversing_tests) {
    data->graph = malloc(sizeof(Graph));
    ragedb_clear(data->graph);
    ragedb_node_add_empty(data->graph, "Node", "one");
    ragedb_node_add_empty(data->graph, "Node", "two");
    ragedb_node_add_empty(data->graph, "Node", "three");
    ragedb_relationship_add_empty(data->graph, "FRIENDS", "Node", "one", "Node", "two");
    ragedb_relationship_add_empty(data->graph, "FRIENDS", "Node", "one", "Node", "three");
    ragedb_relationship_add_empty(data->graph, "FRIENDS", "Node", "three", "Node", "two");
}

CTEST_TEARDOWN(traversing_tests) {
    ragedb_close(data->graph);
}

CTEST2(traversing_tests, shouldGetNodeOutgoingRelationshipsFromKey) {
    int count = 0;
    array_ids_t* friends = ragedb_node_get_outgoing(data->graph, "FRIENDS", "Node", "one");
    for M_EACH(friend, *friends, array_ids_t) {
        count++;
    }
    ASSERT_EQUAL(2, count);
}

CTEST2(traversing_tests, shouldNotGetNodeOutgoingRelationshipsNodeNotThere) {
    int count = 0;
    array_ids_t* friends = ragedb_node_get_outgoing(data->graph, "FRIENDS", "Node", "not_there");
    for M_EACH(friend, *friends, array_ids_t) { count++; }
    ASSERT_EQUAL(0, count);
}

CTEST2(traversing_tests, shouldNotGetNodeOutgoingRelationshipsRelationshipTypeNotThere) {
    int count = 0;
    array_ids_t* friends = ragedb_node_get_outgoing(data->graph, "NOT_THERE", "Node", "one");
    for M_EACH(friend, *friends, array_ids_t) { count++; }
    ASSERT_EQUAL(0, count);
}

CTEST2(traversing_tests, shouldGetNodeOutgoingRelationshipsById) {
    int count = 0;
    array_ids_t* friends = ragedb_node_get_outgoing_by_id(data->graph, "FRIENDS", 0);
    for M_EACH(friend, *friends, array_ids_t) { count++; }
    ASSERT_EQUAL(2, count);

}

CTEST2(traversing_tests, shouldNotGetNodeOutgoingRelationshipsByIdTooHigh) {
    int count = 0;
    array_ids_t* friends = ragedb_node_get_outgoing_by_id(data->graph, "FRIENDS", 99);
    for M_EACH(friend, *friends, array_ids_t) {
        count++;
    }
    ASSERT_EQUAL(0, count);
}

CTEST2(traversing_tests, shouldNotGetNodeOutgoingRelationshipsByIdDeleted) {
    int count = 0;
    ragedb_node_remove_by_id(data->graph, 0);
    array_ids_t* friends = ragedb_node_get_outgoing_by_id(data->graph, "FRIENDS", 0);
    for M_EACH(friend, *friends, array_ids_t) {
                    count++;
                }
    ASSERT_EQUAL(0, count);
}

CTEST2(traversing_tests, shouldGetNodeIncomingRelationshipsFromKey) {
    int count = 0;
    array_ids_t* friends = ragedb_node_get_incoming(data->graph, "FRIENDS", "Node", "two");
    for M_EACH(friend, *friends, array_ids_t) { count++; }
    ASSERT_EQUAL(2, count);
}

CTEST2(traversing_tests, shouldNotGetNodeIncomingRelationshipsNodeNotThere) {
    int count = 0;
    array_ids_t* friends = ragedb_node_get_incoming(data->graph, "FRIENDS", "Node", "not_there");
    for M_EACH(friend, *friends, array_ids_t) { count++; }
    ASSERT_EQUAL(0, count);
}

CTEST2(traversing_tests, shouldNotGetNodeIncomingRelationshipsRelationshipTypeNotThere) {
    int count = 0;
    array_ids_t* friends = ragedb_node_get_incoming(data->graph, "NOT_THERE", "Node", "two");
    for M_EACH(friend, *friends, array_ids_t) { count++; }
    ASSERT_EQUAL(0, count);
}

CTEST2(traversing_tests, shouldGetNodeIncomingRelationshipsById) {
    int count = 0;
    array_ids_t* friends = ragedb_node_get_incoming_by_id(data->graph, "FRIENDS", 1);
    for M_EACH(friend, *friends, array_ids_t) { count++; }
    ASSERT_EQUAL(2, count);
}

CTEST2(traversing_tests, shouldNotGetNodeIncomingRelationshipsByIdTooHigh) {
    int count = 0;
    array_ids_t* friends = ragedb_node_get_incoming_by_id(data->graph, "FRIENDS", 99);
    for M_EACH(friend, *friends, array_ids_t) { count++; }
    ASSERT_EQUAL(0, count);
}

CTEST2(traversing_tests, shouldNotGetNodeIncomingRelationshipsByIdDeleted) {
    int count = 0;
    ragedb_node_remove_by_id(data->graph, 1);
    array_ids_t* friends = ragedb_node_get_incoming_by_id(data->graph, "FRIENDS", 1);
    for M_EACH(friend, *friends, array_ids_t) { count++; }
    ASSERT_EQUAL(0, count);
}
