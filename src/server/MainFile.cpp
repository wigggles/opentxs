// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "MainFile.hpp"

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/core/cron/OTCron.hpp"
#include "opentxs/core/crypto/OTCachedKey.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/OTFolders.hpp"
#include "opentxs/core/util/Tag.hpp"
#include "opentxs/core/AccountList.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/OTStorage.hpp"
#include "opentxs/core/OTStringXML.hpp"
#include "opentxs/core/String.hpp"

#include "Server.hpp"
#include "Transactor.hpp"

#include <irrxml/irrXML.hpp>

#include <cinttypes>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>

#define OT_METHOD "opentxs::Mainfile::"

namespace opentxs::server
{

MainFile::MainFile(Server& server)
    : server_(server)
    , version_()
{
}

bool MainFile::SaveMainFileToString(String& strMainFile)
{
    Tag tag("notaryServer");

    // We're on version 2.0 since adding the master key.
    auto& cachedKey = server_.API().Crypto().DefaultKey();
    tag.add_attribute("version", cachedKey.IsGenerated() ? "2.0" : version_);
    tag.add_attribute("notaryID", server_.GetServerID().str());
    tag.add_attribute("serverNymID", server_.GetServerNym().ID().str());
    tag.add_attribute(
        "transactionNum",
        formatLong(server_.GetTransactor().transactionNumber()));

    if (cachedKey.IsGenerated())  // If it exists, then serialize it.
    {
        Armored ascMasterContents;

        if (cachedKey.SerializeTo(ascMasterContents)) {
            tag.add_tag("cachedKey", ascMasterContents.Get());
        } else
            Log::vError(
                "%s: Failed trying to write master key to notary file.\n",
                __FUNCTION__);
    }

    // Save the basket account information

    for (auto& it : server_.GetTransactor().idToBasketMap_) {
        String strBasketID = it.first.c_str();
        String strBasketAcctID = it.second.c_str();

        const auto BASKET_ACCOUNT_ID = Identifier::Factory(strBasketAcctID);
        auto BASKET_CONTRACT_ID = Identifier::Factory();

        bool bContractID =
            server_.GetTransactor().lookupBasketContractIDByAccountID(
                BASKET_ACCOUNT_ID, BASKET_CONTRACT_ID);

        if (!bContractID) {
            Log::vError(
                "%s: Error: Missing Contract ID for basket ID %s\n",
                __FUNCTION__,
                strBasketID.Get());
            break;
        }

        String strBasketContractID(BASKET_CONTRACT_ID);

        TagPtr pTag(new Tag("basketInfo"));

        pTag->add_attribute("basketID", strBasketID.Get());
        pTag->add_attribute("basketAcctID", strBasketAcctID.Get());
        pTag->add_attribute("basketContractID", strBasketContractID.Get());

        tag.add_tag(pTag);
    }

    server_.GetTransactor().voucherAccounts_.Serialize(tag);

    std::string str_result;
    tag.output(str_result);

    strMainFile.Concatenate("%s", str_result.c_str());

    return true;
}

// Setup the default location for the Sever Main File...
// maybe this should be set differently...
// should be set in the servers configuration.
//
bool MainFile::SaveMainFile()
{
    // Get the loaded (or new) version of the Server's Main File.
    //
    String strMainFile;

    if (!SaveMainFileToString(strMainFile)) {
        Log::vError(
            "%s: Error saving to string. (Giving up on save attempt.)\n",
            __FUNCTION__);
        return false;
    }
    // Try to save the notary server's main datafile to local storage...
    //
    String strFinal;
    Armored ascTemp(strMainFile);

    if (false == ascTemp.WriteArmoredString(strFinal, "NOTARY"))  // todo
                                                                  // hardcoding.
    {

        Log::vError(
            "%s: Error saving notary (failed writing armored string)\n",
            __FUNCTION__);
        return false;
    }
    // Save the Main File to the Harddrive... (or DB, if other storage module is
    // being used).
    //
    const bool bSaved = OTDB::StorePlainString(
        strFinal.Get(),
        server_.API().DataFolder(),
        ".",
        server_.WalletFilename().Get(),
        "",
        "");

    if (!bSaved) {
        Log::vError(
            "%s: Error saving main file: %s\n",
            __FUNCTION__,
            server_.WalletFilename().Get());
    }
    return bSaved;
}

bool MainFile::CreateMainFile(
    const std::string& strContract,
    const std::string& strNotaryID,
    const std::string& strCert,
    const std::string& strNymID,
    const std::string& strCachedKey)
{
    if (!OTDB::StorePlainString(
            strContract,
            server_.API().DataFolder(),
            OTFolders::Contract().Get(),
            strNotaryID,
            "",
            "")) {
        Log::Error("Failed trying to store the server contract.\n");
        return false;
    }

    if (!strCert.empty() && !OTDB::StorePlainString(
                                strCert,
                                server_.API().DataFolder(),
                                OTFolders::Cert().Get(),
                                strNymID,
                                "",
                                "")) {
        Log::Error(
            "Failed trying to store the server Nym's public/private cert.\n");
        return false;
    }

    const char* szBlankFile =  // todo hardcoding.
        "<notaryServer version=\"2.0\"\n"
        " notaryID=\"%s\"\n"
        " serverNymID=\"%s\"\n"
        " transactionNum=\"%ld\" >\n"
        "\n"
        "<cachedKey>\n"
        "%s</cachedKey>\n"
        "\n"
        "<accountList type=\"voucher\" count=\"0\" >\n"
        "\n"
        "</accountList>\n"
        "\n"
        "</notaryServer>\n\n";

    std::int64_t lTransNum = 5;  // a starting point, for the new server.

    String strNotaryFile;
    strNotaryFile.Format(
        szBlankFile,
        strNotaryID.c_str(),
        strNymID.c_str(),
        lTransNum,
        strCachedKey.c_str());

    std::string str_Notary(strNotaryFile.Get());

    if (!OTDB::StorePlainString(
            str_Notary,
            server_.API().DataFolder(),
            ".",
            "notaryServer.xml",
            "",
            ""))  // todo hardcoding.
    {
        Log::Error("Failed trying to store the new notaryServer.xml file.\n");
        return false;
    }
    Armored ascCachedKey;
    ascCachedKey.Set(strCachedKey.c_str());
    auto& cachedKey = server_.API().Crypto().LoadDefaultKey(ascCachedKey);

    if (!cachedKey.HasHashCheck()) {
        OTPassword tempPassword;
        tempPassword.zeroMemory();
        cachedKey.GetMasterPassword(
            cachedKey,
            tempPassword,
            "We do not have a check hash yet for this password, "
            "please enter your password",
            true);
        if (!SaveMainFile()) { OT_FAIL; }
    }
    // At this point, the contract is saved, the cert is saved, and the
    // notaryServer.xml file
    // is saved. All we have left is the Nymfile, which we'll create.

    auto loaded = server_.LoadServerNym(Identifier::Factory(strNymID));
    if (false == loaded) {
        Log::vOutput(0, "%s: Error loading server nym.\n", __FUNCTION__);
    } else {
        Log::vOutput(
            0,
            "%s: OKAY, we have apparently created the new "
            "server.\n"
            "Let's try to load up your new server contract...\n",
            __FUNCTION__);
        return true;
    }

    return false;
}

bool MainFile::LoadMainFile(bool bReadOnly)
{
    if (!OTDB::Exists(
            server_.API().DataFolder(),
            ".",
            server_.WalletFilename().Get(),
            "",
            "")) {
        Log::vError(
            "%s: Error finding file: %s\n",
            __FUNCTION__,
            server_.WalletFilename().Get());
        return false;
    }
    String strFileContents(OTDB::QueryPlainString(
        server_.API().DataFolder(),
        ".",
        server_.WalletFilename().Get(),
        "",
        ""));  // <=== LOADING FROM
               // DATA STORE.

    if (!strFileContents.Exists()) {
        Log::vError(
            "%s: Unable to read main file: %s\n",
            __FUNCTION__,
            server_.WalletFilename().Get());
        return false;
    }

    bool bNeedToSaveAgain = false;

    bool bFailure = false;

    {
        OTStringXML xmlFileContents(strFileContents);

        if (false ==
            xmlFileContents.DecodeIfArmored())  // bEscapedIsAllowed=true
                                                // by default.
        {
            Log::vError(
                "%s: Notary server file apparently was encoded and "
                "then failed decoding. Filename: %s \n"
                "Contents: \n%s\n",
                __FUNCTION__,
                server_.WalletFilename().Get(),
                strFileContents.Get());
            return false;
        }
        irr::io::IrrXMLReader* xml =
            irr::io::createIrrXMLReader(xmlFileContents);
        std::unique_ptr<irr::io::IrrXMLReader> theXMLGuardian(xml);
        while (xml && xml->read()) {
            // strings for storing the data that we want to read out of the file

            String AssetName;
            String InstrumentDefinitionID;

            const String strNodeName(xml->getNodeName());

            switch (xml->getNodeType()) {
                case irr::io::EXN_TEXT:
                    // in this xml file, the only text which occurs is the
                    // messageText
                    // messageText = xml->getNodeData();
                    break;
                case irr::io::EXN_ELEMENT: {
                    if (strNodeName.Compare("notaryServer")) {
                        version_ = xml->getAttributeValue("version");
                        server_.SetNotaryID(Identifier::Factory(
                            String(xml->getAttributeValue("notaryID"))));
                        server_.SetServerNymID(
                            xml->getAttributeValue("serverNymID"));

                        String strTransactionNumber;  // The server issues
                                                      // transaction numbers and
                                                      // stores the counter here
                                                      // for the latest one.
                        strTransactionNumber =
                            xml->getAttributeValue("transactionNum");
                        server_.GetTransactor().transactionNumber(
                            strTransactionNumber.ToLong());

                        Log::vOutput(
                            0,
                            "\nLoading Open Transactions server. File version: "
                            "%s\n"
                            " Last Issued Transaction Number: %" PRId64
                            "\n Notary ID:     "
                            " %s\n Server Nym ID: %s\n",
                            version_.c_str(),
                            server_.GetTransactor().transactionNumber(),
                            server_.GetServerID().str().c_str(),
                            server_.ServerNymID().c_str());

                    }
                    // todo in the future just remove masterkey. I'm leaving it
                    // for
                    // now so people's
                    // data files can get converted over. After a while just
                    // remove
                    // it.
                    //
                    else if (
                        strNodeName.Compare("masterKey") ||
                        strNodeName.Compare("cachedKey")) {
                        Armored ascCachedKey;

                        if (Contract::LoadEncodedTextField(xml, ascCachedKey)) {
                            // We successfully loaded the masterKey from file,
                            // so let's SET it as the master key globally...
                            auto& cachedKey =
                                server_.API().Crypto().LoadDefaultKey(
                                    ascCachedKey);

                            if (!cachedKey.HasHashCheck()) {
                                OTPassword tempPassword;
                                tempPassword.zeroMemory();
                                bNeedToSaveAgain = cachedKey.GetMasterPassword(
                                    cachedKey,
                                    tempPassword,
                                    "We do not have a check hash yet for this "
                                    "password, "
                                    "please enter your password",
                                    true);
                            }
                        }

                        Log::vOutput(
                            0,
                            "\nLoading cachedKey:\n%s\n",
                            ascCachedKey.Get());
                        //
                        // It's only here, AFTER the master key has been loaded,
                        // that we can
                        // go ahead and load the server user, the server
                        // contract,
                        // cron, etc.
                        // (It wasn't that way in version 1, before we had
                        // master
                        // keys.)
                        //
                        // This is, for example, 2.0
                        if (version_ != "1.0") {
                            if (!LoadServerUserAndContract()) {
                                Log::vError(
                                    "%s: Failed calling "
                                    "LoadServerUserAndContract.\n",
                                    __FUNCTION__);
                                bFailure = true;
                            }
                        }
                    } else if (strNodeName.Compare("accountList"))  // the
                                                                    // voucher
                                                                    // reserve
                    // account IDs.
                    {
                        const String strAcctType =
                            xml->getAttributeValue("type");
                        const String strAcctCount =
                            xml->getAttributeValue("count");

                        if ((-1) == server_.GetTransactor()
                                        .voucherAccounts_.ReadFromXMLNode(
                                            xml, strAcctType, strAcctCount))
                            Log::vError(
                                "%s: Error loading voucher accountList.\n",
                                __FUNCTION__);
                    } else if (strNodeName.Compare("basketInfo")) {
                        String strBasketID = xml->getAttributeValue("basketID");
                        String strBasketAcctID =
                            xml->getAttributeValue("basketAcctID");
                        String strBasketContractID =
                            xml->getAttributeValue("basketContractID");

                        const auto BASKET_ID = Identifier::Factory(strBasketID),
                                   BASKET_ACCT_ID =
                                       Identifier::Factory(strBasketAcctID),
                                   BASKET_CONTRACT_ID =
                                       Identifier::Factory(strBasketContractID);

                        if (server_.GetTransactor().addBasketAccountID(
                                BASKET_ID, BASKET_ACCT_ID, BASKET_CONTRACT_ID))
                            Log::vOutput(
                                0,
                                "Loading basket currency info...\n "
                                "Basket ID: %s\n Basket Acct ID: "
                                "%s\n Basket Contract ID: %s\n",
                                strBasketID.Get(),
                                strBasketAcctID.Get(),
                                strBasketContractID.Get());
                        else
                            Log::vError(
                                "Error adding basket currency info...\n "
                                "Basket ID: %s\n Basket Acct ID: %s\n",
                                strBasketID.Get(),
                                strBasketAcctID.Get());
                    }

                    else {
                        // unknown element type
                        Log::vError(
                            "%s: Unknown element type: %s\n",
                            __FUNCTION__,
                            xml->getNodeName());
                    }
                } break;
                default:
                    break;
            }
        }
    }
    if (!bReadOnly) {
        {
            if (bNeedToSaveAgain) SaveMainFile();
        }
    }
    return !bFailure;
}

bool MainFile::LoadServerUserAndContract()
{
    const char* szFunc = "MainFile::LoadServerUserAndContract";
    bool bSuccess = false;
    auto& serverNym = server_.m_nymServer;

    OT_ASSERT(!version_.empty());
    OT_ASSERT(!server_.GetServerID().str().empty());
    OT_ASSERT(!server_.ServerNymID().empty());

    serverNym =
        server_.API().Wallet().Nym(Identifier::Factory(server_.ServerNymID()));

    if (serverNym->HasCapability(NymCapability::SIGN_MESSAGE)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Server nym is viable."
              << std::endl;
    } else {
        otErr << OT_METHOD << __FUNCTION__ << ": Server nym lacks private keys."
              << std::endl;

        return false;
    }

    // Load Cron (now that we have the server Nym.
    // (I WAS loading this erroneously in Server.Init(), before
    // the Nym had actually been loaded from disk. That didn't work.)
    //
    const auto NOTARY_ID = Identifier::Factory(server_.GetServerID());

    // Make sure the Cron object has a pointer to the server's Nym.
    // (For signing stuff...)
    //
    server_.Cron().SetNotaryID(NOTARY_ID);
    server_.Cron().SetServerNym(serverNym);

    if (!server_.Cron().LoadCron())
        Log::vError(
            "%s: Failed loading Cron file. (Did you just create "
            "this server?)\n",
            szFunc);
    Log::vOutput(0, "%s: Loading the server contract...\n", szFunc);

    auto pContract = server_.API().Wallet().Server(NOTARY_ID);

    if (pContract) {
        Log::Output(0, "\n** Main Server Contract Verified **\n");
        bSuccess = true;
    } else {
        Log::vOutput(
            0, "\n%s: Failed reading Main Server Contract:\n\n", szFunc);
    }

    return bSuccess;
}

}  // namespace opentxs::server
