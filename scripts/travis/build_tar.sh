#!/usr/bin/env bash
set -e

os="$1"
if [[ -z "$1" ]] ; then
    os="linux"
fi

# opentxs
cp -r include opentxs/

mkdir -p opentxs/bin
cp build/bin/opentxs opentxs/bin/

mkdir -p opentxs/lib
if [[ "$os" == "linux" ]] ; then
    cp build/lib/*.a opentxs/lib/
    cp build/lib/*.so opentxs/lib/
else
    cp build/lib/*.a opentxs/lib/
    cp build/lib/*.dylib opentxs/lib/
fi

# deps
cp -r deps/containers opentxs/include/
cp -r deps/irrxml opentxs/include/

# tests
mkdir -p opentxs/tests
cp scripts/tests/ot_test.opentxs opentxs/tests/

version=`cat VERSION`
compiler=${CXX}
package="opentxs-${version}-${os}-${compiler}.tar.gz"

echo "Creating package ${package}"
tar -vpczf ${package} opentxs/

mkdir s3
cp ${package} s3/
