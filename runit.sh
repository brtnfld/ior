#!/bin/bash

export USE_SELECTION_IO=1
export HDF5_USE_FILE_LOCKING=0
export H5_IOC_COUNT_PER_NODE=2
export H5_IOC_STRIPE_SIZE=65536
rm -f SDS_row.h5*
mpiexec -n 16 a.out
