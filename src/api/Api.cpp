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

#include "opentxs/stdafx.hpp"

#include "opentxs/api/implementation/Api.hpp"

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/Activity.hpp"
#include "opentxs/api/ContactManager.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/client/MadeEasy.hpp"
#include "opentxs/client/OT_API.hpp"
#include "opentxs/client/OT_ME.hpp"
#include "opentxs/client/OTAPI_Exec.hpp"
#include "opentxs/client/OTME_too.hpp"
#include "opentxs/core/crypto/OTCachedKey.hpp"
#include "opentxs/core/Log.hpp"

namespace opentxs::api::implementation
{
Api::Api(
    api::Activity& activity,
    api::Settings& config,
    api::ContactManager& contacts,
    api::Crypto& crypto,
    api::Identity& identity,
    api::storage::Storage& storage,
    api::Wallet& wallet,
    api::ZMQ& zmq)
    : activity_(activity)
    , config_(config)
    , contacts_(contacts)
    , crypto_(crypto)
    , identity_(identity)
    , storage_(storage)
    , wallet_(wallet)
    , zmq_(zmq)
{
    Init();
}

void Api::Init()
{
    // Changed this to otErr (stderr) so it doesn't muddy the output.
    otLog3 << "\n\nWelcome to Open Transactions -- version " << Log::Version()
           << "\n";

    otLog4 << "(transport build: OTMessage -> OTEnvelope -> ZMQ )\n";

    // TODO in the case of Windows, figure err into this return val somehow.
    // (Or log it or something.)

    ot_api_.reset(new OT_API(
        activity_,
        config_,
        contacts_,
        crypto_,
        identity_,
        storage_,
        wallet_,
        zmq_,
        lock_));

    OT_ASSERT(ot_api_);

    otapi_exec_.reset(new OTAPI_Exec(
        activity_,
        config_,
        contacts_,
        crypto_,
        identity_,
        wallet_,
        zmq_,
        *ot_api_,
        lock_));
    made_easy_.reset(new MadeEasy(lock_, *ot_api_, wallet_));
    ot_me_.reset(new OT_ME(lock_, *ot_api_, *made_easy_, wallet_));
    otme_too_.reset(new OTME_too(
        lock_,
        config_,
        contacts_,
        *ot_api_,
        *otapi_exec_,
        *made_easy_,
        *ot_me_,
        wallet_,
        crypto_.Encode(),
        identity_));
}

OTAPI_Exec& Api::Exec(const std::string&)
{
    OT_ASSERT(otapi_exec_);

    return *otapi_exec_;
}

std::recursive_mutex& Api::Lock() const { return lock_; }

MadeEasy& Api::ME(const std::string&)
{
    OT_ASSERT(made_easy_);

    return *made_easy_;
}

OT_API& Api::OTAPI(const std::string&)
{
    OT_ASSERT(ot_api_);

    return *ot_api_;
}

OT_ME& Api::OTME(const std::string&)
{
    OT_ASSERT(ot_me_);

    return *ot_me_;
}

OTME_too& Api::OTME_TOO(const std::string&)
{
    OT_ASSERT(otme_too_);

    return *otme_too_;
}

void Api::Cleanup()
{
    if (otme_too_) {
        otme_too_->Shutdown();
    }
    otme_too_.reset();
    ot_me_.reset();
    made_easy_.reset();
    otapi_exec_.reset();
    ot_api_.reset();
}
}  // namespace opentxs::api::implementation
