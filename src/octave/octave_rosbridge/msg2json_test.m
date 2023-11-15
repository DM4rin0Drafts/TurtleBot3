clc; clear;

load_ros;
init_ros ();

test_sub = subscribe ("/test", "turtlebot3_exercise_msgs/FKRequest");
test_pub = advertise ("/test_back", "turtlebot3_exercise_msgs/FKResult");

p = Pose([1 2 3], [0.7 0 0.7 1]);
h = Header(2, 100, 0, "world");
p_s = PoseStamped(h, p);
new_msg = turtlebot3_exercise_msgs_FKResult(2, ["arm_link_1"], [p_s]);

unwind_protect
  while (1)
    sleep(1);
    
    if (test_sub.hasNewMessage())
      disp(test_sub.getMessage().toString());
      msg = test_sub.getMessage();
      disp(new_msg.toString());
      test_pub.publish(new_msg);
      fflush(stdout); 
    end
  endwhile
unwind_protect_cleanup
  ros.disconnect();
end_unwind_protect
