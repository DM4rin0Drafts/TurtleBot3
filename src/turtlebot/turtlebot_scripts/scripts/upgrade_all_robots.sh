#!/bin/bash

. $ROSWSS_ROOT/setup.bash ""

# Turtlebot robots
for number in {1..8}; do
  echo
  echo "---------------------------------------------"
  echo ">>> Upgrading turtle$number..."

  roswss "turtle$number" "stop"
  roswss "turtle$number" "update_make"
  roswss "turtle$number" "start"

  echo ">>> Upgrade for turtle$number completed!"
  echo "---------------------------------------------"
  echo
done
