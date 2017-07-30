#! /bin/bash

sh workloads/spec_generator.sh
make superclean
make NDEBUG=1 NINFO=1 -j10

cp third_party/ramulator/configs/DDR4-config.cfg
