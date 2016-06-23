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

#ifndef OPENTXS_CORE_OTSTORAGEPB_HPP
#define OPENTXS_CORE_OTSTORAGEPB_HPP

#include "opentxs/core/util/Assert.hpp"

#include <deque>
#include <iostream>
#include <map>
#include <vector>

#if defined(OTDB_PROTOCOL_BUFFERS)

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4244)
#pragma warning(disable : 4267)
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#ifndef __clang__
// -Wuseless-cast does not exist in clang
#pragma GCC diagnostic ignored "-Wuseless-cast"
#endif
#endif

#include "Generics.pb.h"
#include "Markets.pb.h"
#include "Bitcoin.pb.h"
#include "Moneychanger.pb.h"

#ifdef _WIN32
#pragma warning(pop)
#else
#pragma GCC diagnostic pop
#endif

namespace opentxs
{

namespace OTDB
{

// Interface:    IStorablePB
//
class IStorablePB : public IStorable
{
public:
    virtual ~IStorablePB()
    {
    }

    virtual ::google::protobuf::MessageLite* getPBMessage();
    virtual bool onPack(PackedBuffer& theBuffer, Storable& inObj);
    virtual bool onUnpack(PackedBuffer& theBuffer, Storable& outObj);
    OT_USING_ISTORABLE_HOOKS;
};

// BUFFER for Protocol Buffers.
// Google's protocol buffers serializes to std::strings and streams. How
// conveeeeeenient.

class BufferPB : public PackedBuffer
{
    friend class PackerSubclass<BufferPB>;
    friend class IStorablePB;
    std::string m_buffer;

public:
    BufferPB()
        : PackedBuffer()
    {
    }
    virtual ~BufferPB()
    {
    }
    virtual bool PackString(std::string& theString);
    virtual bool UnpackString(std::string& theString);
    virtual bool ReadFromIStream(std::istream& inStream, int64_t lFilesize);
    virtual bool WriteToOStream(std::ostream& outStream);
    virtual const uint8_t* GetData();
    virtual size_t GetSize();
    virtual void SetData(const uint8_t* pData, size_t theSize);
    std::string& GetBuffer()
    {
        return m_buffer;
    }
};

// Protocol Buffers packer.
//
typedef PackerSubclass<BufferPB> PackerPB;

// Used for subclassing IStorablePB:
//
template <class theBaseType, class theInternalType,
          StoredObjectType theObjectType>
class ProtobufSubclass : public theBaseType, public IStorablePB
{
private:
    theInternalType __pb_obj;
    std::string m_Type;

public:
    static Storable* Instantiate()
    {
        return dynamic_cast<Storable*>(
            new ProtobufSubclass<theBaseType, theInternalType, theObjectType>);
    }

    ProtobufSubclass()
        : theBaseType()
        , IStorablePB()
        , m_Type(StoredObjectTypeStrings[static_cast<int32_t>(theObjectType)])
    {
        m_Type += "PB";
        /*std::cout << m_Type << " -- Constructor" << std::endl;*/ }

    ProtobufSubclass(const ProtobufSubclass<theBaseType, theInternalType,
                                            theObjectType>& rhs)
        : theBaseType()
        , IStorablePB()
        , m_Type(StoredObjectTypeStrings[static_cast<int32_t>(theObjectType)])
    {
        m_Type += "PB";
        /*std::cout << m_Type << " -- Copy Constructor" << std::endl; */ rhs
            .CopyToObject(*this);
    }

    ProtobufSubclass<theBaseType, theInternalType, theObjectType>& operator=(
        const ProtobufSubclass<theBaseType, theInternalType, theObjectType>&
            rhs)
    {
        rhs.CopyToObject(*this);
        return *this;
    }

    void CopyToObject(ProtobufSubclass<theBaseType, theInternalType,
                                       theObjectType>& theNewStorable) const
    {
        OTPacker* pPacker = OTPacker::Create(PACK_PROTOCOL_BUFFERS);
        const OTDB::Storable* pIntermediate =
            dynamic_cast<const OTDB::Storable*>(this);

        if (nullptr == pPacker) {
            OT_FAIL;
        }
        PackedBuffer* pBuffer =
            pPacker->Pack(*(const_cast<OTDB::Storable*>(pIntermediate)));
        if (nullptr == pBuffer) {
            OT_FAIL;
        }
        if (!pPacker->Unpack(*pBuffer, theNewStorable)) {
            OT_FAIL;
        }
        if (nullptr != pPacker) {
            delete pPacker;
            pPacker = nullptr;
        }
        if (nullptr != pBuffer) {
            delete pBuffer;
            pBuffer = nullptr;
        }
    }

    virtual ::google::protobuf::MessageLite* getPBMessage();

    //        IStorable * clone(void) const
    //            {return dynamic_cast<IStorable *>(new
    // ProtobufSubclass<theBaseType, theInternalType, theObjectType>(*this));}

    virtual theBaseType* clone(void) const
    { /*std::cout << "Cloning a " << m_Type << std::endl;*/
        return dynamic_cast<theBaseType*>(do_clone());
    }

    IStorable* do_clone(void) const
    {
        Storable* pNewStorable =
            Storable::Create(theObjectType, PACK_PROTOCOL_BUFFERS);
        if (nullptr == pNewStorable) OT_FAIL;
        CopyToObject(*(dynamic_cast<
            ProtobufSubclass<theBaseType, theInternalType, theObjectType>*>(
            pNewStorable)));
        return dynamic_cast<IStorable*>(pNewStorable);
    }

    virtual ~ProtobufSubclass()
    {
    }
    OT_USING_ISTORABLE_HOOKS;
    virtual void hookBeforePack();  // <=== Implement this if you subclass.
    virtual void hookAfterUnpack(); // <=== Implement this if you subclass.
};

#define DECLARE_PROTOBUF_SUBCLASS(theBaseType, theInternalType, theNewType,    \
                                  theObjectType)                               \
    template <>                                                                \
    void ProtobufSubclass<theBaseType, theInternalType,                        \
                          theObjectType>::hookBeforePack();                    \
    template <>                                                                \
    void ProtobufSubclass<theBaseType, theInternalType,                        \
                          theObjectType>::hookAfterUnpack();                   \
    typedef ProtobufSubclass<theBaseType, theInternalType, theObjectType>      \
        theNewType

// THE ACTUAL SUBCLASSES:

DECLARE_PROTOBUF_SUBCLASS(OTDBString, String_InternalPB, StringPB,
                          STORED_OBJ_STRING);
DECLARE_PROTOBUF_SUBCLASS(Blob, Blob_InternalPB, BlobPB, STORED_OBJ_BLOB);
DECLARE_PROTOBUF_SUBCLASS(StringMap, StringMap_InternalPB, StringMapPB,
                          STORED_OBJ_STRING_MAP);
DECLARE_PROTOBUF_SUBCLASS(BitcoinAcct, BitcoinAcct_InternalPB, BitcoinAcctPB,
                          STORED_OBJ_BITCOIN_ACCT);
DECLARE_PROTOBUF_SUBCLASS(BitcoinServer, BitcoinServer_InternalPB,
                          BitcoinServerPB, STORED_OBJ_BITCOIN_SERVER);
DECLARE_PROTOBUF_SUBCLASS(RippleServer, RippleServer_InternalPB, RippleServerPB,
                          STORED_OBJ_RIPPLE_SERVER);
DECLARE_PROTOBUF_SUBCLASS(LoomServer, LoomServer_InternalPB, LoomServerPB,
                          STORED_OBJ_LOOM_SERVER);
DECLARE_PROTOBUF_SUBCLASS(ServerInfo, ServerInfo_InternalPB, ServerInfoPB,
                          STORED_OBJ_SERVER_INFO);
DECLARE_PROTOBUF_SUBCLASS(ContactAcct, ContactAcct_InternalPB, ContactAcctPB,
                          STORED_OBJ_CONTACT_ACCT);
DECLARE_PROTOBUF_SUBCLASS(ContactNym, ContactNym_InternalPB, ContactNymPB,
                          STORED_OBJ_CONTACT_NYM);
DECLARE_PROTOBUF_SUBCLASS(Contact, Contact_InternalPB, ContactPB,
                          STORED_OBJ_CONTACT);
DECLARE_PROTOBUF_SUBCLASS(AddressBook, AddressBook_InternalPB, AddressBookPB,
                          STORED_OBJ_ADDRESS_BOOK);
DECLARE_PROTOBUF_SUBCLASS(WalletData, WalletData_InternalPB, WalletDataPB,
                          STORED_OBJ_WALLET_DATA);
DECLARE_PROTOBUF_SUBCLASS(MarketData, MarketData_InternalPB, MarketDataPB,
                          STORED_OBJ_MARKET_DATA);
DECLARE_PROTOBUF_SUBCLASS(MarketList, MarketList_InternalPB, MarketListPB,
                          STORED_OBJ_MARKET_LIST);

DECLARE_PROTOBUF_SUBCLASS(BidData, OfferDataMarket_InternalPB, BidDataPB,
                          STORED_OBJ_BID_DATA);
DECLARE_PROTOBUF_SUBCLASS(AskData, OfferDataMarket_InternalPB, AskDataPB,
                          STORED_OBJ_ASK_DATA);
DECLARE_PROTOBUF_SUBCLASS(OfferListMarket, OfferListMarket_InternalPB,
                          OfferListMarketPB, STORED_OBJ_OFFER_LIST_MARKET);
DECLARE_PROTOBUF_SUBCLASS(TradeDataMarket, TradeDataMarket_InternalPB,
                          TradeDataMarketPB, STORED_OBJ_TRADE_DATA_MARKET);
DECLARE_PROTOBUF_SUBCLASS(TradeListMarket, TradeListMarket_InternalPB,
                          TradeListMarketPB, STORED_OBJ_TRADE_LIST_MARKET);
DECLARE_PROTOBUF_SUBCLASS(OfferDataNym, OfferDataNym_InternalPB, OfferDataNymPB,
                          STORED_OBJ_OFFER_DATA_NYM);
DECLARE_PROTOBUF_SUBCLASS(OfferListNym, OfferListNym_InternalPB, OfferListNymPB,
                          STORED_OBJ_OFFER_LIST_NYM);
DECLARE_PROTOBUF_SUBCLASS(TradeDataNym, TradeDataNym_InternalPB, TradeDataNymPB,
                          STORED_OBJ_TRADE_DATA_NYM);
DECLARE_PROTOBUF_SUBCLASS(TradeListNym, TradeListNym_InternalPB, TradeListNymPB,
                          STORED_OBJ_TRADE_LIST_NYM);

typedef OfferDataMarket_InternalPB BidData_InternalPB;
typedef OfferDataMarket_InternalPB AskData_InternalPB;

// !! ALL OF THESE have to provide implementations for hookBeforePack() and
// hookAfterUnpack().
// In .cpp file:
/*
void SUBCLASS_HERE::hookBeforePack()
{
__pb_obj.set_PROPERTY_NAME_GOES_HERE(PROPERTY_NAME_GOES_HERE);
}
void SUBCLASS_HERE::hookAfterUnpack()
{
PROPERTY_NAME_GOES_HERE    = __pb_obj.PROPERTY_NAME_GOES_HERE();
}
*/

} // namespace OTDB

} // namespace opentxs

#endif // defined(OTDB_PROTOCOL_BUFFERS)

#endif // OPENTXS_CORE_OTSTORAGEPB_HPP
