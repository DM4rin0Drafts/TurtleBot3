#-------------------------------------------------------------------------------
# Creates message object for type turtlebot3_exercise_msgs/FKResult
#-------------------------------------------------------------------------------
# Inputs:
#   uid         unique identifier equal to corresponding request
#   name        array of link names
#   pose        array of link pose stamped objects
#   success     indicating if solution was found (bool)
#-------------------------------------------------------------------------------
# Output:
#   msg         created message
#-------------------------------------------------------------------------------
function msg = turtlebot3_exercise_msgs_FKResult (uid, name, pose, success)
  fkresult.uid = uid;
  fkresukt.name = name;
  fkresukt.pose = pose;  
  global messageInclude;
  # {"uid":0,"name":[""],"poses":[{"header":{"seq":0,"stamp":{"secs":0,"nsecs":0},"frame_id":""},"pose":{"position":{"x":0.0,"y":0.0,"z":0.0},"orientation":{"x":0.0,"y":0.0,"z":0.0,"w":0.0}}}]}
  s = ['{"uid":' num2str(uid) ',"name":' array2str(name) ',"pose":' array2str(pose) ',"success":' bool2str(success) '}'];
  msg = javaObject (messageInclude, s);
endfunction