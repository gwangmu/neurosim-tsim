#! /bin/bash

mkdir -p result
j=0
for i in 10 25 64 200 
do
    f=r0"$j".isp"$i"k
    echo "Simulate $f workloads" 

    cd simspec
    rm neurosim.simspec
    ln -s ../workloads/realworld/"$f"/neurosim/simspec/neurosim.simspec \
        neurosim.simspec
    cd ..

    ./neurosim-tsim simspec/neurosim.simspec -p 0 > result/"$f".result \
        2> result/"$f".err

    j=$((j+1))

done
