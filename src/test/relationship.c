#include <stdlib.h>
#include "ctest.h"
#include "../graph.h"

CTEST_DATA(relationship_tests) {
  Graph* graph;
};

CTEST_SETUP(relationship_tests) {
    data->graph = malloc(sizeof(Graph));
    ragedb_clear(data->graph);
    ragedb_node_add_empty(data->graph, "Node", "empty");
    ragedb_node_add(data->graph, "Node", "existing", "{ \"name\":\"max\", \"email\":\"maxdemarzi@hotmail.com\" }");
}

CTEST_TEARDOWN(relationship_tests) {
    ragedb_close(data->graph);
}

CTEST2(relationship_tests, shouldAddRelationship) {
    long long int rel1Id = ragedb_relationship_add_empty(data->graph, "FRIENDS", "Node", "empty", "Node", "existing");
    ASSERT_NOT_EQUAL(-1, rel1Id);
}

CTEST2(relationship_tests, shouldAddRelationshipAfterDeletingOne) {
    long long int rel1Id = ragedb_relationship_add_empty(data->graph, "FRIENDS", "Node", "empty", "Node", "existing");
    ASSERT_NOT_EQUAL(-1, rel1Id);
    ragedb_relationship_remove_by_id(data->graph, rel1Id);
    long long int rel2Id = ragedb_relationship_add_empty(data->graph, "FRIENDS", "Node", "empty", "Node", "existing");
    ASSERT_NOT_EQUAL(-1, rel2Id);
    ASSERT_EQUAL(rel2Id, rel1Id);
}

CTEST2(relationship_tests, shouldNotAddRelationshipNodeNotThere) {
    long long int rel1Id = ragedb_relationship_add_empty(data->graph, "FRIENDS", "Node", "not_there", "Node", "existing");
    ASSERT_EQUAL(-1, rel1Id);
}

CTEST2(relationship_tests, shouldAddRelationshipWithProperties) {
    long long int rel1Id = ragedb_relationship_add(data->graph, "FRIENDS", "Node", "empty", "Node", "existing", "{\"stars\":5}");
    ASSERT_NOT_EQUAL(-1, rel1Id);
    ASSERT_EQUAL(5, ragedb_relationship_property_get_integer(data->graph, rel1Id, "stars"));
}

CTEST2(relationship_tests, shouldAddMultipleRelationshipsSameType) {
    long long int rel1Id = ragedb_relationship_add_empty(data->graph, "FRIENDS", "Node", "empty", "Node", "existing");
    long long int rel2Id = ragedb_relationship_add_empty(data->graph, "FRIENDS", "Node", "empty", "Node", "existing");
    ASSERT_NOT_EQUAL(-1, rel1Id);
    ASSERT_NOT_EQUAL(-1, rel2Id);
    ASSERT_NOT_EQUAL(rel1Id, rel2Id);
    ASSERT_STR("FRIENDS", ragedb_relationship_get_type(data->graph, rel1Id));
    ASSERT_STR("FRIENDS", ragedb_relationship_get_type(data->graph, rel2Id));
    ASSERT_EQUAL(0, ragedb_relationship_get_starting_node_id(data->graph, rel1Id));
    ASSERT_EQUAL(0, ragedb_relationship_get_starting_node_id(data->graph, rel2Id));
    ASSERT_EQUAL(1, ragedb_relationship_get_ending_node_id(data->graph, rel1Id));
    ASSERT_EQUAL(1, ragedb_relationship_get_ending_node_id(data->graph, rel2Id));
}
CTEST2(relationship_tests, shouldAddMultipleRelationshipsSameTypeWithProperties) {
    long long int rel1Id = ragedb_relationship_add(data->graph, "FRIENDS", "Node", "empty", "Node", "existing", "{\"stars\":1}");
    long long int rel2Id = ragedb_relationship_add(data->graph, "FRIENDS", "Node", "empty", "Node", "existing", "{\"stars\":2}");
    ASSERT_NOT_EQUAL(-1, rel1Id);
    ASSERT_NOT_EQUAL(-1, rel2Id);
    ASSERT_NOT_EQUAL(rel1Id, rel2Id);
    ASSERT_EQUAL(1, ragedb_relationship_property_get_integer(data->graph, rel1Id, "stars"));
    ASSERT_EQUAL(2, ragedb_relationship_property_get_integer(data->graph, rel2Id, "stars"));
    ASSERT_STR("FRIENDS", ragedb_relationship_get_type(data->graph, rel1Id));
    ASSERT_STR("FRIENDS", ragedb_relationship_get_type(data->graph, rel2Id));
    ASSERT_EQUAL(0, ragedb_relationship_get_starting_node_id(data->graph, rel1Id));
    ASSERT_EQUAL(0, ragedb_relationship_get_starting_node_id(data->graph, rel2Id));
    ASSERT_EQUAL(1, ragedb_relationship_get_ending_node_id(data->graph, rel1Id));
    ASSERT_EQUAL(1, ragedb_relationship_get_ending_node_id(data->graph, rel2Id));
}

CTEST2(relationship_tests, shouldRemoveRelationship) {
    long long int rel1Id = ragedb_relationship_add_empty(data->graph, "FRIENDS", "Node", "empty", "Node", "existing");
    ASSERT_NOT_EQUAL(-1, rel1Id);
    bool removed = ragedb_relationship_remove_by_id(data->graph, rel1Id);
    ASSERT_TRUE(removed);
    long long int count = ragedb_relationship_type_get_count(data->graph, "FRIENDS");
    ASSERT_EQUAL(0, count);
}

CTEST2(relationship_tests, shouldRemoveRelationshipSameType) {
    long long int rel1Id = ragedb_relationship_add_empty(data->graph, "FRIENDS", "Node", "empty", "Node", "existing");
    long long int rel2Id = ragedb_relationship_add_empty(data->graph, "FRIENDS", "Node", "empty", "Node", "existing");
    ASSERT_NOT_EQUAL(-1, rel1Id);
    ASSERT_NOT_EQUAL(-1, rel2Id);
    ASSERT_NOT_EQUAL(rel1Id, rel2Id);
    long long int count = ragedb_relationship_type_get_count(data->graph, "FRIENDS");
    ASSERT_EQUAL(2, count);
    bool removed = ragedb_relationship_remove_by_id(data->graph, rel1Id);
    ASSERT_TRUE(removed);
    removed = ragedb_relationship_remove_by_id(data->graph, rel2Id);
    ASSERT_TRUE(removed);
    count = ragedb_relationship_type_get_count(data->graph, "FRIENDS");
    ASSERT_EQUAL(0, count);
}

CTEST2(relationship_tests, shouldGetRelationship) {
    long long int rel1Id = ragedb_relationship_add_empty(data->graph, "FRIENDS", "Node", "empty", "Node", "existing");
    ASSERT_NOT_EQUAL(-1, rel1Id);
}

CTEST2(relationship_tests, shouldGetRelationshipWithProperties) {
    long long int rel1Id = ragedb_relationship_add(data->graph, "FRIENDS", "Node", "empty", "Node", "existing", "{\"stars\":5}");
    ASSERT_NOT_EQUAL(-1, rel1Id);
    ASSERT_EQUAL(5, ragedb_relationship_property_get_integer(data->graph, rel1Id, "stars"));
}

CTEST2(relationship_tests, shouldGetMultipleRelationshipsSameTypeWithProperties) {
    long long int rel1Id = ragedb_relationship_add(data->graph, "FRIENDS", "Node", "empty", "Node", "existing", "{\"stars\":1}");
    long long int rel2Id = ragedb_relationship_add(data->graph, "FRIENDS", "Node", "empty", "Node", "existing", "{\"stars\":2}");
    ASSERT_NOT_EQUAL(-1, rel1Id);
    ASSERT_NOT_EQUAL(-1, rel2Id);
    ASSERT_NOT_EQUAL(rel1Id, rel2Id);
    long long int count = ragedb_relationship_type_get_count(data->graph, "FRIENDS");
    ASSERT_EQUAL(2, count);
    ASSERT_EQUAL(1, ragedb_relationship_property_get_integer(data->graph, rel1Id, "stars"));
    ASSERT_EQUAL(2, ragedb_relationship_property_get_integer(data->graph, rel2Id, "stars"));
    ASSERT_STR("FRIENDS", ragedb_relationship_get_type(data->graph, rel1Id));
    ASSERT_STR("FRIENDS", ragedb_relationship_get_type(data->graph, rel2Id));
    ASSERT_EQUAL(0, ragedb_relationship_get_starting_node_id(data->graph, rel1Id));
    ASSERT_EQUAL(0, ragedb_relationship_get_starting_node_id(data->graph, rel2Id));
    ASSERT_EQUAL(1, ragedb_relationship_get_ending_node_id(data->graph, rel1Id));
    ASSERT_EQUAL(1, ragedb_relationship_get_ending_node_id(data->graph, rel2Id));
}

CTEST2(relationship_tests, shouldGetAllRelationshipsIds) {
    long long int rel1Id = ragedb_relationship_add_empty(data->graph, "FRIENDS", "Node", "empty", "Node", "existing");
    ASSERT_NOT_EQUAL(-1, rel1Id);
    long long int rel2Id = ragedb_relationship_add_empty(data->graph, "FRIENDS", "Node", "empty", "Node", "existing");
    ASSERT_NOT_EQUAL(-1, rel2Id);
    ASSERT_NOT_EQUAL(rel1Id, rel2Id);

    int count = 0;
    roaring_uint32_iterator_t * all = ragedb_all_get_relationship_ids(data->graph);
    while(all->has_value) {
        count++;
        roaring_advance_uint32_iterator(all);
    }
    ASSERT_EQUAL(2, count);
}

CTEST2(relationship_tests, shouldGetAllRelationshipsIdsExceptDeleted) {
    long long int rel1Id = ragedb_relationship_add_empty(data->graph, "FRIENDS", "Node", "empty", "Node", "existing");
    ASSERT_NOT_EQUAL(-1, rel1Id);
    long long int rel2Id = ragedb_relationship_add_empty(data->graph, "FRIENDS", "Node", "empty", "Node", "existing");
    ASSERT_NOT_EQUAL(-1, rel2Id);
    ASSERT_NOT_EQUAL(rel1Id, rel2Id);
    ragedb_relationship_remove_by_id(data->graph, rel2Id);

    int count = 0;
    roaring_uint32_iterator_t * all = ragedb_all_get_relationship_ids(data->graph);
    while(all->has_value) {
        count++;
        roaring_advance_uint32_iterator(all);
    }
    ASSERT_EQUAL(1, count);
}

CTEST2(relationship_tests, shouldGetAllRelationshipIdsWithType) {
    long long int rel1Id = ragedb_relationship_add_empty(data->graph, "FRIENDS", "Node", "empty", "Node", "existing");
    ASSERT_NOT_EQUAL(-1, rel1Id);
    long long int rel2Id = ragedb_relationship_add_empty(data->graph, "ENEMIES", "Node", "empty", "Node", "existing");
    ASSERT_NOT_EQUAL(-1, rel2Id);
    ASSERT_NOT_EQUAL(rel1Id, rel2Id);

    int count = 0;
    roaring_uint32_iterator_t * all = ragedb_all_get_relationship_ids_with_type(data->graph, "FRIENDS");
    while(all->has_value) {
        count++;
        roaring_advance_uint32_iterator(all);
    }
    ASSERT_EQUAL(1, count);
}