#include <stdlib.h>
#include <time.h>

#include "../graph.h"

// As seen on https://miteshpatekar.wordpress.com/2014/10/18/how-to-build-a-cpu-benchmarking-tool-in-c/

Graph* graph;
int iterations = 10, item_count = 2000, person_count = 100000, likes_count = 100, weight_check = -1;

void property_traversal_with_weight() {
    printf("\n============================ Property Traversal With Weight ============================");
    clock_t  start, end;

    char buf[12];
    memset(buf,0,sizeof(buf));

    // Initialize a counter for the number of relationships traversed, should be around 5M relationships.
    long long int count = 0;
    long long int count2 = 0;

    //Start the timer
    start = clock();
    for(int i = 0; i < iterations; i++) {
        // Traverse (random_person)-[:LIKES]->(item)<-[:LIKES]-(person)-[:LIKES]->(other_item)
        // Get a random person
        int random_person = rand()%((person_count));
        snprintf(buf, 12, "person%d", random_person);

        for M_EACH(item, *neo4c_node_get_weighted_outgoing(graph, "LIKES", "Person", buf), array_combination_t) {
                        count2++;
                        if (item->weight1>weight_check) {
                            for M_EACH(person, *neo4c_node_get_weighted_incoming_by_id(graph, "LIKES", item->node_id),
                                        array_combination_t) {
                                            count2++;
                                            if (person->weight1>weight_check) {
                                                for M_EACH(other_item,
                                                            *neo4c_node_get_weighted_outgoing_by_id(graph, "LIKES",
                                                                    person->node_id), array_combination_t) {
                                                                count2++;
                                                                if (other_item->weight1 > weight_check) {
                                                                    count++;
                                                                }
                                                            }
                                            }
                                        }
                        }
                    }
    }
    end = clock();
    long double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("\nTraversals of about %Lf relationships took: %Lf milliseconds to execute each", count/(long double)iterations,cpu_time_used/iterations * 1000.0);
    double ops=(iterations)/(double)(cpu_time_used);
    printf("\nIterations per second: %f", ops);
    printf("\nIterations: %d", iterations);

    // Let's see the count as a sanity check
    printf("\nCount: %lld", count);
    printf("\nCount2: %lld", count2);

    printf("\nTraversals per millisecond: %Lf", count/cpu_time_used/1000.0);
}

void simple_traversal() {
    printf("\n============================ Simple Traversal ============================");
    clock_t  start, end;

    char buf[12];
    memset(buf,0,sizeof(buf));

    // Initialize a counter for the number of relationships traversed, should be around 5M relationships.
    long long int count = 0;

    //Start the timer
    start = clock();
    for(int i = 0; i < iterations; i++) {
        // Traverse (random_person)-[:LIKES]->(item)<-[:LIKES]-(person)-[:LIKES]->(other_item)
        // Get a random person
        int random_person = rand()%((person_count));
        snprintf(buf, 12, "person%d", random_person);

        for M_EACH(item, *neo4c_node_get_weighted_outgoing(graph, "LIKES", "Person", buf), array_combination_t) {
            for M_EACH(person, *neo4c_node_get_weighted_incoming_by_id(graph, "LIKES", item->node_id), array_combination_t) {
                for M_EACH(other_item,*neo4c_node_get_weighted_outgoing_by_id(graph, "LIKES", person->node_id), array_combination_t) {
                    count++;
                    }
            }
        }
    }
    end = clock();
    long double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("\nTraversals of about %Lf relationships took: %Lf milliseconds to execute each", count/(long double)iterations,cpu_time_used/iterations * 1000.0);
    double ops=(iterations)/(double)(cpu_time_used);
    printf("\nIterations per second: %f", ops);
    printf("\nIterations: %d", iterations);

    // Let's see the count as a sanity check
    printf("\nCount: %lld", count);
    printf("\nTraversals per millisecond: %Lf", count/cpu_time_used/1000.0);
}

int main(int argc, char** argv) {
    srand(time(0));

    clock_t initialize, created;
    initialize = clock();

    // Initialize the Graph
    graph = malloc(sizeof(Graph));
    neo4c_clear(graph);

    // Create Nodes and Relationships
    char buf[12];
    char buf2[12];
    char buf3[20];
    memset(buf,0,sizeof(buf));
    memset(buf2,0,sizeof(buf2));
    memset(buf3,0,sizeof(buf3));

    for (int i = 0; i<item_count; i++) {
        snprintf(buf, 12, "item%d", i);
        neo4c_node_add_empty(graph, "Item", buf);
        memset(buf,0,sizeof(buf));
    }
    long long int likes = 0;
    for (int i = 0; i<person_count; i++) {
        snprintf(buf, 12, "person%d", i);
        neo4c_node_add_empty(graph, "Person", buf);
        for (int j = 0; j<likes_count; j++) {
            int random_item = rand() % ((item_count+1));
            int random_weight = 1 + (rand() % 10);
            int random_weight_2 = 1 + (rand() % 10);
            snprintf(buf2, 12, "item%d", random_item);
            snprintf(buf3, 20, "{ \"weight\": %d }", random_weight);
            neo4c_relationship_add_with_weights(graph, "LIKES", "Person", buf, "Item", buf2, buf3, random_weight, random_weight_2);
            memset(buf2,0,sizeof(buf2));
            memset(buf3,0,sizeof(buf3));
            likes++;
        }
        memset(buf,0,sizeof(buf));
    }
    created = clock();

    long double time_used = ((double) (created - initialize)) / CLOCKS_PER_SEC;
    printf("\nTime to Create Graph of %d items, %d people, %lld total likes: %Lf seconds", item_count, person_count, likes, time_used);

    simple_traversal();
    //property_traversal();
    //property_traversal_direct();
    property_traversal_with_weight();

    return 0;
}
