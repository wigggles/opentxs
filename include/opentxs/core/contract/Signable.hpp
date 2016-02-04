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

#ifndef OPENTXS_CORE_SIGNABLE_HPP
#define OPENTXS_CORE_SIGNABLE_HPP

#include <memory>

#include <opentxs-proto/verify/VerifyCredentials.hpp>

#include <opentxs/core/Identifier.hpp>
#include <opentxs/core/String.hpp>

namespace opentxs
{

typedef std::shared_ptr<proto::Signature> SerializedSignature;
typedef std::list<SerializedSignature> Signatures;

class Signable
{
private:
    typedef Signable ot_super;

protected:
    Identifier id_;
    Signatures signatures_;
    uint32_t version_ = 0;
    String conditions_; // Human-readable portion

    // Calculate identifier
    virtual Identifier GetID() const = 0;
    // Calculate and unconditionally set id_
    bool CalculateID() { id_ = GetID(); return true; }
    // Calculate the ID and verify that it matches the existing id_ value
    bool CheckID() const { return (GetID() == id_); }

    Signable() = default;

public:
    virtual String ID() const { return id_; }
    virtual String Terms() const { return conditions_; }

    virtual bool Save() const = 0;
    virtual OTData Serialize() const = 0;
    virtual bool Validate() const = 0;

    virtual ~Signable() = default;
};

} // namespace opentxs

#endif // OPENTXS_CORE_SIGNABLE_HPP
