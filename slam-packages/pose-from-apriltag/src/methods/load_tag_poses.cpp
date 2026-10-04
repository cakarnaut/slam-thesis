#include "../../include/Pose-From-Apriltag/base.hpp"

void PoseFromApriltag::GlobalPose::load_tag_poses() {

  // Dosyanın tamamını oku
  this->declare_parameter<std::string>("config_file_path", "");
  std::string yaml_path = this->get_parameter("config_file_path").as_string();

  if (yaml_path.empty()) {
    RCLCPP_FATAL(get_logger(),
                 "config_file_path parametresi verilmedi. Ornek: "
                 "--ros-args -p config_file_path:=/ros2_ws/.../tag_poses.yaml");

    throw SpesificErrors("Dosyayı bulamadı");
    //    return;
  }

  try {
    // dosyayı içe al
    YAML::Node config = YAML::LoadFile(yaml_path);

    // tagsleri seç ve extract et
    const auto &tags_from_file = config["tags"];

    for (const auto &tag : tags_from_file) {
      const int id = tag["id"].as<int>();
      const std::string name = tag["name"].as<std::string>();
      const std::vector<double> pose_vector =
          tag["pose"].as<std::vector<double>>();

      if (pose_vector.size() != 6) {
        throw std::runtime_error(
            "id=" + std::to_string(id) + " icin pose 6 eleman olmali, " +
            std::to_string(pose_vector.size()) + " bulundu");
      }

      m_pose_tag_world.insert_or_assign(id, Pose{pose_vector});
      m_names_frame[id] = name;
    }

    if (m_pose_tag_world.empty()) {
      throw SpesificErrors("dosyada hic tag yok");
    }

    RCLCPP_INFO(this->get_logger(), "YAML okundu %zu tag yüklendi",
                m_pose_tag_world.size());

  } catch (const std::exception &e) {

    RCLCPP_ERROR(this->get_logger(), "YAML okunamadı: %s", e.what());

    throw SpesificErrors(e.what());
  }
}
