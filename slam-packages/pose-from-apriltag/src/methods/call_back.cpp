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

  // const Pose &tag_pose_world = best_it->second;
  const std::string &tag_frame = m_names_frame.at(best->id);

  // işler ciddileşiyor

  // tf ağaçlarından pose ları al
  geometry_msgs::msg::TransformStamped cam_to_tag;
  geometry_msgs::msg::TransformStamped cam_to_base;
  try {
    cam_to_tag = m_buff_tf->lookupTransform("camera_link_optical", tag_frame,
                                            tf2_ros::fromMsg(msg.header.stamp),
                                            tf2::durationFromSec(0.05));

    cam_to_base = m_buff_tf->lookupTransform(
        "camera_link_optical", "base_footprint", tf2::TimePointZero);

  } catch (const tf2::TransformException &ex) {

    RCLCPP_WARN(get_logger(), "TF bulunamadi: %s", ex.what());

    return;
  }

  // pose ları işleme sokarak arabanın konumunu bul(base)
  tf2::Transform T_camera_tag;
  tf2::Transform T_camera_base;

  tf2::fromMsg(cam_to_tag.transform, T_camera_tag);
  tf2::fromMsg(cam_to_base.transform, T_camera_base);

  // map->base_footprint = map->tag * tag->camera * camera->base_footprint
  tf2::Transform T_map_tag = transform_to(best_it->second);
  tf2::Transform T_map_base =
      T_map_tag * T_camera_tag.inverse() * T_camera_base;

  // Publish et, mapa aktarılacak kısım!
  geometry_msgs::msg::PoseWithCovarianceStamped out;
  out.header.stamp = msg.header.stamp;
  out.header.frame_id = "map";
  out.pose.pose.position.x = T_map_base.getOrigin().x();
  out.pose.pose.position.y = T_map_base.getOrigin().y();
  out.pose.pose.position.z = T_map_base.getOrigin().z();
  out.pose.pose.orientation = tf2::toMsg(T_map_base.getRotation());
  for (auto &c : out.pose.covariance)
    c = 0.0;
  out.pose.covariance[0] = out.pose.covariance[7] = out.pose.covariance[14] =
      out.pose.covariance[21] = out.pose.covariance[28] =
          out.pose.covariance[35] = 0.05;

  m_pub_pose->publish(out);
}

