#!/usr/bin/env bash
set -e

os="$1"
if [[ -z "$1" ]] ; then
    os="linux"
fi

mkdir opentxs

# opentxs
cp -r include opentxs/
if [[ "$os" == "linux" ]] ; then
    cp src/otlib/.libs/libot.so opentxs/
    cp src/otlib/.libs/libot.so.0 opentxs/
    cp src/otlib/.libs/libot.so.0.0.0 opentxs/
    cp src/otextensions/.libs/libotextensions.so opentxs/
    cp src/otextensions/.libs/libotextensions.so.0 opentxs/
    cp src/otextensions/.libs/libotextensions.so.0.0.0 opentxs/
    cp src/otapi/.libs/libotapi.so opentxs/
    cp src/otapi/.libs/libotapi.so.0 opentxs/
    cp src/otapi/.libs/libotapi.so.0.0.0 opentxs/
else
    cp src/otlib/.libs/libot.0.dylib opentxs/
    cp src/otlib/.libs/libot.dylib opentxs/
    cp src/otextensions/.libs/libotextensions.0.dylib opentxs/
    cp src/otextensions/.libs/libotextensions.dylib opentxs/
    cp src/otapi/.libs/libotapi.0.dylib opentxs/
    cp src/otapi/.libs/libotapi.dylib opentxs/
fi
cp -r src/.libs/opentxs opentxs/

# deps
mkdir opentxs/include/misc
cp deps/include/misc/Timer.hpp opentxs/include/misc/
cp deps/include/misc/tinythread.hpp opentxs/include/misc/
cp -r deps/include/containers opentxs/include/
cp -r deps/include/irrxml opentxs/include/
cp deps/src/irrxml/.libs/libirrxml.a opentxs/

version=`cat VERSION`
compiler=${CXX}
package="opentxs-${version}-${os}-${compiler}.tar.gz"

echo "Creating package ${package}"
tar -vpczf ${package} opentxs/

mkdir s3
cp ${package} s3/
