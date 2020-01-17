cmake_minimum_required(VERSION 3.15)
project(neo4c C)

set(CMAKE_C_STANDARD 99)

add_executable(perf src/benchmarks/main.c src/graph.h src/graph.c src/include/roaring.h src/include/roaring.c src/include/cJSON.c src/include/cJSON.h src/direction.h src/relationship.h src/combination.h)

add_executable(test src/test/main.c src/test/ctest.h src/test/relationship-types.c src/test/node-degree.c src/test/node.c src/test/relationship.c src/test/node-properties.c src/test/relationship-properties.c src/test/traversing.c src/graph.h src/graph.c src/include/roaring.h src/include/roaring.c src/include/cJSON.c src/include/cJSON.h src/direction.h src/relationship.h src/combination.h )


