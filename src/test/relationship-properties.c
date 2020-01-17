#include <stdlib.h>
#include <float.h>
#include "ctest.h"
#include "../graph.h"

CTEST_DATA(relationship_properties_test) {
  Graph* graph;
};

CTEST_SETUP(relationship_properties_test) {
    data->graph = malloc(sizeof(Graph));
    neo4c_clear(data->graph);
    long long int node1Id = neo4c_node_add_empty(data->graph, "Node", "one");
    long long int node2Id = neo4c_node_add_empty(data->graph, "Node", "two");
}

CTEST_TEARDOWN(relationship_properties_test) {
    neo4c_close(data->graph);
}

CTEST2(relationship_properties_test, shouldGetRelationshipProperty) {
    long long int rel1Id = neo4c_relationship_add(data->graph, "FRIENDS", "Node", "one", "Node", "two", "{\"weight\":5 }");
    ASSERT_NOT_EQUAL(-1, rel1Id);
    ASSERT_EQUAL(5,neo4c_relationship_property_get_integer(data->graph, rel1Id, "weight"));
}

CTEST2(relationship_properties_test, shouldNotGetRelationshipPropertyNotThere) {
    long long int rel1Id = neo4c_relationship_add(data->graph, "FRIENDS", "Node", "one", "Node", "two", "{\"weight\":5 }");
    ASSERT_NOT_EQUAL(-1, rel1Id);
    ASSERT_NULL(neo4c_relationship_property_get_string(data->graph, rel1Id, "not_there"));
    ASSERT_EQUAL(INT_MIN, neo4c_relationship_property_get_integer(data->graph, rel1Id, "not_there"));
    ASSERT_DBL_NEAR(DBL_MIN, neo4c_relationship_property_get_double(data->graph, rel1Id, "not_there"));
}


CTEST2(relationship_properties_test, shouldNotGetRelationshipPropertyRelationshipTooHigh) {
    long long int rel1Id = neo4c_relationship_add(data->graph, "FRIENDS", "Node", "one", "Node", "two", "{\"weight\":5 }");
    ASSERT_NOT_EQUAL(-1, rel1Id);
    neo4c_relationship_remove_by_id(data->graph, rel1Id);
    ASSERT_EQUAL(INT_MIN,neo4c_relationship_property_get_integer(data->graph, 99, "weight"));
    ASSERT_DBL_NEAR(DBL_MIN,neo4c_relationship_property_get_double(data->graph, 99, "weight"));
    ASSERT_NULL(neo4c_relationship_property_get_string(data->graph, 99, "weight"));
    cJSON meta = neo4c_relationship_property_get_object(data->graph, 99, "weight");
    ASSERT_NULL(meta.valuestring);
}

CTEST2(relationship_properties_test, shouldNotGetRelationshipPropertyRelationshipDeleted) {
    long long int rel1Id = neo4c_relationship_add(data->graph, "FRIENDS", "Node", "one", "Node", "two", "{\"weight\":5 }");
    ASSERT_NOT_EQUAL(-1, rel1Id);
    neo4c_relationship_remove_by_id(data->graph, rel1Id);
    ASSERT_EQUAL(INT_MIN,neo4c_relationship_property_get_integer(data->graph, rel1Id, "weight"));
    ASSERT_DBL_NEAR(DBL_MIN,neo4c_relationship_property_get_double(data->graph, rel1Id, "weight"));
    ASSERT_NULL(neo4c_relationship_property_get_string(data->graph, rel1Id, "weight"));
    cJSON meta = neo4c_relationship_property_get_object(data->graph, rel1Id, "weight");
    ASSERT_NULL(meta.valuestring);
}

CTEST2(relationship_properties_test, shouldGetMultipleRelationshipProperty) {
    long long int rel1Id = neo4c_relationship_add(data->graph, "FRIENDS", "Node", "one", "Node", "two", "{\"weight\":1 }");
    ASSERT_NOT_EQUAL(-1, rel1Id);
    long long int rel2Id = neo4c_relationship_add(data->graph, "FRIENDS", "Node", "one", "Node", "two", "{\"weight\":2 }");
    ASSERT_NOT_EQUAL(-1, rel2Id);
    long long int rel3Id = neo4c_relationship_add(data->graph, "FRIENDS", "Node", "one", "Node", "two", "{\"weight\":3 }");
    ASSERT_NOT_EQUAL(-1, rel3Id);
    ASSERT_EQUAL(1,neo4c_relationship_property_get_integer(data->graph, rel1Id, "weight"));
    ASSERT_EQUAL(2,neo4c_relationship_property_get_integer(data->graph, rel2Id, "weight"));
    ASSERT_EQUAL(3,neo4c_relationship_property_get_integer(data->graph, rel3Id, "weight"));
}

CTEST2(relationship_properties_test, shouldNotGetRelationshipPropertyRelationshipNotThere) {
    ASSERT_NULL(neo4c_relationship_property_get_string(data->graph, 99, "not_there"));
    ASSERT_EQUAL(INT_MIN, neo4c_relationship_property_get_integer(data->graph, 99, "not_there"));
    ASSERT_DBL_NEAR(DBL_MIN, neo4c_relationship_property_get_double(data->graph, 99, "not_there"));
}

CTEST2(relationship_properties_test, shouldUpdateRelationshipProperty) {
    long long int rel1Id = neo4c_relationship_add(data->graph, "FRIENDS", "Node", "one", "Node", "two", "{\"where\":\"college\", \"weight\":100, \"height\":1.8034, \"meta\": { \"by\": \"User1\"}}");
    ASSERT_NOT_EQUAL(-1, rel1Id);
    ASSERT_EQUAL(100,neo4c_relationship_property_get_integer(data->graph, rel1Id, "weight"));
    neo4c_relationship_property_set_integer(data->graph, rel1Id, "weight", 105);
    ASSERT_EQUAL(105,neo4c_relationship_property_get_integer(data->graph, rel1Id, "weight"));

    ASSERT_STR("college",neo4c_relationship_property_get_string(data->graph, rel1Id, "where"));
    neo4c_relationship_property_set_string(data->graph, rel1Id, "where", "high school");
    ASSERT_STR("high school",neo4c_relationship_property_get_string(data->graph, rel1Id, "where"));

    ASSERT_DBL_NEAR(1.8034,neo4c_relationship_property_get_double(data->graph, rel1Id, "height"));
    neo4c_relationship_property_set_double(data->graph, rel1Id, "height", 1.75);
    ASSERT_DBL_NEAR(1.75,neo4c_relationship_property_get_double(data->graph, rel1Id, "height"));

    neo4c_relationship_property_set_object(data->graph, rel1Id, "meta", "{ \"by\": \"User2\", \"score\":3}");
    cJSON meta = neo4c_relationship_property_get_object(data->graph, rel1Id, "meta");
    ASSERT_STR("User2", neo4c_property_get_string(&meta, "by"));
    ASSERT_EQUAL(3, neo4c_property_get_integer(&meta, "score"));
}

CTEST2(relationship_properties_test, shouldNotUpdateRelationshipPropertyTooHigh) {
    long long int rel1Id = neo4c_relationship_add(data->graph, "FRIENDS", "Node", "one", "Node", "two", "{\"where\":\"college\", \"weight\":100, \"height\":1.8034, \"meta\": { \"by\": \"User1\"}}");
    ASSERT_NOT_EQUAL(-1, rel1Id);
    ASSERT_FALSE(neo4c_relationship_property_set_integer(data->graph, 99, "weight", 105));
    ASSERT_FALSE(neo4c_relationship_property_set_string(data->graph, 99, "where", "high school"));
    ASSERT_FALSE(neo4c_relationship_property_set_double(data->graph, 99, "height", 1.75));
    ASSERT_FALSE(neo4c_relationship_property_set_object(data->graph, 99, "meta", "{ \"by\": \"User2\", \"score\":3}"));
}

CTEST2(relationship_properties_test, shouldNotUpdateRelationshipPropertyRelationshipDeleted) {
    long long int rel1Id = neo4c_relationship_add(data->graph, "FRIENDS", "Node", "one", "Node", "two", "{\"where\":\"college\", \"weight\":100, \"height\":1.8034, \"meta\": { \"by\": \"User1\"}}");
    ASSERT_NOT_EQUAL(-1, rel1Id);
    neo4c_relationship_remove_by_id(data->graph, rel1Id);
    ASSERT_FALSE(neo4c_relationship_property_set_integer(data->graph, rel1Id, "weight", 105));
    ASSERT_FALSE(neo4c_relationship_property_set_string(data->graph, rel1Id, "where", "high school"));
    ASSERT_FALSE(neo4c_relationship_property_set_double(data->graph, rel1Id, "height", 1.75));
    ASSERT_FALSE(neo4c_relationship_property_set_object(data->graph, rel1Id, "meta", "{ \"by\": \"User2\", \"score\":3}"));
}

CTEST2(relationship_properties_test, shouldNotUpdateRelationshipPropertyRelationshipMetadata) {
    long long int rel1Id = neo4c_relationship_add(data->graph, "FRIENDS", "Node", "one", "Node", "two", "{\"where\":\"college\", \"weight\":100, \"height\":1.8034, \"meta\": { \"by\": \"User1\"}}");
    ASSERT_NOT_EQUAL(-1, rel1Id);
    ASSERT_FALSE(neo4c_relationship_property_set_integer(data->graph, rel1Id, "~weight", 105));
    ASSERT_FALSE(neo4c_relationship_property_set_string(data->graph, rel1Id, "~where", "high school"));
    ASSERT_FALSE(neo4c_relationship_property_set_double(data->graph, rel1Id, "~height", 1.75));
    ASSERT_FALSE(neo4c_relationship_property_set_object(data->graph, rel1Id, "~meta", "{ \"by\": \"User2\", \"score\":3}"));
}

CTEST2(relationship_properties_test, shouldUpdateRelationshipProperties) {
    long long int rel1Id = neo4c_relationship_add(data->graph, "FRIENDS", "Node", "one", "Node", "two", "{\"where\":\"college\", \"weight\":100, \"height\":1.8034, \"meta\": { \"by\": \"User1\"}}");
    ASSERT_NOT_EQUAL(-1, rel1Id);
    ASSERT_EQUAL(100,neo4c_relationship_property_get_integer(data->graph, rel1Id, "weight"));
    ASSERT_STR("college",neo4c_relationship_property_get_string(data->graph, rel1Id, "where"));
    ASSERT_DBL_NEAR(1.8034,neo4c_relationship_property_get_double(data->graph, rel1Id, "height"));

    neo4c_relationship_properties_set(data->graph, rel1Id, "{\"where\":\"high school\", \"weight\":105, \"height\":1.75, \"meta\": { \"by\": \"User2\", \"score\":3}}");
    ASSERT_EQUAL(105,neo4c_relationship_property_get_integer(data->graph, rel1Id, "weight"));
    ASSERT_STR("high school",neo4c_relationship_property_get_string(data->graph, rel1Id, "where"));
    ASSERT_DBL_NEAR(1.75,neo4c_relationship_property_get_double(data->graph, rel1Id, "height"));

    cJSON meta = neo4c_relationship_property_get_object(data->graph, rel1Id, "meta");
    ASSERT_STR("User2", neo4c_property_get_string(&meta, "by"));
    ASSERT_EQUAL(3, neo4c_property_get_integer(&meta, "score"));
}

CTEST2(relationship_properties_test, shouldNotUpdateRelationshipPropertiesTooHigh) {
    long long int rel1Id = neo4c_relationship_add(data->graph, "FRIENDS", "Node", "one", "Node", "two", "{\"where\":\"college\", \"weight\":100, \"height\":1.8034, \"meta\": { \"by\": \"User1\"}}");
    ASSERT_NOT_EQUAL(-1, rel1Id);
    ASSERT_EQUAL(100,neo4c_relationship_property_get_integer(data->graph, rel1Id, "weight"));
    ASSERT_STR("college",neo4c_relationship_property_get_string(data->graph, rel1Id, "where"));
    ASSERT_DBL_NEAR(1.8034,neo4c_relationship_property_get_double(data->graph, rel1Id, "height"));
    ASSERT_FALSE(neo4c_relationship_properties_set(data->graph, 99, "{\"where\":\"high school\", \"weight\":105, \"height\":1.75, \"meta\": { \"by\": \"User2\", \"score\":3}}"));
}

CTEST2(relationship_properties_test, shouldNotUpdateRelationshipPropertiesRelationshipDeleted) {
    long long int rel1Id = neo4c_relationship_add(data->graph, "FRIENDS", "Node", "one", "Node", "two", "{\"where\":\"college\", \"weight\":100, \"height\":1.8034, \"meta\": { \"by\": \"User1\"}}");
    ASSERT_NOT_EQUAL(-1, rel1Id);
    ASSERT_EQUAL(100,neo4c_relationship_property_get_integer(data->graph, rel1Id, "weight"));
    ASSERT_STR("college",neo4c_relationship_property_get_string(data->graph, rel1Id, "where"));
    ASSERT_DBL_NEAR(1.8034,neo4c_relationship_property_get_double(data->graph, rel1Id, "height"));
    neo4c_relationship_remove_by_id(data->graph, rel1Id);
    ASSERT_FALSE(neo4c_relationship_properties_set(data->graph, rel1Id, "{\"where\":\"high school\", \"weight\":105, \"height\":1.75, \"meta\": { \"by\": \"User2\", \"score\":3}}"));
}

CTEST2(relationship_properties_test, shouldDeleteRelationshipProperty) {
    long long int rel1Id = neo4c_relationship_add(data->graph, "FRIENDS", "Node", "one", "Node", "two", "{\"weight\":5 }");
    ASSERT_NOT_EQUAL(-1, rel1Id);
    ASSERT_EQUAL(5,neo4c_relationship_property_get_integer(data->graph, rel1Id, "weight"));
    neo4c_relationship_property_delete(data->graph, rel1Id, "weight");
    ASSERT_EQUAL(INT_MIN,neo4c_relationship_property_get_integer(data->graph, rel1Id, "weight"));
}

CTEST2(relationship_properties_test, shouldNotDeleteRelationshipPropertyTooHigh) {
    long long int rel1Id = neo4c_relationship_add(data->graph, "FRIENDS", "Node", "one", "Node", "two", "{\"weight\":5 }");
    ASSERT_NOT_EQUAL(-1, rel1Id);
    ASSERT_EQUAL(5,neo4c_relationship_property_get_integer(data->graph, rel1Id, "weight"));
    ASSERT_FALSE(neo4c_relationship_property_delete(data->graph, 99, "weight"));
}

CTEST2(relationship_properties_test, shouldNotDeleteRelationshipPropertyRelationshipDeleted) {
    long long int rel1Id = neo4c_relationship_add(data->graph, "FRIENDS", "Node", "one", "Node", "two", "{\"weight\":5 }");
    ASSERT_NOT_EQUAL(-1, rel1Id);
    ASSERT_EQUAL(5,neo4c_relationship_property_get_integer(data->graph, rel1Id, "weight"));
    neo4c_relationship_remove_by_id(data->graph, rel1Id);
    ASSERT_FALSE(neo4c_relationship_property_delete(data->graph, rel1Id, "weight"));
}

CTEST2(relationship_properties_test, shouldNotDeleteRelationshipPropertyMetadata) {
    long long int rel1Id = neo4c_relationship_add(data->graph, "FRIENDS", "Node", "one", "Node", "two", "{\"weight\":5 }");
    ASSERT_NOT_EQUAL(-1, rel1Id);
    ASSERT_EQUAL(5,neo4c_relationship_property_get_integer(data->graph, rel1Id, "weight"));
    ASSERT_FALSE(neo4c_relationship_property_delete(data->graph, rel1Id, "~type"));
}

CTEST2(relationship_properties_test, shouldDeleteRelationshipProperties) {
    long long int rel1Id = neo4c_relationship_add(data->graph, "FRIENDS", "Node", "one", "Node", "two", "{\"weight\":5 }");
    ASSERT_NOT_EQUAL(-1, rel1Id);
    ASSERT_EQUAL(5,neo4c_relationship_property_get_integer(data->graph, rel1Id, "weight"));
    neo4c_relationship_properties_delete(data->graph, rel1Id);
    ASSERT_EQUAL(INT_MIN,neo4c_relationship_property_get_integer(data->graph, rel1Id, "weight"));
    ASSERT_EQUAL(0,neo4c_relationship_get_starting_node_id(data->graph, rel1Id));
}

CTEST2(relationship_properties_test, shouldNotDeleteRelationshipPropertiesTooHigh) {
    long long int rel1Id = neo4c_relationship_add(data->graph, "FRIENDS", "Node", "one", "Node", "two", "{\"weight\":5 }");
    ASSERT_NOT_EQUAL(-1, rel1Id);
    ASSERT_EQUAL(5,neo4c_relationship_property_get_integer(data->graph, rel1Id, "weight"));
    ASSERT_FALSE(neo4c_relationship_properties_delete(data->graph, 99));
}

CTEST2(relationship_properties_test, shouldNotDeleteRelationshipPropertiesRelationshipDeleted) {
    long long int rel1Id = neo4c_relationship_add(data->graph, "FRIENDS", "Node", "one", "Node", "two", "{\"weight\":5 }");
    ASSERT_NOT_EQUAL(-1, rel1Id);
    ASSERT_EQUAL(5,neo4c_relationship_property_get_integer(data->graph, rel1Id, "weight"));
    neo4c_relationship_remove_by_id(data->graph, rel1Id);
    ASSERT_FALSE(neo4c_relationship_properties_delete(data->graph, rel1Id));
}