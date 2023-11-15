#-------------------------------------------------------------------------------
# Creates object for type std_msgs/Header
#-------------------------------------------------------------------------------
# Inputs:
#   seq         Sequence
#   secs        Seconds
#   nsecs       Nanoseconds
#   frame_id    Frame
#-------------------------------------------------------------------------------
# Output:
#   msg         created object
#-------------------------------------------------------------------------------
function obj = Header (seq, secs, nsecs, frame_id)
  obj.seq = seq;
  obj.stamp.secs = secs;
  obj.stamp.nsecs = nsecs;
  obj.frame_id = frame_id;
endfunction