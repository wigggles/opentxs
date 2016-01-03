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

#include <opentxs/storage/Storage.hpp>

#include <iostream>

#include <opentxs/storage/StorageFS.hpp>

namespace opentxs
{
Storage* Storage::instance_pointer_ = nullptr;
std::string Storage::root_ = "";

Storage::Storage()
{
    Init();
}

void Storage::Init()
{
}

Storage& Storage::Instance()
{
    if (nullptr == instance_pointer_)
    {
        std::cout
            << "Warning: you forgot to call the factory first." << std::endl
            << "The storage system is not properly initialized." << std::endl;
        instance_pointer_ = &Factory();
    }

    return *instance_pointer_;
}

Storage& Storage::Factory(std::string param, Type type)
{
    if (nullptr == instance_pointer_)
    {
        switch (type) {
            case Type::ERROR :
                std::cout
                << "Warning: replacing bad type with default." << std::endl;

                //intentional fall-through
            default :
                instance_pointer_ = new StorageFS(param);
        }
    }

    return *instance_pointer_;
}

bool Storage::Store(const proto::Credential& data)
{
    return Store(
        data.id(),
        ProtoAsString<proto::Credential>(data));
}

void Storage::Cleanup()
{
}

Storage::~Storage()
{
    Cleanup();
}


} // namespace opentxs
