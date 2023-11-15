#-------------------------------------------------------------------------------
# Converts array to string. NaN represents an empty array.
#-------------------------------------------------------------------------------
# Inputs:
#   bool        input boolean
#-------------------------------------------------------------------------------
# Output:
#   string      boolean as string
#-------------------------------------------------------------------------------
function string = bool2str (bool)
  if (bool > 0)
    string = 'true';
  else
    string = 'false';
  end
endfunction