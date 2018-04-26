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

#include "opentxs/consensus/Context.hpp"

#include "opentxs/api/client/Wallet.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/String.hpp"

#ifndef OT_MAX_ACK_NUMS
#define OT_MAX_ACK_NUMS 100
#endif

#define SIGNATURE_VERSION 2

#define OT_METHOD "Context::"

namespace opentxs
{
Context::Context(
    const std::uint32_t targetVersion,
    const ConstNym& local,
    const ConstNym& remote,
    const Identifier& server,
    std::mutex& nymfileLock)
    : ot_super(local, targetVersion)
    , nymfile_lock_(nymfileLock)
    , server_id_(Identifier::Factory(server))
    , remote_nym_(remote)
    , available_transaction_numbers_()
    , issued_transaction_numbers_()
    , request_number_(0)
    , acknowledged_request_numbers_()
    , local_nymbox_hash_(Identifier::Factory())
    , remote_nymbox_hash_(Identifier::Factory())
    , target_version_(targetVersion)
{
}

Context::Context(
    const std::uint32_t targetVersion,
    const proto::Context& serialized,
    const ConstNym& local,
    const ConstNym& remote,
    const Identifier& server,
    std::mutex& nymfileLock)
    : ot_super(local, serialized.version())
    , nymfile_lock_(nymfileLock)
    , server_id_(Identifier::Factory(server))
    , remote_nym_(remote)
    , available_transaction_numbers_()
    , issued_transaction_numbers_()
    , request_number_(serialized.requestnumber())
    , acknowledged_request_numbers_()
    , local_nymbox_hash_(Identifier::Factory(serialized.localnymboxhash()))
    , remote_nymbox_hash_(Identifier::Factory(serialized.remotenymboxhash()))
    , target_version_(targetVersion)
{
    for (const auto& it : serialized.acknowledgedrequestnumber()) {
        acknowledged_request_numbers_.insert(it);
    }

    for (const auto& it : serialized.availabletransactionnumber()) {
        available_transaction_numbers_.insert(it);
    }

    for (const auto& it : serialized.issuedtransactionnumber()) {
        issued_transaction_numbers_.insert(it);
    }

    signatures_.push_front(SerializedSignature(
        std::make_shared<proto::Signature>(serialized.signature())));
}

std::set<RequestNumber> Context::AcknowledgedNumbers() const
{
    Lock lock(lock_);

    return acknowledged_request_numbers_;
}

bool Context::add_acknowledged_number(const Lock& lock, const RequestNumber req)
{
    OT_ASSERT(verify_write_lock(lock));

    auto output = acknowledged_request_numbers_.insert(req);

    while (OT_MAX_ACK_NUMS < acknowledged_request_numbers_.size()) {
        acknowledged_request_numbers_.erase(
            acknowledged_request_numbers_.begin());
    }

    return output.second;
}

bool Context::AddAcknowledgedNumber(const RequestNumber req)
{
    Lock lock(lock_);

    return add_acknowledged_number(lock, req);
}

std::size_t Context::AvailableNumbers() const
{
    return available_transaction_numbers_.size();
}

bool Context::ConsumeAvailable(const TransactionNumber& number)
{
    Lock lock(lock_);

    return 1 == available_transaction_numbers_.erase(number);
}

bool Context::ConsumeIssued(const TransactionNumber& number)
{
    Lock lock(lock_);

    if (0 < available_transaction_numbers_.count(number)) {
        otWarn << OT_METHOD << __FUNCTION__
               << ": Consuming an issued number that was still available."
               << std::endl;

        available_transaction_numbers_.erase(number);
    }

    return 1 == issued_transaction_numbers_.erase(number);
}

proto::Context Context::contract(const Lock& lock) const
{
    OT_ASSERT(verify_write_lock(lock));

    auto output = serialize(lock);

    if (0 < signatures_.size()) {
        auto& sigProto = *output.mutable_signature();
        sigProto.CopyFrom(*signatures_.front());
    }

    return output;
}

// This method will remove entries from acknowledged_request_numbers_ if they
// are not on the provided set
void Context::finish_acknowledgements(
    const Lock& lock,
    const std::set<RequestNumber>& req)
{
    OT_ASSERT(verify_write_lock(lock));

    std::set<RequestNumber> toErase;

    for (const auto& number : acknowledged_request_numbers_) {
        if (0 == req.count(number)) {
            toErase.insert(number);
        }
    }

    for (const auto& it : toErase) {
        acknowledged_request_numbers_.erase(it);
    }
}

OTIdentifier Context::GetID(const Lock& lock) const
{
    OT_ASSERT(verify_write_lock(lock));

    auto contract = IDVersion(lock);
    Identifier id;
    id.CalculateDigest(proto::ProtoAsData(contract));

    return id;
}

bool Context::HaveLocalNymboxHash() const
{
    return String(local_nymbox_hash_).Exists();
}

bool Context::HaveRemoteNymboxHash() const
{
    return String(remote_nymbox_hash_).Exists();
}

proto::Context Context::IDVersion(const Lock& lock) const
{
    OT_ASSERT(verify_write_lock(lock));

    proto::Context output;
    output.set_version(version_);

    switch (Type()) {
        case proto::CONSENSUSTYPE_SERVER: {
            if (nym_) {
                output.set_localnym(String(nym_->ID()).Get());
            }

            if (remote_nym_) {
                output.set_remotenym(String(remote_nym_->ID()).Get());
            }

            output.set_localnymboxhash(String(local_nymbox_hash_).Get());
            output.set_remotenymboxhash(String(remote_nymbox_hash_).Get());
        } break;
        case proto::CONSENSUSTYPE_CLIENT: {
            if (nym_) {
                output.set_remotenym(String(nym_->ID()).Get());
            }

            if (remote_nym_) {
                output.set_localnym(String(remote_nym_->ID()).Get());
            }

            output.set_remotenymboxhash(String(local_nymbox_hash_).Get());
            output.set_localnymboxhash(String(remote_nymbox_hash_).Get());
        } break;
        default: {
            OT_FAIL;
        }
    }

    output.set_requestnumber(request_number_.load());

    for (const auto& it : available_transaction_numbers_) {
        output.add_availabletransactionnumber(it);
    }

    for (const auto& it : issued_transaction_numbers_) {
        output.add_issuedtransactionnumber(it);
    }

    return output;
}

RequestNumber Context::IncrementRequest()
{
    Lock lock(lock_);

    return ++request_number_;
}

bool Context::insert_available_number(const TransactionNumber& number)
{
    Lock lock(lock_);

    return available_transaction_numbers_.insert(number).second;
}

bool Context::insert_issued_number(const TransactionNumber& number)
{
    Lock lock(lock_);

    return issued_transaction_numbers_.insert(number).second;
}

bool Context::issue_number(const Lock& lock, const TransactionNumber& number)
{
    OT_ASSERT(verify_write_lock(lock));

    issued_transaction_numbers_.insert(number);
    available_transaction_numbers_.insert(number);
    const bool issued = (1 == issued_transaction_numbers_.count(number));
    const bool available = (1 == available_transaction_numbers_.count(number));
    const bool output = issued && available;

    if (!output) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to issue number "
              << number << std::endl;
        issued_transaction_numbers_.erase(number);
        available_transaction_numbers_.erase(number);
    }

    return output;
}

OTIdentifier Context::LocalNymboxHash() const
{
    Lock lock(lock_);

    return local_nymbox_hash_;
}

Editor<class NymFile> Context::mutable_Nymfile(const OTPasswordData& reason)
{
    std::function<void(class NymFile*, Lock&)> callback =
        [&](class NymFile* in, Lock& lock) -> void { this->save(in, lock); };
    auto nym = Nym::LoadPrivateNym(
        nym_->ID(), false, nullptr, nullptr, &reason, nullptr);

    return Editor<class NymFile>(nymfile_lock_, nym, callback);
}

std::string Context::Name() const
{
    Lock lock(lock_);

    return String(id(lock)).Get();
}

bool Context::NymboxHashMatch() const
{
    Lock lock(lock_);

    if (!HaveLocalNymboxHash()) {

        return false;
    }

    if (!HaveRemoteNymboxHash()) {

        return false;
    }

    return (local_nymbox_hash_ == remote_nymbox_hash_);
}

std::unique_ptr<const class NymFile> Context::Nymfile(
    const OTPasswordData& reason) const
{
    OT_ASSERT(nym_);

    Lock lock(nymfile_lock_);
    std::unique_ptr<class NymFile> output{nullptr};
    output.reset(Nym::LoadPrivateNym(
        nym_->ID(), false, nullptr, nullptr, &reason, nullptr));

    return output;
}

bool Context::RecoverAvailableNumber(const TransactionNumber& number)
{
    if (0 == number) {
        return false;
    }

    Lock lock(lock_);

    const bool issued = 1 == issued_transaction_numbers_.count(number);

    if (!issued) {
        return false;
    }

    return available_transaction_numbers_.insert(number).second;
}

const class Nym& Context::RemoteNym() const
{
    OT_ASSERT(remote_nym_);

    return *remote_nym_;
}

OTIdentifier Context::RemoteNymboxHash() const
{
    Lock lock(lock_);

    return remote_nymbox_hash_;
}

bool Context::remove_acknowledged_number(
    const Lock& lock,
    const std::set<RequestNumber>& req)
{
    OT_ASSERT(verify_write_lock(lock));

    std::size_t removed = 0;

    for (const auto& number : req) {
        removed += acknowledged_request_numbers_.erase(number);
    }

    return (0 < removed);
}

bool Context::RemoveAcknowledgedNumber(const std::set<RequestNumber>& req)
{
    Lock lock(lock_);

    return remove_acknowledged_number(lock, req);
}

RequestNumber Context::Request() const { return request_number_.load(); }

void Context::Reset()
{
    Lock lock(lock_);
    available_transaction_numbers_.clear();
    issued_transaction_numbers_.clear();
    request_number_.store(0);
}

void Context::save(class NymFile* nymfile, const Lock& lock) const
{
    OT_ASSERT(nym_);
    OT_ASSERT(nullptr != nymfile);
    OT_ASSERT(lock.mutex() == &nymfile_lock_)
    OT_ASSERT(lock.owns_lock())

    class Nym* nym = dynamic_cast<class Nym*>(nymfile);

    OT_ASSERT(nullptr != nym);

    const auto saved = nym->SaveSignedNymfile(*nym_);

    OT_ASSERT(saved);
}

proto::Context Context::serialize(
    const Lock& lock,
    const proto::ConsensusType type) const
{
    OT_ASSERT(verify_write_lock(lock));

    proto::Context output;
    output.set_version(version_);
    output.set_type(type);

    if (nym_) {
        output.set_localnym(String(nym_->ID()).Get());
    }

    if (remote_nym_) {
        output.set_remotenym(String(remote_nym_->ID()).Get());
    }

    output.set_localnymboxhash(String(local_nymbox_hash_).Get());
    output.set_remotenymboxhash(String(remote_nymbox_hash_).Get());
    output.set_requestnumber(request_number_.load());

    for (const auto& it : acknowledged_request_numbers_) {
        output.add_acknowledgedrequestnumber(it);
    }

    for (const auto& it : available_transaction_numbers_) {
        output.add_availabletransactionnumber(it);
    }

    for (const auto& it : issued_transaction_numbers_) {
        output.add_issuedtransactionnumber(it);
    }

    return output;
}

OTData Context::Serialize() const { return proto::ProtoAsData(Serialized()); }

proto::Context Context::Serialized() const
{
    Lock lock(lock_);

    return contract(lock);
}

const Identifier& Context::Server() const { return server_id_; }

void Context::SetLocalNymboxHash(const Identifier& hash)
{
    Lock lock(lock_);
    local_nymbox_hash_ = Identifier::Factory(hash);
    CalculateID(lock);
}

void Context::SetRemoteNymboxHash(const Identifier& hash)
{
    Lock lock(lock_);
    remote_nymbox_hash_ = Identifier::Factory(hash);
    CalculateID(lock);
}

void Context::SetRequest(const RequestNumber req)
{
    Lock lock(lock_);

    request_number_.store(req);
}

proto::Context Context::SigVersion(const Lock& lock) const
{
    OT_ASSERT(verify_write_lock(lock));

    auto output = serialize(lock, Type());
    output.clear_signature();

    return output;
}

bool Context::update_signature(const Lock& lock)
{
    OT_ASSERT(verify_write_lock(lock));

    if (!ot_super::update_signature(lock)) {

        return false;
    }

    if (version_ < target_version_) {
        version_ = target_version_;
    }

    bool success = false;
    signatures_.clear();
    auto serialized = SigVersion(lock);
    auto& signature = *serialized.mutable_signature();
    signature.set_version(SIGNATURE_VERSION);
    signature.set_role(proto::SIGROLE_CONTEXT);
    success = nym_->SignProto(serialized, signature);

    if (success) {
        signatures_.emplace_front(new proto::Signature(signature));
    } else {
        otErr << OT_METHOD << __FUNCTION__ << ": failed to create signature."
              << std::endl;
    }

    return success;
}

bool Context::validate(const Lock& lock) const
{
    OT_ASSERT(verify_write_lock(lock));

    if (1 != signatures_.size()) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Error: this context is not signed." << std::endl;

        return false;
    }

    return verify_signature(lock, *signatures_.front());
}

bool Context::VerifyAcknowledgedNumber(const RequestNumber& req) const
{
    Lock lock(lock_);

    return (0 < acknowledged_request_numbers_.count(req));
}

bool Context::VerifyAvailableNumber(const TransactionNumber& number) const
{
    Lock lock(lock_);

    return (0 < available_transaction_numbers_.count(number));
}

bool Context::VerifyIssuedNumber(const TransactionNumber& number) const
{
    Lock lock(lock_);

    return (0 < issued_transaction_numbers_.count(number));
}

bool Context::verify_signature(
    const Lock& lock,
    const proto::Signature& signature) const
{
    OT_ASSERT(verify_write_lock(lock));

    if (!ot_super::verify_signature(lock, signature)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Error: invalid signature."
              << std::endl;

        return false;
    }

    auto serialized = SigVersion(lock);
    auto& sigProto = *serialized.mutable_signature();
    sigProto.CopyFrom(signature);

    return nym_->VerifyProto(serialized, sigProto);
}
}  // namespace opentxs
