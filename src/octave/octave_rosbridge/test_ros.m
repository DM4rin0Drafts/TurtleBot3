clc; clear;

load_ros;
init_ros ();

# publish "Hello World" example
string_pub = advertise ("/echo", "std_msgs/String");
string_pub.publish (std_msgs_String ("Hello World!"));

# subscriber example
string_sub = subscribe ("/echo_back", "std_msgs/String");
msg = string_sub.waitForNewMessage ();
disp (msg.getData ());

# service client example
addTwoInts_client = service_client ("/add_two_ints", "rospy_tutorials/AddTwoInts");
req = rospy_tutorials_AddTwoInts (10, 20);
resp = addTwoInts_client.callServiceAndWait (req);
msg2struct(resp)

ros.disconnect ();
