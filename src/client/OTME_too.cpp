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

#include "opentxs/client/OTME_too.hpp"

#include "opentxs/api/Api.hpp"
#include "opentxs/api/Identity.hpp"
#include "opentxs/api/OT.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/client/MadeEasy.hpp"
#include "opentxs/client/OTAPI_Exec.hpp"
#include "opentxs/client/OTAPI_Wrap.hpp"
#include "opentxs/client/OT_ME.hpp"
#include "opentxs/core/crypto/CryptoEncodingEngine.hpp"
#ifdef ANDROID
#include "opentxs/core/util/android_string.hpp"
#endif // ANDROID
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/OTData.hpp"
#include "opentxs/core/String.hpp"

#include <functional>

#define MASTER_SECTION "Master"
#define PAIRED_NODES_KEY "paired_nodes"
#define INTRODUCTION_SERVER_KEY "introduction_server_id"
#define PAIRED_SECTION_PREFIX "paired_node_"
#define BRIDGE_NYM_KEY "bridge_nym_id"
#define ADMIN_PASSWORD_KEY "admin_password"
#define OWNER_NYM_KEY "owner_nym_id"
#define NOTARY_ID_KEY "notary_id"
#define BACKUP_KEY "backup"
#define CONNECTED_KEY "connected"
#define DONE_KEY "done"
#define ISSUED_UNITS_KEY "issued_units"
#define ISSUED_UNIT_PREFIX_KEY "issued_unit_"
#define ASSET_ID_PREFIX_KEY "unit_definition_"
#define ACCOUNT_ID_PREFIX_KEY "account_id_"
#define NYM_REVISION_SECTION_PREFIX "nym_revision_"
#define RENAME_KEY "rename_started"

namespace opentxs
{

OTME_too::OTME_too(
    std::recursive_mutex& lock,
    Settings& config,
    OT_API& otapi,
    OTAPI_Exec& exec,
    const MadeEasy& madeEasy,
    const OT_ME& otme,
    Wallet& wallet,
    CryptoEncodingEngine& encoding)
        : api_lock_(lock)
        , config_(config)
        , ot_api_(otapi)
        , exec_(exec)
        , made_easy_(madeEasy)
        , otme_(otme)
        , wallet_(wallet)
        , encoding_(encoding)
{
    pairing_.store(false);
    refreshing_.store(false);
    shutdown_.store(false);
    refresh_count_.store(0);
    scan_pairing();
}

Identifier OTME_too::add_background_thread(BackgroundThread thread)
{
    Identifier output;

    if (shutdown_.load()) { return output; }

    std::unique_lock<std::mutex> lock(thread_lock_);

    OTData nonce;
    encoding_.Nonce(32, nonce);
    output.CalculateDigest(nonce);
    auto& item = threads_[output];

    OT_ASSERT(!item.second);

    item.first.store(false);
    item.second.reset(new std::thread(thread, &item.first));

    return output;
}

void OTME_too::build_account_list(serverNymMap& output) const
{
    // Make sure no nyms, servers, or accounts are added or removed while
    // creating the list
    std::unique_lock<std::recursive_mutex> apiLock(api_lock_);
    const auto serverList = OT::App().Contract().ServerList();
    const auto nymCount = exec_.GetNymCount();
    const auto accountCount = exec_.GetAccountCount();

    for (const auto server : serverList) {
        const auto& serverID = server.first;

        for (std::int32_t n = 0; n < nymCount; n++ ) {
            const auto nymID = exec_.GetNym_ID(n);

            if (exec_.IsNym_RegisteredAtServer(nymID, serverID)) {
                output[serverID].insert({nymID, {}});
            }
        }
    }

    for (std::int32_t n = 0; n < accountCount; n++ ) {
        const auto accountID = exec_.GetAccountWallet_ID(n);
        const auto serverID = exec_.GetAccountWallet_NotaryID(accountID);
        const auto nymID = exec_.GetAccountWallet_NymID(accountID);

        auto& server = output[serverID];
        auto& nym = server[nymID];
        nym.push_back(accountID);
    }

    apiLock.unlock();
    yield();
}

void OTME_too::build_nym_list(std::list<std::string>& output) const
{
    output.clear();

    // Make sure no nyms are added or removed while creating the list
    std::unique_lock<std::recursive_mutex> apiLock(api_lock_);
    const auto nymCount = exec_.GetNymCount();

    for (std::int32_t n = 0; n < nymCount; n++ ) {
        output.push_back(exec_.GetNym_ID(n));
    }

    apiLock.unlock();
}

bool OTME_too::check_accounts(PairedNode& node)
{
    const auto& owner = std::get<1>(node);
    const auto& notaryID = std::get<3>(node);

    if (!check_server_registration(owner, notaryID, false, true)) {

        return false;
    }

    auto& unitMap = std::get<4>(node);
    auto& accountMap = std::get<5>(node);
    const auto needed = unitMap.size();
    std::uint64_t have = 0;
    unitTypeMap neededAccounts;
    typeUnitMap neededUnitTypes;

    for (const auto unit : unitMap) {
        const auto& type = unit.first;
        const auto& unitID = unit.second;

        if ((accountMap.find(type) == accountMap.end()) ||
            (accountMap[type].empty())) {
                neededAccounts[type] = unitID;
                neededUnitTypes[unitID] = type;
        } else {
            have++;
        }
    }

    OT_ASSERT(neededAccounts.size() == neededUnitTypes.size());

    // Just in case these were created but failed to be added to config file
    fill_existing_accounts(owner, have, neededUnitTypes, neededAccounts, node);
    yield();

    OT_ASSERT(neededAccounts.size() == neededUnitTypes.size());

    for (const auto& currency : neededAccounts) {
        if (obtain_asset_contract(owner, currency.second, notaryID)) {
            const auto accountID =
                obtain_account(owner, currency.second, notaryID);

            if (!accountID.empty()) {
                std::unique_lock<std::mutex> lock(pair_lock_);
                accountMap[currency.first] = accountID;
                lock.unlock();
                have++;
            }
        }
    }

    return (needed == have);
}

bool OTME_too::check_backup(const std::string& bridgeNymID, PairedNode& node)
{
    auto& backup = std::get<6>(node);

    if (backup) { return true; }

    if (!send_backup(bridgeNymID, node)) { return false; }

    bool dontCare = false;
    String section = PAIRED_SECTION_PREFIX;
    String sectionKey = std::to_string(std::get<0>(node)).c_str();
    section.Concatenate(sectionKey);
    std::lock_guard<std::recursive_mutex> apiLock(api_lock_);

    if (!config_.Set_bool(section, BACKUP_KEY, true, dontCare)) {

        return false;
    }

    return config_.Save();
}

bool OTME_too::check_bridge_nym(
    const std::string& bridgeNym,
    PairedNode& node)
{
    const auto& ownerNym = std::get<1>(node);
    auto bridge = obtain_nym(ownerNym, bridgeNym, "");

    if (!bridge) { return false; }

    auto claims = obtain_contact_data(ownerNym, *bridge, "");

    if (!claims) { return false; }

    auto server = obtain_server_id(ownerNym, bridgeNym, *claims);

    if (server.empty()) {
        otErr << __FUNCTION__ << ": Bridge nym does not advertise a notary."
              << std::endl;

        return false;
    }

    if (!update_notary(server, node)) { return false; }

    yield();

    if (!obtain_server_contract(ownerNym, server, true)) { return false; }

    if (!obtain_assets(bridgeNym, *claims, node)) { return false; }

    if (!update_assets(node)) { return false; }

    yield();

    return true;
}

bool OTME_too::check_introduction_server(const std::string& withNym) const
{
    if (withNym.empty()) {
        otErr << __FUNCTION__ << ": Invalid nym." << std::endl;

        return false;
    }

    std::string serverID = get_introduction_server();
    yield();

    if (serverID.empty()) {
        otErr << __FUNCTION__ << ": No introduction server configured."
              << std::endl;

        return false;
    }

    return check_server_registration(withNym, serverID, false, true);
}

bool OTME_too::check_nym_revision(
    const std::string& nymID,
    const std::string& server) const
{
    std::lock_guard<std::recursive_mutex> apiLock(api_lock_);

    auto nym = OT::App().Contract().Nym(Identifier(nymID));

    if (!nym) { return false; }

    bool dontCare = false;
    String section = NYM_REVISION_SECTION_PREFIX;
    section.Concatenate(String(nymID));
    const String key(server);
    const std::int64_t local(nym->Revision());
    std::int64_t remote = 0;

    if (!config_.Check_long(section, key, remote, dontCare)) { return false; }

    return (local == remote);
}

bool OTME_too::check_pairing(
    const std::string& bridgeNym,
    std::string& password)
{
    bool output = false;

    std::lock_guard<std::mutex> lock(pair_lock_);

    auto it = paired_nodes_.find(bridgeNym);

    if (paired_nodes_.end() != it) {
        auto& node = it->second;
        const auto& nodeIndex = std::get<0>(node);
        const auto& owner = std::get<1>(node);
        auto& existingPassword = std::get<2>(node);

        output = insert_at_index(
            nodeIndex, PairedNodeCount(), owner, bridgeNym, password);

        if (output) {
            existingPassword = password;
        }
    }

    return output;
}

void OTME_too::check_server_names()
{
    ServerNameData serverIDs;
    std::unique_lock<std::mutex> lock(pair_lock_);

    for (const auto& it : paired_nodes_) {
        const auto& bridgeNymID = it.first;
        const auto& node = it.second;
        const auto& done = std::get<9>(node);

        if (!done) {
            const auto& owner = std::get<1>(node);
            const auto& password = std::get<2>(node);
            const auto& notary = std::get<3>(node);
            serverIDs.emplace(notary,
                std::tuple<std::string, std::string, std::string>
                    {owner, bridgeNymID, password});
        }
    }

    lock.unlock();
    yield();

    set_server_names(serverIDs);
}

bool OTME_too::check_server_registration(
    const std::string& nym,
    const std::string& server,
    const bool force,
    const bool publish) const
{
    if (!force) {
        const bool registered = exec_.IsNym_RegisteredAtServer(nym, server);
        yield();

        if (registered) { return true; }
    }

    const bool updated = RegisterNym(nym, server, publish);

    if (!updated) { return false; }

    return update_nym_revision(nym, server);
}

void OTME_too::clean_background_threads()
{
    shutdown_.store(true);

    while (0 < threads_.size()) {
        for (auto it = threads_.begin(); it != threads_.end(); ) {
            auto& hook = it->second;
            auto& running = hook.first;

            if (!running.load()) {
                if (hook.second) {
                    if (hook.second->joinable()) {
                        hook.second->join();
                    }

                    hook.second.reset();
                }

                it = threads_.erase(it);
            } else {
                it++;
            }
        }
    }
}

bool OTME_too::download_nym(
    const std::string& localNym,
    const std::string& remoteNym,
    const std::string& server) const
{
    std::string serverID;

    if (server.empty()) {
        serverID = get_introduction_server();
    } else {
        serverID = server;
    }

    const auto result = otme_.check_nym(serverID, localNym, remoteNym);
    yield();

    return (1 == otme_.VerifyMessageSuccess(result));
}

std::uint64_t OTME_too::extract_assets(
    const proto::ContactData& claims,
    PairedNode& node)
{
    std::uint64_t output = 0;
    auto& unitMap = std::get<4>(node);

    for (const auto& section : claims.section()) {
        if (proto::CONTACTSECTION_CONTRACT == section.name()) {
            for (const auto& item : section.item()) {
                bool primary = false;
                bool active = false;

                for (const auto& attr : item.attribute()) {
                    if (proto::CITEMATTR_PRIMARY == attr) {
                        primary = true;
                    }

                    if (proto::CITEMATTR_ACTIVE == attr) {
                        active = true;
                    }
                }

                if (primary && active) {
                    std::unique_lock<std::mutex> lock(pair_lock_);
                    unitMap[item.type()] = item.value();
                    lock.unlock();
                    output++;
                } else {
                    OT_FAIL;
                }
            }
        }
    }

    return output;
}

std::string OTME_too::extract_server(const proto::ContactData& claims) const
{
    std::string output;

    for (const auto& section : claims.section()) {
        if (proto::CONTACTSECTION_COMMUNICATION == section.name()) {
            for (const auto& item : section.item()) {
                if (proto::CITEMTYPE_OPENTXS == item.type()) {
                    bool primary = false;
                    bool active = false;

                    for (const auto& attr : item.attribute()) {
                        if (proto::CITEMATTR_PRIMARY == attr) {
                            primary = true;
                        }

                        if (proto::CITEMATTR_ACTIVE == attr) {
                            active = true;
                        }
                    }

                    if (primary && active) {
                        output = item.value();

                        return output;
                    }
                }
            }
        }
    }

    return output;
}

std::string OTME_too::extract_server_name(const std::string& serverNymID) const
{
    std::string output;

    const auto serverNym =
        OT::App().Contract().Nym(Identifier(serverNymID));

    if (!serverNym) { return output; }

    const auto serverNymClaims = OT::App().Identity().Claims(*serverNym);

    if (!serverNymClaims) { return output; }

    for (const auto& section : serverNymClaims->section()) {
        if (proto::CONTACTSECTION_SCOPE == section.name()) {
            for (const auto& item : section.item()) {
                if (proto::CITEMTYPE_SERVER == item.type()) {
                    bool primary = false;
                    //bool active = false;

                    for (const auto& attr : item.attribute()) {
                        if (proto::CITEMATTR_PRIMARY == attr) {
                            primary = true;
                        }

                        if (proto::CITEMATTR_ACTIVE == attr) {
                            //active = true;
                        }
                    }

                    if (primary /* && active */) {
                        output = item.value();

                        return output;
                    }
                }
            }
        }
    }

    return output;
}

void OTME_too::fill_existing_accounts(
    const std::string& nym,
    std::uint64_t& have,
    typeUnitMap& neededUnits,
    unitTypeMap& neededAccounts,
    PairedNode& node)
{
    const auto& notaryID = std::get<3>(node);
    std::lock_guard<std::recursive_mutex> apiLock(api_lock_);
    const auto count = exec_.GetAccountCount();

    for (std::int32_t n = 0; n < count; n++ ) {
        const auto id = exec_.GetAccountWallet_ID(n);

        if (nym != exec_.GetAccountWallet_NymID(id)) { continue; }
        if (notaryID != exec_.GetAccountWallet_NotaryID(id)) { continue; }

        const auto unitDefinitionID =
            exec_.GetAccountWallet_InstrumentDefinitionID(id);
        auto it1 = neededUnits.find(unitDefinitionID);

        if (neededUnits.end() != it1) {
            auto it2 = neededAccounts.find(it1->second);
            auto& accountMap = std::get<5>(node);
            std::unique_lock<std::mutex> lock(pair_lock_);
            accountMap[it2->first] = id;
            lock.unlock();
            neededUnits.erase(it1);
            neededAccounts.erase(it2);
            have++;
        }
    }
}

std::unique_ptr<OTME_too::PairedNode> OTME_too::find_node(
    const std::string& identifier) const
{
    std::string notUsed;

    return find_node(identifier, notUsed);
}

std::unique_ptr<OTME_too::PairedNode> OTME_too::find_node(
    const std::string& identifier,
    std::string& bridgeNymId) const
{
    std::unique_ptr<OTME_too::PairedNode> output;

    std::lock_guard<std::mutex> lock(pair_lock_);
    const auto it = paired_nodes_.find(identifier);

    if (paired_nodes_.end() != it) {
        // identifier was bridge nym ID
        output.reset(new OTME_too::PairedNode(it->second));
        bridgeNymId = identifier;

        return output;
    }

    for (const auto& it : paired_nodes_) {
        const auto& bridge = it.first;
        const auto& node = it.second;
        const auto& index = std::get<0>(node);
        const auto& server = std::get<3>(node);

        if ((server == identifier) || (std::to_string(index) == identifier)) {
            output.reset(new OTME_too::PairedNode(node));
            bridgeNymId = bridge;

            return output;
        }
    }

    bridgeNymId.clear();

    return output;
}

std::string OTME_too::get_introduction_server() const
{
    bool keyFound = false;
    String serverID;
    std::lock_guard<std::recursive_mutex> apiLock(api_lock_);
    const bool config = config_.Check_str(
        MASTER_SECTION, INTRODUCTION_SERVER_KEY, serverID, keyFound);

    if (!config || !keyFound || !serverID.Exists()) {
        return "";
    }

    return serverID.Get();
}

std::time_t OTME_too::get_time(const std::string& alias) const
{
    std::time_t output = 0;

    try {
        output = std::stoi(alias);
    } catch (std::invalid_argument) {
        output = 0;
    } catch (std::out_of_range) {
        output = 0;
    }

    return output;
}

bool OTME_too::insert_at_index(
    const std::int64_t index,
    const std::int64_t total,
    const std::string& myNym,
    const std::string& bridgeNym,
    std::string& password) const
{
    std::lock_guard<std::recursive_mutex> apiLock(api_lock_);
    bool dontCare = false;

    if (!config_.Set_long(MASTER_SECTION, PAIRED_NODES_KEY, total, dontCare)) {

        return false;
    }

    String section = PAIRED_SECTION_PREFIX;
    String key = std::to_string(index).c_str();
    section.Concatenate(key);

    if (!config_.Set_str(
        section, BRIDGE_NYM_KEY, String(bridgeNym), dontCare)) {

        return false;
    }

    if (!config_.Set_str(
        section, ADMIN_PASSWORD_KEY, String(password), dontCare)) {

        return false;
    }

    String pw;

    if (!config_.Check_str(
        section, ADMIN_PASSWORD_KEY, pw, dontCare)) {

        return false;
    }

    password = pw.Get();

    if (!config_.Set_str(section, OWNER_NYM_KEY, String(myNym), dontCare)) {

        return false;
    }


    return config_.Save();
}

std::string OTME_too::GetPairedServer(const std::string& identifier) const
{
    std::string output;

    const auto node = find_node(identifier);

    if (node) {
        const auto& notaryID = std::get<3>(*node);
        output = notaryID;
    }

    return output;
}

void OTME_too::mark_connected(PairedNode& node)
{
    auto& connected = std::get<7>(node);
    bool dontCare = false;
    String section = PAIRED_SECTION_PREFIX;
    String sectionKey = std::to_string(std::get<0>(node)).c_str();
    section.Concatenate(sectionKey);
    std::lock_guard<std::recursive_mutex> apiLock(api_lock_);
    connected = true;
    config_.Set_bool(section, CONNECTED_KEY, connected, dontCare);
    config_.Save();
}

void OTME_too::mark_finished(const std::string& bridgeNymID)
{
    std::unique_lock<std::mutex> lock(pair_lock_);
    auto& node = paired_nodes_[bridgeNymID];
    lock.unlock();
    auto& done = std::get<9>(node);
    bool dontCare = false;
    String section = PAIRED_SECTION_PREFIX;
    String sectionKey = std::to_string(std::get<0>(node)).c_str();
    section.Concatenate(sectionKey);
    done = true;
    std::lock_guard<std::recursive_mutex> apiLock(api_lock_);
    config_.Set_bool(section, DONE_KEY, done, dontCare);
    config_.Save();
}

void OTME_too::mark_renamed(const std::string& bridgeNymID)
{
    std::unique_lock<std::mutex> lock(pair_lock_);
    auto& node = paired_nodes_[bridgeNymID];
    lock.unlock();
    auto& renamed = std::get<8>(node);
    bool dontCare = false;
    String section = PAIRED_SECTION_PREFIX;
    String sectionKey = std::to_string(std::get<0>(node)).c_str();
    section.Concatenate(sectionKey);
    renamed = true;
    std::unique_lock<std::recursive_mutex> apiLock(api_lock_);
    config_.Set_bool(section, RENAME_KEY, renamed, dontCare);
    config_.Save();
    apiLock.unlock();
    yield();
}

bool OTME_too::NodeRenamed(const std::string& identifier) const
{
    const auto node = find_node(identifier);

    if (node) {
        const auto& renamed = std::get<8>(*node);

        return renamed;
    }

    return false;
}

std::string OTME_too::obtain_account(
    const std::string& nym,
    const std::string& id,
    const std::string& server) const
{
    const std::string result =
        OT::App().API().OTME().create_asset_acct(server, nym, id);

    if (1 != OTAPI_Wrap::Message_GetSuccess(result)) { return ""; }

    return OTAPI_Wrap::Message_GetNewAcctID(result);
}

bool OTME_too::obtain_asset_contract(
    const std::string& nym,
    const std::string& id,
    const std::string& server) const
{
    std::string contract;
    bool retry = false;

    while (true) {
        contract = exec_.GetAssetType_Contract(id);
        yield();

        if (!contract.empty() || retry) { break; }

        retry = true;

        otme_.retrieve_contract(server, nym, id);
        yield();
    }

    return !contract.empty();
}

bool OTME_too::obtain_assets(
    const std::string& bridgeNym,
    const proto::ContactData& claims,
    PairedNode& node)
{
    const auto& owner = std::get<1>(node);
    const auto& notaryID = std::get<3>(node);

    std::uint64_t assets = 0;
    bool retry = false;

    while (true) {
        assets = extract_assets(claims, node);

        if (0 < assets) { break; }

        if (retry) {
            otErr << __FUNCTION__
                  << ": Bridge nym does not advertise instrument definitions."
                  << std::endl;
            break;
        }

        retry = true;

        if (!download_nym(owner, bridgeNym, notaryID)) {
            otErr << __FUNCTION__ << ": Unable to download nym."
                  << std::endl;
            break;
        }
    }

    return 0 < assets;
}

std::unique_ptr<proto::ContactData> OTME_too::obtain_contact_data(
    const std::string& localNym,
    const Nym& remoteNym,
    const std::string& server) const
{
    std::unique_ptr<proto::ContactData> output;
    bool retry = false;

    while (true) {
        output.reset(OT::App().Identity().Claims(remoteNym).release());

        if (output) { break; }

        if (retry) {
            otErr << __FUNCTION__ << ": Nym has no contact data."
                    << std::endl;
            break;
        }

        retry = true;

        if (!download_nym(localNym, String(remoteNym.ID()).Get(), server)) {
            otErr << __FUNCTION__ << ": Unable to download nym."
                  << std::endl;
            break;
        }
    }

    return output;
}

std::shared_ptr<const Nym> OTME_too::obtain_nym(
    const std::string& localNym,
    const std::string& remoteNym,
    const std::string& server) const
{
    std::shared_ptr<const Nym> output;
    bool retry = false;

    while (true) {
        output = OT::App().Contract().Nym(Identifier(remoteNym));

        if (output || retry) { break; }

        retry = true;

        if (!download_nym(localNym, remoteNym, server)) {
            otErr << __FUNCTION__ << ": Unable to download nym."
                  << std::endl;

            break;
        }
    }

    return output;
}

bool OTME_too::obtain_server_contract(
    const std::string& nym,
    const std::string& server,
    const bool publish) const
{
    std::string contract;
    bool retry = false;

    while (true) {
        contract = exec_.GetServer_Contract(server);
        yield();

        if (!contract.empty() || retry) { break; }

        retry = true;

        otme_.retrieve_contract(get_introduction_server(), nym, server);
        yield();
    }

    if (!contract.empty()) {

        return RegisterNym(nym, server, publish);
    }

    return false;
}

std::string OTME_too::obtain_server_id(
    const std::string& ownerNym,
    const std::string& bridgeNym,
    const proto::ContactData& claims) const
{
    std::string output;
    output = extract_server(claims);

    if (output.empty()) {
        download_nym(ownerNym, bridgeNym, "");
    }

    return output;
}

std::string OTME_too::obtain_server_id(const std::string& nymID) const
{
    std::string output;

    auto nym = wallet_.Nym(Identifier(nymID));

    if (!nym) { return output; }

    auto data = nym->ContactData();

    if (!data) { return output; }

    output = extract_server(*data);

    return output;
}

void OTME_too::pair(const std::string& bridgeNymID)
{
    std::unique_lock<std::mutex> lock(pair_lock_);
    auto& node = paired_nodes_[bridgeNymID];
    lock.unlock();
    const auto& ownerNym = std::get<1>(node);

    if (!check_introduction_server(ownerNym)) { return; }

    if (!check_bridge_nym(bridgeNymID, node)) { return; }

    const bool backup = check_backup(bridgeNymID, node);
    yield();
    const bool accounts = check_accounts(node);
    const bool saved = update_accounts(node);
    yield();

    if (backup && accounts && saved) {
        const auto& notary = std::get<3>(node);
        publish_server_registration(ownerNym, notary, true);
        request_connection(
            ownerNym, notary, bridgeNymID, proto::CONNECTIONINFO_BTCRPC);
        mark_connected(node);
        yield();
    }

    std::lock_guard<std::recursive_mutex> apiLock(api_lock_);
    config_.Save();
}

std::uint64_t OTME_too::PairedNodeCount() const
{
    std::int64_t result = 0;
    bool notUsed = false;
    std::lock_guard<std::recursive_mutex> apiLock(api_lock_);
    config_.Check_long(MASTER_SECTION, PAIRED_NODES_KEY, result, notUsed);

    if (1 > result) {
        return 0;
    }

    return result;
}

void OTME_too::pairing_thread()
{
    std::unique_lock<std::mutex> lock(pair_lock_);
    std::list<std::string> unfinished;

    for (const auto& it : paired_nodes_) {
        const auto& node = it.second;
        const auto& connected = std::get<7>(node);

        if (!connected) {
            unfinished.push_back(it.first);
        }
    }

    lock.unlock();
    yield();

    for (const auto& bridgeNymID : unfinished) {
        pair(bridgeNymID);
        yield();
    }

    unfinished.clear();
    check_server_names();
    pairing_.store(false);
}

bool OTME_too::PairingComplete(const std::string& identifier) const
{
    const auto node = find_node(identifier);

    if (node) {
        const auto& done = std::get<9>(*node);

        return done;
    }

    return false;
}

bool OTME_too::PairingStarted(const std::string& identifier) const
{
    const auto node = find_node(identifier);

    return bool(node);
}

bool OTME_too::PairingSuccessful(const std::string& identifier) const
{
    const auto node = find_node(identifier);

    if (node) {
        const auto& connected = std::get<7>(*node);

        return connected;
    }

    return false;
}

bool OTME_too::PairNode(
    const std::string& myNym,
    const std::string& bridgeNym,
    const std::string& password)
{
    if (myNym.empty()) {
        otErr << __FUNCTION__ << ": missing nym." << std::endl;

        return false;
    }

    if (bridgeNym.empty()) {
        otErr << __FUNCTION__ << ": missing bridge nym." << std::endl;

        return false;
    }

    if (password.empty()) {
        otErr << __FUNCTION__ << ": missing password." << std::endl;

        return false;
    }

    auto pw = CryptoEncodingEngine::SanatizeBase58(password);

    std::unique_lock<std::mutex> startLock(pair_initiate_lock_);
    const bool alreadyPairing = check_pairing(bridgeNym, pw);

    if (alreadyPairing) { return true; }

    startLock.unlock();
    std::unique_lock<std::mutex> lock(pair_lock_);
    auto total = PairedNodeCount();
    yield();
    auto index = scan_incomplete_pairing(bridgeNym);

    if (0 > index) {
        index = total;
        total++;
    }

    const bool saved = insert_at_index(index, total, myNym, bridgeNym, pw);
    yield();

    if (!saved) {
        otErr <<  __FUNCTION__ << ": Failed to update config file."
              << std::endl;

        return false;
    }

    auto& node = paired_nodes_[bridgeNym];
    auto& nodeIndex = std::get<0>(node);
    auto& owner = std::get<1>(node);
    auto& serverPassword = std::get<2>(node);

    nodeIndex = index;
    owner = myNym;
    serverPassword = pw;

    lock.unlock();
    UpdatePairing();

    return true;
}

void OTME_too::parse_pairing_section(std::uint64_t index)
{
    std::lock_guard<std::recursive_mutex> apiLock(api_lock_);
    bool notUsed = false;
    String bridgeNym, adminPassword, ownerNym;
    String section = PAIRED_SECTION_PREFIX;
    String key = std::to_string(index).c_str();
    section.Concatenate(key);
    config_.Check_str(section, BRIDGE_NYM_KEY, bridgeNym, notUsed);
    config_.Check_str(section, ADMIN_PASSWORD_KEY, adminPassword, notUsed);
    config_.Check_str(section, OWNER_NYM_KEY, ownerNym, notUsed);
    const bool ready =
        (bridgeNym.Exists() && adminPassword.Exists() && ownerNym.Exists());

    if (!ready) { return; }

    auto& node = paired_nodes_[bridgeNym.Get()];
    auto& nodeIndex = std::get<0>(node);
    auto& owner = std::get<1>(node);
    auto& password = std::get<2>(node);
    auto& notaryID = std::get<3>(node);
    auto& unitMap = std::get<4>(node);
    auto& accountMap = std::get<5>(node);
    auto& backup = std::get<6>(node);
    auto& connected = std::get<7>(node);
    auto& rename = std::get<8>(node);
    auto& done = std::get<9>(node);

    nodeIndex = index;
    owner = ownerNym.Get();
    password = adminPassword.Get();

    config_.Check_bool(section, BACKUP_KEY, backup, notUsed);
    config_.Check_bool(section, CONNECTED_KEY, connected, notUsed);
    config_.Check_bool(section, RENAME_KEY, rename, notUsed);
    config_.Check_bool(section, DONE_KEY, done, notUsed);

    String notary;
    config_.Check_str(section, NOTARY_ID_KEY, notary, notUsed);
    notaryID = notary.Get();

    std::int64_t issued = 0;
    config_.Check_long(section, ISSUED_UNITS_KEY, issued, notUsed);

    for (std::int64_t n = 0; n < issued; n++) {
        bool exists = false;
        std::int64_t type;
        String key = ISSUED_UNIT_PREFIX_KEY;
        String index = std::to_string(n).c_str();
        key.Concatenate(index);
        const bool checked = config_.Check_long(section, key, type, exists);
        const auto unit = validate_unit(type);
        const bool valid =
            checked && exists && (proto::CITEMTYPE_ERROR != unit);

        if (valid) {
            String contract, account;
            String unitKey = ASSET_ID_PREFIX_KEY;
            String accountKey = ACCOUNT_ID_PREFIX_KEY;
            String unitIndex = std::to_string(type).c_str();
            unitKey.Concatenate(unitIndex);
            accountKey.Concatenate(unitIndex);
            config_.Check_str(section, unitKey, contract, notUsed);
            config_.Check_str(section, accountKey, account, notUsed);
            unitMap[unit] = contract.Get();
            accountMap[unit] = account.Get();
        }
    }
}

bool OTME_too::publish_server_registration(
    const std::string& nymID,
    const std::string& server,
    const bool forcePrimary) const
{
    std::unique_lock<std::recursive_mutex> apiLock(api_lock_);
    auto nym = ot_api_.GetOrLoadPrivateNym(Identifier(nymID), false);

    OT_ASSERT(nullptr != nym);

    std::string claimID;
    const bool alreadyExists = OT::App().Identity().ClaimExists(
        *nym,
        proto::CONTACTSECTION_COMMUNICATION,
        proto::CITEMTYPE_OPENTXS,
        server,
        claimID);

    if (alreadyExists) { return true; }

    bool setPrimary = false;

    if (forcePrimary) {
        setPrimary = true;
    } else {
        std::string primary;
        const bool hasPrimary = OT::App().Identity().HasPrimary(
            *nym,
            proto::CONTACTSECTION_COMMUNICATION,
            proto::CITEMTYPE_OPENTXS,
            primary);
        setPrimary = !hasPrimary;
    }

    std::set<std::uint32_t> attribute;
    attribute.insert(proto::CITEMATTR_ACTIVE);

    if (setPrimary) {
        attribute.insert(proto::CITEMATTR_PRIMARY);
    }

    const Claim input{
        "",
        proto::CONTACTSECTION_COMMUNICATION,
        proto::CITEMTYPE_OPENTXS,
        server,
        0,
        0,
        attribute};

    const bool claimIsSet = OT::App().Identity().AddClaim(*nym, input);
    apiLock.unlock();
    yield();

    return claimIsSet;
}

void OTME_too::refresh_thread()
{
    serverNymMap accounts;
    build_account_list(accounts);

    for (const auto server : accounts) {
        bool updateServerNym = true;
        const auto& serverID = server.first;

        for (const auto nym : server.second) {
            const auto& nymID = nym.first;

            if (updateServerNym) {
                auto contract =
                    OT::App().Contract().Server(Identifier(serverID));

                if (contract) {
                    const auto& serverNymID = contract->Nym()->ID();
                    const auto result = otme_.check_nym(
                        serverID, nymID, String(serverNymID).Get());
                    yield();
                    // If multiple nyms are registered on this server, we only
                    // need to successfully download the nym once.
                    updateServerNym =
                        !(1 == otme_.VerifyMessageSuccess(result));
                }
            }

            bool notUsed = false;
            made_easy_.retrieve_nym(serverID, nymID, notUsed, true);
            yield();

            // If the nym's credentials have been updated since the last time
            // it was registered on the server, upload the new credentials
            if (!check_nym_revision(nymID, serverID)) {
                check_server_registration(nymID, serverID, true, false);
            }

            for (auto& account : nym.second) {
                made_easy_.retrieve_account(serverID, nymID, account, true);
                yield();
            }
        }
    }

    refresh_count_++;
    UpdatePairing();
    resend_peer_requests();
    refreshing_.store(false);
}

void OTME_too::Refresh(const std::string&)
{
    const auto refreshing = refreshing_.exchange(true);

    if (!refreshing) {
        if (refresh_thread_) {
            refresh_thread_->join();
            refresh_thread_.reset();
        }

        refresh_thread_.reset(new std::thread(&OTME_too::refresh_thread, this));
    }
}

bool OTME_too::RegisterNym(
    const std::string& nymID,
    const std::string& server,
    const bool setContactData) const
{
    const auto result = otme_.register_nym(server, nymID);
    yield();
    const bool registered =  (1 == otme_.VerifyMessageSuccess(result));
    yield();

    if (registered) {
        if (setContactData) {

            return publish_server_registration(nymID, server, false);
        } else {

            return true;
        }
    }

    return false;
}

std::uint64_t OTME_too::RefreshCount() const
{
    return refresh_count_.load();
}

bool OTME_too::request_connection(
    const std::string& nym,
    const std::string& server,
    const std::string& bridgeNymID,
    const std::int64_t type) const
{
    const auto result = otme_.request_connection(
        server,
        nym,
        bridgeNymID,
        type);

    return exec_.Message_GetSuccess(result);
}

bool OTME_too::RequestConnection(
    const std::string& nym,
    const std::string& node,
    const std::int64_t type) const
{
    std::string bridgeNymID;
    auto index = find_node(node, bridgeNymID);

    if (!index) { return false; }

    const auto& server = std::get<3>(*index);

    OT_ASSERT(!bridgeNymID.empty());

    return request_connection(nym, server, bridgeNymID, type);
}

void OTME_too::resend_bailment(
    const Identifier& nymID,
    const proto::PeerRequest& request) const
{
    const auto result = otme_.initiate_bailment(
        request.bailment().serverid(),
        request.initiator(),
        request.recipient(),
        request.bailment().unitid());

    if (1 == exec_.Message_GetSuccess(result)) {
        wallet_.PeerRequestDelete(
            nymID, Identifier(request.id()), StorageBox::SENTPEERREQUEST);
    }
}

void OTME_too::resend_bailment_notification(
    const Identifier& nymID,
    const proto::PeerRequest& request) const
{
    const auto result = otme_.notify_bailment(
        request.pendingbailment().serverid(),
        request.initiator(),
        request.recipient(),
        request.pendingbailment().unitid(),
        request.pendingbailment().txid());

    if (1 == exec_.Message_GetSuccess(result)) {
        wallet_.PeerRequestDelete(
            nymID, Identifier(request.id()), StorageBox::SENTPEERREQUEST);
    }
}

void OTME_too::resend_connection_info(
    const Identifier& nymID,
    const proto::PeerRequest& request) const
{
    auto server = request.server();

    if (server.empty()) {
        otErr << __FUNCTION__ << ": This request was saved without the server "
              << "id. Attempting lookup based on recipient nym." << std::endl;

        server = obtain_server_id(request.recipient());
    }

    const auto result = otme_.request_connection(
        server,
        request.initiator(),
        request.recipient(),
        request.connectioninfo().type());

    if (1 == exec_.Message_GetSuccess(result)) {
        wallet_.PeerRequestDelete(
            nymID, Identifier(request.id()), StorageBox::SENTPEERREQUEST);
    }
}

void OTME_too::resend_outbailment(
    const Identifier& nymID,
    const proto::PeerRequest& request) const
{
    const auto result = otme_.initiate_outbailment(
        request.outbailment().serverid(),
        request.initiator(),
        request.recipient(),
        request.outbailment().unitid(),
        request.outbailment().amount(),
        request.outbailment().instructions());

    if (1 == exec_.Message_GetSuccess(result)) {
        wallet_.PeerRequestDelete(
            nymID, Identifier(request.id()), StorageBox::SENTPEERREQUEST);
    }
}

void OTME_too::resend_store_secret(
    const Identifier& nymID,
    const proto::PeerRequest& request) const
{
    auto server = request.server();

    if (server.empty()) {
        otErr << __FUNCTION__ << ": This request was saved without the server "
              << "id. Attempting lookup based on recipient nym." << std::endl;

        server = obtain_server_id(request.recipient());
    }

    const auto result = otme_.store_secret(
        server,
        request.initiator(),
        request.recipient(),
        request.storesecret().type(),
        request.storesecret().primary(),
        request.storesecret().secondary());

    if (1 == exec_.Message_GetSuccess(result)) {
        wallet_.PeerRequestDelete(
            nymID, Identifier(request.id()), StorageBox::SENTPEERREQUEST);
    }
}

void OTME_too::resend_peer_request(
    const Identifier& nymID,
    const Identifier& requestID) const
{
    std::unique_lock<std::recursive_mutex> apiLock(api_lock_);
    std::time_t notUsed;

    auto request = wallet_.PeerRequest(
        nymID, requestID, StorageBox::SENTPEERREQUEST, notUsed);

    if (!request) { return; }

    switch (request->type()) {
        case proto::PEERREQUEST_BAILMENT : {
            resend_bailment(nymID, *request);
        } break;
        case proto::PEERREQUEST_OUTBAILMENT : {
            resend_outbailment(nymID, *request);
        } break;
        case proto::PEERREQUEST_PENDINGBAILMENT : {
            resend_bailment_notification(nymID, *request);
        } break;
        case proto::PEERREQUEST_CONNECTIONINFO : {
            resend_connection_info(nymID, *request);
        } break;
        case proto::PEERREQUEST_STORESECRET : {
            resend_store_secret(nymID, *request);
        } break;
        default : {}
    }

    apiLock.unlock();
}

void OTME_too::resend_peer_requests() const
{
    std::list<std::string> nyms;
    build_nym_list(nyms);
    const std::string empty = std::to_string(std::time_t(0));
    const std::time_t now = std::time(nullptr);
    const std::chrono::hours limit(24);

    for (const auto& nym : nyms) {
        const Identifier nymID(nym);
        const auto sent = wallet_.PeerRequestSent(nymID);

        for (const auto& request : sent) {
            const auto& id = request.first;
            const auto& alias = request.second;

            if (alias == empty) {
                otErr << __FUNCTION__ << ": Timestamp for peer request " << id
                      << " is not set. Setting now." << std::endl;

                wallet_.PeerRequestUpdate(
                    nymID, Identifier(id), StorageBox::SENTPEERREQUEST);
            } else {
                std::chrono::seconds interval(now - get_time(alias));

                if (interval > limit) {
                    otInfo << __FUNCTION__ << ": request " << id << " is "
                        << interval.count() << " seconds old. Resending."
                        << std::endl;
                    resend_peer_request(nymID, Identifier(id));
                } else {
                    otInfo << __FUNCTION__ << ": request " << id << " is "
                        << interval.count() << " seconds old. It's fine."
                        << std::endl;
                }
            }
        }
    }
}

bool OTME_too::send_backup(
    const std::string& bridgeNymID,
    PairedNode& node) const
{
    auto& notaryID = std::get<3>(node);
    auto& owner = std::get<1>(node);
    const auto result = otme_.store_secret(
        notaryID,
        owner,
        bridgeNymID,
        proto::SECRETTYPE_BIP39,
        exec_.Wallet_GetWords(),
        exec_.Wallet_GetPassphrase());
    yield();

    return 1 == otme_.VerifyMessageSuccess(result);
}

void OTME_too::send_server_name(
    const std::string& nym,
    const std::string& server,
    const std::string& password,
    const std::string& name) const
{
    OT::App().API().OTME().request_admin(server, nym, password);

    OT::App().API().OTME().server_add_claim(
        server,
        nym,
        std::to_string(proto::CONTACTSECTION_SCOPE),
        std::to_string(proto::CITEMTYPE_SERVER),
        name,
        true);
}

void OTME_too::set_server_names(const ServerNameData& servers)
{
    for (const auto server : servers) {
        const auto& notaryID = server.first;
        const auto& myNymID = std::get<0>(server.second);
        const auto& bridgeNymID = std::get<1>(server.second);
        const auto& password = std::get<2>(server.second);
        const auto contract = OT::App().Contract().Server(Identifier(notaryID));

        if (!contract) { continue; }

        const std::string localName = exec_.GetServer_Name(notaryID);
        yield();
        const auto serialized = contract->Contract();
        const auto& originalName = serialized.name();
        const auto& serverNymID = serialized.nymid();

        if (localName == originalName) {
            // Never attempted to rename this server. Nothing to do
            continue;
        }

        bool retry = false;
        bool done = false;

        while (true) {
            const std::string credentialName =
                extract_server_name(serverNymID);

            if (localName == credentialName) {
                // Server was renamed, and has published new credentials.
                mark_finished(bridgeNymID);
                yield();
                done = true;
            }

            if (done || retry) { break; }

            retry = true;

            // Perhaps our copy of the server nym credentials is out of date
            download_nym(myNymID, serverNymID, notaryID);
        }

        if (done) { continue; }

        mark_renamed(bridgeNymID);
        send_server_name(myNymID, notaryID, password, localName);
    }
}

std::string OTME_too::SetIntroductionServer(const std::string& contract) const
{
    std::string id = exec_.AddServerContract(contract);
    yield();

    if (!id.empty()) {
        bool dontCare = false;
        std::lock_guard<std::recursive_mutex> apiLock(api_lock_);
        const bool set = config_.Set_str(
            MASTER_SECTION, INTRODUCTION_SERVER_KEY, String(id), dontCare);

        if (!set) {
            id.clear();
        } else {
            config_.Save();
        }
    }

    return id;
}

std::int64_t OTME_too::scan_incomplete_pairing(const std::string& bridgeNym)
{
    std::int64_t index = -1;

    for (std::uint64_t n = 0; n < PairedNodeCount(); n++) {
        yield();
        bool notUsed = false;
        String existing;
        String section = PAIRED_SECTION_PREFIX;
        const String key = std::to_string(n).c_str();
        section.Concatenate(key);
        std::lock_guard<std::recursive_mutex> apiLock(api_lock_);
        config_.Check_str(section, BRIDGE_NYM_KEY, existing, notUsed);
        const std::string compareNym(existing.Get());

        if (compareNym == bridgeNym) {
            index = n;

            break;
        }
    }

    return index;
}

void OTME_too::scan_pairing()
{
    std::lock_guard<std::mutex> lock(pair_lock_);

    for (std::uint64_t n = 0; n < PairedNodeCount(); n++) {
        yield();
        parse_pairing_section(n);
    }
    yield();
}

void OTME_too::Shutdown()
{
    clean_background_threads();

    while (refreshing_.load()) {
        Log::Sleep(std::chrono::milliseconds(250));
    }

    while (pairing_.load()) {
        Log::Sleep(std::chrono::milliseconds(250));
    }

    if (refresh_thread_) {
        refresh_thread_->join();
        refresh_thread_.reset();
    }

    if (pairing_thread_) {
        pairing_thread_->join();
        pairing_thread_.reset();
    }
}

ThreadStatus OTME_too::Status(const Identifier& thread)
{
    if (shutdown_.load()) { return ThreadStatus::SHUTDOWN; }

    std::unique_lock<std::mutex> lock(thread_lock_);

    auto it = threads_.find(thread);

    if (threads_.end() == it) { return ThreadStatus::ERROR; }

    if (it->second.first.load()) { return ThreadStatus::RUNNING; }

    threads_.erase(it);

    return ThreadStatus::FINISHED;
}

bool OTME_too::update_accounts(const PairedNode& node)
{
    bool dontCare = false;
    String section = PAIRED_SECTION_PREFIX;
    String sectionKey = std::to_string(std::get<0>(node)).c_str();
    section.Concatenate(sectionKey);
    const auto& accountMap = std::get<5>(node);
    std::lock_guard<std::recursive_mutex> apiLock(api_lock_);

    for (const auto account : accountMap) {
        const auto& type = account.first;
        const auto& id = account.second;
        String accountKey = ACCOUNT_ID_PREFIX_KEY;
        String index = std::to_string(type).c_str();
        accountKey.Concatenate(index);

        if (!config_.Set_str(section, accountKey, String(id), dontCare)) {

            return false;
        }
    }

    return config_.Save();
}

bool OTME_too::update_assets(PairedNode& node)
{
    bool dontCare = false;
    String section = PAIRED_SECTION_PREFIX;
    String sectionKey = std::to_string(std::get<0>(node)).c_str();
    section.Concatenate(sectionKey);
    auto& unitMap = std::get<4>(node);
    const auto count = unitMap.size();
    std::lock_guard<std::recursive_mutex> apiLock(api_lock_);

    if (!config_.Set_long(section, ISSUED_UNITS_KEY, count, dontCare)) {

        return false;
    }

    std::int64_t index = 0;

    for (const auto unit : unitMap) {
        const auto& type = unit.first;
        const auto& id = unit.second;
        String assetKey = ASSET_ID_PREFIX_KEY;
        String assetIndex = std::to_string(type).c_str();
        assetKey.Concatenate(assetIndex);
        String unitKey = ISSUED_UNIT_PREFIX_KEY;
        String unitIndex = std::to_string(index).c_str();
        unitKey.Concatenate(unitIndex);

        if (!config_.Set_long(section, unitKey, type, dontCare)) {

            return false;
        }

        if (!config_.Set_str(section, assetKey, String(id), dontCare)) {

            return false;
        }

        index++;
    }

    return config_.Save();
}

bool OTME_too::update_notary(const std::string& id, PairedNode& node)
{
    bool dontCare = false;
    String section = PAIRED_SECTION_PREFIX;
    String key = std::to_string(std::get<0>(node)).c_str();
    section.Concatenate(key);
    std::lock_guard<std::recursive_mutex> apiLock(api_lock_);

    const bool set =
        config_.Set_str(section, NOTARY_ID_KEY, String(id), dontCare);

    if (!set) { return false; }

    auto& notary = std::get<3>(node);
    notary = id;

    return config_.Save();
}

bool OTME_too::update_nym_revision(
    const std::string& nymID,
    const std::string& server) const
{
    std::lock_guard<std::recursive_mutex> apiLock(api_lock_);

    auto nym = OT::App().Contract().Nym(Identifier(nymID));

    if (!nym) { return false; }

    bool dontCare = false;
    String section = NYM_REVISION_SECTION_PREFIX;
    section.Concatenate(String(nymID));
    const String key(server);
    const auto& revision = nym->Revision();

    if (!config_.Set_long(section, key, revision, dontCare)) {

        return false;
    }

    return config_.Save();
}

void OTME_too::UpdatePairing(const std::string&)
{
    const auto pairing = pairing_.exchange(true);

    if (!pairing) {
        if (pairing_thread_) {
            pairing_thread_->join();
            pairing_thread_.reset();
        }

        pairing_thread_.reset(new std::thread(&OTME_too::pairing_thread, this));
    }
}

proto::ContactItemType OTME_too::validate_unit(const std::int64_t type)
{
    proto::ContactItemType unit = static_cast<proto::ContactItemType>(type);

    switch (unit) {
        case proto::CITEMTYPE_BTC :
        case proto::CITEMTYPE_ETH :
        case proto::CITEMTYPE_XRP :
        case proto::CITEMTYPE_LTC :
        case proto::CITEMTYPE_DAO :
        case proto::CITEMTYPE_XEM :
        case proto::CITEMTYPE_DASH :
        case proto::CITEMTYPE_MAID :
        case proto::CITEMTYPE_LSK :
        case proto::CITEMTYPE_DOGE :
        case proto::CITEMTYPE_DGD :
        case proto::CITEMTYPE_XMR :
        case proto::CITEMTYPE_WAVES :
        case proto::CITEMTYPE_NXT :
        case proto::CITEMTYPE_SC :
        case proto::CITEMTYPE_STEEM :
        case proto::CITEMTYPE_AMP :
        case proto::CITEMTYPE_XLM :
        case proto::CITEMTYPE_FCT :
        case proto::CITEMTYPE_BTS :
        case proto::CITEMTYPE_USD :
        case proto::CITEMTYPE_EUR :
        case proto::CITEMTYPE_GBP :
        case proto::CITEMTYPE_INR :
        case proto::CITEMTYPE_AUD :
        case proto::CITEMTYPE_CAD :
        case proto::CITEMTYPE_SGD :
        case proto::CITEMTYPE_CHF :
        case proto::CITEMTYPE_MYR :
        case proto::CITEMTYPE_JPY :
        case proto::CITEMTYPE_CNY :
        case proto::CITEMTYPE_NZD :
        case proto::CITEMTYPE_THB :
        case proto::CITEMTYPE_HUF :
        case proto::CITEMTYPE_AED :
        case proto::CITEMTYPE_HKD :
        case proto::CITEMTYPE_MXN :
        case proto::CITEMTYPE_ZAR :
        case proto::CITEMTYPE_PHP :
        case proto::CITEMTYPE_SEK : {
        } break;
        default : { return proto::CITEMTYPE_ERROR; }
    }

    return unit;
}

void OTME_too::yield() const
{
    Log::Sleep(std::chrono::milliseconds(50));
}

OTME_too::~OTME_too()
{
    Shutdown();
}
} // namespace opentxs
