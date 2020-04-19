cmake_minimum_required(VERSION 2.8.11)
project(receiver)

add_subdirectory(libdf)
include_directories(libdf/include)
add_definitions(-Wall)

# add_definitions(-DDETECTOR_DEBUG)
# add_definitions(-DRECEIVER_DEBUG)
# add_definitions(-DDUMP_DEBUG)
# add_definitions(-DIMPORT_DEBUG)
add_definitions(-DMAIN_DEBUG)
# add_definitions(-DDEBUG_ALL)
add_definitions(-DDEBUG_SOURCE)

set(SRC
    main.c
    dump.c
    import.c
    detector.c
    receiver.c)

add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME} df)