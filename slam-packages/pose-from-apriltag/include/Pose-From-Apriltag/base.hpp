#pragma once
#include <apriltag_msgs/msg/april_tag_detection_array.hpp>
#include <filesystem>
#include <iostream>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include <string.h>
#include <unordered_map>
#include <yaml-cpp/yaml.h>

namespace PoseFromApriltag {

struct Pose {
  double x;
  double y;
  double z;
  double roll;
  double pitch;
  double yaw;

public:
  Pose(const std::vector<double> &vec) {

    if (vec.size() >= 6) {
      x = vec[0];
      y = vec[1];
      z = vec[2];
      roll = vec[3];
      pitch = vec[4];
      yaw = vec[5];
    } else {
      x = y = z = roll = pitch = yaw = 0.0;
    }
  }
};

class GlobalPose : public rclcpp::Node {

private:
  // private members
  std::unordered_map<int, Pose> m_pose_tag_world;
  std::unordered_map<int, std::string> m_names_frame;
  rclcpp::Subscription<apriltag_msgs::msg::AprilTagDetectionArray>::SharedPtr
      m_sub_detection;

  // private methods
  void pose_call_back(const apriltag_msgs::msg::AprilTagDetectionArray &msg);
  void load_tag_poses();

public:
  // public methdos
  GlobalPose();
  void init();
};

} // namespace PoseFromApriltag

