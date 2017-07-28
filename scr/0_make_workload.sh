#! /bin/bash

cd workloads
sh spec_generator.sh
cd ..

make superclean
make NDEBUG=1 NINFO=1 -j10
