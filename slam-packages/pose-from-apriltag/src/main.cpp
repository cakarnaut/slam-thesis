#include "../include/Pose-From-Apriltag/base.hpp"

int main(int argc, char **argv) {

  rclcpp::init(argc, argv);

  auto node = std::make_shared<PoseFromApriltag::GlobalPose>();

  try {

    node->init();

  } catch (const std::exception &e) {
    std::cout << "sonlandırıyoruz" << std::endl;

    rclcpp::shutdown();

    return 1;
  }

  rclcpp::spin(node);

  rclcpp::shutdown();

  return 0;
}
