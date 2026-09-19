#!/bin/bash
XACRO_FILE="car.urdf"

# Yolu script'in kendi konumundan turet (elle duzeltmeye gerek yok).
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WORKSPACE_DIR="$(dirname "${SCRIPT_DIR}")"

cd "${WORKSPACE_DIR}/urdf" || exit 1

# xacro'yu HER ZAMAN yeniden uret: aksi halde car.urdf.xacro'da yapilan
# degisiklik (ornegin kamera pitch'i) eski car.urdf yuzunden sessizce
# goz ardi edilir.
echo "xacro isleniyor: car.urdf.xacro -> $XACRO_FILE"
xacro car.urdf.xacro > "$XACRO_FILE"

python3 "${SCRIPT_DIR}/make_rsp_params.py" "$XACRO_FILE" /tmp/rsp_params.yaml

ros2 run robot_state_publisher robot_state_publisher --ros-args \
  --params-file /tmp/rsp_params.yaml \
  -p use_sim_time:=true
