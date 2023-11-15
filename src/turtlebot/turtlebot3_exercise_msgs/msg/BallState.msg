std_msgs/Header header
uint32 uid

geometry_msgs/Point position
geometry_msgs/Point velocity

float64 diameter  # real ball diameter

# Row-major representation of the 3x3 covariance matrix
# The orientation parameters use a fixed-axis representation.
# In order, the parameters are:
# (x, y, z, rotation about X axis, rotation about Y axis, rotation about Z axis)
float64[6] covariance

# computed confidence value [0..1] representing the trust into the model
float64 confidence
