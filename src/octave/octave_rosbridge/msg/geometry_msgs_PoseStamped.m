#-------------------------------------------------------------------------------
# Creates message object for type geometry_msgs/PoseStamped
#-------------------------------------------------------------------------------
# Inputs:
#   header      Header object
#   pose        Pose object
#-------------------------------------------------------------------------------
# Output:
#   msg         created message
#-------------------------------------------------------------------------------
function msg = geometry_msgs_PoseStamped (header, pose)
  pose_stamped.header = header;
  pose_stamped.pose = pose;
  global messageInclude;
  # {"header":{"seq":1,"stamp":{"secs":0,"nsecs":0},"frame_id":""},"pose":{"position":{"x":0.0,"y":0.0,"z":0.0},"orientation":{"x":0.0,"y":0.0,"z":0.0,"w":0.0}}}  
  msg = javaObject (messageInclude, savejson ('', pose_stamped));
endfunction