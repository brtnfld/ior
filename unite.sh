#!/bin/bash

if [ $# -eq 0 ]; then
    nfiles=$(find . -maxdepth 1 -type f -iname "*.config" -printf '.' | wc -m)
    if [[ "$nfiles" != "1" ]]; then
       echo "More than one .config file found in current directory."
       exit 1
    fi
    file_config=$(find . -maxdepth 1 -type f -iname "*.config")
else
    file_config=$1
fi

if [ ! -f "$file_config" ]; then
    echo "$file_config does not exist."
    exit 1
fi

stripe_size=$(grep "stripe_size=" $file_config  | cut -d "=" -f2)
if test -z "$stripe_size"; then
    echo "failed to find stripe_size in $file_config"
    exit 1
fi

subfiles=( $( sed -e '1,/hdf5_file=/d' $file_config ) )
for i in "${subfiles[@]}"; do
      echo "$i"
done
if test -z "$subfiles"; then
    echo "failed to find subfiles list in $file_config"
    exit 1
fi

hdf5_file=$(grep "hdf5_file=" $file_config  | cut -d "=" -f2)
if test -z "$hdf5_file"; then
    echo "failed to find hdf5 output file in $file_config"
    exit 1
fi

rm -f $hdf5_file

skip=0
status=$nfiles
while [ $status -gt 0 ]; do
  icnt=0
  for i in "${subfiles[@]}"; do
      EXEC="dd count=1 bs=$stripe_size if=$i of=$hdf5_file skip=$skip oflag=append conv=notrunc"
      echo "$EXEC"
      err="$( $EXEC 2>&1 > /dev/null)"
      if [[ "$err" == *" cannot "* ]]; then
          subfiles=("${subfiles[@]:0:$icnt}" "${subfiles[@]:$(($icnt+1))}")
          status=${#subfiles[@]}
      else
          icnt=$(($icnt+1)) 
      fi
  done
  skip=$(($skip+1))
done


