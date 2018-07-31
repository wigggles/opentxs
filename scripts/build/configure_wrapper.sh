#!/bin/sh

# "${1}" configure script
# "${2}" configure options
# "${3}" CC
# "${4}" CXX
# "${5}" CFLAGS
# "${6}" CXXFLAGS
# "${7}" LDFLAGS

CC="${3}" CXX="${4}" CFLAGS="${5}" CXXFLAGS="${6}" LDFLAGS="${7}" "${1}" ${2}
