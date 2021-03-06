#include <stdlib.h>
#include <float.h>
#include "ctest.h"
#include "../graph.h"

CTEST_DATA(node_properties_test) {
  Graph* graph;
};

CTEST_SETUP(node_properties_test) {
    data->graph = malloc(sizeof(Graph));
    neo4c_clear(data->graph);
    long long int node1Id = neo4c_node_add_empty(data->graph, "Node", "one");
    long long int node2Id = neo4c_node_add_empty(data->graph, "Node", "two");
}

CTEST_TEARDOWN(node_properties_test) {
    neo4c_close(data->graph);
}

CTEST2(node_properties_test, shouldGetNodeProperty) {
    long long int created = neo4c_node_add(data->graph, "Node", "key",
            "{ \"name\":\"max\", \"email\":\"maxdemarzi@hotmail.com\" }");
    ASSERT_NOT_EQUAL(0, created);
    ASSERT_STR("max", neo4c_node_property_get_string(data->graph, "Node", "key", "name"));
}

CTEST2(node_properties_test, shouldNotGetNodePropertyNotThere) {
    long long int created = neo4c_node_add(data->graph, "Node", "key",
            "{ \"name\":\"max\", \"email\":\"maxdemarzi@hotmail.com\" }");
    ASSERT_NOT_EQUAL(0, created);
    ASSERT_NULL(neo4c_node_property_get_string(data->graph, "Node", "key", "not_there"));
    ASSERT_EQUAL(INT_MIN, neo4c_node_property_get_integer(data->graph, "Node", "key", "not_there"));
    ASSERT_DBL_NEAR(DBL_MIN, neo4c_node_property_get_double(data->graph, "Node", "key", "not_there"));
    cJSON meta = neo4c_node_property_get_object(data->graph, "Node", "key", "not_there");
    ASSERT_NULL(meta.valuestring);
}

CTEST2(node_properties_test, shouldNotGetNodePropertyNodeNotThere) {
    long long int created = neo4c_node_add(data->graph, "Node", "key",
            "{ \"name\":\"max\", \"email\":\"maxdemarzi@hotmail.com\" }");
    ASSERT_NOT_EQUAL(0, created);
    ASSERT_NULL(neo4c_node_property_get_string(data->graph, "Node", "not_exists", "not_there"));
    ASSERT_EQUAL(INT_MIN, neo4c_node_property_get_integer(data->graph, "Node", "not_exists", "not_there"));
    ASSERT_DBL_NEAR(DBL_MIN, neo4c_node_property_get_double(data->graph, "Node", "not_exists", "not_there"));
    cJSON meta = neo4c_node_property_get_object(data->graph, "Node", "not_exists", "not_there");
    ASSERT_NULL(meta.valuestring);
}

CTEST2(node_properties_test, shouldUpdateNodeProperty) {
    long long int created = neo4c_node_add(data->graph, "Node", "key",
            "{ \"name\":\"max\", \"email\":\"maxdemarzi@hotmail.com\", \"weight\":105, \"height\":1.8034, \"meta\": { \"by\": \"User1\"} }");
    ASSERT_NOT_EQUAL(0, created);
    ASSERT_STR("Node", neo4c_node_property_get_string(data->graph, "Node", "key", "~label"));
    ASSERT_STR("key", neo4c_node_property_get_string(data->graph, "Node", "key", "~key"));
    ASSERT_EQUAL(created, neo4c_node_property_get_integer(data->graph, "Node", "key", "~id"));
    ASSERT_STR("max", neo4c_node_property_get_string(data->graph, "Node", "key", "name"));
    ASSERT_STR("maxdemarzi@hotmail.com", neo4c_node_property_get_string(data->graph, "Node", "key", "email"));
    neo4c_node_property_set_string(data->graph, "Node", "key", "name", "alex");
    ASSERT_STR("alex", neo4c_node_property_get_string(data->graph, "Node", "key", "name"));
    neo4c_node_property_set_integer(data->graph, "Node", "key", "weight", 100);
    ASSERT_EQUAL(100, neo4c_node_property_get_integer(data->graph, "Node", "key", "weight"));
    neo4c_node_property_set_double(data->graph, "Node", "key", "height", 1.75);
    ASSERT_DBL_NEAR(1.75, neo4c_node_property_get_double(data->graph, "Node", "key", "height"));
    neo4c_node_property_set_object(data->graph, "Node", "key", "meta", "{ \"by\": \"User2\", \"score\":3}");
    cJSON meta = neo4c_node_property_get_object(data->graph, "Node", "key", "meta");
    ASSERT_STR("User2", neo4c_property_get_string(&meta, "by"));
    ASSERT_EQUAL(3, neo4c_property_get_integer(&meta, "score"));
}

CTEST2(node_properties_test, shouldNotUpdateNodePropertyNodeNotThere) {
    ASSERT_FALSE(neo4c_node_property_set_string(data->graph, "Node", "not_exists", "name", "alex"));
    ASSERT_FALSE(neo4c_node_property_set_integer(data->graph, "Node", "not_exists", "weight", 100));
    ASSERT_FALSE(neo4c_node_property_set_double(data->graph, "Node", "not_exists", "height", 1.75));
    ASSERT_FALSE(neo4c_node_property_set_object(data->graph, "Node", "not_exists", "meta", "{ \"by\": \"User2\", \"score\":3}"));
}

CTEST2(node_properties_test, shouldNotUpdateNodePropertyMetadata) {
    ASSERT_FALSE(neo4c_node_property_set_string(data->graph, "Node", "one", "~key", "alex"));
    ASSERT_FALSE(neo4c_node_property_set_integer(data->graph, "Node", "one", "~key", 100));
    ASSERT_FALSE(neo4c_node_property_set_double(data->graph, "Node", "one", "~key", 1.75));
    ASSERT_FALSE(neo4c_node_property_set_object(data->graph, "Node", "one", "~key", "{ \"by\": \"User2\", \"score\":3}"));
}

CTEST2(node_properties_test, shouldUpdateNodeProperties) {
    long long int created = neo4c_node_add(data->graph, "Node", "key",
            "{ \"name\":\"max\", \"email\":\"maxdemarzi@hotmail.com\", \"weight\":105, \"height\":1.8034, \"meta\": { \"by\": \"User1\"} }");
    ASSERT_NOT_EQUAL(0, created);
    ASSERT_STR("Node", neo4c_node_property_get_string(data->graph, "Node", "key", "~label"));
    ASSERT_STR("key", neo4c_node_property_get_string(data->graph, "Node", "key", "~key"));
    ASSERT_EQUAL(created, neo4c_node_property_get_integer(data->graph, "Node", "key", "~id"));
    ASSERT_STR("max", neo4c_node_property_get_string(data->graph, "Node", "key", "name"));
    ASSERT_STR("maxdemarzi@hotmail.com", neo4c_node_property_get_string(data->graph, "Node", "key", "email"));

    neo4c_node_properties_set(data->graph, "Node", "key", "{ \"name\":\"alex\", \"email\":\"maxdemarzi@hotmail.com\", \"weight\":100, \"height\":1.75, \"meta\": { \"by\": \"User2\", \"score\":3} }");
    ASSERT_STR("alex", neo4c_node_property_get_string(data->graph, "Node", "key", "name"));
    ASSERT_EQUAL(100, neo4c_node_property_get_integer(data->graph, "Node", "key", "weight"));
    ASSERT_DBL_NEAR(1.75, neo4c_node_property_get_double(data->graph, "Node", "key", "height"));
    cJSON meta = neo4c_node_property_get_object(data->graph, "Node", "key", "meta");
    ASSERT_STR("User2", neo4c_property_get_string(&meta, "by"));
    ASSERT_EQUAL(3, neo4c_property_get_integer(&meta, "score"));
    ASSERT_STR("Node", neo4c_node_property_get_string(data->graph, "Node", "key", "~label"));
    ASSERT_STR("key", neo4c_node_property_get_string(data->graph, "Node", "key", "~key"));
}

CTEST2(node_properties_test, shouldNotUpdateNodePropertiesNodeNotThere) {
    ASSERT_FALSE(neo4c_node_properties_set(data->graph, "Node", "not_exists", "{ \"name\":\"alex\", \"email\":\"maxdemarzi@hotmail.com\", \"weight\":100, \"height\":1.75, \"meta\": { \"by\": \"User2\", \"score\":3} }"));
}

CTEST2(node_properties_test, shouldDeleteNodeProperty) {
    long long int created = neo4c_node_add(data->graph, "Node", "key",
            "{ \"name\":\"max\", \"email\":\"maxdemarzi@hotmail.com\" }");
    ASSERT_NOT_EQUAL(0, created);
    ASSERT_STR("max", neo4c_node_property_get_string(data->graph, "Node", "key", "name"));
    neo4c_node_property_delete(data->graph, "Node", "key", "name");
    ASSERT_NULL(neo4c_node_property_get_string(data->graph, "Node", "key", "name"));
}

CTEST2(node_properties_test, shouldNotDeleteNodePropertyNodeNotThere) {
    ASSERT_FALSE(neo4c_node_property_delete(data->graph, "Node", "not_exists", "not_there"));
}

CTEST2(node_properties_test, shouldNotDeleteNodePropertyMetadata) {
    ASSERT_FALSE(neo4c_node_property_delete(data->graph, "Node", "one", "~label"));
}

CTEST2(node_properties_test, shouldDeleteNodeProperties) {
    long long int created = neo4c_node_add(data->graph, "Node", "key",
            "{ \"name\":\"max\", \"email\":\"maxdemarzi@hotmail.com\" }");
    ASSERT_NOT_EQUAL(0, created);
    ASSERT_STR("max", neo4c_node_property_get_string(data->graph, "Node", "key", "name"));
    neo4c_node_properties_delete(data->graph, "Node", "key");
    ASSERT_NULL(neo4c_node_property_get_string(data->graph, "Node", "key", "name"));
    ASSERT_STR("Node", neo4c_node_property_get_string(data->graph, "Node", "key", "~label"));
    ASSERT_STR("key", neo4c_node_property_get_string(data->graph, "Node", "key", "~key"));
}

CTEST2(node_properties_test, shouldNotDeleteNodePropertiesNodeNotThere) {
    ASSERT_FALSE(neo4c_node_properties_delete(data->graph, "Node", "not_exists"));
}