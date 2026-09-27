#include "../../include/Pose-From-Apriltag/base.hpp"

void PoseFromApriltag::GlobalPose::init() {

  // Sbcscireber ı başlat!
  m_sub_detection =
      this->create_subscription<apriltag_msgs::msg::AprilTagDetectionArray>(
          "/detections", 10,
          [this](
              const apriltag_msgs::msg::AprilTagDetectionArray::SharedPtr msg) {
            this->pose_call_back(*msg);
          });



}
