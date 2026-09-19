#include "../include/Pose-From-Apriltag/base.hpp"

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);

  try {
    rclcpp::spin(std::make_shared<Global_pose>());
  } catch (const std::exception & e) {
    RCLCPP_FATAL(
      rclcpp::get_logger("apriltag_global_pose"),
      "Node baslatilamadi: %s", e.what());
    rclcpp::shutdown();
    return 1;
  }

  rclcpp::shutdown();
  return 0;
}
