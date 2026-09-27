#include "../include/Pose-From-Apriltag/base.hpp"

int main(int argc, char **argv) {

  rclcpp::init(argc, argv);

  auto node = std::make_shared<PoseFromApriltag::GlobalPose>();

  node->init();

  rclcpp::spin(node);

  rclcpp::shutdown();

  return 0;
}
