#!/usr/bin/env bash
set -e

os="$1"
if [[ -z "$1" ]] ; then
    os="linux"
fi

# opentxs
mkdir opentxs

cp -r include opentxs/

cp build/bin/opentxs opentxs/
if [[ "$os" == "linux" ]] ; then
    cp build/lib/*.a opentxs/
    cp build/lib/*.so opentxs/
else
    cp build/lib/*.a opentxs/
    cp build/lib/*.dylib opentxs/
fi

# deps
cp -r deps/include/containers opentxs/include/
cp -r deps/include/irrxml opentxs/include/

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
