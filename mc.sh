#!/bin/bash
make clean
autoreconf -i
./configure
make
