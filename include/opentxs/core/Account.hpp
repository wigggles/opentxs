/************************************************************
 *
 *  OTAccount.hpp
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

#ifndef OPENTXS_CORE_OTACCOUNT_HPP
#define OPENTXS_CORE_OTACCOUNT_HPP

#include "OTTransactionType.hpp"
#include <cstdint>

namespace opentxs
{

class String;
class OTLedger;
class Message;
class Nym;

class Account : public OTTransactionType
{
    friend OTTransactionType* OTTransactionType::TransactionFactory(
        String input);

public:
    // If you add any types to this list, update the list of strings at the
    // top of the .cpp file.
    enum AccountType {
        simple,    // used by users
        issuer,    // used by issuers    (these can only go negative.)
        basket,    // issuer acct used by basket currencies (these can only go
                   // negative)
        basketsub, // used by the server (to store backing reserves for basket
                   // sub-accounts)
        mint,      // used by mints (to store backing reserves for cash)
        voucher,   // used by the server (to store backing reserves for
                   // vouchers)
        stash,     // used by the server (to store backing reserves for stashes,
                   // for smart contracts.)
        err_acct
    };

public:
    EXPORT Account(const Identifier& userId, const Identifier& accountId,
                   const Identifier& notaryID, const String& name);
    EXPORT Account(const Identifier& userId, const Identifier& accountId,
                   const Identifier& notaryID);

    EXPORT virtual ~Account();

    EXPORT virtual void Release();
    // overriding this so I can set filename automatically inside based on ID.
    EXPORT virtual bool LoadContract();
    EXPORT virtual bool SaveContractWallet(String& contents) const;
    EXPORT virtual bool DisplayStatistics(String& contents) const;

    inline void MarkForDeletion()
    {
        markForDeletion_ = true;
    }

    inline bool IsMarkedForDeletion() const
    {
        return markForDeletion_;
    }

    EXPORT bool IsInternalServerAcct() const;

    EXPORT bool IsOwnedByUser() const;
    EXPORT bool IsOwnedByEntity() const;

    EXPORT bool IsAllowedToGoNegative() const;
    EXPORT bool IsIssuer() const;

    // For accounts used by smart contracts, to stash funds while running.
    EXPORT bool IsStashAcct() const
    {
        return (acctType_ == stash);
    }

    EXPORT const int64_t& GetStashTransNum() const
    {
        return stashTransNum_;
    }

    EXPORT void SetStashTransNum(const int64_t& transNum)
    {
        stashTransNum_ = transNum;
    }

    EXPORT void InitAccount();

    EXPORT void Release_Account();
    EXPORT static Account* GenerateNewAccount(const Identifier& userId,
                                              const Identifier& notaryID,
                                              const Nym& serverNym,
                                              const Message& message,
                                              AccountType acctType = simple,
                                              int64_t stashTransNum = 0);

    EXPORT bool GenerateNewAccount(const Nym& server, const Message& message,
                                   AccountType acctType = simple,
                                   int64_t stashTransNum = 0);
    // Let's say you don't have or know the UserID, and you just want to load
    // the damn thing up.
    // Then call this function. It will set userID for you.
    EXPORT static Account* LoadExistingAccount(const Identifier& accountId,
                                               const Identifier& notaryID);
    // Caller responsible to delete.
    EXPORT OTLedger* LoadInbox(Nym& nym) const;
    // Caller responsible to delete.
    EXPORT OTLedger* LoadOutbox(Nym& nym) const;

    // If you pass the identifier in, the inbox hash is recorded there
    EXPORT bool SaveInbox(OTLedger& box, Identifier* hash = nullptr);
    // If you pass the identifier in, the outbox hash is recorded there
    EXPORT bool SaveOutbox(OTLedger& box, Identifier* nash = nullptr);
    EXPORT const Identifier& GetInstrumentDefinitionID() const;
    EXPORT int64_t GetBalance() const;
    // Debit a certain amount from the account (presumably the same amount is
    // being added somewhere)
    EXPORT bool Debit(const int64_t& amount);
    // Credit a certain amount from the account (presumably the same amount is
    // being subtracted somewhere)
    EXPORT bool Credit(const int64_t& amount);
    // Compares the NymID loaded from the account file with whatever Nym the
    // programmer wants to verify.
    EXPORT bool VerifyOwner(const Nym& candidate) const;
    EXPORT bool VerifyOwnerByID(const Identifier& nymId) const;
    // generates filename based on accounts path and account ID. Saves to the
    // standard location for an acct.
    EXPORT bool SaveAccount();

    EXPORT void SetInboxHash(const Identifier& input);
    EXPORT bool GetInboxHash(Identifier& output);

    EXPORT void SetOutboxHash(const Identifier& input);
    EXPORT bool GetOutboxHash(Identifier& output);
    EXPORT static char const* _GetTypeString(AccountType accountType);

    EXPORT char const* GetTypeString() const
    {
        return _GetTypeString(acctType_);
    }

protected:
    Account(const Identifier& userId, const Identifier& notaryID);
    Account();

    // return -1 if error, 0 if nothing, and 1 if the node was processed.
    virtual int32_t ProcessXMLNode(irr::io::IrrXMLReader*& xml);

    virtual void UpdateContents();

protected:
    AccountType acctType_;
    // These are all the variables from the account file itself.
    Identifier acctInstrumentDefinitionID_;
    String balanceDate_;
    String balanceAmount_;
    // the Transaction Number of a smart contract running on cron, if this is a
    // stash account.
    int64_t stashTransNum_;
    // Default FALSE. When set to true, saves a "DELETED" flag with this Account
    bool markForDeletion_;
    // for easy cleanup later when the server is doing some maintenance.
    // Hash of this account's Inbox, so we don't download it more often than
    // necessary.
    Identifier inboxHash_;
    // Hash of this account's Outbox, so we don't download it more often than
    // necessary.
    Identifier outboxHash_;
};

} // namespace opentxs

#endif // OPENTXS_CORE_OTACCOUNT_HPP
