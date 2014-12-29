/************************************************************
 *
 *  CmdNewOffer.cpp
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

#include "CmdNewOffer.hpp"

#include <opentxs/client/ot_otapi_ot.hpp>
#include <opentxs/client/OT_ME.hpp>

#include <opentxs/core/Log.hpp>
#include <opentxs/core/OTStorage.hpp>

using namespace opentxs;
using namespace std;

CmdNewOffer::CmdNewOffer()
{
    command = "newoffer";
    // FIX more arguments
    args[0] = "--myacct <assetaccount>";
    args[1] = "--hisacct <currencyaccount>";
    args[2] = "--type <ask|bid>";
    args[3] = "--scale <1|10|100|...>";
    args[4] = "--mininc <min increment>";
    args[5] = "--quantity <quantity>";
    args[6] = "--price <price>";
    args[7] = "[--lifespan <seconds> (default 86400 (1 day))]";
    category = catMarkets;
    help = "Create a new market offer.";
    usage = "A price of 0 means a market order at any price.";
}

CmdNewOffer::~CmdNewOffer()
{
}

int32_t CmdNewOffer::runWithOptions()
{
    return run(getOption("myacct"), getOption("hisacct"), getOption("type"),
               getOption("scale"), getOption("mininc"), getOption("quantity"),
               getOption("price"), getOption("lifespan"));
}

int32_t CmdNewOffer::run(string myacct, string hisacct, string type,
                         string scale, string mininc, string quantity,
                         string price, string lifespan)
{
    if (!checkAccount("myacct", myacct)) {
        return -1;
    }

    if (!checkAccount("hisacct", hisacct)) {
        return -1;
    }

    if (type != "ask" && type != "bid") {
        otOut << "Error: type: mandatory ask/bid parameter not specified.\n";
        return -1;
    }

    if (!checkValue("scale", scale)) {
        return -1;
    }

    if (!checkValue("quantity", quantity)) {
        return -1;
    }

    if (!checkValue("price", price)) {
        return -1;
    }

    if ("" != lifespan && !checkValue("lifespan", lifespan)) {
        return -1;
    }

    string server = OTAPI_Wrap::GetAccountWallet_NotaryID(myacct);
    if ("" == server) {
        otOut << "Error: cannot determine server from myacct.\n";
        return -1;
    }

    string mynym = OTAPI_Wrap::GetAccountWallet_NymID(myacct);
    if ("" == mynym) {
        otOut << "Error: cannot determine mynym from myacct.\n";
        return -1;
    }

    string hisserver = OTAPI_Wrap::GetAccountWallet_NotaryID(hisacct);
    if ("" == hisserver) {
        otOut << "Error: cannot determine server from myacct.\n";
        return -1;
    }

    string hisnym = OTAPI_Wrap::GetAccountWallet_NymID(hisacct);
    if ("" == hisnym) {
        otOut << "Error: cannot determine hisnym from hisacct.\n";
        return -1;
    }

    if (mynym != hisnym) {
        otOut << "Error: you must own both myacct and hisacct.\n";
        return -1;
    }

    if (server != hisserver) {
        otOut << "Error: accounts must be on the same server.\n";
        return -1;
    }

    OT_ME ot_me;
    ot_me.get_nym_market_offers(server, mynym);

    if (0 > cleanMarketOfferList(server, mynym, myacct, hisacct, type, scale,
                                 price)) {
        return -1;
    }

    // OKAY! Now that we've cleaned out any undesirable offers, let's place the
    // the offer itself!
    int64_t s, m, q, p, l;
    sscanf(scale.c_str(), "%" SCNd64, &s);
    sscanf(mininc.c_str(), "%" SCNd64, &m);
    sscanf(quantity.c_str(), "%" SCNd64, &q);
    sscanf(price.c_str(), "%" SCNd64, &p);
    sscanf(lifespan.c_str(), "%" SCNd64, &l);
    string response = ot_me.create_market_offer(myacct, hisacct, s, m, q, p,
                                                type == "ask", l, "", 0);
    return responseReply(response, server, mynym, myacct,
                         "create_market_offer");
}

// NOTE: This function has nothing to do with placing a new offer. Instead,
// as a convenience for knotwork, it first checks to see if there are any
// existing offers within certain parameters based on this new one, and
// removes them if so. Only then, after that is done, does it actually place
// the new offer. (Meaning: most of the code you see here at first is not
// actually necessary for placing offers, but was done at the request of a
// server operator.)

// me: How about this — when you do "opentxs newoffer" I can alter that script
// to automatically cancel any sell offers for a lower amount than my new buy
// offer, if they're on the same market at the same scale. And vice versa.
// Vice versa meaning, cancel any bid offers for a higher amount than my new
// sell offer.
//
// knotwork: yeah that would work.
//
// So when placing a buy offer, check all the other offers I already have at
// the same scale, same asset and currency ID. (That is, the same "market" as
// denoted by strMapKey in "opentxs showmyoffers") For each, see if it's a sell
// offer and if so, if the amount is lower than the amount on the new buy offer,
// then cancel that sell offer from the market. (Because I don't want to
// buy-high, sell-low.)
//
// Similarly, if placing a sell offer, then check all the other offers I already
// have at the same scale, same asset and currency ID, (the same "market" as
// denoted by strMapKey....) For each, see if it's a buy offer and if so, if
// the amount is higher than the amount of my new sell offer, then cancel that
// buy offer from the market. (Because I don't want some old buy offer for $10
// laying around for the same stock that I'm SELLING for $8! If I dump 100
// shares, I'll receive $800 --I don't want my software to automatically turn
// around and BUY those same shares again for $1000! That would be a $200 loss.)

int32_t CmdNewOffer::cleanMarketOfferList(
    const string& server, const string& mynym, const string& myacct,
    const string& hisacct, const string& type, const string& scale,
    const string& price)
{
    OTDB::OfferListNym* offerList = loadNymOffers(server, mynym);
    if (nullptr == offerList) {
        otOut << "Error: cannot load market offer list.\n";
        return -1;
    }

    // LOOP THROUGH THE OFFERS and sort them into a map_of_maps, key is:
    // scale-instrumentDefinitionID-currencyID. the value for each key is a
    // sub-map, with
    // the key: transaction ID and value: the offer data itself.
    int32_t items = offerList->GetOfferDataNymCount();
    if (0 > items) {
        otOut << "Error: cannot load market offer list count.\n";
        return -1;
    }

    if (0 == items) {
        otOut << "The market offer list is empty.\n";
        return 0;
    }

    MapOfMaps* map_of_maps = convert_offerlist_to_maps(*offerList);
    if (nullptr == map_of_maps) {
        otOut << "Error: cannot convert offer list to map.\n";
        return -1;
    }

    // find_strange_offers is called for each offer, for this nym, as it
    // iterates through the maps. When it's done, extra.the_vector
    // will contain a vector of all the transaction numbers for offers that
    // we should cancel, before placing the new offer. (Such as an offer to
    // sell for 30 clams when our new offer buys for 40...)

    the_lambda_struct extra;
    extra.the_asset_acct = myacct;
    extra.the_currency_acct = hisacct;
    extra.the_scale = scale;
    extra.the_price = price;
    extra.bSelling = type == "ask";

    if (0 > iterate_nymoffers_maps(*map_of_maps, find_strange_offers, extra)) {
        otOut << "Error: cannot iterate nym's offers.\n";
        return -1;
    }

    // Okay -- if there are any offers we need to cancel, extra.the_vector
    // now contains the transaction number for each one. Let's remove them from
    // the market before starting up the new offer...
    for (size_t i = 0; i < extra.the_vector.size(); i++) {
        const string& id = extra.the_vector[i];
        otOut << "Canceling market offer with transaction number: " << id
              << ".\n";

        OT_ME ot_me;
        int64_t j;
        sscanf(id.c_str(), "%" SCNd64, &j);
        string response = ot_me.kill_market_offer(server, mynym, myacct, j);
        if (0 > processTxResponse(server, mynym, myacct, response,
                                  "kill market offer")) {
            return -1;
        }
    }
    extra.the_vector.clear();

    return 1;
}
