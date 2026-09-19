#pragma once
#include "ament_index_cpp/get_package_share_directory.hpp"
#include <apriltag_msgs/msg/april_tag_detection_array.hpp>
#include <filesystem>
#include <iostream>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include <string.h>
#include <unordered_map>
#include <yaml-cpp/yaml.h>

class Global_pose : public rclcpp::Node {
private:
  struct TagPose {
    double x, y, z, roll, pitch, yaw;
  };

  std::unordered_map<int, TagPose> tag_world_poses;
  std::unordered_map<int, std::string> tag_frame_names;
  rclcpp::Subscription<apriltag_msgs::msg::AprilTagDetectionArray>::SharedPtr
      detections_subs;

  // Methods

  void load_tag_poses(const std::string &file_path) {
    /*
    Yaml Dosyasından değerleri al ve data konteynırına kaydet
     */

    YAML::Node root = YAML::LoadFile(file_path);
    for (const auto &t : root["tags"]) {
      auto id = t["id"].as<int>();
      auto v = t["pose"].as<std::vector<double>>();
      tag_world_poses[id] = TagPose{v[0], v[1], v[2], v[3], v[4], v[5]};
      tag_frame_names[id] = t["name"].as<std::string>();
    }
  }

  // Callbacks
  void
  callback_detections(const apriltag_msgs::msg::AprilTagDetectionArray &msg) {
    if (msg.detections.empty())
      return;

    auto best_it =
        std::max_element(msg.detections.begin(), msg.detections.end(),
                         [](const auto &a, const auto &b) {
                           return a.decision_margin < b.decision_margin;
                         });

    auto it = tag_world_poses.find(best_it->id);

    if (it == tag_world_poses.end()) {
      RCLCPP_WARN(get_logger(), "id=%d lookup table da yok", best_it->id);

      return;
    }
  }

public:
  Global_pose() : Node("apriltag_global_pose") {

    const std::string default_path =
        ament_index_cpp::get_package_share_directory("pose_from_apriltag") +
        "/tag_poses.yaml";

    // Önce parametreleri yükle
    this->declare_parameter<std::string>("file_tag_poses", "default_path");
    const auto file_path = this->get_parameter("file_tag_poses").as_string();

    if (file_path.empty()) {
      throw std::runtime_error("file_tag_poses verilmedi");
    }

    if (!std::filesystem::exists(file_path)) {
      throw std::runtime_error("'file_tag_poses' bulunamadı: " + file_path);
    }

    load_tag_poses(file_path);

    RCLCPP_INFO(get_logger(), "Tag pozları yüklendi: %s", file_path.c_str());

    // detectionslara subscibe oli
    detections_subs =
        this->create_subscription<apriltag_msgs::msg::AprilTagDetectionArray>(
            "/detections", rclcpp::SensorDataQoS(),
            [this](const apriltag_msgs::msg::AprilTagDetectionArray &msg) {
              this->callback_detections(msg);
            });
  }
};

