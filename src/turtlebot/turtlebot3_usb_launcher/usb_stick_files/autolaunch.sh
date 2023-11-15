#!/bin/bash

source $ROSWSS_ROOT/setup.bash ""
source $ROSWSS_BASE_SCRIPTS/helper/helper.sh

# wait for roscore
roswss wait_for_roscore
echo

# simlink to install-folder on usb stick
TURTLESTICK_DIR=/media/$(whoami)/TURTLESTICK
INSTALL_DIR=$TURTLESTICK_DIR/install/

# if install folder is present, create a simlink and execute exercise task
if [ -d $INSTALL_DIR ]; then

  if [ -d $ROSWSS_ROOT/install ]; then 
    unlink $ROSWSS_ROOT/install
  fi

	ln -s $INSTALL_DIR $ROSWSS_ROOT/

	# source install file
	if [ -f $ROSWSS_ROOT/install/setup.sh ]; then
		echo_info ">>> Sourcing $ROSWSS_ROOT/install/setup.sh"
		source $ROSWSS_ROOT/install/setup.sh
    echo
	fi
fi

LOG_DIR="$TURTLESTICK_DIR/logs"
PREEXEC="source $ROSWSS_ROOT/install/setup.sh --extend"
DIRECTORIES="/media/$(whoami)/TURTLESTICK/scripts /media/$(whoami)/TURTLESTICK/launch"

# run execute script with script and launch folder
bash $ROSWSS_BASE_SCRIPTS/helper/run_all.sh $DIRECTORIES -p "$PREEXEC" -l $LOG_DIR

# keep open
cat
