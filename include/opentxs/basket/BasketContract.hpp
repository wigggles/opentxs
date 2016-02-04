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

#ifndef OPENTXS_CORE_BASKET_BASKETCONTRACT_HPP
#define OPENTXS_CORE_BASKET_BASKETCONTRACT_HPP

#include "opentxs/core/contract/UnitDefinition.hpp"

namespace opentxs
{

class Basket;
class Nym;

class BasketContract : public UnitDefinition
{
public:
    EXPORT BasketContract(Basket& basket, Nym& signer);
    virtual ~BasketContract();

    virtual void CreateContents();

protected:
    // return -1 if error, 0 if nothing, and 1 if the node was processed.
    virtual int32_t ProcessXMLNode(irr::io::IrrXMLReader*& xml);
};

} // namespace opentxs

#endif // OPENTXS_CORE_BASKET_BASKETCONTRACT_HPP
