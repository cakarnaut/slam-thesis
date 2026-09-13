#!/bin/bash
XACRO_FILE="car.urdf"

cd /workspace/src/slam_thesis_gazebo/urdf || exit 1

if [ ! -f "$XACRO_FILE" ]; then
    echo "$XACRO_FILE bulunamadı, oluşturuluyor..."
    xacro car.urdf.xacro > "$XACRO_FILE"
else
    echo "Dosya zaten mevcut."
fi

python3 ../scripts/make_rsp_params.py "$XACRO_FILE" /tmp/rsp_params.yaml

ros2 run robot_state_publisher robot_state_publisher --ros-args --params-file /tmp/rsp_params.yaml
