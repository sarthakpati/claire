#!/bin/bash

# In this script we showcase how to run CLAIRE in parallel.
# We use a neuroimaging dataset. We search for an adequate
# regularization parameter using a binary search. The data
# are two brain images of size 128x150x128.




# define directory for binary (relative path)
bindir=../../bin

# define directory for data (relative path)
datdir=../data

# execute claire with default settings in parallel
mpirun -np 20 $bindir/claire -mr $datdir/brain01.nii.gz -mt $datdir/brain02.nii.gz -train binary


#### commands explained
# > mpirun -np 20  --- execute with 20 MPI tasks 
# > $bindir/claire --- name of the binary 
# > -mr <file>     --- reference image (fixed image)
# > -mt <file>     --- template image (image to be registered) 
# > -train binary  --- 'binary' search with default bound to 
#                      identify an adequate regularization 
#                      parameter (see output of solver) 
#                      'reduce' method does coarser registration but isn't as robust
# > -jbound val    --- bounds are [val,1/val]

## output flags
# > -velocity      --- this flags generates the velocity field
# > -deftemplate   --- this will give the final registered image
# > -detdefgrad    --- this will save the jacobians
# > -x /outPath/s1_--- this will write all the output files at '/outPath/' with the prefix 's1_'




################################################################
# This script is part of the C++ software
#                        ---  CLAIRE  ---
# For details see https://github.com/andreasmang/claire
################################################################
