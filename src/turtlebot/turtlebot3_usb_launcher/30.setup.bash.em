#!/bin/bash

# DO NOT REMOVE THIS BLOCK UNLESS YOU DON'T WANT TO USE CUSTOM SCRIPTS
@[if DEVELSPACE]@
export ROSWSS_SCRIPTS="@(PROJECT_SOURCE_DIR)/scripts:$ROSWSS_SCRIPTS"
@[else]@
export ROSWSS_SCRIPTS="@(CMAKE_INSTALL_PREFIX)/@(CATKIN_PACKAGE_SHARE_DESTINATION)/scripts:$ROSWSS_SCRIPTS"
@[end if]@

add_completion "usb_stick" "_usb_stick_complete"
