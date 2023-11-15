uint64 uid                            # unique id match request and result; only used internally

string[] name                         # list of link names
geometry_msgs/PoseStamped[] pose      # corresponding link poses

bool success                          # indicates if solution was found
