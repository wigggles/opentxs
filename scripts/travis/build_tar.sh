#!/usr/bin/env bash
set -e

os="$1"
if [[ -z "$1" ]] ; then
    os="linux"
fi

mkdir -p opentxs/include
mkdir -p opentxs/bin
mkdir -p opentxs/lib
mkdir -p opentxs/tests

cp -r include opentxs/
cp build/bin/opentxs opentxs/bin/

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
cp scripts/tests/ot_test.opentxs opentxs/tests/
cp scripts/ot/*.ot opentxs/tests/

git fetch --tags origin
version=$(git describe)
compiler=${CXX}
package="opentxs-${version%%-*}-${os}-${compiler}.tar.gz"

echo "Creating package ${package}"
tar -vpczf ${package} opentxs

mkdir s3
cp ${package} s3/
