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

#include "opentxs/core/util/Assert.hpp"

#include "opentxs/core/util/stacktrace.h"
#include "opentxs/core/Log.hpp"

#include <cstring>
#include <exception>
#include <iostream>

#if defined (ANDROID)

#include <iomanip>

#include <unwind.h>
#include <dlfcn.h>

#include <sstream>
#include <android/log.h>



struct BacktraceState
{
    void** current;
    void** end;
};

//namespace {
size_t captureBacktrace(void** buffer, size_t max);
//void dumpBacktrace(std::ostream& os, void** buffer, size_t count);

static _Unwind_Reason_Code unwindCallback(struct _Unwind_Context* context, void* arg)
{
    BacktraceState* state = static_cast<BacktraceState*>(arg);
    uintptr_t pc = _Unwind_GetIP(context);
    if (pc) {
        if (state->current == state->end) {
            return _URC_END_OF_STACK;
        } else {
            *state->current++ = reinterpret_cast<void*>(pc);
        }
    }
    return _URC_NO_REASON;
}

size_t captureBacktrace(void** buffer, size_t max)
{
    BacktraceState state = {buffer, buffer + max};
    _Unwind_Backtrace(unwindCallback, &state);

    return state.current - buffer;
}

static inline void dumpBacktrace(std::ostream& os, void** buffer, size_t count)
{
    for (size_t idx = 0; idx < count; ++idx) {
        const void* addr = buffer[idx];
        const char* symbol = "";

        Dl_info info;
        if (dladdr(addr, &info) && info.dli_sname) {
            symbol = info.dli_sname;
        }

        os << "  #" << std::setw(2) << idx << ": " << addr << "  " << symbol << "\n";
    }
}

//void backtraceToLogcat()
//{
//    const size_t max = 30;
//    void* buffer[max];
//    std::ostringstream oss;
//
//    dumpBacktrace(oss, buffer, captureBacktrace(buffer, max));
//
//    __android_log_print(ANDROID_LOG_INFO, "app_name", "%s", oss.str().c_str());
//}

//}
#endif

Assert* Assert::s_pAssert = new Assert(Assert::doAssert);

Assert::Assert(fpt_Assert_sz_n_sz& fp1)
    : m_fpt_Assert(fp1)
{
}

size_t Assert::m_AssertDefault(const char* filename, size_t linenumber,
                               const char* message)
{
    if (message) {
        if (std::strcmp(message, "") != 0) {
            std::cerr << message << "\n";
            std::cerr.flush();
        }
    }

    const char* file = filename ? filename : "nullptr";

    std::cerr << "HERE WE ARE!!!!\n";

    std::cerr << "OT_ASSERT in " << file << " at line " << linenumber << "\n";

#if defined (ANDROID)

    //backtraceToLogcat();

    const size_t max = 30;
    void* buffer[max];
    std::ostringstream oss;

    dumpBacktrace(oss, buffer, captureBacktrace(buffer, max));

    __android_log_print(ANDROID_LOG_INFO, "app_name", "%s", oss.str().c_str());

#else
    print_stacktrace();
#endif

    std::cerr.flush();

    return 0; // since we are not logging.
}

size_t Assert::doAssert(const char* filename, size_t linenumber,
                        const char* message)
{
    if (Assert::s_pAssert == nullptr) std::terminate();
    return Assert::s_pAssert->m_fpt_Assert(filename, linenumber, message);
}
