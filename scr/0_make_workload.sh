#! /bin/bash

sh workloads/spec_generator.sh
make superclean
make NDEBUG=1 NINFO=1 -j10
