#!/bin/bash

source $ROSWSS_BASE_SCRIPTS/helper/helper.sh

# this script will take a usb drive as input argument
# it will format the stick, creating a new partition and folder layout

trap "exit" INT # prevent script to continue w/ next step after ctrl + c

# take usb drive as argument
if [ $# -ne 1 ]; then
  echo "Usage: turtle usb_stick DEVICE" >&2
  exit 1
fi

DEVICE="$1"

# ask if all data shall be overwritten
echo -ne "${YELLOW}WARNING: All data on device \"$DEVICE\" will be lost. Continue? (y/N) ${NOCOLOR}" >&2
read OPTION
[[ "$OPTION" == [yY] ]] || exit 2

if [[ "$DEVICE" == *"sda"* ]]; then
	echo_warn "/dev/sda usually is the disk running the os. Are you still sure you gave the right disk to format? (y/N) " >&2
  read OPTION
  [[ "$OPTION" == [yY] ]] || exit 2
fi

# unmount partitions
echo_info ">>> Unmounting partitions"
for part in $(ls -1 ${DEVICE}*); do 
  sudo umount $part 2>/dev/null
  RETURNVAL="$?"
  if [ "$RETURNVAL" -ne 0 ] && [ "$RETURNVAL" -ne 32 ] ; then 
    echo_error "ERROR: Partition seems to be in use. Aborting" >&2
  exit 2
  fi
done

# erase stick
echo_info ">>> Removing old partitions"
for partition in $(sudo parted -s "$DEVICE" print|awk '/^ / {print $1}')
do
  sudo parted -s "$DEVICE" rm ${partition} >/dev/null 2>&1
done

# make new partition
sudo parted --script $DEVICE -- mkpart primary 1 -1 >/dev/null 2>&1
sleep 3 # wait for the system to find partition again

# Format the partition as vfat
PARTITION="${DEVICE}1"
echo_info ">>> Formatting partition \"$PARTITION\" with vfat" >&2
sudo /sbin/mkfs.vfat -n "TURTLESTICK" -I $PARTITION
if [ $? -ne 0 ]; then
  echo_error "ERROR: Formatting failed!" >&2
  exit 3
fi

# mount disk
MOUNT_POINT="/tmp/mnt"
mkdir -p "$MOUNT_POINT"
sudo mount "$PARTITION" "$MOUNT_POINT"

# unmount
sudo umount "$MOUNT_POINT"

echo_info ">>> Done!"
echo
echo_warn "NOTE: Please mount the stick manually before proceeding with the make process!"

exit 0
