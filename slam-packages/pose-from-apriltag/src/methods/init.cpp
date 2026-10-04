#include "../../include/Pose-From-Apriltag/base.hpp"

void PoseFromApriltag::GlobalPose::init() {

  try {

    load_tag_poses();

  } catch (const SpesificErrors &e) {
    RCLCPP_INFO(this->get_logger(), "YAML dosyası okunamadı");
	throw std::runtime_error("yamlda hata");
  }

  m_buff_tf = std::make_unique<tf2_ros::Buffer>(get_clock());

  m_listener_tf = std::make_shared<tf2_ros::TransformListener>(*m_buff_tf);

  m_pub_pose = create_publisher<geometry_msgs::msg::PoseWithCovarianceStamped>(
      "/apriltag_pose", 10);

  // Sbcscireber ı başlat!
  m_sub_detection =
      this->create_subscription<apriltag_msgs::msg::AprilTagDetectionArray>(
          "/detections", 10,
          [this](
              const apriltag_msgs::msg::AprilTagDetectionArray::SharedPtr msg) {
            this->pose_call_back(*msg);
          });
}
