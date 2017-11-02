/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#include "opentxs/core/stdafx.hpp"

#include "opentxs/api/OT.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/util/Signals.hpp"

extern "C" {
#include <signal.h>
}

#define OT_METHOD "opentxs::Signals::"

namespace opentxs
{
Signals::Signals(std::atomic<bool>& shutdown)
    : shutdown_(shutdown)
    , thread_(nullptr)
{
    thread_.reset(new std::thread(&Signals::handle, this));
}

void Signals::Block()
{
    sigset_t allSignals;
    sigfillset(&allSignals);
    pthread_sigmask(SIG_SETMASK, &allSignals, nullptr);
}

void Signals::handle()
{
    sigset_t allSignals;
    sigfillset(&allSignals);

    while (false == shutdown_.load()) {
        int sig{0};

        if (0 == sigwait(&allSignals, &sig)) {
            process(sig);
        } else {
            otErr << OT_METHOD << __FUNCTION__
                  << ": ERROR: Invalid signal received." << std::endl;
        }
    }
}

void Signals::process(const int signal)
{
    switch (signal) {
        case SIGINT:
        case SIGQUIT:
        case SIGKILL:
        case SIGSEGV:
        case SIGSTOP:
        case SIGSYS: {
            shutdown();
        } break;
        case SIGHUP:
        case SIGILL:
        case SIGTRAP:
        case SIGABRT:
        case SIGBUS:
        case SIGFPE:
        case SIGUSR1:
        case SIGUSR2:
        case SIGPIPE:
        case SIGALRM:
        case SIGTERM:
        case SIGSTKFLT:
        case SIGCLD:
        case SIGCONT:
        case SIGTSTP:
        case SIGTTIN:
        case SIGTTOU:
        case SIGURG:
        case SIGXCPU:
        case SIGXFSZ:
        case SIGVTALRM:
        case SIGPROF:
        case SIGWINCH:
        case SIGPOLL:
        case SIGPWR:
        default: {
        }
    }
}

void Signals::shutdown() { OT::App().Cleanup(); }

Signals::~Signals()
{
    if (thread_) {
        thread_->detach();
    }
}
}  // namespace opentxs
