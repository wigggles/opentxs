// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_OT_HPP
#define OPENTXS_OT_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/Flag.hpp"
#include "opentxs/Types.hpp"

#include <atomic>
#include <chrono>
#include <map>
#include <string>

namespace opentxs
{
/** \brief Static methods for starting up the native api.
 *  \ingroup native
 */
class OT
{
public:
    /** Context accessor
     *
     *  Returns a reference to the context singleton after it has been
     *  initialized.
     */
    static const api::Context& App();
    /** OT shutdown method
     *
     *  Call this when the application is closing, after all OT operations
     *  are complete.
     */
    static void Cleanup();
    static const api::client::Manager& ClientFactory(
        const ArgList& args,
        const std::chrono::seconds gcInterval = std::chrono::seconds(0),
        OTCaller* externalPasswordCallback = nullptr);
    static const api::client::Manager& RecoverClient(
        const ArgList& args,
        const std::string& words,
        const std::string& passphrase,
        const std::chrono::seconds gcInterval = std::chrono::seconds(0),
        OTCaller* externalPasswordCallback = nullptr);
    static void Join();
    static const opentxs::Flag& Running();
    static const api::server::Manager& ServerFactory(
        const ArgList& args,
        const std::chrono::seconds gcInterval = std::chrono::seconds(0),
        OTCaller* externalPasswordCallback = nullptr);
    static const api::Context& Start(
        const ArgList& args,
        const std::chrono::seconds gcInterval = std::chrono::seconds(0),
        OTCaller* externalPasswordCallback = nullptr);

private:
    static api::Context* instance_pointer_;
    static OTFlag running_;

    OT() = delete;
    OT(const OT&) = delete;
    OT(OT&&) = delete;
    OT& operator=(const OT&) = delete;
    OT& operator=(OT&&) = delete;
};
}  // namespace opentxs
#endif
