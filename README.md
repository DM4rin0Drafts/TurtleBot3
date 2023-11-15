# turtlebot_install
Provides rosinstall files and scripts for making installation of turtlebot software more convenient.

## Basic Desktop/Laptop Install

General remarks:

*The standard computer setup we use is Ubuntu 20.04/64Bit with ROS Noetic*
* Note that other setups might work, but cannot be supported due to the overhead that would involve.

Checkout software (please take note of the . at the end):
<pre>
mkdir ~/turtle
cd ~/turtle
git clone git@git.sim.informatik.tu-darmstadt.de:TurtleBot/turtlebot_install.git . -b noetic-devel
</pre>

If not already done, *install ROS Noetic* using the setup script, which is based on the official [tutorial](http://wiki.ros.org/noetic/Installation/Ubuntu) (using desktop variant):
<pre>
./install_noetic.sh
</pre>

Install software:
<pre>
./install.sh
</pre>

Add following line to your .bashrc:
<pre>
. ~/turtle/setup.bash
</pre>

Open a new terminal before starting to work on turtlebot software.

## Install Gazebo Simulation

For simulation you need to install following rosinstalls:
<pre>
turtle install simulation ui
turtle update_make
</pre>

## Running in Simulation

Start default simulation environment
<pre>
turtle sim
</pre>
