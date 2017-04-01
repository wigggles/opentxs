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

#include "opentxs/core/script/OTAgent.hpp"

#include "opentxs/api/Identity.hpp"
#include "opentxs/api/OT.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/consensus/Context.hpp"
#include "opentxs/core/recurring/OTAgreement.hpp"
#include "opentxs/core/script/OTParty.hpp"
#include "opentxs/core/script/OTPartyAccount.hpp"
#include "opentxs/core/script/OTSmartContract.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/Tag.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/Types.hpp"

#include <cstdint>
#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <utility>

// Have the agent try to verify his own signature against any contract.
//
// NOTE: This function assumes that you have already taken actions that would
// have loaded the Nym's pointer
// and placed it within this Agent. This is a low-level call and it expects that
// you have already been using
// calls such as HasAgent(), HasAuthorizingAgent(), LoadAuthorizingAgent(), etc.
// This function also assumes that once you are done, you will call
// ClearTemporaryPointers().
//

namespace opentxs
{

bool OTAgent::VerifySignature(const Contract& theContract) const
{
    // Only individual agents can sign for things, not groups (groups vote, they
    // don't sign.)
    // Thus, an individual can verify a signature, whereas a voting group would
    // verify an election result (or whatever.)
    //
    if (!IsAnIndividual() || !DoesRepresentHimself()) {
        otErr << "OTAgent::VerifySignature: Entities and roles are not yet "
                 "supported. Agent: "
              << m_strName << ".\n";
        return false;
    }  // todo: when adding entities, this will change.

    //    if (DoesRepresentAnEntity)
    //    {
    //        // The original version of a smartcontract might show that Frank,
    // the Sales Director, signed it.
    //        // Years later, Frank is fired, and Jim is appointed to his former
    // Role of sales director, in the same entity.
    //        // The original copy of the smart contract still contains Frank's
    // signature, and thus we still need to load Frank
    //        // in order to verify that original signature.  That's why we load
    // Frank by the NymID stored there. He was the Nym
    //        // at the time, so that's the key we load.
    //        //
    //        // NEXT: What if JIM tries to verify the signature on the
    // contract, even though FRANK was the original signer?
    //        // Should OTAgent be smart enough here to substitute Frank
    // whenever Jim tries to verify? I argue no: this function is
    //        // too low-level. Plus it's backwards. If Jim tries to DO an
    // action, THEN OT should be smart enough to verify that Jim
    //        // is in the proper Role and that Jim's signature is good enough
    // to authorize actions. But if OT is verifying Frank's
    //        // signature on some old copy of something that Frank formerly
    // signed, then this function should clearly tell me if Frank's
    //        // sig verified... or not.
    //        //
    //        // Therefore the "DoesRepresentAnEntity()" option is useless here,
    // since we are verifying the same Nym's signature whether
    //        // he represents an entity or not.
    //        //
    //    }
    //    else
    if (nullptr == m_pNym) {
        String strTemp(theContract);
        otErr << "OTAgent::VerifySignature: Attempted to verify signature on "
                 "contract, "
                 "but no Nym had ever been loaded for this agent:\n\n"
              << strTemp << "\n\n";
        return false;
    }

    return theContract.VerifySignature(*m_pNym);
}

// Low-level.
// Caller is responsible to delete.
// Don't call this unless you're sure the same Nym isn't already loaded, or
// unless
// you are prepared to compare the returned Nym with all the Nyms you already
// have loaded.
//
// This call may always fail for a specific agent, if the agent isn't a Nym
// (the agent could be a voting group.)
//
Nym* OTAgent::LoadNym(Nym& theServerNym)
{
    Identifier theAgentNymID;
    bool bNymID = GetNymID(theAgentNymID);

    if (bNymID) {
        Nym* pNym = new Nym;
        OT_ASSERT(nullptr != pNym);

        pNym->SetIdentifier(theAgentNymID);

        if (!pNym->LoadPublicKey()) {
            String strNymID(theAgentNymID);
            otErr << "OTAgent::LoadNym: Failure loading "
                     "agent's public key:\n"
                  << strNymID << "\n";
            delete pNym;
            pNym = nullptr;
        } else if (
            pNym->VerifyPseudonym() && pNym->LoadSignedNymfile(theServerNym)) {
            SetNymPointer(
                *pNym);  // set this pointer in case I need it for later.
            // also remember, caller is responsible to delete, so there's no
            // guarantee the pointer
            return pNym;  // is any good.  Then again, caller is also
                          // responsible
                          // to call ClearTemporaryPointers().
        } else {
            String strNymID(theAgentNymID);
            otErr << "OTAgent::LoadNym: Failure verifying agent's public key "
                     "or loading signed nymfile: "
                  << strNymID << "\n";
            delete pNym;
            pNym = nullptr;
        }
    } else
        otErr << "OTAgent::LoadNym: Failure. Are you sure this agent IS a Nym "
                 "at all? \n";

    return nullptr;
}

OTAgent::OTAgent()
    : m_bNymRepresentsSelf(false)
    , m_bIsAnIndividual(false)
    , m_pNym(nullptr)
    , m_pForParty(nullptr)
{
}

OTAgent::OTAgent(
    bool bNymRepresentsSelf,
    bool bIsAnIndividual,
    const String& strName,
    const String& strNymID,
    const String& strRoleID,
    const String& strGroupName)
    : m_bNymRepresentsSelf(bNymRepresentsSelf)
    , m_bIsAnIndividual(bIsAnIndividual)
    , m_pNym(nullptr)
    , m_pForParty(nullptr)
    , m_strName(strName)
    , m_strNymID(strNymID)
    , m_strRoleID(strRoleID)
    , m_strGroupName(strGroupName)
{
}

OTAgent::OTAgent(
    const std::string& str_agent_name,
    Nym& theNym,
    const bool bNymRepresentsSelf)
    /*IF false, then: ROLE parameter goes here.*/
    : m_bNymRepresentsSelf(bNymRepresentsSelf),
      m_bIsAnIndividual(true),
      m_pNym(&theNym),
      m_pForParty(nullptr),
      m_strName(str_agent_name.c_str())
{
    // Grab m_strNymID
    Identifier theNymID;
    theNym.GetIdentifier(theNymID);
    theNymID.GetString(m_strNymID);

    //

    if (!bNymRepresentsSelf) {
        // Todo: if the Nym represents an Entity, then RoleID should
        // be passed in, and set here.  I WILL PROBABLY make that part into a
        // SEPARATE CONSTRUCTOR.
        // (Once I get around to adding Entities.)
        //
        otErr << "OTAgent::OTAgent: THIS HASN'T BEEN WRITTEN YET!!\n";
    }
}

void OTAgent::SetParty(OTParty& theOwnerParty)  // This happens when the agent
                                                // is
                                                // added to the party.
{
    m_pForParty = &theOwnerParty;

    // A Nym can only act as agent for himself or for an entity
    // (never for another Nym. Start an entity if you want that.)
    // If the owner party is a Nym, therefore this agent IS the Nym acting for
    // himself.
    // Whereas if the owner party were an entity, then this agent could be a Nym
    // or a voting group.
    // Since inside this block the owner party IS a Nym, not an entity, then the
    // agent can therefore
    // only be THAT Nym, acting as an agent for himself. Remember, a Nym cannot
    // act as agent for
    // another Nym. That is only possible by an agreement between them, and that
    // agreement becomes
    // the entity (therefore it's only possible using an entity.)
    //
    if (theOwnerParty.IsNym())  // Thus, this basically means the agent IS the
                                // party.
    {
        m_bNymRepresentsSelf = true;
        m_bIsAnIndividual = true;

        bool bGetOwnerNymID = false;
        const std::string str_owner_nym_id =
            theOwnerParty.GetNymID(&bGetOwnerNymID);
        m_strNymID.Set(bGetOwnerNymID ? str_owner_nym_id.c_str() : "");

        // Todo here, instead of copying the Owner's Nym ID like above, just
        // make sure they match.
        // Similarly, make sure that the RoleID or GroupName, whichever is
        // relevant, is validated for the owner.
    }
}

OTAgent::~OTAgent()
{
    m_pNym =
        nullptr;  // this pointer is not owned by this object, and is here for
                  // convenience only.
    m_pForParty =
        nullptr;  // The agent probably has a pointer to the party it acts
                  // on behalf of.
}

// If the agent is a Nym acting for himself, this will be true. Otherwise, if
// agent is a Nym acting in a role for an entity, or if agent is a voting group
// acting for the entity to which it belongs, either way, this will be false.

bool OTAgent::DoesRepresentHimself() const { return m_bNymRepresentsSelf; }

// Whether the agent is a voting group acting for an entity, or is a Nym acting
// in a Role for an entity, this will be true either way. (Otherwise, if agent
// is a Nym acting for himself, then this will be false.)

bool OTAgent::DoesRepresentAnEntity() const { return !m_bNymRepresentsSelf; }

// Only one of these can be true:
//
// - Agent is either a Nym acting for himself or some entity,
// - or agent is a group acting for some entity.

// Agent is an individual Nym. (Meaning either he IS ALSO the party and thus
// represents himself, OR he is an agent for an entity who is the party, and
// he's acting in a role for that entity.) If agent were a group, this would be
// false.

bool OTAgent::IsAnIndividual() const { return m_bIsAnIndividual; }

// OR: Agent is a voting group, which cannot take proactive or instant action,
// but only passive and delayed. Entity-ONLY. (A voting group cannot decide on
// behalf of individual, but only on behalf of the entity it belongs to.)

bool OTAgent::IsAGroup() const { return !m_bIsAnIndividual; }

// A Nym cannot act as "agent" for another Nym.
// Nor can a Group act as "agent" for a Nym. Why not? Because:
//
// An entity is COMPOSED of its voting groups and its employee Nyms.
// These don't merely act "on behalf" of the entity, but in fact they comprise
// the entity.
// Therefore the entity can use voting groups and employee Nyms to make
// decisions BECAUSE
// IT **HAS** voting groups and employee nyms that it can use.
//
// Whereas an individual Nym is NOT composed of voting groups or employee Nyms.
// So how could
// he designate to act "on his behalf" something that does not even exist?
// So we must ask, how can another Nym, then, be appointed to act as my agent
// without some
// agreement designating him as such? And that agreement is the entity itself,
// which is nothing
// more than an agreement between several owners to designate agents to operate
// according to their
// interests.

// To directly appoint one Nym to act on behalf of another, yet WITHOUT any
// agreement in place,
// is to behave as one is the same as the other. But still, who has the key? If
// both have absolutely
// access and rights to the same key, then why not just both keep a copy of it?
// In which case now
// it really IS only one Nym in reality, as well as in the software.
// But if one prefers to have his private key, and another his, then they will
// begin as separate and
// independent individuals. One Nym will not be found within the other as a
// int64_t-lost, under-developed
// twin!
// Just as reality enforces separate individuals, so does the software end up in
// the situation where
// two separate Nyms now wish to act with one as agent for the other. This wish
// is perfectly valid and
// can be accommodated, but logically the software cannot provide the same
// ONENESS of ACTUALLY SHARING
// THE PRIVATE KEY IN REAL LIFE (which allows the software to ACTUALLY only deal
// with a single key),
// as opposed to the oneness of "we have separate keys but we want to act in
// this way". There is oneness,
// and then there is oneness. OT is about contracts between Nyms, and therefore
// that is the mechanism
// for implementing ANY OTHER FORM OF AGENCY. Whether that agent is your "best
// man" or a board of voters,
// and whether the agent acts on behalf of you, or some corporation or
// democracy, either way, either his
// key is the same bits as your key, or they are separate keys in which case
// there is an agreement
// between them. Otherwise OT simply does not know which agents have
// authority--and which do not. This
// knowledge is necessary just for being able to function, and is imposed by the
// natural law.

// IDEA: Have a factory for smart contracts, such that not only are different
// subclasses instantiated,
// but more usefully, common configurations such as new democracy, new
// corporation, new board-with-veto, etc.
//

// For when the agent is an individual:
//

// If IsIndividual(), then this is his own personal NymID,
// (whether he DoesRepresentHimself() or DoesRepresentAnEntity() -- either way).
// Otherwise if IsGroup(), this returns false.
//
bool OTAgent::GetNymID(Identifier& theOutput) const
{
    if (IsAnIndividual()) {
        theOutput.SetString(m_strNymID);

        return true;
    }

    return false;
}

// IF IsIndividual() AND DoesRepresentAnEntity(), then this is his RoleID within
// that Entity. Otherwise, if IsGroup() or DoesRepresentHimself(), then this
// returns false.

bool OTAgent::GetRoleID(Identifier& theOutput) const
{
    if (IsAnIndividual() && DoesRepresentAnEntity()) {
        theOutput.SetString(m_strRoleID);

        return true;
    }

    return false;
}

// Notice if the agent is a voting group, then it has no signer. (Instead it
// will have an election.)
// That is why certain agents are unacceptable in certain scripts. They are
// PASSIVE.
//
// There is an "active" agent who has a signerID, but there is also a "passive"
// agent who only has
// a group name, and acts based on notifications and replies in the
// int64_t-term, versus being immediately
// able to act as part of the operation of a script.
//
// Basically if !IsIndividual(), then GetSignerID() will fail and thus anything
// needing that,
// as part of the script, would also therefore be impossible.
//
bool OTAgent::GetSignerID(Identifier& theOutput) const
{
    // If IsIndividual() and DoesRepresentAnEntity() then this returns
    // GetRoleID().
    // else if Individual() and DoesRepresentHimself() then this returns
    // GetNymID().
    // else (if IsGroup()) then return false;

    if (IsAnIndividual()) {
        if (DoesRepresentAnEntity()) {
            return GetRoleID(theOutput);
        } else  // DoesRepresentHimself()
        {
            return GetNymID(theOutput);
        }
    }

    // else IsGroup()... unable to sign directly; must hold votes instead.
    //
    return false;
}

bool OTAgent::IsValidSignerID(const Identifier& theNymID)
{
    Identifier theAgentNymID;
    bool bNymID = GetNymID(theAgentNymID);

    // If there's a NymID on this agent, and it matches theNymID...
    //
    if (bNymID && (theNymID == theAgentNymID)) return true;

    // TODO Entities...
    //
    return false;
}

// See if theNym is a valid signer for this agent.
//
bool OTAgent::IsValidSigner(Nym& theNym)
{
    Identifier theAgentNymID;
    bool bNymID = GetNymID(theAgentNymID);

    // If there's a NymID on this agent, and it matches theNym's ID...
    //
    if (bNymID && theNym.CompareID(theAgentNymID)) {
        // That means theNym *is* the Nym for this agent!
        // We'll save his pointer, for future reference...
        //
        SetNymPointer(theNym);

        return true;
    }

    // TODO Entity: Perhaps the original Nym was fired from his role... another
    // Nym has now
    // taken his place. In which case, the original Nym should be refused as a
    // valid
    // signer, and the new Nym should be allowed to sign in his place!
    //
    // This means if DoesRepresentAnEntity(), then I have to load the Role, and
    // verify
    // the Nym against that Role (which contains the updated status). Since I
    // haven't
    // coded Entities/Roles yet, then I don't have to do this just yet...
    // Might even update the NymID on this agent, for updated copies of the
    // agreement.
    // (Obviously the original can't be changed...)
    //

    return false;
}

// For when the agent DoesRepresentAnEntity():
//
// Whether this agent IsGroup() (meaning he is a voting group that
// DoesRepresentAnEntity()),
// OR whether this agent is an individual acting in a role for an entity
// (IsIndividual() && DoesRepresentAnEntity())
// ...EITHER WAY, the agent DoesRepresentAnEntity(), and this function returns
// the ID of that Entity.
//
// Otherwise, if the agent DoesRepresentHimself(), then this returns false.
// I'm debating making this function private along with DoesRepresentHimself /
// DoesRepresentAnEntity().
//
bool OTAgent::GetEntityID(Identifier& theOutput) const
{
    // IF represents an entity, then this is its ID. Else fail.
    //
    if (DoesRepresentAnEntity() && (nullptr != m_pForParty) &&
        m_pForParty->IsEntity()) {
        bool bSuccessEntityID = false;
        std::string str_entity_id = m_pForParty->GetEntityID(&bSuccessEntityID);

        if (bSuccessEntityID && (str_entity_id.size() > 0)) {
            String strEntityID(str_entity_id.c_str());
            theOutput.SetString(strEntityID);

            return true;
        }
    }

    return false;
}

// Returns true/false whether THIS agent is the authorizing agent for his party.
//
bool OTAgent::IsAuthorizingAgentForParty()
{
    if (nullptr == m_pForParty) return false;

    if (m_strName.Compare(m_pForParty->GetAuthorizingAgentName().c_str()))
        return true;

    return false;
}

// Returns the number of accounts, owned by this agent's party, that this agent
// is the authorized agent FOR.
//
int32_t OTAgent::GetCountAuthorizedAccts()
{
    if (nullptr == m_pForParty) {
        otErr << "OTAgent::CountAuthorizedAccts: Error: m_pForParty was "
                 "nullptr.\n";
        return 0;  // Maybe should log here...
    }

    return m_pForParty->GetAccountCount(m_strName.Get());
}

// For when the agent is a voting group:
// If !IsGroup() aka IsIndividual(), then this will return false.
//
bool OTAgent::GetGroupName(String& strGroupName)
{
    if (IsAGroup()) {
        strGroupName.Set(m_strGroupName);

        return true;
    }

    return false;
}

// PARTY is either a NYM or an ENTITY. This returns ID for that Nym or Entity.
//
bool OTAgent::GetPartyID(Identifier& theOutput) const
{
    if (DoesRepresentHimself()) return GetNymID(theOutput);

    return GetEntityID(theOutput);
}

void OTAgent::RetrieveNymPointer(mapOfNyms& map_Nyms_Already_Loaded)
{
    const std::string str_agent_name(m_strName.Get());

    //  We actually have a Nym pointer on this agent somehow (so let's add it to
    // the list.)
    //
    if (nullptr != m_pNym) {
        if (!m_strName.Exists())  // Whoaa!! Can't add it without the agent's
                                  // name for the map!
        {
            otErr << "OTAgent::RetrieveNymPointers: Failed: m_strName is "
                     "empty!\n";
        } else if (
            map_Nyms_Already_Loaded.end() ==
            map_Nyms_Already_Loaded.insert(
                map_Nyms_Already_Loaded.begin(),
                std::pair<std::string, Nym*>(str_agent_name, m_pNym)))
            otErr << "OTAgent::RetrieveNymPointer: Failed on insertion, as "
                     "though another nym were already "
                     "there with the same agent name! ("
                  << m_strName << ")\n";
        // (else it was inserted successfully.)
    }
    // else nothing, since it's normal that most of them are nullptr, even when
    // one
    // is goood.
}

bool OTAgent::VerifyAgencyOfAccount(const Account& theAccount) const
{
    Identifier theSignerID;

    if (!GetSignerID(theSignerID)) {
        otErr << "OTAgent::VerifyAgencyOfAccount: ERROR: Entities and roles "
                 "haven't been coded yet.\n";
        return false;
    }

    return theAccount.VerifyOwnerByID(theSignerID);  // todo when entities and
                                                     // roles come, won't this
    // "just work", or do I also
    // have to warn the acct
    // whether it's a Nym or a
    // Role being passed?
}

bool OTAgent::DropFinalReceiptToInbox(
    mapOfNyms* pNymMap,
    const String& strNotaryID,
    Nym& theServerNym,
    OTSmartContract& theSmartContract,
    const Identifier& theAccountID,
    const int64_t& lNewTransactionNumber,
    const int64_t& lClosingNumber,
    const String& strOrigCronItem,
    String* pstrNote,
    String* pstrAttachment)
{
    // TODO: When entites and ROLES are added, this function may change a bit to
    // accommodate them.
    const char* szFunc = "OTAgent::DropFinalReceiptToInbox";

    Identifier theAgentNymID;
    bool bNymID = GetNymID(theAgentNymID);

    // Not all agents have Nyms. (Might be a voting group.)
    // But in the case of Inboxes for asset accounts, shouldn't the agent be a
    // Nym?
    // Perhaps not... perhaps not... we shall see.

    if (true == bNymID)  // therefore IsAnIndividual() is definitely true.
    {
        Nym* pNym = nullptr;
        std::unique_ptr<Nym> theNymAngel;

        // If a list of pre-loaded Nyms was passed in, see if one of them is
        // ours.
        //
        if (nullptr != pNymMap) {
            const String strNymID(theAgentNymID);
            OT_ASSERT(strNymID.Exists());

            auto ittt = pNymMap->find(strNymID.Get());

            if (pNymMap->end() != ittt)  // found it!
            {
                pNym = ittt->second;
                OT_ASSERT(nullptr != pNym);
            }
        }

        if (nullptr ==
            pNym)  // It wasn't on the list of already-loaded nyms that
                   // was passed in, so we have to load it.
        {
            // By this point we also know that pNym is NOT the server Nym, nor
            // is it the
            // Originator, nor pActingNym, nor pPartyNym, as they were all
            // loaded already and
            // were added to pNymMap, yet we didn't find the Nym we were looking
            // for among them.
            //
            // (Therefore this is some new Nym, and doesn't need to be verified
            // against those Nyms again,
            // before loading it. Let's load it up!)
            //
            if (nullptr == (pNym = LoadNym(theServerNym)))
                otErr << szFunc << ": Failed loading Nym.\n";
            else
                theNymAngel.reset(pNym);  // CLEANUP  :-)
        }

        // I call this because LoadNym sets my internal Nym pointer to pNym, and
        // then
        // it goes out of scope before the end of this function and gets
        // cleaned-up.
        // Therefore, no point in letting this agent continue to point to bad
        // memory...
        //
        ClearTemporaryPointers();

        if ((nullptr != pNym) && (lClosingNumber > 0) &&
            pNym->VerifyIssuedNum(
                strNotaryID,
                lClosingNumber))  // <====================
        {
            return theSmartContract.DropFinalReceiptToInbox(
                theAgentNymID,
                theAccountID,
                lNewTransactionNumber,
                lClosingNumber,
                strOrigCronItem,
                theSmartContract.GetOriginType(),
                pstrNote,
                pstrAttachment);  // pActualAcct=nullptr here. (This call will
                                  // load
                                  // the acct up and update its inbox hash.)
        } else
            otErr << szFunc
                  << ": Error: pNym is nullptr, or lClosingNumber <=0, "
                     "or pNym->VerifyIssuedNum(strNotaryID, "
                     "lClosingNumber)) failed to verify.\n";
    } else
        otErr << szFunc << ": No NymID available for this agent...\n";

    return false;
}

bool OTAgent::DropFinalReceiptToNymbox(
    OTSmartContract& theSmartContract,
    const int64_t& lNewTransactionNumber,
    const String& strOrigCronItem,
    String* pstrNote,
    String* pstrAttachment,
    Nym* pActualNym)  // IF the Nym was already loaded, then I
                      // HAD to pass it here. But it may not be
                      // here. Also: It may not be the right
                      // Nym, so need to check before actually
                      // using for anything.
{
    Identifier theAgentNymID;
    bool bNymID = GetNymID(theAgentNymID);

    // Not all agents have Nyms. (Might be a voting group.)

    if (true == bNymID) {
        Nym* pToActualNym = nullptr;

        if ((nullptr != pActualNym) && pActualNym->CompareID(theAgentNymID))
            pToActualNym = pActualNym;

        return theSmartContract.DropFinalReceiptToNymbox(
            theAgentNymID,
            lNewTransactionNumber,
            strOrigCronItem,
            theSmartContract.GetOriginType(),
            pstrNote,
            pstrAttachment,
            pToActualNym);
    }

    // TODO: When entites and roles are added, this function may change a bit to
    // accommodate them.

    return false;
}

bool OTAgent::DropServerNoticeToNymbox(
    bool bSuccessMsg,  // Added this so we can notify smart contract parties
                       // when
                       // it FAILS to activate.
    Nym& theServerNym,
    const Identifier& theNotaryID,
    const int64_t& lNewTransactionNumber,
    const int64_t& lInReferenceTo,
    const String& strReference,
    String* pstrNote,
    String* pstrAttachment,
    Nym* pActualNym)
{
    Identifier theAgentNymID;
    bool bNymID = GetNymID(theAgentNymID);

    // Not all agents have Nyms. (Might be a voting group.)

    if (true == bNymID) {
        Nym* pToActualNym = nullptr;

        if ((nullptr != pActualNym) && pActualNym->CompareID(theAgentNymID))
            pToActualNym = pActualNym;
        else if ((nullptr != m_pNym) && m_pNym->CompareID(theAgentNymID))
            pToActualNym = m_pNym;

        return OTAgreement::DropServerNoticeToNymbox(
            bSuccessMsg,
            theServerNym,
            theNotaryID,
            theAgentNymID,
            lNewTransactionNumber,
            lInReferenceTo,
            strReference,
            originType::origin_smart_contract,
            pstrNote,
            pstrAttachment,
            pToActualNym);
    }

    // TODO: When entites and roles are added, this function may change a bit to
    // accommodate them.

    return false;
}

bool OTAgent::SignContract(Contract& theInput) const
{
    if (!IsAnIndividual() || !DoesRepresentHimself()) {
        otErr << "OTAgent::SignContract: Entities and roles are not yet "
                 "supported. Agent: "
              << m_strName << ".\n";
        return false;
    }  // todo: when adding entities, this will change.

    if (nullptr == m_pNym) {
        otErr << "OTAgent::SignContract: Nym was nullptr while trying to sign "
                 "contract. Agent: "
              << m_strName << ".\n";
        return false;
    }  // todo: when adding entities, this will change.

    return theInput.SignContract(*m_pNym);
}

bool OTAgent::VerifyIssuedNumber(
    const int64_t& lNumber,
    const String& strNotaryID)
{
    // Todo: this function may change when entities / roles are added.
    if (!IsAnIndividual() || !DoesRepresentHimself()) {
        otErr << "OTAgent::VerifyIssuedNumber:  Error: Entities and Roles are "
                 "not yet supported. Agent: "
              << m_strName << "\n";
        return false;
    }

    if (nullptr != m_pNym)
        return m_pNym->VerifyIssuedNum(strNotaryID, lNumber);
    else
        otErr << "OTAgent::VerifyIssuedNumber: Error: m_pNym was nullptr. For "
                 "agent: "
              << m_strName << "\n";

    return false;
}

bool OTAgent::VerifyTransactionNumber(
    const int64_t& lNumber,
    const String& strNotaryID)
{
    // Todo: this function may change when entities / roles are added.
    if (!IsAnIndividual() || !DoesRepresentHimself()) {
        otErr << "OTAgent::VerifyTransactionNumber:  Error: Entities and Roles "
                 "are not yet supported. Agent: "
              << m_strName << "\n";
        return false;
    }

    if (nullptr != m_pNym)
        return m_pNym->VerifyTransactionNum(strNotaryID, lNumber);
    else
        otErr << "OTAgent::VerifyTransactionNumber: Error: m_pNym was nullptr. "
                 "For agent: "
              << m_strName << "\n";

    return false;
}

// Done
// ASSUMES m_pNym is set already -- doesn't bother loading the nym!
//
bool OTAgent::HarvestTransactionNumber(
    const int64_t& lNumber,
    const String& strNotaryID,
    bool bSave,       // Each agent's nym is used if pSignerNym is nullptr,
                      // whereas the server
    Nym* pSignerNym)  // uses this optional arg to substitute
                      // serverNym as signer.
{

    // Todo: this function may change when entities / roles are added.
    //
    if (!IsAnIndividual() || !DoesRepresentHimself()) {
        otErr << __FUNCTION__
              << ":  Error: Entities and Roles are not yet supported. Agent: "
              << m_strName << "\n";
        return false;
    }

    if (nullptr != m_pNym) {
        const Identifier theNotaryID(strNotaryID);
        auto context = OT::App().Contract().mutable_Context(
            theNotaryID,
            m_pNym->ID());

        // If a signer wasn't passed in (the server-side uses server nym to
        // sign)
        // then we use the Nym himself as his own signer (common to
        // client-side.)
        //
        if (nullptr == pSignerNym) pSignerNym = m_pNym;

        // This won't "add it back" unless we're SURE he had it in the first
        // place...
        //
        const bool bSuccess = m_pNym->ClawbackTransactionNumber(
            theNotaryID, lNumber, bSave, pSignerNym);

        if (bSuccess) {
            // The transaction is being removed from play, so we will remove it
            // from this list. That is, when we called RemoveTransactionNumber,
            // the number was being put into play until RemoveIssuedNumber is
            // called to close it out. But now RemoveIssuedNumber won't ever be
            // called, since we are harvesting it back for future use. Therefore
            // the number is currently no longer in play, therefore we remove it
            // from the list of open cron numbers.
            context.It().CloseCronItem(lNumber);

            return true;
        } else
            otErr << __FUNCTION__ << ": Number (" << lNumber
                  << ") failed to verify for agent: " << m_strName
                  << " (Thus didn't bother 'adding it back'.)\n";
    } else
        otErr << __FUNCTION__
              << ": Error: m_pNym was nullptr. For agent: " << m_strName
              << "\n";

    return false;
}

// This means the transaction number has just been USED (and it now must stay
// open/outstanding until CLOSED.)
// Therefore we also add it to the set of open cron items, which the server
// keeps track of (for opening AND closing numbers.)
//
bool OTAgent::RemoveTransactionNumber(
    const int64_t& lNumber,
    const String& strNotaryID,
    Nym& SIGNER_NYM,
    bool bSave)
{
    // Todo: this function may change when entities / roles are added.
    if (!IsAnIndividual() || !DoesRepresentHimself()) {
        otErr << "OTAgent::" << __FUNCTION__
              << ":  Error: Entities and Roles are not yet supported. Agent: "
              << m_strName << "\n";
        return false;
    }

    if (nullptr != m_pNym) {
        auto context = OT::App().Contract().mutable_Context(
            Identifier(strNotaryID),
            m_pNym->ID());
        const bool bSuccess = m_pNym->RemoveTransactionNum(
            strNotaryID, lNumber);  // Doesn't save.

        if (bSuccess) {
            context.It().OpenCronItem(lNumber);

            if (bSave) { m_pNym->SaveSignedNymfile(SIGNER_NYM); }
        } else
            otErr << "OTAgent::" << __FUNCTION__
                  << ": Error, should never happen. (I'd assume you aren't "
                     "removing numbers without verifying first if they're "
                     "there.)\n";
        return bSuccess;
    } else
        otErr << "OTAgent::" << __FUNCTION__
              << ": Error: m_pNym was nullptr. For agent: " << m_strName
              << "\n";

    return false;
}

// This means the transaction number has just been CLOSED.
// Therefore we remove it from the set of open cron items, which the server
// keeps track of (for opening AND closing numbers.)
//
bool OTAgent::RemoveIssuedNumber(
    const int64_t& lNumber,
    const String& strNotaryID,
    bool bSave,
    Nym* pSignerNym)
{
    // Todo: this function may change when entities / roles are added.
    if (!IsAnIndividual() || !DoesRepresentHimself()) {
        otErr << "OTAgent::" << __FUNCTION__
              << ":  Error: Entities and Roles are not yet supported. Agent: "
              << m_strName << "\n";
        return false;
    }

    if (nullptr != m_pNym) {
        auto context = OT::App().Contract().mutable_Context(
            Identifier(strNotaryID),
            m_pNym->ID());

        const bool bSuccess =
            m_pNym->RemoveIssuedNum(strNotaryID, lNumber);  // Doesn't save.

        if (bSuccess) {
            if (nullptr == pSignerNym) { pSignerNym = m_pNym; }

            context.It().CloseCronItem(lNumber);

            if (bSave) { m_pNym->SaveSignedNymfile(*pSignerNym); }
        } else
            otErr << "OTAgent::" << __FUNCTION__
                  << ": Error, should never happen. (I'd assume you aren't "
                     "removing issued numbers without verifying first if "
                     "they're there.)\n";
        return bSuccess;
    } else
        otErr << "OTAgent::" << __FUNCTION__
              << ": Error: m_pNym was nullptr. For agent: " << m_strName
              << "\n";

    return false;
}

// Done
bool OTAgent::ReserveClosingTransNum(
    const String& strNotaryID,
    OTPartyAccount& thePartyAcct)
{
    if (IsAnIndividual() && DoesRepresentHimself() && (nullptr != m_pNym)) {
        int64_t lTransactionNumber = 0;
        if (thePartyAcct.GetClosingTransNo() > 0) {
            otOut << "OTAgent::ReserveClosingTransNum: Failure: The account "
                     "ALREADY has a closing transaction number "
                     "set on it. Don't you want to save that first, before "
                     "overwriting it?\n";
            return false;
        }

        if (m_pNym->GetTransactionNumCount(Identifier(strNotaryID)) <
            1)  // Need a closing number...
        {
            otOut << "OTAgent::ReserveClosingTransNum: *** Failure *** Nym "
                     "needs at least 1 transaction number available in order "
                     "to do this.\n";
            return false;
        } else if (
            false ==
            m_pNym->GetNextTransactionNum(
                *m_pNym, strNotaryID, lTransactionNumber)) {
            otErr << "OTAgent::ReserveClosingTransNum: Error: Strangely, "
                     "unable to get a transaction number, even though "
                     "supposedly one was there.\n";
            return false;
        }

        // BELOW THIS POINT, TRANSACTION # HAS BEEN RESERVED, AND MUST BE
        // SAVED...
        // Any errors below this point will require this call before returning:
        // HarvestAllTransactionNumbers(strNotaryID);
        //
        thePartyAcct.SetClosingTransNo(lTransactionNumber);
        thePartyAcct.SetAgentName(m_strName);

        return true;
    } else  // todo: when entities and roles are added... this function will
            // change.
    {
        otErr << "OTAgent::ReserveClosingTransNum: Either the Nym pointer "
                 "isn't set properly, "
                 "or you tried to use Entities when they haven't been coded "
                 "yet. Agent: "
              << m_strName << " \n";
    }

    return false;
}

// Done
bool OTAgent::ReserveOpeningTransNum(const String& strNotaryID)
{
    if (IsAnIndividual() && DoesRepresentHimself() && (nullptr != m_pNym)) {
        int64_t lTransactionNumber = 0;
        if (nullptr == m_pForParty) {
            otErr << "OTAgent::ReserveOpeningTransNum: Error: Party pointer "
                     "was nullptr.  SHOULD NEVER HAPPEN!!\n";
            return false;
        }
        if (m_pForParty->GetOpeningTransNo() > 0) {
            otOut << "OTAgent::ReserveOpeningTransNum: Failure: Party ALREADY "
                     "had an opening transaction number "
                     "set on it. Don't you want to save that first, before "
                     "overwriting it?\n";
            return false;
        }

        // Need opening number...
        if (m_pNym->GetTransactionNumCount(Identifier(strNotaryID)) < 1) {
            otOut << "OTAgent::ReserveOpeningTransNum: *** Failure *** Nym "
                     "needs at least 1 transaction number available in order "
                     "to do this.\n";
            return false;
        } else if (
            false ==
            m_pNym->GetNextTransactionNum(
                *m_pNym, strNotaryID, lTransactionNumber)) {
            otErr << "OTAgent::ReserveOpeningTransNum: Error: Strangely, "
                     "unable to get a transaction number, even though "
                     "supposedly one was there.\n";
            return false;
        }

        // BELOW THIS POINT, TRANSACTION # HAS BEEN RESERVED, AND MUST BE
        // SAVED...
        // Any errors below this point will require this call before returning:
        // HarvestAllTransactionNumbers(strNotaryID);
        //
        m_pForParty->SetOpeningTransNo(lTransactionNumber);
        m_pForParty->SetAuthorizingAgentName(m_strName.Get());

        return true;
    } else  // todo: when entities and roles are added... this function will
            // change.
    {
        otErr << "OTAgent::ReserveOpeningTransNum: Either the Nym pointer "
                 "isn't set properly, "
                 "or you tried to use Entities when they haven't been coded "
                 "yet. Agent: "
              << m_strName << " \n";
    }

    return false;
}

void OTAgent::Serialize(Tag& parent) const
{
    TagPtr pTag(new Tag("agent"));

    pTag->add_attribute("name", m_strName.Get());
    pTag->add_attribute(
        "doesAgentRepresentHimself", formatBool(m_bNymRepresentsSelf));
    pTag->add_attribute("isAgentAnIndividual", formatBool(m_bIsAnIndividual));
    pTag->add_attribute("nymID", m_strNymID.Get());
    pTag->add_attribute("roleID", m_strRoleID.Get());
    pTag->add_attribute("groupName", m_strGroupName.Get());

    parent.add_tag(pTag);
}

}  // namespace opentxs
