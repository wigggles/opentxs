// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"         // IWYU pragma: associated
#include "1_Internal.hpp"       // IWYU pragma: associated
#include "core/Identifier.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <type_traits>
#include <utility>

#include "core/Data.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Item.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/protobuf/ContactEnums.pb.h"
#include "opentxs/protobuf/Enums.pb.h"

template class opentxs::Pimpl<opentxs::Identifier>;
template class std::set<opentxs::OTIdentifier>;
template class std::map<opentxs::OTIdentifier, std::set<opentxs::OTIdentifier>>;

namespace std
{
auto less<opentxs::Pimpl<opentxs::Identifier>>::operator()(
    const opentxs::OTIdentifier& lhs,
    const opentxs::OTIdentifier& rhs) const -> bool
{
    return lhs.get() < rhs.get();
}
}  // namespace std

namespace opentxs
{
auto operator==(const OTIdentifier& lhs, const Identifier& rhs) -> bool
{
    return lhs.get().operator==(rhs);
}

auto operator==(const OTNymID& lhs, const opentxs::Identifier& rhs) -> bool
{
    return lhs.get().operator==(rhs);
}

auto operator==(const OTServerID& lhs, const opentxs::Identifier& rhs) -> bool
{
    return lhs.get().operator==(rhs);
}

auto operator==(const OTUnitID& lhs, const opentxs::Identifier& rhs) -> bool
{
    return lhs.get().operator==(rhs);
}

auto operator!=(const OTIdentifier& lhs, const Identifier& rhs) -> bool
{
    return lhs.get().operator!=(rhs);
}

auto operator!=(const OTNymID& lhs, const opentxs::Identifier& rhs) -> bool
{
    return lhs.get().operator!=(rhs);
}

auto operator!=(const OTServerID& lhs, const opentxs::Identifier& rhs) -> bool
{
    return lhs.get().operator!=(rhs);
}

auto operator!=(const OTUnitID& lhs, const opentxs::Identifier& rhs) -> bool
{
    return lhs.get().operator!=(rhs);
}

auto operator<(const OTIdentifier& lhs, const opentxs::Identifier& rhs) -> bool
{
    return lhs.get().operator<(rhs);
}

auto operator<(const OTNymID& lhs, const opentxs::Identifier& rhs) -> bool
{
    return lhs.get().operator<(rhs);
}

auto operator<(const OTServerID& lhs, const opentxs::Identifier& rhs) -> bool
{
    return lhs.get().operator<(rhs);
}

auto operator<(const OTUnitID& lhs, const opentxs::Identifier& rhs) -> bool
{
    return lhs.get().operator<(rhs);
}

auto operator>(const OTIdentifier& lhs, const opentxs::Identifier& rhs) -> bool
{
    return lhs.get().operator>(rhs);
}

auto operator>(const OTNymID& lhs, const opentxs::Identifier& rhs) -> bool
{
    return lhs.get().operator>(rhs);
}

auto operator>(const OTServerID& lhs, const opentxs::Identifier& rhs) -> bool
{
    return lhs.get().operator>(rhs);
}

auto operator>(const OTUnitID& lhs, const opentxs::Identifier& rhs) -> bool
{
    return lhs.get().operator>(rhs);
}

auto operator<=(const OTIdentifier& lhs, const opentxs::Identifier& rhs) -> bool
{
    return lhs.get().operator<=(rhs);
}

auto operator<=(const OTNymID& lhs, const opentxs::Identifier& rhs) -> bool
{
    return lhs.get().operator<=(rhs);
}

auto operator<=(const OTServerID& lhs, const opentxs::Identifier& rhs) -> bool
{
    return lhs.get().operator<=(rhs);
}

auto operator<=(const OTUnitID& lhs, const opentxs::Identifier& rhs) -> bool
{
    return lhs.get().operator<=(rhs);
}

auto operator>=(const OTIdentifier& lhs, const opentxs::Identifier& rhs) -> bool
{
    return lhs.get().operator>=(rhs);
}

auto operator>=(const OTNymID& lhs, const opentxs::Identifier& rhs) -> bool
{
    return lhs.get().operator>=(rhs);
}

auto operator>=(const OTServerID& lhs, const opentxs::Identifier& rhs) -> bool
{
    return lhs.get().operator>=(rhs);
}

auto operator>=(const OTUnitID& lhs, const opentxs::Identifier& rhs) -> bool
{
    return lhs.get().operator>=(rhs);
}

auto Identifier::Factory() -> OTIdentifier
{
    return OTIdentifier(new implementation::Identifier());
}

auto identifier::Nym::Factory() -> OTNymID
{
    return OTNymID(new implementation::Identifier());
}

auto identifier::Server::Factory() -> OTServerID
{
    return OTServerID(new implementation::Identifier());
}

auto identifier::UnitDefinition::Factory() -> OTUnitID
{
    return OTUnitID(new implementation::Identifier());
}

auto Identifier::Factory(const Identifier& rhs) -> OTIdentifier
{
    return OTIdentifier(
#ifndef _WIN32
        rhs.clone()
#else
        dynamic_cast<Identifier*>(rhs.clone())
#endif
    );
}

auto Identifier::Factory(const std::string& rhs) -> OTIdentifier
{
    return OTIdentifier(new implementation::Identifier(rhs));
}

auto identifier::Nym::Factory(const std::string& rhs) -> OTNymID
{
    return OTNymID(new implementation::Identifier(rhs));
}

auto identifier::Server::Factory(const std::string& rhs) -> OTServerID
{
    return OTServerID(new implementation::Identifier(rhs));
}

auto identifier::UnitDefinition::Factory(const std::string& rhs) -> OTUnitID
{
    return OTUnitID(new implementation::Identifier(rhs));
}

auto Identifier::Factory(const String& rhs) -> OTIdentifier
{
    return OTIdentifier(new implementation::Identifier(rhs));
}

auto identifier::Nym::Factory(const String& rhs) -> OTNymID
{
    return OTNymID(new implementation::Identifier(rhs));
}

auto identifier::Server::Factory(const String& rhs) -> OTServerID
{
    return OTServerID(new implementation::Identifier(rhs));
}

auto identifier::UnitDefinition::Factory(const String& rhs) -> OTUnitID
{
    return OTUnitID(new implementation::Identifier(rhs));
}

auto Identifier::Factory(const identity::Nym& nym) -> OTIdentifier
{
    return OTIdentifier(new implementation::Identifier(nym));
}

auto identifier::Nym::Factory(const identity::Nym& rhs) -> OTNymID
{
    return OTNymID(new implementation::Identifier(rhs));
}

auto Identifier::Factory(const Contract& contract) -> OTIdentifier
{
    return OTIdentifier(new implementation::Identifier(contract));
}

auto Identifier::Factory(const Cheque& cheque) -> OTIdentifier
{
    return OTIdentifier(
        implementation::Identifier::contract_contents_to_identifier(cheque));
}

auto Identifier::Factory(const Item& item) -> OTIdentifier
{
    return OTIdentifier(
        implementation::Identifier::contract_contents_to_identifier(item));
}

auto Identifier::Factory(
    const proto::ContactItemType type,
    const proto::HDPath& path) -> OTIdentifier
{
    return OTIdentifier(new implementation::Identifier(type, path));
}

auto Identifier::Random() -> OTIdentifier
{
    OTIdentifier output{new implementation::Identifier};
    auto nonce = Data::Factory();
    Context().Crypto().Encode().Nonce(32, nonce);

    OT_ASSERT(32 == nonce->size());

    output->CalculateDigest(nonce->Bytes());

    OT_ASSERT(false == output->empty());

    return output;
}

auto Identifier::Validate(const std::string& input) -> bool
{
    if (input.empty()) { return false; }

    const auto id = Factory(input);

    return (0 < id->size());
}
}  // namespace opentxs

namespace opentxs::implementation
{
Identifier::Identifier()
    : ot_super()
{
}

Identifier::Identifier(const std::string& theStr)
    : ot_super()
{
    SetString(theStr);
}

Identifier::Identifier(const String& theStr)
    : ot_super()
{
    SetString(theStr);
}

Identifier::Identifier(const Contract& theContract)
    : ot_super()  // Get the contract's ID into this identifier.
{
    (const_cast<Contract&>(theContract)).GetIdentifier(*this);
}

Identifier::Identifier(const identity::Nym& theNym)
    : ot_super()  // Get the Nym's ID into this identifier.
{
    (const_cast<identity::Nym&>(theNym)).GetIdentifier(*this);
}

Identifier::Identifier(const Vector& data, const ID type)
    : ot_super(data)
    , type_(type)
{
}

Identifier::Identifier(
    const proto::ContactItemType type,
    const proto::HDPath& path)
    : ot_super()
{
    CalculateDigest(path_to_data(type, path)->Bytes(), DefaultType);
}

auto Identifier::operator==(const opentxs::Identifier& s2) const -> bool
{
    const auto ots1 = String::Factory(*this), ots2 = String::Factory(s2);
    return ots1->Compare(ots2);
}

auto Identifier::operator!=(const opentxs::Identifier& s2) const -> bool
{
    const auto ots1 = String::Factory(*this), ots2 = String::Factory(s2);
    return !(ots1->Compare(ots2));
}

auto Identifier::operator>(const opentxs::Identifier& s2) const -> bool
{
    const auto ots1 = String::Factory(*this), ots2 = String::Factory(s2);
    return ots1->operator>(ots2);
}

auto Identifier::operator<(const opentxs::Identifier& s2) const -> bool
{
    const auto ots1 = String::Factory(*this), ots2 = String::Factory(s2);
    return ots1->operator<(ots2);
}

auto Identifier::operator<=(const opentxs::Identifier& s2) const -> bool
{
    const auto ots1 = String::Factory(*this), ots2 = String::Factory(s2);
    return ots1->operator<=(ots2);
}

auto Identifier::operator>=(const opentxs::Identifier& s2) const -> bool
{
    const auto ots1 = String::Factory(*this), ots2 = String::Factory(s2);
    return ots1->operator>=(ots2);
}

auto Identifier::CalculateDigest(const ReadView bytes, const ID type) -> bool
{
    type_ = type;

    return Context().Crypto().Hash().Digest(
        IDToHashType(type_), bytes, WriteInto());
}

auto Identifier::clone() const -> Identifier*
{
    return new Identifier(data_, type_);
}

auto Identifier::contract_contents_to_identifier(const Contract& in)
    -> Identifier*
{
    auto output = new Identifier();

    OT_ASSERT(nullptr != output);

    output->CalculateDigest(String::Factory(in)->Bytes());

    return output;
}

// This Identifier is stored in binary form.
// But what if you want a pretty string version of it?
// Just call this function.
void Identifier::GetString(String& id) const
{
    auto data = Data::Factory();
    data->Assign(&type_, sizeof(type_));

    OT_ASSERT(1 == data->size());

    if (0 == size()) { return; }

    data->Concatenate(this->data(), size());

    auto output = String::Factory("ot");
    output->Concatenate(String::Factory(
        Context().Crypto().Encode().IdentifierEncode(data).c_str()));
    id.swap(output);
}

auto Identifier::IDToHashType(const ID type) -> proto::HashType
{
    switch (type) {
        case (ID::sha256): {
            return proto::HASHTYPE_SHA256;
        }
        case (ID::blake2b): {
            return proto::HASHTYPE_BLAKE2B160;
        }
        default: {
            return proto::HASHTYPE_NONE;
        }
    }
}

auto Identifier::path_to_data(
    const proto::ContactItemType type,
    const proto::HDPath& path) -> OTData
{
    auto output = Data::Factory(static_cast<const void*>(&type), sizeof(type));
    output += Data::Factory(path.root().c_str(), path.root().size());

    for (const auto& child : path.child()) {
        output += Data::Factory(&child, sizeof(child));
    }

    return output;
}

// SET (binary id) FROM ENCODED STRING
void Identifier::SetString(const String& encoded)
{
    return SetString(std::string(encoded.Get()));
}

void Identifier::SetString(const std::string& encoded)
{
    empty();

    if (MinimumSize > encoded.size()) { return; }

    if ('o' != encoded.at(0)) { return; }
    if ('t' != encoded.at(1)) { return; }

    std::string input(encoded.data() + 2, encoded.size() - 2);
    auto data = Context().Crypto().Encode().IdentifierDecode(input);

    if (!data.empty()) {
        type_ = static_cast<ID>(data[0]);

        switch (type_) {
            case (ID::sha256): {
            } break;
            case (ID::blake2b): {
            } break;
            default: {
                type_ = ID::invalid;

                return;
            }
        }

        Assign(
            (reinterpret_cast<const std::uint8_t*>(data.data()) + 1),
            (data.size() - 1));
    }
}

auto Identifier::str() const -> std::string
{
    auto output = String::Factory();
    GetString(output);

    return output->Get();
}

void Identifier::swap(opentxs::Identifier& rhs)
{
    auto& input = dynamic_cast<Identifier&>(rhs);
    ot_super::swap(std::move(input));
    std::swap(type_, input.type_);
}
}  // namespace opentxs::implementation
