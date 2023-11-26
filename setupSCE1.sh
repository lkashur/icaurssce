#!/bin/bash 
source /cvmfs/icarus.opensciencegrid.org/products/icarus/setup_icarus.sh
kx509
voms-proxy-init -noregen -rfc -voms 'fermilab:/fermilab/icarus/Role=Analysis'  # create a "proxy" from the certificate
setup root v6_28_04c -q e20:p3915:prof
make
