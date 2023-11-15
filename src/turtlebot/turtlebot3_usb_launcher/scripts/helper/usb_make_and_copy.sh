#!/bin/bash

source $ROSWSS_BASE_SCRIPTS/helper/helper.sh

# prevent script to continue w/ next step after ctrl + c but leave w/ defined state
function shutdown(){
  # changing back
  echo
  echo_warn "SIGINT detected. Aborting!"
  echo_info ">>> Changing profile settings back to normal"
  catkin config --no-install >/dev/null 2>&1
  catkin profile set default >/dev/null 2>&1
  exit 2
}
trap shutdown INT

function usage(){
  echo "Usage: turtle usb_stick make [--behaviors-onboard] [--directory <directory>] <packages>"
}

COPY_BEHAVIOR_LAUNCH=false

while [[ $# -gt 0 ]]
do
  key="$1"

  case $key in
    --behaviors-onboard)
    COPY_BEHAVIOR_LAUNCH=true
    shift
    ;;
    -d|--directory)
    shift
    DIRECTORY=$1
    shift
    ;;
    -h|--help)
    usage
    exit 0
    shift
    ;;
    -*|--*)
    echo "Error: Unknown option: $key"
    usage
    exit 1
    shift
    ;;
    *)
    PACKET_LIST+=($key)
    shift
    ;;
  esac
done


if [ -z "$PACKET_LIST" ]; then
  echo_info ">>> Building and installing ALL packets"
  PACKET_LIST="all"
else
  echo_info ">>> Building and installing following packets:"
  for i in ${PACKET_LIST[@]}; do
    echo_note "   $i"
  done
fi
echo

# If the --directory argument was given, copy files to that directory, otherwise copy to the
# default mounting point of the USB stick
if [ -z "$DIRECTORY" ]; then
    DESTINATION=/media/$(whoami)/TURTLESTICK/
else
    DESTINATION=$(realpath $DIRECTORY)
    mkdir -p $DESTINATION
fi

# config catkin so that an install folder is created
cd $ROSWSS_ROOT

# switch to install profile. if it does not exist yet, create it
echo_info ">>> Switching to install profile"
catkin profile set install >/dev/null 2>&1
if [ $? -ne 0 ]; then
  catkin config --profile install -x _stick
  catkin profile set install >/dev/null 2>&1
fi
catkin config --install >/dev/null 2>&1

echo_info ">>> Building"
if [ "$PACKET_LIST" == "all" ]; then
  catkin build
else
  #echo -e ">>> Cleaning old devel build and install folders of install profile.\n Otherwise packages from older build might remain.\n If you want to keep them you can skip the clean"
  catkin clean --yes >/dev/null 2>&1
  catkin build ${PACKET_LIST[@]}
fi

# changing back
echo_info ">>> Changing profile settings back to normal"
catkin config --no-install >/dev/null 2>&1
catkin profile set default >/dev/null 2>&1

# copy files to $DESTINATION
cd $DESTINATION >/dev/null 2>&1
[ $? -ne 0 ] && echo_error "Could not access directory: $DESTINATION." && exit 2
[ -d install ] &&  rm -rf install 	# delete old install folder at $DESTINATION
echo_info ">>> Copying files"

FILES_DIR="$(rospack find turtlebot3_usb_launcher)/usb_stick_files"

if [ ! -d $FILES_DIR ]; then
  echo_error ">>> Files not found in $FILES_DIR. Are you using another directory?"
else
  cp -rp $FILES_DIR/* $DESTINATION
fi

cp -r $ROSWSS_ROOT/install_stick/. install/  # copy current install folder to $DESTINATION

# change old username to turtlebot in: setup.sh, etc/.../*.bash files
cd install/
OLDSTR="$ROSWSS_ROOT/install_stick"
NEWSTR="/home/turtlebot/turtle/install"
grep -r $OLDSTR -l | grep .sh | tr '\n' ' ' | xargs sed -i "s@$OLDSTR@$NEWSTR@g" >/dev/null 2>&1

# Copy the start_robot.launch files to $DESTINATION/launch
cd $DESTINATION >/dev/null 2>&1
rm launch/*.launch >/dev/null 2>&1

if [ "$PACKET_LIST" == "all" ]; then
  PACKETS=$(rospack list-names)
else
  PACKETS=${PACKET_LIST[@]}
fi

for PACKET in $PACKETS
do
  START_ROBOT_PATH=$(rospack find $PACKET)/launch/start_robot.launch
  # Copy start_robot.launch of turtlebot3_behavior_exercise only if the --behaviors-onboard flag is set
  if [ -e "$START_ROBOT_PATH" ] && { [ $PACKET != "turtlebot3_behavior_exercise" ] ||  [ $COPY_BEHAVIOR_LAUNCH == true ]; }; then
    START_ROBOT_NEW_NAME=${PACKET/turtlebot3_/}
    cp "$START_ROBOT_PATH" "launch/${START_ROBOT_NEW_NAME}.launch"
  fi
done

echo_info ">>> Done!"
echo

# If no destination directory was given, cd out of stick and unmount if wanted
if [ -z "$DIRECTORY" ]; then
    cd ~
    echo_warn  "NOTE: Please safely eject the stick manually as unplugging might corrupt data."
    echo -ne "${YELLOW}Do you want to eject the stick now? (y/N) ${NOCOLOR}"
    read OPTION
    if [[ "$OPTION" == [yY] ]]; then
      umount $DESTINATION
    fi
fi


exit 0
