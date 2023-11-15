#-------------------------------------------------------------------------------
# Advertises specified topic
#-------------------------------------------------------------------------------
# Inputs:
#   topic       topic path to advertise to
#   type        type of published data at topic
#-------------------------------------------------------------------------------
# Output:
#   pub         instance of publisher
#-------------------------------------------------------------------------------
function pub = advertise (topic, type)
  global topicInclude;
  global ros;
  pub = javaObject (topicInclude, ros, topic, type);
endfunction