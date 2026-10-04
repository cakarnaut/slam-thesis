cmake_minimum_required(VERSION 3.8)
project(pose_from_apriltag)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)

find_package(apriltag_msgs REQUIRED)

find_package(geometry_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(yaml-cpp REQUIRED)


add_executable(pose_from_apriltag_node 
src/main.cpp
src/methods/call_back.cpp
src/methods/init.cpp
src/methods/load_tag_poses.cpp
src/helpers/transform.cpp
)

target_compile_features(pose_from_apriltag_node PUBLIC cxx_std_17)

ament_target_dependencies(pose_from_apriltag_node
  rclcpp
  apriltag_msgs
  geometry_msgs
  tf2
  tf2_ros
  tf2_geometry_msgs
)


target_link_libraries(pose_from_apriltag_node ${YAML_CPP_LIBRARIES})

include_directories(include)

install(TARGETS pose_from_apriltag_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()

