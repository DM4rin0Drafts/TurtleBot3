#-------------------------------------------------------------------------------
# Creates message object for type std_msgs/Header
#-------------------------------------------------------------------------------
# Inputs:
#   header      Header object
#-------------------------------------------------------------------------------
# Output:
#   msg         created message
#-------------------------------------------------------------------------------
function msg = std_msgs_Header (header)
  global messageInclude;
  # {"seq":1,"stamp":{"secs":0,"nsecs":0},"frame_id":""}
  msg = javaObject (messageInclude, savejson ('', header));
endfunction