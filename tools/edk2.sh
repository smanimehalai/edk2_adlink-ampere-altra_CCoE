#!/bin/bash
export PACKAGES_PATH=$WORKSPACE/edk2-platforms_CCoE:$WORKSPACE/edk2-platforms_CCoE/Features/Intel:$WORKSPACE/edk2_CCoE
export PYTHON_COMMAND=/usr/bin/python3.6
# export CROSS_COMPILE=aarch64-ampere-linux-gnu-
. $WORKSPACE/edk2_CCoE/edksetup.sh
