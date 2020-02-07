// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/util/PIDFile.hpp"

#ifndef WIN32
#include <sys/types.h>
#include <csignal>
#include <unistd.h>
#endif

#include <atomic>
#include <cstdlib>
#include <fstream>

#include "PIDFile.hpp"

// On certain platforms we can actually check to see if the PID is running, and
// if not, we can proceed even if the PID is in the file. (To spare the user
// from having to delete the pid file by hand.)
#ifdef PREDEF_PLATFORM_UNIX
#if defined(ANDROID)
// blank
#elif defined(TARGET_OS_MAC)
#if TARGET_OS_MAC
#include <sys/wait.h>
#define OT_CHECK_PID 1
#endif
#else
#include <sys/wait.h>
#define OT_CHECK_PID 1
#endif
#endif

#define OT_METHOD "opentxs::implementation::PIDFile::"

namespace opentxs
{
opentxs::PIDFile* Factory::PIDFile(const std::string& path)
{
    return new implementation::PIDFile(path);
}
}  // namespace opentxs

namespace opentxs::implementation
{
PIDFile::PIDFile(const std::string& path)
    : path_(path)
    , open_(false)
{
    if (path_.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(" empty path").Flush();

        OT_FAIL;
    }

    if (3 > path_.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(" invalid path").Flush();

        OT_FAIL;
    }
}

bool PIDFile::can_recover(std::uint32_t pid)
{
#ifdef OT_CHECK_PID
    while (waitpid(-1, nullptr, WNOHANG) > 0) {
        // Wait for defunct....
    }

    if (0 != kill(pid, 0)) { return true; }
#endif

    return false;
}

void PIDFile::Close()
{
    if (false == isOpen()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(path_)(" not open.").Flush();

        OT_FAIL;
    }

    // PID -- Set it to 0 in the lock file so the next time we run OT, it knows
    // there isn't another copy already running (otherwise we might wind up with
    // two copies trying to write to the same data folder simultaneously, which
    // could corrupt the data...)
    std::ofstream pid_outfile(path_.c_str());

    if (pid_outfile.is_open()) {
        std::uint32_t the_pid{0};
        pid_outfile << the_pid;
        pid_outfile.close();
        open_.store(false);
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed trying to open ")(path_)
            .Flush();
        open_.store(true);
    }
}

void PIDFile::Open()
{
    if (isOpen()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(path_)(" already open.")
            .Flush();

        OT_FAIL;
    }

    LogDetail(OT_METHOD)(__FUNCTION__)(": Using Pid File: ")(path_).Flush();

    // 1. READ A FILE STORING THE PID. (It will already exist, if OT is
    // already running.)
    //
    // We open it for reading first, to see if it already exists. If it does, we
    // read the number. 0 is fine, since we overwrite with 0 on shutdown. But
    // any OTHER number means OT is still running. Or it means it was killed
    // while running and didn't shut down properly, and that you need to delete
    // the pid file by hand before running OT again. (This is all for the
    // purpose of preventing two copies of OT running at the same time and
    // corrupting the data folder.)
    std::ifstream pid_infile(path_);

    // 2. (IF FILE EXISTS WITH ANY PID INSIDE, THEN DIE.)

    if (pid_infile.is_open()) {
        std::uint32_t old_pid{0};
        pid_infile >> old_pid;
        pid_infile.close();
        const auto running = (old_pid != 0) && (false == can_recover(old_pid));

        if (running) {
#if !(defined(ANDROID) || (defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE))
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": opentxs is apparently already running.")
                .Flush();
            LogOutput("If the OT process with PID ")(old_pid)(
                " is truly not running anymore, then erase ")(path_)(
                " and restart.")
                .Flush();
            open_.store(false);

            return;
#endif
        }
    }

    // Next let's record our PID to the same file, so other copies of OT can't
    // trample on US.
    // 3. GET THE CURRENT (ACTUAL) PROCESS ID.
    std::uint64_t the_pid{0};

#ifdef _WIN32
    the_pid = GetCurrentProcessId();
#else
    the_pid = getpid();
#endif

    // 4. OPEN THE FILE IN WRITE MODE, AND SAVE THE PID TO IT.
    std::ofstream pid_outfile(path_.c_str());

    if (pid_outfile.is_open()) {
        pid_outfile << the_pid;
        pid_outfile.close();
        open_.store(true);
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed trying to open ")(path_)
            .Flush();
        open_.store(false);
    }
}

PIDFile::~PIDFile()
{
    if (isOpen()) { Close(); }
}
}  // namespace opentxs::implementation
