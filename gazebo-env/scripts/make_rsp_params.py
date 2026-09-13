#!/usr/bin/env python3
"""
Bu script, URDF'i bir YAML parametre
dosyasina duzgun kacisli (escaped) sekilde yazarak bu sorunu tamamen
ortadan kaldirir.

Kullanim:
  xacro car.urdf.xacro > car.urdf
  python3 make_rsp_params.py car.urdf /tmp/rsp_params.yaml
  ros2 run robot_state_publisher robot_state_publisher --ros-args --params-file /tmp/rsp_params.yaml
"""
import sys
import yaml


def main():
    if len(sys.argv) != 3:
        print("Kullanim: make_rsp_params.py <urdf_dosyasi> <cikti_yaml_dosyasi>")
        sys.exit(1)

    urdf_path, out_path = sys.argv[1], sys.argv[2]
    with open(urdf_path) as f:
        urdf_content = f.read()

    data = {
        'robot_state_publisher': {
            'ros__parameters': {
                'robot_description': urdf_content
            }
        }
    }

    with open(out_path, 'w') as f:
        yaml.dump(data, f, default_flow_style=False)

    print(f"Yazildi: {out_path}")


if __name__ == '__main__':
    main()
