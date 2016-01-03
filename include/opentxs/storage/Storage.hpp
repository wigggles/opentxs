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

#ifndef OPENTXS_STORAGE_STORAGE_HPP
#define OPENTXS_STORAGE_STORAGE_HPP

#include <cstdint>
#include <string>

#include <opentxs-proto/verify/VerifyCredentials.hpp>

namespace opentxs
{

// Interface for local storage
class Storage
{
private:
    static Storage* instance_pointer_;

    Storage(Storage const&) = delete;
    Storage& operator=(Storage const&) = delete;


protected:
    static std::string root_;

    Storage();
    virtual void Init();

public:
    enum class Type : std::uint8_t {
        ERROR = 0,
        FS = 1
    };

    static Storage& Instance();
    static Storage& Factory(std::string param = "", Type type = Type::ERROR);

    bool Store(const proto::Credential& data);
    virtual bool Store(const std::string& key, const std::string& value) = 0;

    virtual void Cleanup();
    virtual ~Storage();
};

template<class T>
std::string ProtoAsString(const T& serialized)
{
    int size = serialized.ByteSize();
    char* protoArray = new char [size];

    serialized.SerializeToArray(protoArray, size);
    std::string serializedData(protoArray, size);
    delete[] protoArray;
    return serializedData;
}

}  // namespace opentxs
#endif // OPENTXS_STORAGE_STORAGE_HPP
