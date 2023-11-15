#-------------------------------------------------------------------------------
# connects to rosbridge via websocket on given IP
#-------------------------------------------------------------------------------
# Inputs:
#   address     ip address of rosbridge websocket (port 9090)
#-------------------------------------------------------------------------------
# Output:
#   ros         instance of jrosbridge
#-------------------------------------------------------------------------------
function ros = init_ros (address = "127.0.0.1")
  global rosInclude;
  global ros;
  ros = javaObject (rosInclude, address);
  ros.connect ();
endfunction