#!/bin/bash

trace_dir=/afs/cs.cmu.edu/academic/class/15346-s22/public/traces/coher
for file in "$trace_dir"/*
do
  if [[ "$file" =~ (.*)_8_(.*) ]];
  then
    ./cadss-engine -s ex_proc.config -t $file -n 8 -c simpleCache
  else
    ./cadss-engine -s ex_proc.config -t $file -n 4 -c simpleCache
  fi
  echo "$file"
done
