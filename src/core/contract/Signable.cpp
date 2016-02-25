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

#include "opentxs/core/contract/Signable.hpp"

#include "opentxs/core/Nym.hpp"

namespace opentxs
{

String Signable::Name() const
{
    if (nullptr != nym_) {

        return nym_->GetNymName();

    } else {

        return "";
    }
}

const Nym* Signable::PublicNym() const
{
    auto nym = nym_->SerializeCredentialIndex(Nym::FULL_CREDS);

    Nym* tempNym = new Nym(String(nym.nymid()));
    tempNym->LoadCredentialIndex(nym);

    return tempNym;
}

bool Signable::SetName(const String& name)
{
    if (nullptr != nym_) {
        nym_->SetNymName(name);

        return nym_->SavePseudonym();
    }

    return false;
}

} // namespace opentxs
