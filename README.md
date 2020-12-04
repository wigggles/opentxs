# Open-Transactions Library Project

[![License: MPL 2.0](https://img.shields.io/badge/License-MPL%202.0-brightgreen.svg)](https://opensource.org/licenses/MPL-2.0)

[![Android](https://github.com/open-transactions/opentxs/workflows/Android/badge.svg)](https://github.com/Open-Transactions/opentxs/actions)
[![Linux](https://github.com/open-transactions/opentxs/workflows/Linux/badge.svg)](https://github.com/Open-Transactions/opentxs/actions)
[![Windows](https://github.com/open-transactions/opentxs/workflows/Windows/badge.svg)](https://github.com/Open-Transactions/opentxs/actions)

The Open-Transactions project is a collaborative effort to develop a robust,
commercial-grade, fully-featured, free-software toolkit implementing the OTX
protocol as well as a full-strength financial cryptography library, API, CLI,
and prototype server. The project is managed by a worldwide community of
volunteers that use the Internet to communicate, plan, and develop the
Open-Transactions toolkit and its related documentation.

### Official Wiki

http://opentransactions.org/

### About

Open-Transactions democratizes financial and monetary actions. You can use it
for issuing currencies/stock, paying dividends, creating asset accounts,
sending/receiving digital cash, writing/depositing cheques, cashier's cheques,
creating basket currencies, trading on markets, scripting custom agreements,
recurring payments, escrow, etc.

Open-Transactions uses strong crypto. The balances are unchangeable (even by a
malicious server.) The receipts are destructible and redundant. The transactions
are unforgeable. The cash is untraceable. The cheques are non-repudiable. Etc.

This product includes software developed by Ben Laurie for use in the Lucre
project.

## Contributing

All development goes in develop branch - please don't submit pull requests to
master.

Use clang-format and cmake-format.

#### CppCheck and clang-format Git hooks

For convenience please enable the git hooks which will trigger cppcheck and
clang-format each time you push or commit. To do so type in the repo directory:

    cd .git/hooks
    ln -s ../../cmake/scripts/git_hooks/pre-push
    ln -s ../../cmake/scripts/git_hooks/pre-commit

To check your code without pushing the following command can be used:

    git push -n

## Build Instructions:

Ensure opentxs is fully checked out prior to running cmake:

    git submodule update --init --recursive

Grab VCpkg cross compiler libs:

    cd deps/vcpkg
    ./vcpkg integrate install
    ./vcpkg.exe install --triplet <OS triplet value>  @..\..\vcpkg\vcpkg.txt

### Prep Build make-

Shared Instructions:

    mkdir build
    cd build
    cmake -GNinja -DBUILD_SHARED_LIBS=ON ..
    cmake --build .

### Test Source-

    ctest -j4

### System Specific Building-

Unix:

    sudo cmake --install .

Windows powershell:

    Start-Process powershell -Verb runAs
    cd <full local repo pull path>\opentxs\build
    cmake --install .

## Optional Build Configuration Variables-

    -DCMAKE_BUILD_TYPE=<target> [Release, Debug]
    -DCMAKE_TOOLCHAIN_FILE=../deps/vcpkg/scripts/buildsystems/vcpkg.cmake
