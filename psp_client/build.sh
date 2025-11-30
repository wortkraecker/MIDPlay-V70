#!/bin/sh
export PSPDEV=/usr/local/pspdev
export PATH=$PATH:$PSPDEV/bin
make clean && make
