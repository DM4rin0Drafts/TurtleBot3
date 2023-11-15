#!/bin/bash

# DO NOT REMOVE THIS BLOCK UNLESS YOU DON'T WANT TO USE CUSTOM SCRIPTS
@[if DEVELSPACE]@
export ROSWSS_SCRIPTS="@(PROJECT_SOURCE_DIR)/scripts:$ROSWSS_SCRIPTS"
@[else]@
export ROSWSS_SCRIPTS="@(CMAKE_INSTALL_PREFIX)/@(CATKIN_PACKAGE_SHARE_DESTINATION)/scripts:$ROSWSS_SCRIPTS"
@[end if]@

# SET HERE YOUR WORKSPACE PREFIX
export ROSWSS_PREFIX="turtle"
export ROSWSS_ROOT_RELATIVE_PATH="../.."
export ROSWSS_INSTALL_DIR="rosinstall"

# SETUP YOUR ENVIRONMENT HERE (all fields are optional)
export ONBOARD_LAUNCH_PKG="turtlebot3_onboard_launch"
export UI_LAUNCH_PKG="turtlebot3_ui_launch"
export GAZEBO_LAUNCH_PKG="tuda_turtlebot3_gazebo"
export GAZEBO_DEFAULT_LAUNCH_FILE="turtlebot3.launch"
export GAZEBO_WORLDS_PKG="tuda_turtlebot3_gazebo"
export AUTOSTART_LAUNCH_PKG="turtlebot3_robot_bringup"
export ROBOT_MASTER_HOSTNANE=""
export ROBOT_HOSTNAMES="turtle1 turtle2 turtle3 turtle4 turtle5 turtle6 turtle7 turtle8"
export ROBOT_USER="turtlebot"

add_completion "behavior_test" "_roswss_behavior_test_complete"

# Turtlebot robots
# Syntax:
#   add_remote_pc "<script_name>" "<host_name>" "<screen_name>" "<command>"
for number in {1..8}; do
    add_remote_pc "turtle$number" "turtle$number" "turtle_start" "~/turtle/src/turtlebot/turtlebot3_robot_bringup/scripts/start.sh"
done
