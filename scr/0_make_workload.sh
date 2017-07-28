#! /bin/bash

cd workloads
make
cd ..

make superclean
make NDEBUG=1 NINFO=1 -j10
