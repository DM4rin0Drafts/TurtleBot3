uint64 uid                            # unique id match request and result; only used internally

geometry_msgs/PoseStamped goal_pose   # target pose of endeffector

string[] name                         # list of joint names
float64[] position                    # corresponding joint values (used as initial configuration)

uint64 max_solving_time               # max planning time in ms
float64 max_eps                       # max acceptable deviation in pose of ik solution
