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

#ifndef OPENTXS_UTIL_SIGNALS_HPP
#define OPENTXS_UTIL_SIGNALS_HPP

#include "opentxs/Version.hpp"

#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <thread>

namespace opentxs
{

class Signals
{
public:
    static void Block();

    Signals(std::atomic<bool>& shutdown);

    ~Signals();

private:
    static const std::map<int, std::function<void()>> handler_;

    std::atomic<bool>& shutdown_;
    std::unique_ptr<std::thread> thread_{nullptr};

    /** SIGHUP */
    static void handle_1() { ignore(); };
    /** SIGINT */
    static void handle_2() { shutdown(); };
    /** SIGQUIT */
    static void handle_3() { shutdown(); };
    /** SIGILL */
    static void handle_4() { shutdown(); };
    /** SIGTRAP */
    static void handle_5() { ignore(); };
    /** SIGABRT, SIGIOT */
    static void handle_6() { ignore(); };
    /** SIGBUS */
    static void handle_7() { ignore(); };
    /** SIGFPE */
    static void handle_8() { ignore(); };
    /** SIGKILL */
    static void handle_9() { shutdown(); };
    /** SIGUSR1 */
    static void handle_10() { ignore(); };
    /** SIGSEGV */
    static void handle_11() { ignore(); };
    /** SIGUSR2 */
    static void handle_12() { ignore(); };
    /** SIGPIPE */
    static void handle_13() { ignore(); };
    /** SIGALRM */
    static void handle_14() { ignore(); };
    /** SIGTERM */
    static void handle_15() { shutdown(); };
    /** SIGSTKFLT */
    static void handle_16() { ignore(); };
    /** SIGCLD, SIGCHLD */
    static void handle_17() { ignore(); };
    /** SIGCONT */
    static void handle_18() { ignore(); };
    /** SIGSTOP */
    static void handle_19() { shutdown(); };
    /** SIGTSTP */
    static void handle_20() { ignore(); };
    /** SIGTTIN */
    static void handle_21() { ignore(); };
    /** SIGTTOU */
    static void handle_22() { ignore(); };
    /** SIGURG */
    static void handle_23() { ignore(); };
    /** SIGXCPU */
    static void handle_24() { ignore(); };
    /** SIGXFSZ */
    static void handle_25() { ignore(); };
    /** SIGVTALRM */
    static void handle_26() { ignore(); };
    /** SIGPROF */
    static void handle_27() { ignore(); };
    /** SIGWINCH */
    static void handle_28() { ignore(); };
    /** SIGPOLL, SIGIO */
    static void handle_29() { ignore(); };
    /** SIGPWR */
    static void handle_30() { ignore(); };
    /** SIGSYS, SIGUNUSED */
    static void handle_31() { shutdown(); };
    static void ignore(){};
    static void shutdown();

    void handle();
    void process(const int signal);

    Signals() = delete;
    Signals(const Signals&) = delete;
    Signals(Signals&&) = delete;
    Signals& operator=(const Signals&) = delete;
    Signals& operator=(Signals&&) = delete;
};
}  // namespace opentxs

#endif  // OPENTXS_UTIL_SIGNALS_HPP
