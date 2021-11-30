#!/bin/bash

HDF5_DIR=$HOME/packages/hdf5.brtnfld/build/hdf5
FLAGS="-I$HDF5_DIR/../../src -I$HDF5_DIR/include -I$HDF5_DIR/../../src/mercury/include"

h5pcc test.c $FLAGS 
