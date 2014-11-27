/************************************************************
 *
 *  InstantiateContract.hpp
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

#ifndef OPENTXS_EXT_INSTANTIATECONTRACT_HPP
#define OPENTXS_EXT_INSTANTIATECONTRACT_HPP

// Factory (though rarely used; was just added recently for the API.)
//
// If you want to instantiate a contract that you already have in string form,
// this function will figure out what kind of contract it is, and instantiate
// the
// right subclass, then load it up and return it.
//
// CALLER IS RESPONSIBLE to cleanup!

#include <opentxs/cash/Mint.hpp>
#include <opentxs/cash/Token.hpp>
#include <opentxs/core/recurring/OTPaymentPlan.hpp>
#include <opentxs/core/crypto/OTSignedFile.hpp>
#include <opentxs/core/script/OTSmartContract.hpp>
#include <opentxs/core/trade/OTOffer.hpp>
#include <opentxs/core/trade/OTTrade.hpp>
#include <opentxs/core/util/Assert.hpp>
#include <opentxs/core/Log.hpp>
#include <opentxs/core/Contract.hpp>
#include <opentxs/core/AssetContract.hpp>
#include <opentxs/core/Cheque.hpp>
#include <opentxs/core/Message.hpp>
#include <opentxs/core/OTServerContract.hpp>

namespace
{

opentxs::Contract* InstantiateContract(opentxs::String strInput)
{

    using namespace opentxs;
    String strContract, strFirstLine; // output for the below function.
    const bool bProcessed =
        Contract::DearmorAndTrim(strInput, strContract, strFirstLine);

    if (bProcessed) {

        Contract* pContract = nullptr;

        if (strFirstLine.Contains(
                "-----BEGIN SIGNED SMARTCONTRACT-----")) // this string is 36
                                                         // chars long.
        {
            pContract = new OTSmartContract();
            OT_ASSERT(nullptr != pContract);
        }

        if (strFirstLine.Contains(
                "-----BEGIN SIGNED PAYMENT PLAN-----")) // this string is 35
                                                        // chars long.
        {
            pContract = new OTPaymentPlan();
            OT_ASSERT(nullptr != pContract);
        }
        else if (strFirstLine.Contains(
                       "-----BEGIN SIGNED TRADE-----")) // this string is 28
                                                        // chars long.
        {
            pContract = new OTTrade();
            OT_ASSERT(nullptr != pContract);
        }
        else if (strFirstLine.Contains("-----BEGIN SIGNED OFFER-----")) {
            pContract = new OTOffer();
            OT_ASSERT(nullptr != pContract);
        }
        else if (strFirstLine.Contains("-----BEGIN SIGNED INVOICE-----")) {
            pContract = new Cheque();
            OT_ASSERT(nullptr != pContract);
        }
        else if (strFirstLine.Contains("-----BEGIN SIGNED VOUCHER-----")) {
            pContract = new Cheque();
            OT_ASSERT(nullptr != pContract);
        }
        else if (strFirstLine.Contains("-----BEGIN SIGNED CHEQUE-----")) {
            pContract = new Cheque();
            OT_ASSERT(nullptr != pContract);
        }
        else if (strFirstLine.Contains("-----BEGIN SIGNED MESSAGE-----")) {
            pContract = new Message();
            OT_ASSERT(nullptr != pContract);
        }
        else if (strFirstLine.Contains("-----BEGIN SIGNED MINT-----")) {
            pContract = Mint::MintFactory();
            OT_ASSERT(nullptr != pContract);
        }
        else if (strFirstLine.Contains("-----BEGIN SIGNED FILE-----")) {
            pContract = new OTSignedFile();
            OT_ASSERT(nullptr != pContract);
        }
        else if (strFirstLine.Contains("-----BEGIN SIGNED CASH-----")) {
            pContract = Token::LowLevelInstantiate(strFirstLine);
            OT_ASSERT(nullptr != pContract);
        }
        else if (strFirstLine.Contains("-----BEGIN SIGNED CASH TOKEN-----")) {
            pContract = Token::LowLevelInstantiate(strFirstLine);
            OT_ASSERT(nullptr != pContract);
        }
        else if (strFirstLine.Contains(
                       "-----BEGIN SIGNED LUCRE CASH TOKEN-----")) {
            pContract = Token::LowLevelInstantiate(strFirstLine);
            OT_ASSERT(nullptr != pContract);
        }

        // TODO: Might want to clarify in Asset and Server Contracts,
        // so I don't have to do this crap... The ones above are cleaner.
        //
        else if (strFirstLine.Contains("-----BEGIN SIGNED CONTRACT-----")) {
            if (strContract.Contains(
                    "<notaryProviderContract version=\"1.0\">")) {
                pContract = new OTServerContract();
                OT_ASSERT(nullptr != pContract);
            }
            else if (strContract.Contains(
                           "<instrumentDefinition version=\"1.0\">")) {
                pContract = new AssetContract();
                OT_ASSERT(nullptr != pContract);
            }
        }

        // The string didn't match any of the options in the factory.
        //
        if (nullptr == pContract)
            otOut << __FUNCTION__
                  << ": Object type not yet supported by class factory: "
                  << strFirstLine << "\n";
        // Does the contract successfully load from the string passed in?
        else if (!pContract->LoadContractFromString(strContract)) {
            otOut << __FUNCTION__
                  << ": Failed loading contract from string (first line): "
                  << strFirstLine << "\n";
            delete pContract;
            pContract = nullptr;
        }
        else
            return pContract;
    }
    return nullptr;
}

} // unnamed namespace

#endif // OPENTXS_EXT_INSTANTIATECONTRACT_HPP
