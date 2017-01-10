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

#include "opentxs/client/MadeEasy.hpp"
#include "opentxs/client/OTAPI_Exec.hpp"
#include "opentxs/client/OTAPI_Wrap.hpp"
#include "opentxs/client/OT_ME.hpp"
#include "opentxs/core/app/App.hpp"
#include "opentxs/core/app/Api.hpp"
#include "opentxs/core/app/Identity.hpp"
#include "opentxs/core/app/Settings.hpp"
#include "opentxs/core/app/Wallet.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/Identifier.hpp"

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

namespace opentxs
{

OTME_too::OTME_too(
    std::recursive_mutex& lock,
    Settings& config,
    OTAPI_Exec& exec,
    const MadeEasy& madeEasy,
    const OT_ME& otme)
        : api_lock_(lock)
        , config_(config)
        , exec_(exec)
        , made_easy_(madeEasy)
        , otme_(otme)
{
    pairing_.store(false);
    refreshing_.store(false);
    scan_pairing();
}

bool OTME_too::check_accounts(PairedNode& node)
{
    const auto& owner = std::get<1>(node);
    const auto& notaryID = std::get<3>(node);

    if (!check_server_registration(owner, notaryID)) { return false; }

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
    std::lock_guard<std::mutex> lock(pair_lock_);

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

    if (!obtain_server_contract(ownerNym, server)) { return false; }

    if (!obtain_assets(bridgeNym, *claims, node)) { return false; }

    if (!update_assets(node)) { return false; }

    return true;
}

bool OTME_too::check_introduction_server(const std::string& withNym) const
{
    if (withNym.empty()) {
        otErr << __FUNCTION__ << ": Invalid nym." << std::endl;

        return false;
    }

    std::string serverID = get_introduction_server();

    if (serverID.empty()) {
        otErr << __FUNCTION__ << ": No introduction server configured."
              << std::endl;

        return false;
    }

    return check_server_registration(withNym, serverID);
}

void OTME_too::check_server_names()
{
    ServerNameData serverIDs;
    std::unique_lock<std::mutex> lock(pair_lock_);

    for (const auto& it : paired_nodes_) {
        const auto& bridgeNymID = it.first;
        const auto& node = it.second;
        const auto& done = std::get<8>(node);

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

    set_server_names(serverIDs);
}

bool OTME_too::check_server_registration(
    const std::string& nym,
    const std::string& server) const
{
    if (exec_.IsNym_RegisteredAtServer(nym, server)) { return true; }

    return 1 == otme_.VerifyMessageSuccess(otme_.register_nym(server, nym));
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

    return 0 < otme_.VerifyMessageSuccess(
        otme_.check_nym(serverID, localNym, remoteNym));
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
        App::Me().Contract().Nym(Identifier(serverNymID));

    if (!serverNym) { return output; }

    const auto serverNymClaims = App::Me().Identity().Claims(*serverNym);

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
    std::unique_ptr<OTME_too::PairedNode> output;

    std::lock_guard<std::mutex> lock(pair_lock_);
    const auto it = paired_nodes_.find(identifier);

    if (paired_nodes_.end() != it) {
        // identifier was bridge nym ID
        output.reset(new OTME_too::PairedNode(it->second));

        return output;
    }

    for (const auto& it : paired_nodes_) {
        const auto& node = it.second;
        const auto& index = std::get<0>(node);
        const auto& server = std::get<3>(node);

        if ((server == identifier) || (std::to_string(index) == identifier)) {
            output.reset(new OTME_too::PairedNode(node));

            return output;
        }
    }

    return output;
}

std::string OTME_too::get_introduction_server() const
{
    bool keyFound = false;
    String serverID;
    const bool config = config_.Check_str(
        MASTER_SECTION, INTRODUCTION_SERVER_KEY, serverID, keyFound);

    if (!config || !keyFound || !serverID.Exists()) {
        return "";
    }

    return serverID.Get();
}

bool OTME_too::insert_at_index(
    const std::int64_t index,
    const std::int64_t total,
    const std::string& myNym,
    const std::string& bridgeNym,
    const std::string& password) const
{
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


    if (!config_.Set_str(section, OWNER_NYM_KEY, String(myNym), dontCare)) {

        return false;
    }


    return config_.Save();
}

void OTME_too::mark_connected(PairedNode& node)
{
    auto& connected = std::get<7>(node);
    bool dontCare = false;
    String section = PAIRED_SECTION_PREFIX;
    String sectionKey = std::to_string(std::get<0>(node)).c_str();
    section.Concatenate(sectionKey);
    std::lock_guard<std::mutex> lock(pair_lock_);
    connected = true;
    config_.Set_bool(section, CONNECTED_KEY, connected, dontCare);
    config_.Save();
}

void OTME_too::mark_finished(const std::string& bridgeNymID)
{
    std::lock_guard<std::mutex> lock(pair_lock_);
    auto& node = paired_nodes_[bridgeNymID];
    auto& done = std::get<8>(node);
    bool dontCare = false;
    String section = PAIRED_SECTION_PREFIX;
    String sectionKey = std::to_string(std::get<0>(node)).c_str();
    section.Concatenate(sectionKey);
    done = true;
    config_.Set_bool(section, DONE_KEY, done, dontCare);
    config_.Save();
}

std::string OTME_too::obtain_account(
    const std::string& nym,
    const std::string& id,
    const std::string& server) const
{
    const std::string result =
        App::Me().API().OTME().create_asset_acct(server, nym, id);

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

        if (!contract.empty() || retry) { break; }

        retry = true;

        App::Me().API().OTME().retrieve_contract(server, nym, id);
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
        output.reset(App::Me().Identity().Claims(remoteNym).release());

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
        output = App::Me().Contract().Nym(Identifier(remoteNym));

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
    const std::string& server) const
{
    std::string contract;
    bool retry = false;

    while (true) {
        contract = exec_.GetServer_Contract(server);

        if (!contract.empty() || retry) { break; }

        retry = true;

        otme_.retrieve_contract(
            get_introduction_server(), nym, server);
    }

    if (!contract.empty()) {
        const auto result = otme_.register_nym(server, nym);

        return (1 == otme_.VerifyMessageSuccess(result));
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

void OTME_too::pair(const std::string& bridgeNymID)
{
    std::unique_lock<std::mutex> lock(pair_lock_);
    auto& node = paired_nodes_[bridgeNymID];
    lock.unlock();
    const auto& ownerNym = std::get<1>(node);

    if (!check_introduction_server(ownerNym)) { return; }
    if (!check_bridge_nym(bridgeNymID, node)) { return; }

    const bool backup = check_backup(bridgeNymID, node);
    const bool accounts = check_accounts(node);
    const bool saved = update_accounts(node);

    if (backup && accounts && saved) {
        mark_connected(node);
    }

    config_.Save();
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

    for (const auto& bridgeNymID : unfinished) {
        pair(bridgeNymID);
    }

    unfinished.clear();
    check_server_names();
    pairing_.store(false);
}

bool OTME_too::PairingComplete(const std::string& identifier) const
{
    const auto node = find_node(identifier);

    if (node) {
        const auto& done = std::get<8>(*node);

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

    std::unique_lock<std::mutex> startLock(pair_initiate_lock_);
    const bool alreadyPairing = PairingStarted(bridgeNym);

    if (alreadyPairing) { return true; }

    startLock.unlock();
    std::unique_lock<std::mutex> lock(pair_lock_);
    auto total = paired_nodes();
    auto index = scan_incomplete_pairing(bridgeNym);

    if (0 > index) {
        index = total;
        total++;
    }

    const bool saved =
        insert_at_index(index, total, myNym, bridgeNym, password);

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
    serverPassword = password;

    lock.unlock();
    UpdatePairing();

    return true;
}

std::uint64_t OTME_too::paired_nodes() const
{
    std::int64_t result = 0;
    bool notUsed = false;
    config_.Check_long(MASTER_SECTION, PAIRED_NODES_KEY, result, notUsed);

    if (1 > result) {
        return 0;
    }

    return result;
}

void OTME_too::parse_pairing_section(std::uint64_t index)
{
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
    auto& done = std::get<8>(node);

    nodeIndex = index;
    owner = ownerNym.Get();
    password = adminPassword.Get();

    config_.Check_bool(section, BACKUP_KEY, backup, notUsed);
    config_.Check_bool(section, CONNECTED_KEY, connected, notUsed);
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

void OTME_too::refresh_thread()
{
    typedef std::map<std::string, std::list<std::string>> nymAccountMap;
    typedef std::map<std::string, nymAccountMap> serverNymMap;

    serverNymMap accounts;

    std::unique_lock<std::recursive_mutex> apiLock(api_lock_);
    const auto serverList = App::Me().Contract().ServerList();
    const auto nymCount = exec_.GetNymCount();
    const auto accountCount = exec_.GetAccountCount();

    for (const auto server : serverList) {
        const auto& serverID = server.first;

        for (std::int32_t n = 0; n < nymCount; n++ ) {
            const auto nymID = exec_.GetNym_ID(n);

            if (exec_.IsNym_RegisteredAtServer(nymID, serverID)) {
                accounts[serverID].insert({nymID, {}});
            }
        }
    }

    for (std::int32_t n = 0; n < accountCount; n++ ) {
        const auto accountID = exec_.GetAccountWallet_ID(n);
        const auto serverID = exec_.GetAccountWallet_NotaryID(accountID);
        const auto nymID = exec_.GetAccountWallet_NymID(accountID);

        auto& server = accounts[serverID];
        auto& nym = server[nymID];
        nym.push_back(accountID);
    }

    apiLock.unlock();
    std::this_thread::yield();

    for (const auto server : accounts) {
        const auto& serverID = server.first;

        for (const auto nym : server.second) {
            const auto& nymID = nym.first;
            bool notUsed = false;
            made_easy_.retrieve_nym(serverID, nymID, notUsed, true);
            std::this_thread::yield();

            for (auto& account : nym.second) {
                made_easy_.retrieve_account(serverID, nymID, account, true);
                std::this_thread::yield();
            }
        }
    }

    UpdatePairing();
    refreshing_.store(false);
}

void OTME_too::Refresh(const std::string&)
{
    if (!refreshing_.load()) {
        refreshing_.store(true);

        if (refresh_thread_) {
            refresh_thread_->join();
            refresh_thread_.reset();
        }

        refresh_thread_.reset(new std::thread(&OTME_too::refresh_thread, this));
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

    return 1 == otme_.VerifyMessageSuccess(result);
}

void OTME_too::send_server_name(
    const std::string& nym,
    const std::string& server,
    const std::string& password,
    const std::string& name) const
{
    App::Me().API().OTME().request_admin(server, nym, password);

    App::Me().API().OTME().server_add_claim(
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
        const auto contract = App::Me().Contract().Server(Identifier(notaryID));

        if (!contract) { continue; }

        const std::string localName = exec_.GetServer_Name(notaryID);
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
                done = true;
            }

            if (done || retry) { break; }

            retry = true;

            // Perhaps our copy of the server nym credentials is out of date
            download_nym(myNymID, serverNymID, notaryID);
        }

        if (done) { continue; }

        send_server_name(myNymID, notaryID, password, localName);
    }
}

std::string OTME_too::SetIntroductionServer(const std::string& contract) const
{
    std::string id = exec_.AddServerContract(contract);

    if (!id.empty()) {
        bool dontCare = false;
        std::lock_guard<std::mutex> lock(pair_lock_);
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

    for (std::uint64_t n = 0; n < paired_nodes(); n++) {
        bool notUsed = false;
        String existing;
        String section = PAIRED_SECTION_PREFIX;
        const String key = std::to_string(n).c_str();
        section.Concatenate(key);
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

    for (std::uint64_t n = 0; n < paired_nodes(); n++) {
        parse_pairing_section(n);
    }

}

void OTME_too::Shutdown() const
{
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

bool OTME_too::update_accounts(const PairedNode& node)
{
    bool dontCare = false;
    String section = PAIRED_SECTION_PREFIX;
    String sectionKey = std::to_string(std::get<0>(node)).c_str();
    section.Concatenate(sectionKey);
    const auto& accountMap = std::get<5>(node);
    std::lock_guard<std::mutex> lock(pair_lock_);

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
    std::lock_guard<std::mutex> lock(pair_lock_);

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
    std::lock_guard<std::mutex> lock(pair_lock_);

    const bool set =
        config_.Set_str(section, NOTARY_ID_KEY, String(id), dontCare);

    if (!set) { return false; }

    auto& notary = std::get<3>(node);
    notary = id;

    return config_.Save();
}

void OTME_too::UpdatePairing(const std::string&)
{
    if (!pairing_.load()) {
        pairing_.store(true);

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

OTME_too::~OTME_too()
{
    Shutdown();
}
} // namespace opentxs
