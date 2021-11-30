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
    echo "failed to find stripe_size in .config"
    exit 1
fi

array=( SDS_row.h5.subfile_158671996_0_of_2 SDS_row.h5.subfile_158671996_1_of_2 )
#array=( SDS_row.h5.subfile_158671996_0_of_1 )

subfile=$( sed -e '1,/hdf5_file=/d' $file_config )
for i in "${subfile[@]}"; do
      echo "$i"
done
if test -z "$subfile"; then
    echo "failed to find subfile list in .config"
    exit 1
fi

hdf5_file=$(grep "hdf5_file=" $file_config  | cut -d "=" -f2)
if test -z "$hdf5_file"; then
    echo "failed to find hdf5 output file in .config"
    exit 1
fi

rm -f $hdf5_file

N=0
status=0
while [ $status == 0 ]; do
  for i in "${array[@]}"; do
      EXEC="dd count=1 bs=$stripe_size if=$i of=$hdf5_file skip=$N oflag=append conv=notrunc"
      echo "$EXEC"
      err="$( $EXEC 2>&1 > /dev/null)"
      if [[ "$err" == *"cannot"* ]]; then
          status=1
      fi
  done 
  N=$((N+1))
done


