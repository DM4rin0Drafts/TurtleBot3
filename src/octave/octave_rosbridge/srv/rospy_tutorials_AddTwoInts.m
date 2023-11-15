#-------------------------------------------------------------------------------
# Creates service request object for type rospy_tutorials/AddTwoInts
#-------------------------------------------------------------------------------
# Inputs:
#   a           first float
#   b           second float
#-------------------------------------------------------------------------------
# Output:
#   req         created request
#-------------------------------------------------------------------------------
function req = rospy_tutorials_AddTwoInts (a, b)
  global requestInclude;
  s.a = a;
  s.b = b;
  # "{\"a\":a, \"b\":b}"
  req = javaObject (requestInclude, savejson('', s));
endfunction