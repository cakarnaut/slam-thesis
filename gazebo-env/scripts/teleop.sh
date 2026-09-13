#!/bin/bash
# --- 8. Hatirlatma + teleop (on planda, interaktif) ---
echo ""
echo "============================================================"
echo "  Gazebo penceresinde Play (>) tusuna basmayi unutma!"
echo "  Arac henuz fizik calismadigi icin havada/duraklatilmis durabilir."
echo "============================================================"
echo ""
echo "Teleop baslatiliyor (u/i/o/j/k/l/,/./m ile sur, Ctrl+C ile cik)..."
ros2 run teleop_twist_keyboard teleop_twist_keyboard
