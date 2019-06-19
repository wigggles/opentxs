## Open-Transactions Library Project

[![License](http://img.shields.io/:License-MPLv2-yellow.svg)](LICENSE)


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

### Contributing

All development goes in develop branch - please don't submit pull requests to
master.

Please do *NOT* use an editor that automatically reformats.

#### Running the tests

The OT directory in `~/.ot` is deleted on every `ninja test` in the `build`
directory.

BE ADVISED: Run `ninja test` in *development only*.

#### CppCheck and clang-format Git hooks

For convenience please enable the git hooks which will trigger cppcheck and
clang-format each time you push or commit. To do so type in the repo directory:

    cd .git/hooks
    ln -s ../../scripts/git_hooks/pre-push
    ln -s ../../scripts/git_hooks/pre-commit

To check your code without pushing the following command can be used:

    git push -n

### Dependencies

The default build configuration will compile all dependencies into the library.

If this behavior is not desired, modify the CMake command line as follows:

#### System opentxs-proto

Requires opentxs-proto (https://github.com/open-transactions/opentxs-proto)

    cmake -DOT_BUNDLED_OPENTXS_PROTO=OFF

#### System protobuf

Requires Google Protocol Buffers

    cmake -DOT_BUNDLED_PROTOBUF=OFF -DOT_GENERATE_PROTO=ON

#### System OpenSSL

Requires OpenSSL or compatible library installed

    cmake -DOT_BUNDLED_SSL=OFF

#### System Sodium

Requires Libsodium

    cmake -DOT_BUNDLED_SODIUM=OFF

#### System LMDB

Requires lmbd

    cmake -DOT_BUNDLED_LMDB=OFF

#### System ZeroMQ

Requires zeromq

    cmake -DOT_BUNDLED_LIBZMQ=OFF

### Build Instructions

OpenTransactions uses the CMake build system. The basic steps are:

    mkdir build
    cd build
    cmake -GNinja ..
    ninja
    ninja install
