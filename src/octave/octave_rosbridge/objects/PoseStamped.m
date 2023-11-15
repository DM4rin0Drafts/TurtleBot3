#-------------------------------------------------------------------------------
# Creates object for type std_msgs/PoseStamped
#-------------------------------------------------------------------------------
# Inputs:
#   header      header object
#   pose        pose object
#-------------------------------------------------------------------------------
# Output:
#   msg         created object
#-------------------------------------------------------------------------------
function obj = PoseStamped (header, pose)
  obj.header = header;
  obj.pose = pose;
endfunction