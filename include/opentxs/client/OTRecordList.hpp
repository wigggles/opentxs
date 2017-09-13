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

#ifndef OPENTXS_CLIENT_OTRECORDLIST_HPP
#define OPENTXS_CLIENT_OTRECORDLIST_HPP

#include "opentxs/client/OTRecord.hpp"
#include "opentxs/core/util/Common.hpp"

#include <stdint.h>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace opentxs
{

/** For address book lookups. Your client app inherits this and provides addr
 * storage/lookup through this simple interface. OTRecordList then calls it. */
class OTNameLookup
{
public:
    EXPORT OTNameLookup() {}
    EXPORT virtual ~OTNameLookup();
    EXPORT virtual std::string GetNymName(
        const std::string& str_id,  // NymID
        const std::string p_notary_id) const;
    EXPORT virtual std::string GetContactName(
        const std::string& str_id) const;  // ContactId
    EXPORT virtual std::string GetAcctName(
        const std::string& str_id,  // AcctID
        const std::string p_nym_id,
        const std::string p_notary_id,
        const std::string p_instrument_definition_id) const;
    /** Used for Bitmessage and other special addresses. */
    EXPORT virtual std::string GetAddressName(
        const std::string& str_address) const;
    // Let's say that OTRecordList just deposited a cheque. (Which it does
    // automatically.) Or let's say the user just asked it to activate a smart
    // contract. Whatever. RecordList will call this and pass the server's
    // "success" transaction contents, along with whatever other useful IDs it's
    // gleaned.
    // That way, when Moneychanger overrides notifyOfSuccessfulNotarization,
    // Moneychanger will get a notification whenever the recordlist has
    // deposited a cheque. Then Moneychanger can take the cheque deposit
    // (transaction reply from server) and add it to its internal database, in
    // its payment table.
    EXPORT virtual void notifyOfSuccessfulNotarization(
        const std::string& str_acct_id,
        const std::string p_nym_id,
        const std::string p_notary_id,
        const std::string p_txn_contents,
        int64_t lTransactionNum,
        int64_t lTransNumForDisplay) const;
};

/*
 // OVERLOAD THE ABOVE CLASS; make a subclass that does an address book lookup
 however is appropriate for your client.
 //
 class OTNameLookupIPhone : public OTNameLookup
 {
 public:
    virtual std::string GetNymName(const std::string& str_id) const;
    virtual std::string GetAcctName(const std::string& str_id) const;
    virtual std::string GetAddressName(const std::string& str_id) const; //
 Used for Bitmessage and other special addresses.
 };
 */

// ---------------------------------------------------------------------

// Client app makes an instance of its own subclass of OTNameLookup.
// Client app also makes an instance of OTLookupCaller (below.)
// Client app then gives the caller a pointer to the namelookup.
// Client app then passes the caller to OT via OT_API_Set_AddrBookCallback.
// OTRecord and OTRecordList then call the caller.
class OTLookupCaller
{
protected:
    OTNameLookup* _callback;

public:
    EXPORT OTLookupCaller()
        : _callback(nullptr)
    {
    }
    EXPORT ~OTLookupCaller();

    EXPORT OTNameLookup* getCallback() { return _callback; }
    EXPORT void delCallback();
    EXPORT void setCallback(OTNameLookup* cb);
    EXPORT bool isCallbackSet() const;

    EXPORT std::string GetNymName(
        const std::string& str_id,  // NymID
        const std::string notary_id) const;

    EXPORT virtual std::string GetContactName(
        const std::string& str_id  // ContactId
        ) const;

    EXPORT std::string GetAcctName(
        const std::string& str_id,  // AcctID
        const std::string p_nym_id,
        const std::string p_notary_id,
        const std::string p_instrument_definition_id) const;

    EXPORT std::string GetAddressName(const std::string& str_address) const;
};

EXPORT bool OT_API_Set_AddrBookCallback(
    OTLookupCaller& theCaller);  // OTLookupCaller must have OTNameLookup
                                 // attached already.

typedef std::weak_ptr<OTRecord> weak_ptr_OTRecord;
typedef std::shared_ptr<OTRecord> shared_ptr_OTRecord;

typedef std::vector<shared_ptr_OTRecord> vec_OTRecordList;
typedef std::list<std::string> list_of_strings;
typedef std::map<std::string, std::string> map_of_strings;

class OTRecordList
{
    const OTNameLookup* m_pLookup;
    // Defaults to false. If you set it true, it will run a lot faster. (And
    // give you less data.)
    bool m_bRunFast;
    bool m_bAutoAcceptCheques;  // Cheques and vouchers, NOT invoices.
    bool m_bAutoAcceptReceipts;
    bool m_bAutoAcceptTransfers;
    bool m_bAutoAcceptCash;
    static std::string s_strTextTo;    // "To: "
    static std::string s_strTextFrom;  // "From: "
    list_of_strings m_servers;
    map_of_strings m_assets;  // <instrument_definition_id, asset_name>
    list_of_strings m_accounts;
    list_of_strings m_nyms;
    vec_OTRecordList m_contents;
    static const std::string s_blank;
    static const std::string s_message_type;

public:  // ADDRESS BOOK CALLBACK
    static bool setAddrBookCaller(OTLookupCaller& theCaller);
    static OTLookupCaller* getAddrBookCaller();

protected:  // ADDRESS BOOK CALLER
    static OTLookupCaller* s_pCaller;

public:
    EXPORT OTRecordList();  // This one expects that s_pCaller is not nullptr.
    EXPORT explicit OTRecordList(const OTNameLookup& theLookup);
    EXPORT ~OTRecordList();
    EXPORT static const char* textTo() { return s_strTextTo.c_str(); }
    EXPORT static const char* textFrom() { return s_strTextFrom.c_str(); }

    EXPORT static void setTextTo(std::string text) { s_strTextTo = text; }
    EXPORT static void setTextFrom(std::string text) { s_strTextFrom = text; }
    EXPORT void SetFastMode() { m_bRunFast = true; }
    // SETUP:
    /** Set the default server here. */
    EXPORT void SetNotaryID(std::string str_id);
    /** Unless you have many servers, then use this. */
    EXPORT void AddNotaryID(std::string str_id);
    /** Also clears m_contents */
    EXPORT void ClearServers();
    EXPORT void SetInstrumentDefinitionID(std::string str_id);
    EXPORT void AddInstrumentDefinitionID(std::string str_id);
    /** Also clears m_contents */
    EXPORT void ClearAssets();
    EXPORT void SetNymID(std::string str_id);
    EXPORT void AddNymID(std::string str_id);
    /** Also clears m_contents */
    EXPORT void ClearNyms();
    EXPORT void SetAccountID(std::string str_id);
    EXPORT void AddAccountID(std::string str_id);
    /** Also clears m_contents */
    EXPORT void ClearAccounts();
    EXPORT const list_of_strings& GetNyms() const;
    EXPORT void AcceptChequesAutomatically(bool bVal = true);
    EXPORT void AcceptReceiptsAutomatically(bool bVal = true);
    EXPORT void AcceptTransfersAutomatically(bool bVal = true);
    EXPORT void AcceptCashAutomatically(bool bVal = true);
    EXPORT bool DoesAcceptChequesAutomatically() const;
    EXPORT bool DoesAcceptReceiptsAutomatically() const;
    EXPORT bool DoesAcceptTransfersAutomatically() const;
    EXPORT bool DoesAcceptCashAutomatically() const;
    /** Before populating, process out any items we're supposed to accept
     * automatically. POPULATE: */
    EXPORT bool PerformAutoAccept();
    EXPORT void notifyOfSuccessfulNotarization(
        const std::string& str_acct_id,
        const std::string p_nym_id,
        const std::string p_notary_id,
        const std::string p_txn_contents,
        int64_t lTransactionNum,
        int64_t lTransNumForDisplay) const;
    /** Populates m_contents from OT API. Calls ClearContents(). */
    EXPORT bool Populate();
    /** Clears m_contents (NOT nyms, accounts, servers, or instrument
     * definitions.) */
    EXPORT void ClearContents();
    /** Populate already sorts. But if you have to add some external records
     * after Populate, then you can sort again. P.S. sorting is performed based
     * on the "from" date. */
    EXPORT void SortRecords();
    /** Let's say you also want to add some Bitmessages. (Or any other external
     * source.) This is where you do that. Make sure to call Populate, then use
     * AddSpecialMsg a few times, then call SortRecords. */
    EXPORT void AddSpecialMsg(
        const std::string& str_msg_id,  // The ID of this message, from whatever
                                        // system it came from.
        bool bIsOutgoing,
        int32_t nMethodID,
        const std::string& str_contents,  // Make sure to concatentate subject
                                          // with contents, before passing here.
        const std::string& str_address,
        const std::string& str_other_address,
        const std::string& str_type,
        const std::string& str_type_display,
        std::string str_my_nym_id = "",
        time64_t tDate = OT_TIME_ZERO);
    // RETRIEVE:
    EXPORT int32_t size() const;
    EXPORT OTRecord GetRecord(int32_t nIndex);
    EXPORT bool RemoveRecord(int32_t nIndex);
};

}  // namespace opentxs

#endif  // OPENTXS_CLIENT_OTRECORDLIST_HPP
