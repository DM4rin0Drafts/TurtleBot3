function T = pose2trans(pose)
  T = eye(4);
  T(1:3,4) = [pose.position.x pose.position.y pose.position.z]';
  T(1:3,1:3) = quat2trans(pose.orientation.x, pose.orientation.y, pose.orientation.z, pose.orientation.w);
end