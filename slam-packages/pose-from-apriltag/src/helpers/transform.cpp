#include "../../include/Pose-From-Apriltag/base.hpp"

tf2::Transform
PoseFromApriltag::transform_to(const PoseFromApriltag::Pose &pose) {

  tf2::Quaternion q;

  q.setRPY(pose.roll, pose.pitch, pose.yaw);

  return tf2::Transform(q, tf2::Vector3(pose.x, pose.y, pose.z));
}

