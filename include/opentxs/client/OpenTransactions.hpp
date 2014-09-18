/************************************************************
 *
 *  OpenTransactions.hpp
 *
 */

/************************************************************
 -----BEGIN PGP SIGNED MESSAGE-----
 Hash: SHA1

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
 *  Copyright (C) 2010-2013 by "Fellow Traveler" (A pseudonym)
 *
 *  EMAIL:
 *  FellowTraveler@rayservers.net
 *
 *  BITCOIN:  1NtTPVVjDsUfDWybS4BwvHpG2pdS9RnYyQ
 *
 *  KEY FINGERPRINT (PGP Key in license file):
 *  9DD5 90EB 9292 4B48 0484  7910 0308 00ED F951 BB8E
 *
 *  OFFICIAL PROJECT WIKI(s):
 *  https://github.com/FellowTraveler/Moneychanger
 *  https://github.com/FellowTraveler/Open-Transactions/wiki
 *
 *  WEBSITE:
 *  http://www.OpenTransactions.org/
 *
 *  Components and licensing:
 *   -- Moneychanger..A Java client GUI.....LICENSE:.....GPLv3
 *   -- otlib.........A class library.......LICENSE:...LAGPLv3
 *   -- otapi.........A client API..........LICENSE:...LAGPLv3
 *   -- opentxs/ot....Command-line client...LICENSE:...LAGPLv3
 *   -- otserver......Server Application....LICENSE:....AGPLv3
 *  Github.com/FellowTraveler/Open-Transactions/wiki/Components
 *
 *  All of the above OT components were designed and written by
 *  Fellow Traveler, with the exception of Moneychanger, which
 *  was contracted out to Vicky C (bitcointrader4@gmail.com).
 *  The open-source community has since actively contributed.
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This program is free software: you can redistribute it
 *   and/or modify it under the terms of the GNU Affero
 *   General Public License as published by the Free Software
 *   Foundation, either version 3 of the License, or (at your
 *   option) any later version.
 *
 *   ADDITIONAL PERMISSION under the GNU Affero GPL version 3
 *   section 7: (This paragraph applies only to the LAGPLv3
 *   components listed above.) If you modify this Program, or
 *   any covered work, by linking or combining it with other
 *   code, such other code is not for that reason alone subject
 *   to any of the requirements of the GNU Affero GPL version 3.
 *   (==> This means if you are only using the OT API, then you
 *   don't have to open-source your code--only your changes to
 *   Open-Transactions itself must be open source. Similar to
 *   LGPLv3, except it applies to software-as-a-service, not
 *   just to distributing binaries.)
 *
 *   Extra WAIVER for OpenSSL, Lucre, and all other libraries
 *   used by Open Transactions: This program is released under
 *   the AGPL with the additional exemption that compiling,
 *   linking, and/or using OpenSSL is allowed. The same is true
 *   for any other open source libraries included in this
 *   project: complete waiver from the AGPL is hereby granted to
 *   compile, link, and/or use them with Open-Transactions,
 *   according to their own terms, as long as the rest of the
 *   Open-Transactions terms remain respected, with regard to
 *   the Open-Transactions code itself.
 *
 *   Lucre License:
 *   This code is also "dual-license", meaning that Ben Lau-
 *   rie's license must also be included and respected, since
 *   the code for Lucre is also included with Open Transactions.
 *   See Open-Transactions/src/otlib/lucre/LUCRE_LICENSE.txt
 *   The Laurie requirements are light, but if there is any
 *   problem with his license, simply remove the Lucre code.
 *   Although there are no other blind token algorithms in Open
 *   Transactions (yet. credlib is coming), the other functions
 *   will continue to operate.
 *   See Lucre on Github:  https://github.com/benlaurie/lucre
 *   -----------------------------------------------------
 *   You should have received a copy of the GNU Affero General
 *   Public License along with this program.  If not, see:
 *   http://www.gnu.org/licenses/
 *
 *   If you would like to use this software outside of the free
 *   software license, please contact FellowTraveler.
 *   (Unfortunately many will run anonymously and untraceably,
 *   so who could really stop them?)
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will be
 *   useful, but WITHOUT ANY WARRANTY; without even the implied
 *   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *   PURPOSE.  See the GNU Affero General Public License for
 *   more details.

 -----BEGIN PGP SIGNATURE-----
 Version: GnuPG v1.4.9 (Darwin)

 iQIcBAEBAgAGBQJRSsfJAAoJEAMIAO35UbuOQT8P/RJbka8etf7wbxdHQNAY+2cC
 vDf8J3X8VI+pwMqv6wgTVy17venMZJa4I4ikXD/MRyWV1XbTG0mBXk/7AZk7Rexk
 KTvL/U1kWiez6+8XXLye+k2JNM6v7eej8xMrqEcO0ZArh/DsLoIn1y8p8qjBI7+m
 aE7lhstDiD0z8mwRRLKFLN2IH5rAFaZZUvj5ERJaoYUKdn4c+RcQVei2YOl4T0FU
 LWND3YLoH8naqJXkaOKEN4UfJINCwxhe5Ke9wyfLWLUO7NamRkWD2T7CJ0xocnD1
 sjAzlVGNgaFDRflfIF4QhBx1Ddl6wwhJfw+d08bjqblSq8aXDkmFA7HeunSFKkdn
 oIEOEgyj+veuOMRJC5pnBJ9vV+7qRdDKQWaCKotynt4sWJDGQ9kWGWm74SsNaduN
 TPMyr9kNmGsfR69Q2Zq/FLcLX/j8ESxU+HYUB4vaARw2xEOu2xwDDv6jt0j3Vqsg
 x7rWv4S/Eh18FDNDkVRChiNoOIilLYLL6c38uMf1pnItBuxP3uhgY6COm59kVaRh
 nyGTYCDYD2TK+fI9o89F1297uDCwEJ62U0Q7iTDp5QuXCoxkPfv8/kX6lS6T3y9G
 M9mqIoLbIQ1EDntFv7/t6fUTS2+46uCrdZWbQ5RjYXdrzjij02nDmJAm2BngnZvd
 kamH0Y/n11lCvo1oQxM+
 =uSzz
 -----END PGP SIGNATURE-----
 **************************************************************/

#ifndef OPENTXS_CLIENT_OPENTRANSACTIONS_HPP
#define OPENTXS_CLIENT_OPENTRANSACTIONS_HPP

#include "opentxs/core/util/Common.hpp"

#ifndef SWIG
#ifndef NOT_SWIG
#define NOT_SWIG
#endif
#else
#ifndef EXPORT
#define EXPORT
#endif
#endif

#ifdef NOT_SWIG

#include <opentxs/core/OTString.hpp>

namespace opentxs
{

class TransportCallback;
class OT_API;
class OTAccount;
class OTAssetContract;
class Basket;
class OTCheque;
class OTClient;
class OTEnvelope;
class OTLedger;
class OTMessage;
class OTNumList;
class OTNym_or_SymmetricKey;
class OTPassword;
class OTPasswordData;
class OTPayment;
class OTPaymentPlan;
class OTServerContract;
class OTSocket;
class OTWallet;
class Mint;
class Purse;
class Token;

// The C++ high-level interface to the Open Transactions client-side.
class OT_API
{
private:
    OT_API(const OT_API&);
    OT_API& operator=(const OT_API&);
    static bool bInitOTApp;
    static bool bCleanupOTApp;

public:
    EXPORT static bool InitOTApp();
    EXPORT static bool CleanupOTApp();

private:
    class Pid;
    Pid* const m_pPid; // only one pid reference per instance, must not change

    bool m_bInitialized;
    bool m_bDefaultStore;

    TransportCallback* m_pTransportCallback;
    OTSocket* m_pSocket;

    OTString m_strDataPath;
    OTString m_strWalletFilename;
    OTString m_strWalletFilePath;
    OTString m_strConfigFilename;
    OTString m_strConfigFilePath;

    OTWallet* m_pWallet;
    OTClient* m_pClient;

public:
    EXPORT OT_API();  // calls Init();
    EXPORT ~OT_API(); // calls Cleanup();

private:
    EXPORT bool Init();    // Per instance. (called automaticly by constructor)
    EXPORT bool Cleanup(); // Per instance. (called automaticly by constructor)

public:
    EXPORT bool IsInitialized() const
    {
        return m_bInitialized;
    }

    EXPORT bool SetTransportCallback(TransportCallback* pTransportCallback);
    EXPORT TransportCallback* GetTransportCallback() const;
    EXPORT bool TransportFunction(const OTServerContract& theServerContract,
                                  const OTEnvelope& theEnvelope) const;
    EXPORT bool GetWalletFilename(OTString& strPath) const;
    EXPORT bool SetWalletFilename(const OTString& strPath);
    EXPORT OTWallet* GetWallet(const char* szFuncName = nullptr) const;

    inline OTClient* GetClient() const
    {
        return m_pClient;
    }

    EXPORT bool LoadConfigFile();
    EXPORT bool SetWallet(const OTString& strFilename);
    EXPORT bool WalletExists() const;
    EXPORT bool LoadWallet() const;

    EXPORT time64_t GetTime() const;
    EXPORT bool NumList_Add(OTNumList& theList,
                            const OTNumList& theNewNumbers) const;
    EXPORT bool NumList_Remove(OTNumList& theList,
                               const OTNumList& theOldNumbers) const;
    EXPORT bool NumList_VerifyQuery(const OTNumList& theList,
                                    const OTNumList& theQueryNumbers) const;
    EXPORT bool NumList_VerifyAll(const OTNumList& theList,
                                  const OTNumList& theQueryNumbers) const;
    EXPORT int32_t NumList_Count(const OTNumList& theList) const;
    // Reading data about the local wallet.. presumably already loaded.

    EXPORT int32_t GetNymCount() const;
    EXPORT int32_t GetServerCount() const;
    EXPORT int32_t GetAssetTypeCount() const;
    EXPORT int32_t GetAccountCount() const;

    EXPORT bool GetNym(int32_t iIndex, OTIdentifier& NYM_ID,
                       OTString& NYM_NAME) const;
    EXPORT bool GetServer(int32_t iIndex, OTIdentifier& THE_ID,
                          OTString& THE_NAME) const;
    EXPORT bool GetAssetType(int32_t iIndex, OTIdentifier& THE_ID,
                             OTString& THE_NAME) const;
    EXPORT bool GetAccount(int32_t iIndex, OTIdentifier& THE_ID,
                           OTString& THE_NAME) const;
    // In this case, the ID is input, the pointer is output.
    // Gets the data from Wallet.
    EXPORT OTPseudonym* GetNym(const OTIdentifier& NYM_ID,
                               const char* szFuncName = nullptr) const;
    EXPORT OTServerContract* GetServer(const OTIdentifier& THE_ID,
                                       const char* szFuncName = nullptr) const;
    EXPORT OTAssetContract* GetAssetType(const OTIdentifier& THE_ID,
                                         const char* szFuncName =
                                             nullptr) const;
    EXPORT OTAccount* GetAccount(const OTIdentifier& THE_ID,
                                 const char* szFuncName = nullptr) const;

    EXPORT OTPseudonym* GetNymByIDPartialMatch(const std::string PARTIAL_ID,
                                               const char* szFuncName =
                                                   nullptr) const;
    EXPORT OTServerContract* GetServerContractPartialMatch(
        const std::string PARTIAL_ID, const char* szFuncName = nullptr) const;
    EXPORT OTAssetContract* GetAssetContractPartialMatch(
        const std::string PARTIAL_ID, const char* szFuncName = nullptr) const;
    EXPORT OTAccount* GetAccountPartialMatch(const std::string PARTIAL_ID,
                                             const char* szFuncName =
                                                 nullptr) const;
    EXPORT OTPseudonym* GetOrLoadPublicNym(const OTIdentifier& NYM_ID,
                                           const char* szFuncName =
                                               nullptr) const;
    EXPORT OTPseudonym* GetOrLoadPrivateNym(const OTIdentifier& NYM_ID,
                                            bool bChecking = false,
                                            const char* szFuncName = nullptr,
                                            const OTPasswordData* pPWData =
                                                nullptr,
                                            const OTPassword* pImportPassword =
                                                nullptr) const;
    EXPORT OTPseudonym* GetOrLoadNym(const OTIdentifier& NYM_ID,
                                     bool bChecking = false,
                                     const char* szFuncName = nullptr,
                                     const OTPasswordData* pPWData =
                                         nullptr) const;

    EXPORT OTAccount* GetOrLoadAccount(const OTPseudonym& theNym,
                                       const OTIdentifier& ACCT_ID,
                                       const OTIdentifier& SERVER_ID,
                                       const char* szFuncName = nullptr) const;

    EXPORT OTAccount* GetOrLoadAccount(const OTIdentifier& NYM_ID,
                                       const OTIdentifier& ACCT_ID,
                                       const OTIdentifier& SERVER_ID,
                                       const char* szFuncName = nullptr) const;
    // The name is basically just a client-side label.
    // This function lets you change it.
    EXPORT bool SetNym_Name(const OTIdentifier& NYM_ID,
                            const OTIdentifier& SIGNER_NYM_ID,
                            const OTString& NYM_NEW_NAME) const;

    EXPORT bool SetAccount_Name(const OTIdentifier& ACCT_ID,
                                const OTIdentifier& SIGNER_NYM_ID,
                                const OTString& ACCT_NEW_NAME) const;

    EXPORT bool SetAssetType_Name(const OTIdentifier& ASSET_ID,
                                  const OTString& STR_NEW_NAME) const;

    EXPORT bool SetServer_Name(const OTIdentifier& SERVER_ID,
                               const OTString& STR_NEW_NAME) const;
    // Accessing local storage...
    // (Caller responsible to delete.)
    EXPORT OTPseudonym* LoadPublicNym(const OTIdentifier& NYM_ID,
                                      const char* szFuncName = nullptr) const;
    EXPORT OTPseudonym* LoadPrivateNym(const OTIdentifier& NYM_ID,
                                       bool bChecking = false,
                                       const char* szFuncName = nullptr,
                                       const OTPasswordData* pPWData = nullptr,
                                       const OTPassword* pImportPassword =
                                           nullptr) const;

    EXPORT OTPseudonym* CreateNym(int32_t nKeySize = 1024,
                                  const std::string str_id_source = "",
                                  const std::string str_alt_location = "")
        const; // returns a new nym (with key pair)
    // and files created. (Or nullptr.) Adds
    // to wallet.

    // This works by checking to see if the Nym has a request number for the
    // given server.
    // That's why it's important, when registering at a specific server, to
    // immediately do a
    // "get request number" since that's what locks in the clients ability to be
    // able to tell
    // that it's registered there.
    EXPORT bool IsNym_RegisteredAtServer(const OTIdentifier& NYM_ID,
                                         const OTIdentifier& SERVER_ID) const;
    EXPORT bool Wallet_ChangePassphrase() const;

    EXPORT bool Wallet_CanRemoveServer(const OTIdentifier& SERVER_ID) const;
    EXPORT bool Wallet_CanRemoveAssetType(const OTIdentifier& ASSET_ID) const;
    EXPORT bool Wallet_CanRemoveNym(const OTIdentifier& NYM_ID) const;
    EXPORT bool Wallet_CanRemoveAccount(const OTIdentifier& ACCOUNT_ID) const;

    EXPORT bool Wallet_RemoveServer(const OTIdentifier& SERVER_ID) const;
    EXPORT bool Wallet_RemoveAssetType(const OTIdentifier& ASSET_ID) const;
    EXPORT bool Wallet_RemoveNym(const OTIdentifier& NYM_ID) const;
    // OT has the capability to export a Nym (normally stored in several files)
    // as an encoded
    // object (in base64-encoded form) and then import it again.
    //
    // Returns bool on success, and strOutput will contain the exported data.
    EXPORT bool Wallet_ExportNym(const OTIdentifier& NYM_ID,
                                 OTString& strOutput) const;
    // OT has the capability to export a Nym (normally stored in several files)
    // as an encoded
    // object (in base64-encoded form) and then import it again.
    //
    // Returns bool on success, and if pNymID is passed in, will set it to the
    // new NymID.
    // Also on failure, if the Nym was already there with that ID, and if pNymID
    // is passed,
    // then it will be set to the ID that was already there.
    EXPORT bool Wallet_ImportNym(const OTString& FILE_CONTENTS,
                                 OTIdentifier* pNymID = nullptr) const;
    // In this case, instead of importing a special "OT Nym all-in-one exported"
    // file format,
    // we are importing the public/private keys only, from their Cert file
    // contents, and then
    // creating a blank Nymfile to go along with it. This is for when people
    // wish to import
    // pre-existing keys to create a new Nym.
    //
    // Returns bool on success, and if pNymID is passed in, will set it to the
    // new NymID.
    // Also on failure, if the Nym was already there with that ID, and if pNymID
    // is passed,
    // then it will be set to the ID that was already there.
    EXPORT bool Wallet_ImportCert(const OTString& DISPLAY_NAME,
                                  const OTString& FILE_CONTENTS,
                                  OTIdentifier* pNymID = nullptr) const;
    // Removes master key and sets a normal passphrase on the Cert.
    // Similar to ExportNym except it only exports the Cert portion.
    EXPORT bool Wallet_ExportCert(const OTIdentifier& NYM_ID,
                                  OTString& strOutput) const;
    // First three arguments denote the existing purse.
    // Fourth argument is the NEW purse being imported.
    // (Which may have a different owner Nym, or be protected
    // by a symmetric key instead of a Nym.)
    bool Wallet_ImportPurse(
        const OTIdentifier& SERVER_ID, const OTIdentifier& ASSET_TYPE_ID,
        const OTIdentifier& SIGNER_ID, // We must know the SIGNER_ID in order to
                                       // know which "old purse" to load and
                                       // merge into. The New Purse may have a
                                       // different one, but its ownership will
                                       // be re-assigned in that case, as part
                                       // of the merging process, to SIGNER_ID.
                                       // Otherwise the New Purse might be
                                       // symmetrically encrypted (instead of
                                       // using a Nym) in which case again, its
                                       // ownership will be re-assigned from
                                       // that key, to SIGNER_ID, as part of the
                                       // merging process.
        const OTString& THE_PURSE, const OTString* pstrDisplay = nullptr);
    //
    // ENCODE, DECODE, SIGN, VERIFY, ENCRYPT, DECRYPT

    /** OT-encode a plaintext string.
     This will pack, compress, and base64-encode a plain string.
     Returns the base64-encoded string, or nullptr.
     */
    EXPORT bool Encode(const OTString& strPlaintext, OTString& strOutput,
                       bool bLineBreaks = true) const;
    /** Decode an OT-encoded string (back to plaintext.)
    This will base64-decode, uncompress, and unpack an OT-encoded string.
    Returns the plaintext string, or nullptr.
    */
    EXPORT bool Decode(const OTString& strEncoded, OTString& strOutput,
                       bool bLineBreaks = true) const;
    /** OT-ENCRYPT a plaintext string.
    This will encode, ENCRYPT, and encode a plain string.
    Returns the base64-encoded ciphertext, or nullptr.
    */
    EXPORT bool Encrypt(const OTIdentifier& theRecipientNymID,
                        const OTString& strPlaintext,
                        OTString& strOutput) const;
    /** OT-DECRYPT an OT-encrypted string back to plaintext.
    Decrypts the base64-encoded ciphertext back into a normal string plaintext.
    Returns the plaintext string, or nullptr.
    */
    EXPORT bool Decrypt(const OTIdentifier& theRecipientNymID,
                        const OTString& strCiphertext,
                        OTString& strOutput) const;
    /** OT-Sign a piece of flat text. (With no discernible bookends around it.)
        strType contains the OT type. For example, if you are trying to sign a
        ledger (which does not have any existing signatures on it) then you
       would
        pass LEDGER for strType, resulting in -----BEGIN OT SIGNED LEDGER-----
     */
    bool FlatSign(const OTIdentifier& theSignerNymID, const OTString& strInput,
                  const OTString& strContractType, OTString& strOutput) const;
    /** OT-Sign a CONTRACT.  (First signature)
    Tries to instantiate the contract object, based on the string passed in.
    Then it releases ALL signatures, and then signs the contract.
    Returns the signed contract, or nullptr if failure.
    */
    EXPORT bool SignContract(const OTIdentifier& theSignerNymID,
                             const OTString& strContract,
                             OTString& strOutput) const;
    /** OT-Sign a CONTRACT.  (Add a signature)
    Tries to instantiate the contract object, based on the string passed in.
    Signs the contract, WITHOUT releasing any signatures that are already there.
    Returns the signed contract, or nullptr if failure.
    */
    EXPORT bool AddSignature(const OTIdentifier& theSignerNymID,
                             const OTString& strContract,
                             OTString& strOutput) const;
    /** OT-Verify the signature on a CONTRACT.
     Returns true/false (success/fail)
     */
    EXPORT bool VerifySignature(const OTString& strContract,
                                const OTIdentifier& theSignerNymID,
                                OTContract** ppContract =
                                    nullptr) const; // If you use this optional
                                                    // parameter, then YOU are
    // responsible to clean it up.

    /// Verify and Retrieve XML Contents.
    EXPORT bool VerifyAndRetrieveXMLContents(const OTString& strContract,
                                             const OTIdentifier& theSignerNymID,
                                             OTString& strOutput);
    /// === Verify Account Receipt ===
    /// Returns bool. Verifies any asset account (intermediary files) against
    /// its own last signed receipt.
    /// Obviously this will fail for any new account that hasn't done any
    /// transactions yet, and thus has no receipts.
    EXPORT bool VerifyAccountReceipt(const OTIdentifier& SERVER_ID,
                                     const OTIdentifier& USER_ID,
                                     const OTIdentifier& ACCOUNT_ID) const;

    // Returns an OTCheque pointer, or nullptr.
    // (Caller responsible to delete.)
    EXPORT OTCheque* WriteCheque(
        const OTIdentifier& SERVER_ID, const int64_t& CHEQUE_AMOUNT,
        const time64_t& VALID_FROM, const time64_t& VALID_TO,
        const OTIdentifier& SENDER_ACCT_ID, const OTIdentifier& SENDER_USER_ID,
        const OTString& CHEQUE_MEMO,
        const OTIdentifier* pRECIPIENT_USER_ID = nullptr) const;

    // DISCARD CHEQUE (recover the transaction number for re-use, so the
    // cheque itself can be discarded.)
    EXPORT bool DiscardCheque(const OTIdentifier& SERVER_ID,
                              const OTIdentifier& USER_ID,
                              const OTIdentifier& ACCT_ID,
                              const OTString& THE_CHEQUE) const;

    // PROPOSE PAYMENT PLAN (called by Merchant)
    //
    // Returns an OTPaymentPlan pointer, or nullptr.
    // (Caller responsible to delete.)
    //
    // Payment Plan Delay, and Payment Plan Period, both default to 30 days (if
    // you pass 0),
    // measured in seconds.
    //
    // Payment Plan Length, and Payment Plan Max Payments, both default to 0,
    // which means
    // no maximum length and no maximum number of payments.
    EXPORT OTPaymentPlan* ProposePaymentPlan(
        const OTIdentifier& SERVER_ID,
        const time64_t& VALID_FROM, // 0 defaults to the current time in seconds
                                    // since Jan 1970.
        const time64_t& VALID_TO,   // 0 defaults to "no expiry." Otherwise this
                                    // value is ADDED to VALID_FROM. (It's a
                                    // length.)
        const OTIdentifier& SENDER_ACCT_ID, const OTIdentifier& SENDER_USER_ID,
        const OTString& PLAN_CONSIDERATION, // like a memo.
        const OTIdentifier& RECIPIENT_ACCT_ID,
        const OTIdentifier& RECIPIENT_USER_ID,
        // ----------------------------------------  // If it's above zero, the
        // initial
        const int64_t& INITIAL_PAYMENT_AMOUNT, // amount will be processed after
        const time64_t& INITIAL_PAYMENT_DELAY, // delay (seconds from now.)
        // ----------------------------------------  // AND SEPARATELY FROM
        // THIS...
        const int64_t& PAYMENT_PLAN_AMOUNT, // The regular amount charged,
        const time64_t& PAYMENT_PLAN_DELAY, // which begins occuring after delay
        const time64_t& PAYMENT_PLAN_PERIOD, // (seconds from now) and happens
        // ----------------------------------------  // every period, ad
        // infinitum, until
        time64_t PAYMENT_PLAN_LENGTH =
            OT_TIME_ZERO,                     // after the length (in seconds)
        int32_t PAYMENT_PLAN_MAX_PAYMENTS = 0 // expires, or after the maximum
        ) const;                              // number of payments. These last

    // CONFIRM PAYMENT PLAN (called by Customer)
    EXPORT bool ConfirmPaymentPlan(const OTIdentifier& SERVER_ID,
                                   const OTIdentifier& SENDER_USER_ID,
                                   const OTIdentifier& SENDER_ACCT_ID,
                                   const OTIdentifier& RECIPIENT_USER_ID,
                                   OTPaymentPlan& thePlan) const;
    EXPORT Purse* LoadPurse(const OTIdentifier& SERVER_ID,
                            const OTIdentifier& ASSET_ID,
                            const OTIdentifier& USER_ID,
                            const OTString* pstrDisplay = nullptr) const;
    EXPORT bool SavePurse(const OTIdentifier& SERVER_ID,
                          const OTIdentifier& ASSET_ID,
                          const OTIdentifier& USER_ID, Purse& THE_PURSE) const;
    EXPORT Purse* CreatePurse(const OTIdentifier& SERVER_ID,
                              const OTIdentifier& ASSET_ID,
                              const OTIdentifier& OWNER_ID) const;
    EXPORT Purse* CreatePurse_Passphrase(const OTIdentifier& SERVER_ID,
                                         const OTIdentifier& ASSET_ID) const;
    // This is a low-level utility function. Probably should
    // make this private so people don't confuse it with the API.
    // All the purse functions use this.
    EXPORT OTNym_or_SymmetricKey* LoadPurseAndOwnerFromString(
        const OTIdentifier& theServerID, const OTIdentifier& theAssetTypeID,
        const OTString& strPurse, Purse& thePurse, // output
        OTPassword& thePassword, // Only used in the case of password-protected
                                 // purses. Passed in so it won't go out of
                                 // scope when return value has a member set to
                                 // point to it.
        bool bForEncrypting = true, // true==encrypting,false==decrypting.
        const OTIdentifier* pOWNER_ID = nullptr, // This can be nullptr, **IF**
                                                 // purse is password-protected.
                                                 // (It's
        // just ignored in that case.) Otherwise MUST contain the
        // NymID for the Purse owner.
        const OTString* pstrDisplay1 = nullptr,
        const OTString* pstrDisplay2 = nullptr) const;
    EXPORT OTNym_or_SymmetricKey* LoadPurseAndOwnerForMerge(
        const OTString& strPurse, Purse& thePurse, // output
        OTPassword& thePassword, // Only used in the case of password-protected
                                 // purses. Passed in so it won't go out of
                                 // scope when pOwner is set to point to it.
        bool bCanBePublic = false, // true==private nym isn't mandatory.
                                   // false==private nym IS mandatory.
                                   // (Only relevant if there's an owner.)
        const OTIdentifier* pOWNER_ID = nullptr, // This can be nullptr, **IF**
                                                 // purse is password-protected.
                                                 // (It's
        // just ignored in that case.) Otherwise if it's
        // Nym-protected, the purse will have a NymID on it already.
        // If not (it's optional), then pOWNER_ID is the ID it will
        // try next, before failing.
        const OTString* pstrDisplay = nullptr) const;
    EXPORT Token* Purse_Peek(
        const OTIdentifier& SERVER_ID, const OTIdentifier& ASSET_TYPE_ID,
        const OTString& THE_PURSE,
        const OTIdentifier* pOWNER_ID = nullptr, // This can be nullptr, **IF**
                                                 // purse is password-protected.
                                                 // (It's
        // just ignored in that case.) Otherwise MUST contain the
        // NymID for the Purse owner (necessary to decrypt the token.)
        const OTString* pstrDisplay = nullptr) const;

    EXPORT Purse* Purse_Pop(const OTIdentifier& SERVER_ID,
                            const OTIdentifier& ASSET_TYPE_ID,
                            const OTString& THE_PURSE,
                            const OTIdentifier* pOWNER_ID =
                                nullptr, // This can be nullptr, **IF** purse
                                         // is
                                         // password-protected. (It's just
                            // ignored in that case.) Otherwise MUST
                            // contain the NymID for the Purse owner
                            // (necessary to decrypt the token.)
                            const OTString* pstrDisplay = nullptr) const;

    EXPORT Purse* Purse_Empty(const OTIdentifier& SERVER_ID,
                              const OTIdentifier& ASSET_TYPE_ID,
                              const OTString& THE_PURSE,
                              const OTString* pstrDisplay = nullptr) const;

    EXPORT Purse* Purse_Push(
        const OTIdentifier& SERVER_ID, const OTIdentifier& ASSET_TYPE_ID,
        const OTString& THE_PURSE, const OTString& THE_TOKEN,
        const OTIdentifier* pOWNER_ID = nullptr, // This can be nullptr, **IF**
                                                 // purse is password-protected.
                                                 // (It's
        // just ignored in that case.) Otherwise MUST contain the
        // NymID for the Purse owner (recipient. necessary to encrypt
        // the token to him.)
        const OTString* pstrDisplay = nullptr) const;

    EXPORT Token* Token_ChangeOwner(
        const OTIdentifier& SERVER_ID, const OTIdentifier& ASSET_TYPE_ID,
        const OTString& THE_TOKEN, const OTIdentifier& SIGNER_NYM_ID,
        const OTString& OLD_OWNER, // Pass a NymID here, or a purse.
        const OTString& NEW_OWNER, // Pass a NymID here, or a purse.
        const OTString* pstrDisplay = nullptr) const;
    EXPORT Mint* LoadMint(const OTIdentifier& SERVER_ID,
                          const OTIdentifier& ASSET_ID) const;
    EXPORT OTAssetContract* LoadAssetContract(const OTIdentifier& ASSET_ID)
        const;
    EXPORT OTServerContract* LoadServerContract(const OTIdentifier& SERVER_ID)
        const;
    EXPORT bool IsBasketCurrency(const OTIdentifier& BASKET_ASSET_TYPE_ID)
        const;

    EXPORT int64_t GetBasketMinimumTransferAmount(
        const OTIdentifier& BASKET_ASSET_TYPE_ID) const;

    EXPORT int32_t
    GetBasketMemberCount(const OTIdentifier& BASKET_ASSET_TYPE_ID) const;

    EXPORT bool GetBasketMemberType(const OTIdentifier& BASKET_ASSET_TYPE_ID,
                                    int32_t nIndex,
                                    OTIdentifier& theOutputMemberType) const;

    EXPORT int64_t GetBasketMemberMinimumTransferAmount(
        const OTIdentifier& BASKET_ASSET_TYPE_ID, int32_t nIndex) const;
    EXPORT OTAccount* LoadAssetAccount(const OTIdentifier& SERVER_ID,
                                       const OTIdentifier& USER_ID,
                                       const OTIdentifier& ACCOUNT_ID) const;
    EXPORT OTLedger* LoadNymbox(const OTIdentifier& SERVER_ID,
                                const OTIdentifier& USER_ID) const;

    EXPORT OTLedger* LoadNymboxNoVerify(const OTIdentifier& SERVER_ID,
                                        const OTIdentifier& USER_ID) const;

    EXPORT OTLedger* LoadInbox(const OTIdentifier& SERVER_ID,
                               const OTIdentifier& USER_ID,
                               const OTIdentifier& ACCOUNT_ID) const;

    EXPORT OTLedger* LoadInboxNoVerify(const OTIdentifier& SERVER_ID,
                                       const OTIdentifier& USER_ID,
                                       const OTIdentifier& ACCOUNT_ID) const;

    EXPORT OTLedger* LoadOutbox(const OTIdentifier& SERVER_ID,
                                const OTIdentifier& USER_ID,
                                const OTIdentifier& ACCOUNT_ID) const;

    EXPORT OTLedger* LoadOutboxNoVerify(const OTIdentifier& SERVER_ID,
                                        const OTIdentifier& USER_ID,
                                        const OTIdentifier& ACCOUNT_ID) const;
    EXPORT OTLedger* LoadPaymentInbox(const OTIdentifier& SERVER_ID,
                                      const OTIdentifier& USER_ID) const;

    EXPORT OTLedger* LoadPaymentInboxNoVerify(
        const OTIdentifier& SERVER_ID, const OTIdentifier& USER_ID) const;
    // LoadRecordBox
    // Note: depending on the record type, the Account ID may contain the User
    // ID.
    EXPORT OTLedger* LoadRecordBox(const OTIdentifier& SERVER_ID,
                                   const OTIdentifier& USER_ID,
                                   const OTIdentifier& ACCOUNT_ID) const;

    EXPORT OTLedger* LoadRecordBoxNoVerify(
        const OTIdentifier& SERVER_ID, const OTIdentifier& USER_ID,
        const OTIdentifier& ACCOUNT_ID) const;

    EXPORT bool ClearRecord(
        const OTIdentifier& SERVER_ID, const OTIdentifier& USER_ID,
        const OTIdentifier& ACCOUNT_ID, // USER_ID can be passed here as well.
        int32_t nIndex, bool bClearAll = false // if true, nIndex is ignored.
        ) const;
    EXPORT OTLedger* LoadExpiredBox(const OTIdentifier& SERVER_ID,
                                    const OTIdentifier& USER_ID) const;

    EXPORT OTLedger* LoadExpiredBoxNoVerify(const OTIdentifier& SERVER_ID,
                                            const OTIdentifier& USER_ID) const;

    EXPORT bool ClearExpired(const OTIdentifier& SERVER_ID,
                             const OTIdentifier& USER_ID, int32_t nIndex,
                             bool bClearAll =
                                 false // if true, nIndex is ignored.
                             ) const;
    // Note: if instrument is expired BEFORE being recorded, it will go into the
    // expired box instead of the record box.
    EXPORT bool RecordPayment(
        const OTIdentifier& SERVER_ID, const OTIdentifier& USER_ID,
        bool bIsInbox,  // true == payments inbox. false == payments outbox.
        int32_t nIndex, // removes payment instrument (from payments in or out
                        // box) and moves to record box.
        bool bSaveCopy) const; // If false, copy of instrument will NOT be
                               // saved.
    // So the client side knows which ones he has in storage, vs which ones he
    // still needs to download.
    EXPORT bool DoesBoxReceiptExist(
        const OTIdentifier& SERVER_ID,
        const OTIdentifier& USER_ID,    // Unused here for now, but still
                                        // convention.
        const OTIdentifier& ACCOUNT_ID, // If for Nymbox (vs inbox/outbox) then
                                        // pass USER_ID in this field also.
        int32_t nBoxType,               // 0/nymbox, 1/inbox, 2/outbox
        const int64_t& lTransactionNum) const;
    // Incoming
    EXPORT OTMessage* PopMessageBuffer(const int64_t& lRequestNumber,
                                       const OTIdentifier& SERVER_ID,
                                       const OTIdentifier& USER_ID) const;
    void FlushMessageBuffer();
    // Outgoing
    EXPORT OTMessage* GetSentMessage(const int64_t& lRequestNumber,
                                     const OTIdentifier& SERVER_ID,
                                     const OTIdentifier& USER_ID) const;
    EXPORT bool RemoveSentMessage(const int64_t& lRequestNumber,
                                  const OTIdentifier& SERVER_ID,
                                  const OTIdentifier& USER_ID) const;
    EXPORT void FlushSentMessages(bool bHarvestingForRetry,
                                  const OTIdentifier& SERVER_ID,
                                  const OTIdentifier& USER_ID,
                                  const OTLedger& THE_NYMBOX) const;

    EXPORT bool HaveAlreadySeenReply(const OTIdentifier& SERVER_ID,
                                     const OTIdentifier& USER_ID,
                                     const int64_t& lRequestNumber) const;

    EXPORT bool ResyncNymWithServer(OTPseudonym& theNym,
                                    const OTLedger& theNymbox,
                                    const OTPseudonym& theMessageNym) const;

    // These commands below send messages to the server:

    EXPORT int32_t checkServerID(const OTIdentifier& SERVER_ID,
                                 const OTIdentifier& USER_ID) const;

    EXPORT int32_t createUserAccount(const OTIdentifier& SERVER_ID,
                                     const OTIdentifier& USER_ID) const;

    EXPORT int32_t deleteUserAccount(const OTIdentifier& SERVER_ID,
                                     const OTIdentifier& USER_ID) const;

    EXPORT int32_t checkUser(const OTIdentifier& SERVER_ID,
                             const OTIdentifier& USER_ID,
                             const OTIdentifier& USER_ID_CHECK) const;

    EXPORT int32_t usageCredits(const OTIdentifier& SERVER_ID,
                                const OTIdentifier& USER_ID,
                                const OTIdentifier& USER_ID_CHECK,
                                int64_t lAdjustment = 0) const;

    EXPORT int32_t getRequest(const OTIdentifier& SERVER_ID,
                              const OTIdentifier& USER_ID) const;

    EXPORT int32_t sendUserMessage(const OTIdentifier& SERVER_ID,
                                   const OTIdentifier& USER_ID,
                                   const OTIdentifier& USER_ID_RECIPIENT,
                                   const OTString& RECIPIENT_PUBKEY,
                                   const OTString& THE_MESSAGE) const;

    EXPORT int32_t
    sendUserInstrument(const OTIdentifier& SERVER_ID,
                       const OTIdentifier& USER_ID,
                       const OTIdentifier& USER_ID_RECIPIENT,
                       const OTString& RECIPIENT_PUBKEY,
                       const OTPayment& THE_INSTRUMENT,
                       const OTPayment* pINSTRUMENT_FOR_SENDER = nullptr) const;

    EXPORT int32_t issueAssetType(const OTIdentifier& SERVER_ID,
                                  const OTIdentifier& USER_ID,
                                  const OTString& THE_CONTRACT) const;

    EXPORT int32_t getContract(const OTIdentifier& SERVER_ID,
                               const OTIdentifier& USER_ID,
                               const OTIdentifier& ASSET_ID) const;

    EXPORT int32_t getMint(const OTIdentifier& SERVER_ID,
                           const OTIdentifier& USER_ID,
                           const OTIdentifier& ASSET_ID) const;

    EXPORT int32_t
    getBoxReceipt(const OTIdentifier& SERVER_ID, const OTIdentifier& USER_ID,
                  const OTIdentifier& ACCOUNT_ID, // If for Nymbox (vs
                                                  // inbox/outbox) then pass
                                                  // USER_ID in this field also.
                  int32_t nBoxType,               // 0/nymbox, 1/inbox, 2/outbox
                  const int64_t& lTransactionNum) const;

    EXPORT int32_t queryAssetTypes(const OTIdentifier& SERVER_ID,
                                   const OTIdentifier& USER_ID,
                                   const OTASCIIArmor& ENCODED_MAP) const;

    EXPORT int32_t createAssetAccount(const OTIdentifier& SERVER_ID,
                                      const OTIdentifier& USER_ID,
                                      const OTIdentifier& ASSET_ID) const;

    EXPORT int32_t deleteAssetAccount(const OTIdentifier& SERVER_ID,
                                      const OTIdentifier& USER_ID,
                                      const OTIdentifier& ACCOUNT_ID) const;

    EXPORT int32_t getAccount(const OTIdentifier& SERVER_ID,
                              const OTIdentifier& USER_ID,
                              const OTIdentifier& ACCT_ID) const;

    EXPORT int32_t getAccountFiles(const OTIdentifier& SERVER_ID,
                                   const OTIdentifier& USER_ID,
                                   const OTIdentifier& ACCT_ID) const;

    EXPORT Basket* GenerateBasketCreation(const OTIdentifier& USER_ID,
                                          int64_t MINIMUM_TRANSFER)
        const; // Must be above zero. If <= 0,
               // defaults to 10.

    EXPORT bool AddBasketCreationItem(
        const OTIdentifier& USER_ID,       // for signature.
        Basket& theBasket,                 // created in above call.
        const OTIdentifier& ASSET_TYPE_ID, // Adding an asset type to the new
                                           // basket.
        int64_t MINIMUM_TRANSFER) const;   // The amount of the asset type
                                           // that is
                                           // in the basket.

    EXPORT int32_t issueBasket(const OTIdentifier& SERVER_ID,
                               const OTIdentifier& USER_ID,
                               const OTString& BASKET_INFO) const;

    EXPORT Basket* GenerateBasketExchange(
        const OTIdentifier& SERVER_ID, const OTIdentifier& USER_ID,
        const OTIdentifier& BASKET_ASSET_TYPE_ID,
        const OTIdentifier& BASKET_ASSET_ACCT_ID,
        int32_t TRANSFER_MULTIPLE) const; // 1            2             3
    // 5=2,3,4  OR  10=4,6,8  OR 15=6,9,12

    EXPORT bool AddBasketExchangeItem(const OTIdentifier& SERVER_ID,
                                      const OTIdentifier& USER_ID,
                                      Basket& theBasket,
                                      const OTIdentifier& ASSET_TYPE_ID,
                                      const OTIdentifier& ASSET_ACCT_ID) const;

    EXPORT int32_t
    exchangeBasket(const OTIdentifier& SERVER_ID, const OTIdentifier& USER_ID,
                   const OTIdentifier& BASKET_ASSET_ID,
                   const OTString& BASKET_INFO, bool bExchangeInOrOut) const;

    EXPORT int32_t getTransactionNumber(const OTIdentifier& SERVER_ID,
                                        const OTIdentifier& USER_ID) const;

    EXPORT int32_t notarizeWithdrawal(const OTIdentifier& SERVER_ID,
                                      const OTIdentifier& USER_ID,
                                      const OTIdentifier& ACCT_ID,
                                      const int64_t& AMOUNT) const;

    EXPORT int32_t notarizeDeposit(const OTIdentifier& SERVER_ID,
                                   const OTIdentifier& USER_ID,
                                   const OTIdentifier& ACCT_ID,
                                   const OTString& THE_PURSE) const;

    EXPORT int32_t
    notarizeTransfer(const OTIdentifier& SERVER_ID, const OTIdentifier& USER_ID,
                     const OTIdentifier& ACCT_FROM, const OTIdentifier& ACCT_TO,
                     const int64_t& AMOUNT, const OTString& NOTE) const;

    EXPORT int32_t
    getNymbox(const OTIdentifier& SERVER_ID, const OTIdentifier& USER_ID) const;

    EXPORT int32_t getInbox(const OTIdentifier& SERVER_ID,
                            const OTIdentifier& USER_ID,
                            const OTIdentifier& ACCT_ID) const;

    EXPORT int32_t getOutbox(const OTIdentifier& SERVER_ID,
                             const OTIdentifier& USER_ID,
                             const OTIdentifier& ACCT_ID) const;

    // Returns:
    // -1 if error.
    //  0 if Nymbox is empty.
    //  1 or more: Count of items in Nymbox before processing.
    EXPORT int32_t processNymbox(const OTIdentifier& SERVER_ID,
                                 const OTIdentifier& USER_ID) const;

    EXPORT int32_t processInbox(const OTIdentifier& SERVER_ID,
                                const OTIdentifier& USER_ID,
                                const OTIdentifier& ACCT_ID,
                                const OTString& ACCT_LEDGER) const;

    EXPORT int32_t
    withdrawVoucher(const OTIdentifier& SERVER_ID, const OTIdentifier& USER_ID,
                    const OTIdentifier& ACCT_ID,
                    const OTIdentifier& RECIPIENT_USER_ID,
                    const OTString& CHEQUE_MEMO, const int64_t& AMOUNT) const;

    EXPORT int32_t payDividend(
        const OTIdentifier& SERVER_ID,
        const OTIdentifier& ISSUER_USER_ID,        // must be issuer of
                                                   // SHARES_ASSET_TYPE_ID
        const OTIdentifier& DIVIDEND_FROM_ACCT_ID, // if dollars paid for pepsi
                                                   // shares, then this is the
                                                   // issuer's dollars account.
        const OTIdentifier& SHARES_ASSET_TYPE_ID,  // if dollars paid for pepsi
                                                   // shares,
        // then this is the pepsi shares
        // asset type id.
        const OTString& DIVIDEND_MEMO, // user-configurable note that's added to
                                       // the
                                       // payout request message.
        const int64_t& AMOUNT_PER_SHARE) const; // number of dollars to be paid
                                                // out
    // PER SHARE (multiplied by total
    // number of shares issued.)

    EXPORT int32_t depositCheque(const OTIdentifier& SERVER_ID,
                                 const OTIdentifier& USER_ID,
                                 const OTIdentifier& ACCT_ID,
                                 const OTString& THE_CHEQUE) const;

    EXPORT int32_t
    triggerClause(const OTIdentifier& SERVER_ID, const OTIdentifier& USER_ID,
                  const int64_t& lTransactionNum, const OTString& strClauseName,
                  const OTString* pStrParam = nullptr) const;

    EXPORT bool Create_SmartContract(
        const OTIdentifier& SIGNER_NYM_ID, // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        time64_t VALID_FROM,               // Default (0 or nullptr) == NOW
        time64_t VALID_TO, // Default (0 or nullptr) == no expiry / cancel
                           // anytime
        OTString& strOutput) const;

    EXPORT bool SmartContract_AddBylaw(
        const OTString& THE_CONTRACT, // The contract, about to have the bylaw
                                      // added to it.
        const OTIdentifier& SIGNER_NYM_ID, // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const OTString& BYLAW_NAME, // The Bylaw's NAME as referenced in the
                                    // smart contract. (And the scripts...)
        OTString& strOutput) const;

    EXPORT bool SmartContract_AddClause(
        const OTString& THE_CONTRACT, // The contract, about to have the clause
                                      // added to it.
        const OTIdentifier& SIGNER_NYM_ID, // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const OTString& BYLAW_NAME,  // Should already be on the contract. (This
                                     // way we can find it.)
        const OTString& CLAUSE_NAME, // The Clause's name as referenced in the
                                     // smart contract. (And the scripts...)
        const OTString& SOURCE_CODE, // The actual source code for the clause.
        OTString& strOutput) const;

    EXPORT bool SmartContract_AddVariable(
        const OTString& THE_CONTRACT, // The contract, about to have the clause
                                      // added to it.
        const OTIdentifier& SIGNER_NYM_ID, // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const OTString& BYLAW_NAME, // Should already be on the contract. (This
                                    // way we can find it.)
        const OTString& VAR_NAME,   // The Variable's name as referenced in the
                                    // smart contract. (And the scripts...)
        const OTString& VAR_ACCESS, // "constant", "persistent", or "important".
        const OTString& VAR_TYPE,   // "string", "int64_t", or "bool"
        const OTString& VAR_VALUE,  // Contains a string. If type is int64_t,
                                    // atol() will be used to convert value to a
                                    // int64_t. If type is bool, the strings
                                    // "true" or "false" are expected here in
                                    // order to convert to a bool.
        OTString& strOutput) const;

    EXPORT bool SmartContract_AddCallback(
        const OTString& THE_CONTRACT, // The contract, about to have the clause
                                      // added to it.
        const OTIdentifier& SIGNER_NYM_ID, // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const OTString& BYLAW_NAME, // Should already be on the contract. (This
                                    // way we can find it.)
        const OTString& CALLBACK_NAME, // The Callback's name as referenced in
                                       // the smart contract. (And the
                                       // scripts...)
        const OTString& CLAUSE_NAME, // The actual clause that will be triggered
                                     // by the callback. (Must exist.)
        OTString& strOutput) const;

    EXPORT bool SmartContract_AddHook(
        const OTString& THE_CONTRACT, // The contract, about to have the clause
                                      // added to it.
        const OTIdentifier& SIGNER_NYM_ID, // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const OTString& BYLAW_NAME, // Should already be on the contract. (This
                                    // way we can find it.)
        const OTString& HOOK_NAME, // The Hook's name as referenced in the smart
                                   // contract. (And the scripts...)
        const OTString& CLAUSE_NAME, // The actual clause that will be triggered
                                     // by the hook. (You can call this multiple
                                     // times, and have multiple clauses trigger
                                     // on the same hook.)
        OTString& strOutput) const;

    EXPORT bool SmartContract_AddParty(
        const OTString& THE_CONTRACT, // The contract, about to have the bylaw
                                      // added to it.
        const OTIdentifier& SIGNER_NYM_ID, // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const OTString& PARTY_NAME, // The Party's NAME as referenced in the
                                    // smart contract. (And the scripts...)
        const OTString& AGENT_NAME, // An AGENT will be added by default for
                                    // this party. Need Agent NAME.
        OTString& strOutput) const;

    EXPORT bool SmartContract_AddAccount(
        const OTString& THE_CONTRACT, // The contract, about to have the clause
                                      // added to it.
        const OTIdentifier& SIGNER_NYM_ID, // Use any Nym you wish here. (The
                                           // signing at this point is only to
                                           // cause a save.)
        const OTString& PARTY_NAME, // The Party's NAME as referenced in the
                                    // smart contract. (And the scripts...)
        const OTString& ACCT_NAME,  // The Account's name as referenced in the
                                    // smart contract
        const OTString& ASSET_TYPE_ID, // Asset Type ID for the Account.
        OTString& strOutput) const;

    EXPORT int32_t SmartContract_CountNumsNeeded(
        const OTString& THE_CONTRACT, // The contract, about to have the bylaw
                                      // added to it.
        const OTString& AGENT_NAME) const; // An AGENT will be added by default
                                           // for
                                           // this party. Need Agent NAME.

    EXPORT bool SmartContract_ConfirmAccount(const OTString& THE_CONTRACT,
                                             const OTIdentifier& SIGNER_NYM_ID,
                                             const OTString& PARTY_NAME,
                                             const OTString& ACCT_NAME,
                                             const OTString& AGENT_NAME,
                                             const OTString& ACCT_ID,
                                             OTString& strOutput) const;

    EXPORT bool SmartContract_ConfirmParty(
        const OTString& THE_CONTRACT, // The smart contract, about to be changed
                                      // by this function.
        const OTString& PARTY_NAME,   // Should already be on the contract. This
                                      // way we can find it.
        const OTIdentifier& NYM_ID,   // Nym ID for the party, the actual owner,
        OTString& strOutput) const; // ===> AS WELL AS for the default AGENT of
                                    // that
                                    // party. (For now, until I code entities)
    EXPORT bool Msg_HarvestTransactionNumbers(
        const OTMessage& theMsg, const OTIdentifier& USER_ID,
        bool bHarvestingForRetry, bool bReplyWasSuccess, bool bReplyWasFailure,
        bool bTransactionWasSuccess, bool bTransactionWasFailure) const;

    EXPORT bool HarvestClosingNumbers(const OTIdentifier& SERVER_ID,
                                      const OTIdentifier& NYM_ID,
                                      const OTString& THE_CRON_ITEM) const;

    EXPORT bool HarvestAllNumbers(const OTIdentifier& SERVER_ID,
                                  const OTIdentifier& NYM_ID,
                                  const OTString& THE_CRON_ITEM) const;
    EXPORT int32_t
    activateSmartContract(const OTIdentifier& SERVER_ID,
                          const OTIdentifier& USER_ID,
                          const OTString& THE_SMART_CONTRACT) const;

    EXPORT int32_t depositPaymentPlan(const OTIdentifier& SERVER_ID,
                                      const OTIdentifier& USER_ID,
                                      const OTString& THE_PAYMENT_PLAN) const;
    EXPORT int32_t issueMarketOffer(
        const OTIdentifier& SERVER_ID, const OTIdentifier& USER_ID,
        const OTIdentifier& ASSET_ACCT_ID, const OTIdentifier& CURRENCY_ACCT_ID,
        const int64_t& MARKET_SCALE,      // Defaults to minimum of 1. Market
                                          // granularity.
        const int64_t& MINIMUM_INCREMENT, // This will be multiplied by the
                                          // Scale. Min 1.
        const int64_t& TOTAL_ASSETS_ON_OFFER, // Total assets available for sale
                                              // or purchase. Will be multiplied
                                              // by minimum increment.
        const int64_t& PRICE_LIMIT,           // Per Minimum Increment...
        bool bBuyingOrSelling, // BUYING == false, SELLING == true.
        time64_t tLifespanInSeconds =
            OT_TIME_DAY_IN_SECONDS, // 86400 seconds == 1 day.
        char STOP_SIGN = 0,         // For stop orders, set to '<' or '>'
        int64_t ACTIVATION_PRICE =
            0) const; // For stop orders, set the threshold price here.
    EXPORT int32_t getMarketList(const OTIdentifier& SERVER_ID,
                                 const OTIdentifier& USER_ID) const;
    EXPORT int32_t
    getMarketOffers(const OTIdentifier& SERVER_ID, const OTIdentifier& USER_ID,
                    const OTIdentifier& MARKET_ID, const int64_t& lDepth) const;
    EXPORT int32_t getMarketRecentTrades(const OTIdentifier& SERVER_ID,
                                         const OTIdentifier& USER_ID,
                                         const OTIdentifier& MARKET_ID) const;
    EXPORT int32_t getNym_MarketOffers(const OTIdentifier& SERVER_ID,
                                       const OTIdentifier& USER_ID) const;
    // For cancelling market offers and payment plans.
    EXPORT int32_t cancelCronItem(const OTIdentifier& SERVER_ID,
                                  const OTIdentifier& USER_ID,
                                  const OTIdentifier& ASSET_ACCT_ID,
                                  const int64_t& lTransactionNum) const;

    EXPORT void AddServerContract(const OTServerContract& pContract) const;
    EXPORT void AddAssetContract(const OTAssetContract& theContract) const;
};

} // namespace opentxs

#endif // NOT_SWIG

#endif // OPENTXS_CLIENT_OPENTRANSACTIONS_HPP
