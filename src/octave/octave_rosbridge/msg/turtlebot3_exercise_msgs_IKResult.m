#-------------------------------------------------------------------------------
# Creates message object for type turtlebot3_exercise_msgs/IKResult
#-------------------------------------------------------------------------------
# Inputs:
#   uid         unique identifier equal to corresponding request
#   name        array of joint names
#   position    array of joint positions
#   eps         remaining error of ik solution
#   success     indicating if solution was found (bool)
#-------------------------------------------------------------------------------
# Output:
#   msg         created message
#-------------------------------------------------------------------------------
function msg = turtlebot3_exercise_msgs_IKResult (uid, name, position, eps, success)
  global messageInclude;
  # {"uid":0,"name":[],"position":[],"eps":0.0,"success":false}  
  s = ['{"uid":' num2str(uid) ',"name":' array2str(name) ',"position":' array2str(position) ',"eps":' num2str(eps) ',"success":' bool2str(success) '}'];
  msg = javaObject (messageInclude, s);
endfunction