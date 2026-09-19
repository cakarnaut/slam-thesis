cmake_minimum_required(VERSION 3.8)
project(pose_from_apriltag)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)

# apriltag_msgs: /detections topic'inin tipi (AprilTagDetectionArray).
# NOT: Bu mesajin icinde POSE YOK -- sadece id, family, hamming,
# decision_margin, centre ve corners var. Tag'in pose'u apriltag_node
# tarafindan yalnizca TF olarak yayinlanir, o yuzden tf2_ros da gerekli.
find_package(apriltag_msgs REQUIRED)

find_package(geometry_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)

add_executable(pose_from_apriltag_node
  src/pose_from_apriltag_node.cpp
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

install(TARGETS pose_from_apriltag_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
