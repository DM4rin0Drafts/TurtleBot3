uint64 uid                            # unique id match request and result; only used internally

string[] name                         # list of joint names
float64[] position                    # corresponding joint values

float64 eps                           # remaining error in ik solution 
bool success                          # indicates if solution was found
