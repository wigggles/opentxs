// Factory (though rarely used; was just added recently for the API.)
//
// If you want to instantiate a contract that you already have in string form,
// this function will figure out what kind of contract it is, and instantiate
// the
// right subclass, then load it up and return it.
//
// CALLER IS RESPONSIBLE to cleanup!

#include "opentxs/cash/Mint.hpp"
#include "opentxs/cash/Token.hpp"
#include "opentxs/core/crypto/OTSignedFile.hpp"
#include "opentxs/core/script/OTSmartContract.hpp"
#include "opentxs/core/trade/OTOffer.hpp"
#include "opentxs/core/trade/OTTrade.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/OTLog.hpp"
#include "opentxs/core/OTContract.hpp"
#include "opentxs/core/OTAssetContract.hpp"
#include "opentxs/core/OTCheque.hpp"
#include "opentxs/core/OTMessage.hpp"
#include "opentxs/core/OTPaymentPlan.hpp"
#include "opentxs/core/OTServerContract.hpp"

namespace
{

opentxs::OTContract* InstantiateContract(opentxs::OTString strInput)
{

    using namespace opentxs;
    OTString strContract, strFirstLine; // output for the below function.
    const bool bProcessed =
        OTContract::DearmorAndTrim(strInput, strContract, strFirstLine);

    if (bProcessed) {

        OTContract* pContract = nullptr;

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
            pContract = new OTCheque();
            OT_ASSERT(nullptr != pContract);
        }
        else if (strFirstLine.Contains("-----BEGIN SIGNED VOUCHER-----")) {
            pContract = new OTCheque();
            OT_ASSERT(nullptr != pContract);
        }
        else if (strFirstLine.Contains("-----BEGIN SIGNED CHEQUE-----")) {
            pContract = new OTCheque();
            OT_ASSERT(nullptr != pContract);
        }
        else if (strFirstLine.Contains("-----BEGIN SIGNED MESSAGE-----")) {
            pContract = new OTMessage();
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
                           "<digitalAssetContract version=\"1.0\">")) {
                pContract = new OTAssetContract();
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
        else if (false == pContract->LoadContractFromString(strContract)) {
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
