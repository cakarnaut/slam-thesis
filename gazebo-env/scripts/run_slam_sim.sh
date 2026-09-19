#!/bin/bash
# ============================================================
# run_slam_sim.sh
# Gazebo'yu room.sdf ile acar, araci spawn eder, ROS2 bridge'i
# ve apriltag_node'u baslatir.
#
# Kullanim:
#   chmod +x run_slam_sim.sh
#   ./run_slam_sim.sh
#
# Cikista (Ctrl+C ya da teleop'u kapatinca) Gazebo, bridge ve
# apriltag_node arka plan surecleri de otomatik kapatilir.
# ============================================================

set -e

# --- Ayarlar ---
# WORKSPACE_DIR'i script'in KENDI konumundan turetiyoruz; boylece depo
# nereye klonlanirsa klonlansin elle yol duzeltmeye gerek kalmiyor.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WORKSPACE_DIR="$(dirname "${SCRIPT_DIR}")"
WORLD_FILE="${WORKSPACE_DIR}/worlds/room.sdf"
XACRO_FILE="${WORKSPACE_DIR}/urdf/car.urdf.xacro"
CAR_URDF="${WORKSPACE_DIR}/urdf/car.urdf"
TAGS_CONFIG="${WORKSPACE_DIR}/config/tags.yaml"
WORLD_NAME="slam_thesis_room"
CAR_NAME="slam_car"

# --- On kontrol: dosyalar gercekten var mi ---
if [ ! -f "$WORLD_FILE" ]; then
  echo "HATA: world dosyasi bulunamadi: $WORLD_FILE"
  exit 1
fi
if [ ! -f "$XACRO_FILE" ]; then
  echo "HATA: xacro dosyasi bulunamadi: $XACRO_FILE"
  exit 1
fi
if [ ! -f "$TAGS_CONFIG" ]; then
  echo "HATA: apriltag config dosyasi bulunamadi: $TAGS_CONFIG"
  exit 1
fi

# --- Paket isimlendirmesini otomatik algila (ros_ign_* vs ros_gz_*) ---
if ros2 pkg list 2>/dev/null | grep -qx "ros_gz_sim"; then
  SPAWN_PKG="ros_gz_sim"
elif ros2 pkg list 2>/dev/null | grep -qx "ros_ign_gazebo"; then
  SPAWN_PKG="ros_ign_gazebo"
else
  echo "HATA: ne ros_gz_sim ne ros_ign_gazebo bulunamadi."
  exit 1
fi

if ros2 pkg list 2>/dev/null | grep -qx "ros_gz_bridge"; then
  BRIDGE_PKG="ros_gz_bridge"
elif ros2 pkg list 2>/dev/null | grep -qx "ros_ign_bridge"; then
  BRIDGE_PKG="ros_ign_bridge"
else
  echo "HATA: ne ros_gz_bridge ne ros_ign_bridge bulunamadi."
  exit 1
fi

if ! ros2 pkg list 2>/dev/null | grep -qx "apriltag_ros"; then
  echo "HATA: apriltag_ros bulunamadi. Kurulum: sudo apt install ros-humble-apriltag-ros"
  exit 1
fi

echo "Kullanilacak paketler: spawn=${SPAWN_PKG}, bridge=${BRIDGE_PKG}"

# --- Cikista arka plan sureclerini temizle ---
GAZEBO_PID=""
BRIDGE_PID=""
APRILTAG_PID=""
cleanup() {
  echo ""
  echo "Kapatiliyor..."
  [ -n "$APRILTAG_PID" ] && kill "$APRILTAG_PID" 2>/dev/null
  [ -n "$BRIDGE_PID" ] && kill "$BRIDGE_PID" 2>/dev/null
  [ -n "$GAZEBO_PID" ] && kill "$GAZEBO_PID" 2>/dev/null
}
trap cleanup EXIT INT TERM

# --- 1. Gazebo'yu ac ---
export IGN_GAZEBO_RESOURCE_PATH="${IGN_GAZEBO_RESOURCE_PATH}:${WORKSPACE_DIR}/models"
echo "Gazebo aciliyor: ${WORLD_FILE}"
ign gazebo "${WORLD_FILE}" &
GAZEBO_PID=$!

# --- 2. Gazebo'nun gercekten hazir olmasini bekle (sabit sleep yerine polling) ---
echo "Gazebo'nun hazir olmasi bekleniyor..."
READY=0
for i in $(seq 1 30); do
  if ign topic -l 2>/dev/null | grep -q "/world/${WORLD_NAME}/stats"; then
    READY=1
    break
  fi
  sleep 1
done
if [ "$READY" -eq 0 ]; then
  echo "UYARI: Gazebo 30 saniyede hazir olmadi, yine de devam ediliyor..."
else
  echo "Gazebo hazir."
fi

# --- 3. xacro -> urdf ---
echo "xacro isleniyor..."
xacro "${XACRO_FILE}" > "${CAR_URDF}"

# --- 4. Onceden spawn edilmis ayni isimli arac varsa temizle (script'i tekrar calistirilabilir yapmak icin) ---
ign service -s "/world/${WORLD_NAME}/remove" \
  --reqtype ignition.msgs.Entity --reptype ignition.msgs.Boolean \
  --timeout 1000 --req "name: \"${CAR_NAME}\" type: MODEL" >/dev/null 2>&1 || true
sleep 1

# --- 5. Araci spawn et ---
echo "Arac spawn ediliyor..."
ros2 run "${SPAWN_PKG}" create -file "${CAR_URDF}" -name "${CAR_NAME}" -x 0 -y 0 -z 0.02

sleep 2

# --- 6. ROS2 bridge'i arka planda baslat ---
#
# Yon isaretleri (ros_gz_bridge sozdizimi):
#   @ = cift yonlu, [ = SADECE gz -> ROS, ] = SADECE ROS -> gz
# Her topic'e gercekten ihtiyac duyulan tek yon veriliyor; eskiden hepsi
# cift yonluydu, bu gereksiz publisher/abonelik uretiyordu ve ozellikle
# /tf'te geri-besleme riski tasiyordu.
#
# /clock KRITIK: Gazebo mesajlari sim-time damgali (0'dan baslar), ROS
# node'lari varsayilan olarak wall-clock kullanir. /clock koprulenmeden
# ve node'lara use_sim_time:=true verilmeden TF lookup'lari (ve ileride
# EKF) zaman uyusmazligindan patlar.
echo "ROS2 bridge baslatiliyor..."
ros2 run "${BRIDGE_PKG}" parameter_bridge \
  "/clock@rosgraph_msgs/msg/Clock[ignition.msgs.Clock" \
  "/cmd_vel@geometry_msgs/msg/Twist]ignition.msgs.Twist" \
  "/odom@nav_msgs/msg/Odometry[ignition.msgs.Odometry" \
  "/tf@tf2_msgs/msg/TFMessage[ignition.msgs.Pose_V" \
  "/scan@sensor_msgs/msg/LaserScan[ignition.msgs.LaserScan" \
  "/imu@sensor_msgs/msg/Imu[ignition.msgs.IMU" \
  "/camera/image@sensor_msgs/msg/Image[ignition.msgs.Image" \
  "/camera/camera_info@sensor_msgs/msg/CameraInfo[ignition.msgs.CameraInfo" \
  "/world/${WORLD_NAME}/model/${CAR_NAME}/joint_state@sensor_msgs/msg/JointState[ignition.msgs.Model" \
  --ros-args \
  -r "/world/${WORLD_NAME}/model/${CAR_NAME}/joint_state:=/joint_states" &
BRIDGE_PID=$!

sleep 2

# --- 7. apriltag_node'u ARKA PLANDA baslat (bu bir servis/node, bitmez --
#     & olmadan script burada sonsuza kadar takili kalirdi ve teleop hic
#     acilmazdi) ---
# use_sim_time: apriltag_node'un yayinladigi detection/TF damgalari
# Gazebo'nun sim saatiyle ayni eksende olmali. parameter_bridge'e bu
# parametre BILEREK verilmiyor -- /clock'u bizzat o yayinliyor, kendi
# yayinladigi saati beklemesi baslangicta kilitlenmeye yol acar.
echo "apriltag_node baslatiliyor..."
ros2 run apriltag_ros apriltag_node --ros-args \
  -r image_rect:=/camera/image \
  -r camera_info:=/camera/camera_info \
  -p use_sim_time:=true \
  --params-file "${TAGS_CONFIG}" &
APRILTAG_PID=$!

sleep 2

# --- 8. On planda bekle ---
#
# BU ADIM ZORUNLU. Gazebo, bridge ve apriltag_node'un hepsi arka planda
# (&) calisiyor; yukarida kurulan "trap cleanup EXIT" ise script BITER
# BITMEZ hepsini oldurur. Dolayisiyla script'i on planda tutan bir is
# yoksa, son satira gelir gelmez normal sekilde cikar ve kendi
# baslattigi her seyi birkac saniye icinde kapatir. Bu adim o yuzden
# "on planda tutan is" gorevini ustleniyor.
#
# Eskiden bu isi teleop yapiyordu ama teleop blogu yoruma alinmisti;
# "Gazebo kendiliginden kapaniyor" sikayetinin sebebi tam olarak buydu.
# Teleop'u buraya geri koymak yerine `wait` kullaniyoruz, boylece teleop
# ayri bir terminalde (scripts/teleop.sh) calistirilabiliyor.
echo ""
echo "============================================================"
echo "  Gazebo penceresinde Play (>) tusuna basmayi unutma!"
echo "  Arac fizik baslamadan havada/duraklatilmis durabilir."
echo ""
echo "  Surmek icin AYRI bir terminalde:  scripts/teleop.sh"
echo ""
echo "  Kapatmak icin burada Ctrl+C."
echo "============================================================"
echo ""

# wait: arka plandaki sureclerden biri bitene kadar on planda bekle.
# set -e aktif oldugu icin `|| true` sart -- Ctrl+C sonrasi wait sifirdan
# farkli doner ve bu, cleanup calismadan cikmaya yol acabilirdi.
wait || true
