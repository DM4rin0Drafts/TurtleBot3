#-------------------------------------------------------------------------------
# Creates message object for type std_msgs/MultiArrayDimension
#-------------------------------------------------------------------------------
# Inputs:
#   label       label of given dimension
#   size        size of given dimension (in type units)
#   stride      stride of given dimension
#-------------------------------------------------------------------------------
# Output:
#   msg         created message
#-------------------------------------------------------------------------------
function msg = std_msgs_MultiArrayDimension (label, size, stride)
  global messageInclude;
  mad.label = label;
  mad.size = size;
  mad.stride = stride;
  msg = javaObject (messageInclude, savejson ('', mad));
endfunction