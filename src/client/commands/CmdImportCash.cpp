/************************************************************
 *
 *  CmdImportCash.cpp
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

#include "CmdImportCash.hpp"

#include <opentxs/client/OTAPI.hpp>
#include <opentxs/core/OTLog.hpp>

using namespace opentxs;
using namespace std;

CmdImportCash::CmdImportCash()
{
    command = "importcash";
    args[0] = "[--mynym <nym>]";
    category = catInstruments;
    help = "Import a pasted cash purse.";
    usage = "Specify mynym when a signer nym cannot be deduced.";
}

CmdImportCash::~CmdImportCash()
{
}

int32_t CmdImportCash::runWithOptions()
{
    return run(getOption("mynym"));
}

int32_t CmdImportCash::run(string mynym)
{
    if ("" != mynym && !checkNym("mynym", mynym)) {
        return -1;
    }

    string instrument = inputText("a cash purse");
    if ("" == instrument) {
        return -1;
    }

    string type = OTAPI_Wrap::Instrmnt_GetType(instrument);
    if ("" == type) {
        otOut << "Error: cannot determine instrument type.\n";
        return -1;
    }

    string server = OTAPI_Wrap::Instrmnt_GetServerID(instrument);
    if ("" == server) {
        otOut << "Error: cannot determine instrument server.\n";
        return -1;
    }

    if ("PURSE" != type) {
        // Todo: case "TOKEN"
        //
        // NOTE: This is commented out because since it is guessing the NymID as
        // MyNym, then it will just create a purse for MyNym and import it into
        // that purse, and then later when doing a deposit, THAT's when it tries
        // to DECRYPT that token and re-encrypt it to the SERVER's nym... and
        // that's when we might find out that it never was encrypted to MyNym in
        // the first place -- we had just assumed it was here, when we did the
        // import. Until I can look at that in more detail, it will remain
        // commented out.
        // bool bImportedToken = importCashPurse(server, mynym, assetID,
        //                                      userInput, isPurse);
        // if (importCashPurse(server, mynym, assetID, userInput, isPurse))
        //{
        //    otOut << "\n\n Success importing cash token!\nServer: "
        //        << server << "\nAsset Type: " << assetID
        //        << "\nNym: " << MyNym << "\n\n";
        //    return 1;
        //}

        otOut << "Error: invalid instrument type. Expected PURSE.\n";
        return -1;
    }

    string purseOwner = "";
    if (!OTAPI_Wrap::Purse_HasPassword(server, instrument)) {
        purseOwner = OTAPI_Wrap::Instrmnt_GetRecipientUserID(instrument);
    }

    // Whether the purse was password-protected (and thus had no Nym ID) or
    // whether it does have a Nym ID (but it wasn't listed on the purse)
    // Then either way, in those cases purseOwner will still be empty.
    //
    // (The third case is that the purse is Nym protected and the ID WAS
    // available, in which case we'll skip this block, since we already
    // have it.)
    //
    // But even in the case where there's no Nym at all (password protected)
    // we STILL need to pass a Signer Nym ID into
    // OTAPI_Wrap::Wallet_ImportPurse.
    // So if it's still empty here, then we use --mynym to make the call.
    // And also, even in the case where there IS a Nym but it's not listed,
    // we must assume the USER knows the appropriate NymID, even if it's not
    // listed on the purse itself. And in that case as well, the user can
    // simply specify the Nym using --mynym.
    //
    // Bottom line: by this point, if it's still not set, then we just use
    // MyNym, and if THAT's not set, then we return failure.
    if ("" == purseOwner) {
        purseOwner = mynym;
        if ("" == purseOwner) {
            otOut << "Error: cannot determine purse owner.\n"
                     "Please specify mynym.\n";
            return -1;
        }
    }

    string assetID = OTAPI_Wrap::Instrmnt_GetAssetID(instrument);
    if ("" == assetID) {
        otOut << "Error: cannot determine asset type ID.\n";
        return -1;
    }

    if (!OTAPI_Wrap::Wallet_ImportPurse(server, assetID, purseOwner,
                                        instrument)) {
        otOut << "Error: cannot import purse.\n";
        return -1;
    }

    return 1;
}
