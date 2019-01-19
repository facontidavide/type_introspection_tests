cmake_minimum_required(VERSION 2.8.3)
project(ros_introspection_test)

find_package(Boost REQUIRED COMPONENTS regex)


find_package(catkin REQUIRED COMPONENTS 
    ros_type_introspection
    rosbag
    rosbag_storage
    topic_tools
    sensor_msgs
    geometry_msgs
    message_generation
    genmsg
    )

# Build flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 ")

add_message_files( FILES
    MotorStatus.msg
    )

generate_messages( DEPENDENCIES
    std_msgs
    )

###################################
## catkin specific configuration ##
###################################
## The catkin_package macro generates cmake config files for your package
## Declare things to be passed to dependent projects
## INCLUDE_DIRS: uncomment this if you package contains header files
## LIBRARIES: libraries you create in this project that dependent projects also need
## CATKIN_DEPENDS: catkin_packages dependent projects also need
## DEPENDS: system dependencies of this project that dependent projects also need
catkin_package(
    INCLUDE_DIRS
    LIBRARIES
    CATKIN_DEPENDS
    ros_type_introspection
    rosbag
    rosbag_storage
    topic_tools
    sensor_msgs
    geometry_msgs
    message_runtime
    DEPENDS
    )

###########
## Build ##
###########

## Specify additional locations of header files
## Your package locations should be listed before other locations

include_directories(
    ${catkin_INCLUDE_DIRS}
    )

find_package(benchmark CONFIG)
if (benchmark_FOUND)

    add_executable(ros_introspection_benchmark tests/benchmark.cpp)
    add_dependencies(ros_introspection_benchmark
        ${${PROJECT_NAME}_EXPORTED_TARGETS}
        ${catkin_EXPORTED_TARGETS})

    target_link_libraries(ros_introspection_benchmark
        ${catkin_LIBRARIES}
        benchmark
        pthread
        )

endif(benchmark_FOUND)

add_executable(simple_example          example/simple.cpp)
target_link_libraries(simple_example   ${catkin_LIBRARIES})

add_executable(rosbag_example          example/rosbag_example.cpp)
target_link_libraries(rosbag_example   ${catkin_LIBRARIES})

add_executable(generic_subscriber        example/generic_subscriber.cpp)
target_link_libraries(generic_subscriber ${catkin_LIBRARIES})

add_executable(multi_subscriber        example/multi_subscriber.cpp)
target_link_libraries(multi_subscriber ${catkin_LIBRARIES})

#############
## Install ##
#############

# all install targets should use catkin DESTINATION variables
# See http://ros.org/doc/api/catkin/html/adv_user_guide/variables.html


#############
## Testing ##
#############
if(CATKIN_ENABLE_TESTING)

    catkin_add_gtest(ros_introspection_test
        tests/parser_test.cpp
        tests/deserializer_test.cpp
        tests/renamer_test.cpp
        )

    target_link_libraries(ros_introspection_test
        ${catkin_LIBRARIES}
        boost_regex
        )

endif()
