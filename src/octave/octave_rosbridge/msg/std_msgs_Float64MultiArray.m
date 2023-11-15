#-------------------------------------------------------------------------------
# Creates message object for type std_msgs/Float64MultiArray
#-------------------------------------------------------------------------------
# Inputs:
#   dim         array of dimension properties (see http://docs.ros.org/jade/api/std_msgs/html/msg/MultiArrayLayout.html)
#               NOT SUPPORTED YET
#   data_offset padding elements at front of data
#   float       input float array
#-------------------------------------------------------------------------------
# Output:
#   msg         created message
#-------------------------------------------------------------------------------
function msg = std_msgs_Float64MultiArray (dim, data_offset, floats)
  global messageInclude;
  # "{"layout":{"dim":[],"data_offset":0},"data":[]}"  
  s = ['{"layout":{"dim":[],"data_offset":' num2str(data_offset) '},"data":' array2str(floats) '}'];
  msg = javaObject (messageInclude, s);
endfunction
