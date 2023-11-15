function q = trans2quat(T)
  w = sqrt(1 + T(1,1) + T(2,2) + T(3,3)) / 2;
  x = (T(3,2) - T(2,3))/(4*w);
  y = (T(1,3) - T(3,1))/(4*w);
  z = (T(2,1) - T(1,2))/(4*w);
  q = [x y z w];
end