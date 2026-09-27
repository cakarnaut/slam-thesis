#include "../../include/Pose-From-Apriltag/base.hpp"

void PoseFromApriltag::GlobalPose::pose_call_back(
    const apriltag_msgs::msg::AprilTagDetectionArray &msg) {

  // mesajı kontrol et
  if (msg.detections.empty()) {
    RCLCPP_INFO_THROTTLE(get_logger(), *get_clock(), 2000,
                         "Gorunen tag yok, pose guncellenmiyor");

    return;
  }

  // En yüksek decision_marginli apriltag datasını oku

  const apriltag_msgs::msg::AprilTagDetection *best = nullptr;
  auto best_it = m_pose_tag_world.cend();

  for (const auto &d : msg.detections) {
    const auto it = m_pose_tag_world.find(d.id);
    if (it == m_pose_tag_world.end()) {
      continue;
    }
    if (best == nullptr || d.decision_margin > best->decision_margin) {
      best = &d;
      best_it = it;
    }
  }

  if (best == nullptr) {
    RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 2000,
                         "Gorunen %zu tag'in hicbiri tabloda yok",
                         msg.detections.size());
    return;
  }

  const Pose &tag_pose_world = best_it->second;
  const std::string &tag_frame = m_names_frame.at(best->id);




    }

