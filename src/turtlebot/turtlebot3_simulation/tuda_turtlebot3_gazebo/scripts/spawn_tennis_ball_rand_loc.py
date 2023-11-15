#!/usr/bin/env python

import sys
import random
import os
import math

node_num = int(sys.argv[1])
min_x = float(sys.argv[2])
max_x = float(sys.argv[3])
min_y = float(sys.argv[4])
max_y = float(sys.argv[5])
min_z = float(sys.argv[6])
max_z = float(sys.argv[7])

rand_pos = "-x %f -y %f -z %f -R %f -P %f -Y %f" % (random.uniform(min_x, max_x), random.uniform(min_y, max_y), random.uniform(min_z, max_z),
                                                    random.uniform(-math.pi, math.pi), random.uniform(-math.pi, math.pi), random.uniform(-math.pi, math.pi))

os.system("rosrun gazebo_ros spawn_model __name:=spawn_ball_%i_node -database tennis_ball -sdf -model tennis_ball_%i %s" % (node_num, node_num, rand_pos))

