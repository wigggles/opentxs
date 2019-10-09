// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/opentxs.hpp"

#include <gtest/gtest.h>

namespace ot = opentxs;

namespace
{
bool init_{false};
const ot::proto::ContactItemType individual_{ot::proto::CITEMTYPE_INDIVIDUAL};
const ot::blockchain::Type btc_chain_{ot::blockchain::Type::Bitcoin};
const ot::blockchain::Type bch_chain_{ot::blockchain::Type::BitcoinCash};
const ot::blockchain::Type ltc_chain_{ot::blockchain::Type::Litecoin};
std::unique_ptr<ot::OTPasswordPrompt> reason_p_{nullptr};
std::unique_ptr<ot::OTNymID> invalid_nym_p_{nullptr};
std::unique_ptr<ot::OTNymID> nym_not_in_wallet_p_{nullptr};
std::unique_ptr<ot::OTNymID> alex_p_{nullptr};
std::unique_ptr<ot::OTNymID> bob_p_{nullptr};
std::unique_ptr<ot::OTNymID> chris_p_{nullptr};
std::unique_ptr<ot::OTNymID> daniel_p_{nullptr};
std::unique_ptr<ot::OTData> address_1_p_{nullptr};
std::unique_ptr<ot::OTIdentifier> empty_p_{nullptr};
std::unique_ptr<ot::OTIdentifier> contact_alex_p_{nullptr};
std::unique_ptr<ot::OTIdentifier> contact_bob_p_{nullptr};
std::unique_ptr<ot::OTIdentifier> contact_chris_p_{nullptr};
std::unique_ptr<ot::OTIdentifier> contact_daniel_p_{nullptr};
std::unique_ptr<ot::OTIdentifier> account_1_id_p_{nullptr};
std::unique_ptr<ot::OTIdentifier> account_2_id_p_{nullptr};
std::unique_ptr<ot::OTIdentifier> account_3_id_p_{nullptr};
std::unique_ptr<ot::OTIdentifier> account_4_id_p_{nullptr};
std::unique_ptr<ot::OTIdentifier> account_5_id_p_{nullptr};
std::unique_ptr<ot::OTIdentifier> account_6_id_p_{nullptr};
std::unique_ptr<ot::OTIdentifier> account_7_id_p_{nullptr};
const std::string fingerprint_a_{};
const std::string fingerprint_b_{};
const std::string fingerprint_c_{};
const std::string dummy_script_{"00000000000000000000000000000000"};
const std::string txid_0_{"00000000000000000000000000000000"};
const std::string txid_1_{"11111111111111111111111111111111"};
const std::string txid_2_{"22222222222222222222222222222222"};
const std::string txid_3_{"33333333333333333333333333333333"};
const std::string txid_4_{"44444444444444444444444444444444"};
const std::string memo_1_{"memo 1"};
const std::string memo_2_{"memo 2"};
const std::string memo_3_{"memo 3"};
const std::string memo_4_{"memo 4"};
const std::string empty_string_{};
const std::vector<std::string> alex_external_{
    "1K9teXNg8iKYwUPregT8QTmMepb376oTuX",
    "1GgpoMuPBfaa4ZT6ZeKaTY8NH9Ldx4Q89t",
    "1FXb97adaza32zYQ5U29nxHZS4FmiCfXAJ",
    "1Dx4k7daUS1VNNeoDtZe1ujpt99YeW7Yz",
    "19KhniSVj1CovZWg1P5JvoM199nQR3gkhp",
    "1CBnxZdo58Vu3upwEt96uTMZLAxVx4Xeg9",
    "12vm2SqQ7RhhYPi6bJqqQzyJomV6H3j4AX",
    "1D2fNJYjyWL1jn5qRhJZL6EbGzeyBjHuP3",
    "19w4gVEse89JjE7TroavXZ9pyfJ78h4arG",
    "1DVYvYAmTNtvML7vBrhBBhyePaEDVCCNaw",
};
const std::vector<std::string> alex_internal_{
    "179XLYWcaHiPMnPUsSdrPiAwNcybx2vpaa",
    "1FPoX1BUe9a6ugobnQkzFyn1Uycyns4Ejp",
    "17jfyBx8ZHJ3DT9G2WehYEPKwT7Zv3kcLs",
    "15zErgibP264JkEMqihXQDp4Kb7vpvDpd5",
    "1KvRA5nngc4aA8y57A6TuS83Gud4xR5oPK",
    "14wC1Ph9z6S82QJA6yTaDaSZQjng9kDihT",
    "1FjW1pENbM6g5PAUpCdjQQykBYH6bzs5hU",
    "1Bt6BP3bXfRJbKUEFS15BrWa6Hca8G9W1L",
    "197TU7ptMMnhufMLFrY1o2Sgi5zcw2e3qv",
    "176aRLv3W94vyWPZDPY9csUrLNrqDFrzCs",
};
const std::vector<std::string> bob_external_{
    "1AngXb5xQoQ4nT8Bn6dDdr6AFS4yMZU2y",
    "1FQMy3HkD5C3gGZZHeeH9rjHgyqurxC44q",
    "1APXZ5bCTbj2ZRV3ZHyAa59CmsXRP4HkTh",
    "1M966pvtChYbceTsou73eB2hutwoZ7QtVv",
    "1HcN6BWFZKLNEdBo15oUPQGXpDJ26SVKQE",
    "1NcaLRLFr4edY4hUcR81aNMpveHaRqzxPR",
    "1CT86ZmqRFZW57aztRscjWuzkhJjgHjiMS",
    "1CXT6sU5s4mxP4UattFA6fGN7yW4dkkARn",
    "12hwhKpxTyfiSGDdQw63SWVzefRuRxrFqb",
    "18SRAzD6bZ2GsTK4J4RohhYneEyZAUvyqp",
};
const std::vector<std::string> bob_internal_{
    "1GXj4LrpYKugu4ps7BvYHkUgJLErjBcZc",
    "18yFFsUUe7ATjku2NfKizdnNfZGx99LmLJ",
    "19hDov3sMJdXkgrinhfD2seaKhcb6FiDKL",
    "1W9fEcakg5ZshPuAt5j2vTYkV6txNoiwq",
    "1EPTv3qdCJTbgqUZw83nUbjoKBmy4sHbhd",
    "17mcj9bmcuBfSZqc2mQnjLiT1mtPxGD1yu",
    "1LT2ZEnj1kmpgDbBQodiXVrAj6nRBmWUcH",
    "1HZmwsMWU87WFJxYDNQbnCW52KqUoLiCqZ",
    "16SdtUXrRey55j49Ae84YwVVNZXwGL2tLU",
    "1N2Y3mM828N4JQGLzDfxNjU2WK9CMMekVg",
};
const std::vector<std::string> chris_btc_external_{
    "1MWZN5PtYjfHA7WC1czB43HK9NjTKig1rA",
    "16Ach28pUQbWDpVhe75AjwoCJws144Nd25",
};
const std::vector<std::string> chris_btc_internal_{
    "1PsjtCRUQ32t5F18W2K8Zzpn1aVmuRmTdB",
    "15xi7Z3kVPg88ZYA82V8zPyodnQnamSZvN",
};
const std::vector<std::string> chris_bch_external_{
    "14Et9A6QnwpnUH2Ym9kZ4Zz1FN2GixG9qS",
    "17u11yKTfr13Xkm4k7h4bx3o3ssz4HSwGJ",
};
const std::vector<std::string> chris_bch_internal_{
    "1FkAAgJWW1YWSqa5ByvHFe8dQvfNLT2rQN",
    "1HyweNdaw2QoRU1YfuJQWcZKUAVqMXyJsj",
};
const std::vector<std::string> chris_ltc_external_{
    "LWDn8duKKwbP9hhCWpmX9o8BxywgCSTg41",
    "LSyrWGpCUm457F9TaXWAhvZs7Vu5g7a4Do",
};
const std::vector<std::string> chris_ltc_internal_{
    "LX3FAVopX2moW5h2ZwAKcrCKTChTyWqWze",
    "LMoZuWNnoTEJ1FjxQ4NXTcNbMK3croGpaF",
};

class Test_BlockchainAPI : public ::testing::Test
{
public:
    using AddressStyle = ot::api::client::blockchain::AddressStyle;
    using Subchain = ot::api::client::blockchain::Subchain;
    using ThreadData = std::tuple<std::string, std::uint64_t, std::string>;
    using ThreadVectors = std::map<int, std::vector<ThreadData>>;

    const ot::api::client::Manager& api_;
    ot::PasswordPrompt& reason_;
    const ot::identifier::Nym& invalid_nym_;
    const ot::identifier::Nym& nym_not_in_wallet_;
    const ot::identifier::Nym& alex_;
    const ot::identifier::Nym& bob_;
    const ot::identifier::Nym& chris_;
    const ot::identifier::Nym& daniel_;
    const ot::Data& address_1_;
    const ot::Identifier& empty_id_;
    const ot::Identifier& contact_alex_;
    const ot::Identifier& contact_bob_;
    const ot::Identifier& contact_chris_;
    const ot::Identifier& contact_daniel_;
    ot::Identifier& account_1_id_;  // alex, btc, bip32 *
    ot::Identifier& account_2_id_;  // daniel, btc, bip32
    ot::Identifier& account_3_id_;  // chris, btc, bip32
    ot::Identifier& account_4_id_;  // chris, btc, bip44
    ot::Identifier& account_5_id_;  // chris, bch, bip44 *
    ot::Identifier& account_6_id_;  // bob, btc, bip32 *
    ot::Identifier& account_7_id_;  // chris, ltc, bip44
    const ThreadVectors threads_;

    static const ot::api::client::Manager& init()
    {
        const auto& api = ot::Context().StartClient({}, 0);

        if (false == init_) {
            reason_p_.reset(new ot::OTPasswordPrompt{
                api.Factory().PasswordPrompt(__FUNCTION__)});
            invalid_nym_p_.reset(new ot::OTNymID{api.Factory().NymID("junk")});
            nym_not_in_wallet_p_.reset(new ot::OTNymID{
                api.Factory().NymID("ottaRUsttUuJZj738f9AE6kJJMBp6iedFYQ")});
            const_cast<std::string&>(fingerprint_a_) =
                api.Exec().Wallet_ImportSeed(
                    "response seminar brave tip suit recall often sound stick "
                    "owner lottery motion",
                    "");
            const_cast<std::string&>(fingerprint_b_) =
                api.Exec().Wallet_ImportSeed(
                    "reward upper indicate eight swift arch injury crystal "
                    "super wrestle already dentist",
                    "");
            const_cast<std::string&>(fingerprint_c_) =
                api.Exec().Wallet_ImportSeed(
                    "predict cinnamon gauge spoon media food nurse improve "
                    "employ similar own kid genius seed ghost",
                    "");
            alex_p_.reset(new ot::OTNymID{
                api.Wallet()
                    .Nym(*reason_p_, "Alex", {fingerprint_a_, 0}, individual_)
                    ->ID()});
            bob_p_.reset(new ot::OTNymID{
                api.Wallet()
                    .Nym(*reason_p_, "Bob", {fingerprint_b_, 0}, individual_)
                    ->ID()});
            chris_p_.reset(new ot::OTNymID{
                api.Wallet()
                    .Nym(*reason_p_, "Chris", {fingerprint_c_, 0}, individual_)
                    ->ID()});
            daniel_p_.reset(new ot::OTNymID{
                api.Wallet()
                    .Nym(*reason_p_, "Daniel", {fingerprint_a_, 1}, individual_)
                    ->ID()});

            address_1_p_.reset(new ot::OTData{api.Factory().Data(
                "0xf54a5851e9372b87810a8e60cdd2e7cfd80b6e31",
                ot::StringStyle::Hex)});
            empty_p_.reset(new ot::OTIdentifier{api.Factory().Identifier()});
            contact_alex_p_.reset(
                new ot::OTIdentifier{api.Contacts().ContactID(*alex_p_)});
            contact_bob_p_.reset(
                new ot::OTIdentifier{api.Contacts().ContactID(*bob_p_)});
            contact_chris_p_.reset(
                new ot::OTIdentifier{api.Contacts().ContactID(*chris_p_)});
            contact_daniel_p_.reset(
                new ot::OTIdentifier{api.Contacts().ContactID(*daniel_p_)});
            account_1_id_p_.reset(
                new ot::OTIdentifier{api.Factory().Identifier()});
            account_2_id_p_.reset(
                new ot::OTIdentifier{api.Factory().Identifier()});
            account_3_id_p_.reset(
                new ot::OTIdentifier{api.Factory().Identifier()});
            account_4_id_p_.reset(
                new ot::OTIdentifier{api.Factory().Identifier()});
            account_5_id_p_.reset(
                new ot::OTIdentifier{api.Factory().Identifier()});
            account_6_id_p_.reset(
                new ot::OTIdentifier{api.Factory().Identifier()});
            account_7_id_p_.reset(
                new ot::OTIdentifier{api.Factory().Identifier()});
            init_ = true;
        }

        return api;
    }

    bool check_thread(
        const ot::identifier::Nym& nym,
        const ot::Identifier& contact,
        const int index)
    {
        const auto pThread = api_.Activity().Thread(nym, contact);

        EXPECT_TRUE(pThread);

        if (false == bool(pThread)) { return false; }

        const auto& thread = *pThread;

        try {
            const auto& vector = threads_.at(index);
            const auto expectedCount = vector.size();

            EXPECT_EQ(expectedCount, thread.item().size());

            if (expectedCount >
                static_cast<std::size_t>(thread.item().size())) {
                return false;
            }

            auto i = int{0};

            for (const auto& [exID, exIndex, exAccount] : vector) {
                const auto& item = thread.item(i);

                EXPECT_EQ(item.id(), exID);
                EXPECT_EQ(item.index(), exIndex);
                EXPECT_EQ(
                    item.box(),
                    static_cast<std::uint32_t>(ot::StorageBox::BLOCKCHAIN));
                EXPECT_EQ(item.account(), exAccount);

                ++i;
            }
        } catch (...) {
            EXPECT_TRUE(false);

            return false;
        }

        return true;
    }

    bool check_transaction_count(const std::size_t count)
    {
        auto transactions = api_.Storage().BlockchainTransactionList();

        EXPECT_EQ(count, transactions.size());

        if (count > transactions.size()) { return false; }

        auto it = transactions.cbegin();

        if (1 > count) { return true; }

        EXPECT_EQ(txid_1_, it++->first);

        if (2 > count) { return true; }

        EXPECT_EQ(txid_2_, it++->first);

        if (3 > count) { return true; }

        EXPECT_EQ(txid_3_, it++->first);

        return true;
    }

    bool empty_thread(
        const ot::identifier::Nym& nym,
        const ot::Identifier& contact)
    {
        const auto pThread = api_.Activity().Thread(nym, contact);

        EXPECT_TRUE(pThread);

        if (false == bool(pThread)) { return false; }

        const auto& thread = *pThread;

        EXPECT_EQ(0, thread.item().size());

        return true;
    }

    bool null_thread(
        const ot::identifier::Nym& nym,
        const ot::Identifier& contact)
    {
        const auto pThread = api_.Activity().Thread(nym, contact);

        EXPECT_FALSE(pThread);

        return true;
    }

    bool transaction_state_0()
    {
        EXPECT_TRUE(check_transaction_count(0));

        EXPECT_TRUE(null_thread(alex_, contact_alex_));
        EXPECT_TRUE(null_thread(alex_, contact_bob_));
        EXPECT_TRUE(null_thread(alex_, contact_chris_));
        EXPECT_TRUE(null_thread(alex_, contact_daniel_));

        EXPECT_TRUE(null_thread(bob_, contact_alex_));
        EXPECT_TRUE(null_thread(bob_, contact_bob_));
        EXPECT_TRUE(null_thread(bob_, contact_chris_));
        EXPECT_TRUE(null_thread(bob_, contact_daniel_));

        EXPECT_TRUE(null_thread(chris_, contact_alex_));
        EXPECT_TRUE(null_thread(chris_, contact_bob_));
        EXPECT_TRUE(null_thread(chris_, contact_chris_));
        EXPECT_TRUE(null_thread(chris_, contact_daniel_));

        EXPECT_TRUE(null_thread(daniel_, contact_alex_));
        EXPECT_TRUE(null_thread(daniel_, contact_bob_));
        EXPECT_TRUE(null_thread(daniel_, contact_chris_));
        EXPECT_TRUE(null_thread(daniel_, contact_daniel_));

        return true;
    }

    bool transaction_state_1()
    {
        EXPECT_TRUE(check_transaction_count(1));

        EXPECT_TRUE(null_thread(alex_, contact_alex_));
        EXPECT_TRUE(null_thread(alex_, contact_bob_));
        EXPECT_TRUE(null_thread(alex_, contact_chris_));
        EXPECT_TRUE(null_thread(alex_, contact_daniel_));

        EXPECT_TRUE(null_thread(bob_, contact_alex_));
        EXPECT_TRUE(null_thread(bob_, contact_bob_));
        EXPECT_TRUE(null_thread(bob_, contact_chris_));
        EXPECT_TRUE(null_thread(bob_, contact_daniel_));

        EXPECT_TRUE(null_thread(chris_, contact_alex_));
        EXPECT_TRUE(null_thread(chris_, contact_bob_));
        EXPECT_TRUE(null_thread(chris_, contact_chris_));
        EXPECT_TRUE(null_thread(chris_, contact_daniel_));

        EXPECT_TRUE(null_thread(daniel_, contact_alex_));
        EXPECT_TRUE(null_thread(daniel_, contact_bob_));
        EXPECT_TRUE(null_thread(daniel_, contact_chris_));
        EXPECT_TRUE(null_thread(daniel_, contact_daniel_));

        return true;
    }

    bool transaction_state_2()
    {
        EXPECT_TRUE(check_transaction_count(3));

        EXPECT_TRUE(null_thread(alex_, contact_alex_));
        EXPECT_TRUE(check_thread(alex_, contact_bob_, 0));
        EXPECT_TRUE(null_thread(alex_, contact_chris_));
        EXPECT_TRUE(null_thread(alex_, contact_daniel_));

        EXPECT_TRUE(check_thread(bob_, contact_alex_, 1));
        EXPECT_TRUE(null_thread(bob_, contact_bob_));
        EXPECT_TRUE(null_thread(bob_, contact_chris_));
        EXPECT_TRUE(null_thread(bob_, contact_daniel_));

        EXPECT_TRUE(null_thread(chris_, contact_alex_));
        EXPECT_TRUE(null_thread(chris_, contact_bob_));
        EXPECT_TRUE(null_thread(chris_, contact_chris_));
        EXPECT_TRUE(null_thread(chris_, contact_daniel_));

        EXPECT_TRUE(null_thread(daniel_, contact_alex_));
        EXPECT_TRUE(null_thread(daniel_, contact_bob_));
        EXPECT_TRUE(null_thread(daniel_, contact_chris_));
        EXPECT_TRUE(null_thread(daniel_, contact_daniel_));

        return true;
    }

    bool transaction_state_3()
    {
        EXPECT_TRUE(check_transaction_count(3));

        EXPECT_TRUE(null_thread(alex_, contact_alex_));
        EXPECT_TRUE(check_thread(alex_, contact_bob_, 0));
        EXPECT_TRUE(null_thread(alex_, contact_chris_));
        EXPECT_TRUE(null_thread(alex_, contact_daniel_));

        EXPECT_TRUE(check_thread(bob_, contact_alex_, 1));
        EXPECT_TRUE(null_thread(bob_, contact_bob_));
        EXPECT_TRUE(null_thread(bob_, contact_chris_));
        EXPECT_TRUE(null_thread(bob_, contact_daniel_));

        EXPECT_TRUE(null_thread(chris_, contact_alex_));
        EXPECT_TRUE(check_thread(chris_, contact_bob_, 2));
        EXPECT_TRUE(null_thread(chris_, contact_chris_));
        EXPECT_TRUE(null_thread(chris_, contact_daniel_));

        EXPECT_TRUE(null_thread(daniel_, contact_alex_));
        EXPECT_TRUE(null_thread(daniel_, contact_bob_));
        EXPECT_TRUE(null_thread(daniel_, contact_chris_));
        EXPECT_TRUE(null_thread(daniel_, contact_daniel_));

        return true;
    }

    bool transaction_state_4()
    {
        EXPECT_TRUE(check_transaction_count(3));

        EXPECT_TRUE(null_thread(alex_, contact_alex_));
        EXPECT_TRUE(check_thread(alex_, contact_bob_, 0));
        EXPECT_TRUE(null_thread(alex_, contact_chris_));
        EXPECT_TRUE(null_thread(alex_, contact_daniel_));

        EXPECT_TRUE(empty_thread(bob_, contact_alex_));
        EXPECT_TRUE(null_thread(bob_, contact_bob_));
        EXPECT_TRUE(check_thread(bob_, contact_chris_, 3));
        EXPECT_TRUE(null_thread(bob_, contact_daniel_));

        EXPECT_TRUE(null_thread(chris_, contact_alex_));
        EXPECT_TRUE(check_thread(chris_, contact_bob_, 2));
        EXPECT_TRUE(null_thread(chris_, contact_chris_));
        EXPECT_TRUE(null_thread(chris_, contact_daniel_));

        EXPECT_TRUE(null_thread(daniel_, contact_alex_));
        EXPECT_TRUE(null_thread(daniel_, contact_bob_));
        EXPECT_TRUE(null_thread(daniel_, contact_chris_));
        EXPECT_TRUE(null_thread(daniel_, contact_daniel_));

        return true;
    }

    bool transaction_state_5()
    {
        EXPECT_TRUE(check_transaction_count(3));

        EXPECT_TRUE(null_thread(alex_, contact_alex_));
        EXPECT_TRUE(empty_thread(alex_, contact_bob_));
        EXPECT_TRUE(null_thread(alex_, contact_chris_));
        EXPECT_TRUE(null_thread(alex_, contact_daniel_));

        EXPECT_TRUE(empty_thread(bob_, contact_alex_));
        EXPECT_TRUE(null_thread(bob_, contact_bob_));
        EXPECT_TRUE(check_thread(bob_, contact_chris_, 3));
        EXPECT_TRUE(null_thread(bob_, contact_daniel_));

        EXPECT_TRUE(null_thread(chris_, contact_alex_));
        EXPECT_TRUE(check_thread(chris_, contact_bob_, 2));
        EXPECT_TRUE(null_thread(chris_, contact_chris_));
        EXPECT_TRUE(null_thread(chris_, contact_daniel_));

        EXPECT_TRUE(null_thread(daniel_, contact_alex_));
        EXPECT_TRUE(null_thread(daniel_, contact_bob_));
        EXPECT_TRUE(null_thread(daniel_, contact_chris_));
        EXPECT_TRUE(null_thread(daniel_, contact_daniel_));

        return true;
    }

    bool transaction_state_6()
    {
        EXPECT_TRUE(check_transaction_count(4));

        EXPECT_TRUE(null_thread(alex_, contact_alex_));
        EXPECT_TRUE(empty_thread(alex_, contact_bob_));
        EXPECT_TRUE(null_thread(alex_, contact_chris_));
        EXPECT_TRUE(null_thread(alex_, contact_daniel_));

        EXPECT_TRUE(check_thread(bob_, contact_alex_, 4));
        EXPECT_TRUE(null_thread(bob_, contact_bob_));
        EXPECT_TRUE(check_thread(bob_, contact_chris_, 3));
        EXPECT_TRUE(check_thread(bob_, contact_daniel_, 5));

        EXPECT_TRUE(null_thread(chris_, contact_alex_));
        EXPECT_TRUE(check_thread(chris_, contact_bob_, 2));
        EXPECT_TRUE(null_thread(chris_, contact_chris_));
        EXPECT_TRUE(null_thread(chris_, contact_daniel_));

        EXPECT_TRUE(null_thread(daniel_, contact_alex_));
        EXPECT_TRUE(null_thread(daniel_, contact_bob_));
        EXPECT_TRUE(null_thread(daniel_, contact_chris_));
        EXPECT_TRUE(null_thread(daniel_, contact_daniel_));

        return true;
    }

    bool transaction_state_7()
    {
        EXPECT_TRUE(check_transaction_count(4));

        EXPECT_TRUE(null_thread(alex_, contact_alex_));
        EXPECT_TRUE(empty_thread(alex_, contact_bob_));
        EXPECT_TRUE(null_thread(alex_, contact_chris_));
        EXPECT_TRUE(null_thread(alex_, contact_daniel_));

        EXPECT_TRUE(check_thread(bob_, contact_alex_, 4));
        EXPECT_TRUE(null_thread(bob_, contact_bob_));
        EXPECT_TRUE(check_thread(bob_, contact_chris_, 6));
        EXPECT_TRUE(check_thread(bob_, contact_daniel_, 5));

        EXPECT_TRUE(null_thread(chris_, contact_alex_));
        EXPECT_TRUE(check_thread(chris_, contact_bob_, 2));
        EXPECT_TRUE(null_thread(chris_, contact_chris_));
        EXPECT_TRUE(null_thread(chris_, contact_daniel_));

        EXPECT_TRUE(null_thread(daniel_, contact_alex_));
        EXPECT_TRUE(null_thread(daniel_, contact_bob_));
        EXPECT_TRUE(null_thread(daniel_, contact_chris_));
        EXPECT_TRUE(null_thread(daniel_, contact_daniel_));

        return true;
    }

    Test_BlockchainAPI()
        : api_(init())
        , reason_(reason_p_->get())
        , invalid_nym_(invalid_nym_p_->get())
        , nym_not_in_wallet_(nym_not_in_wallet_p_->get())
        , alex_(alex_p_->get())
        , bob_(bob_p_->get())
        , chris_(chris_p_->get())
        , daniel_(daniel_p_->get())
        , address_1_(address_1_p_->get())
        , empty_id_(empty_p_->get())
        , contact_alex_(contact_alex_p_->get())
        , contact_bob_(contact_bob_p_->get())
        , contact_chris_(contact_chris_p_->get())
        , contact_daniel_(contact_daniel_p_->get())
        , account_1_id_(account_1_id_p_->get())
        , account_2_id_(account_2_id_p_->get())
        , account_3_id_(account_3_id_p_->get())
        , account_4_id_(account_4_id_p_->get())
        , account_5_id_(account_5_id_p_->get())
        , account_6_id_(account_6_id_p_->get())
        , account_7_id_(account_7_id_p_->get())
        , threads_({
              {0,
               {
                   {txid_3_, 0, ""},
               }},
              {1,
               {
                   {txid_2_, 0, ""},
               }},
              {2,
               {
                   {txid_1_, 0, ""},
               }},
              {3,
               {
                   {txid_2_, 0, ""},
               }},
              {4,
               {
                   {txid_4_, 1, ""},
               }},
              {5,
               {
                   {txid_4_, 0, ""},
               }},
              {6,
               {
                   {txid_2_, 0, ""},
                   {txid_4_, 1, ""},
               }},
          })
    {
    }
};

TEST_F(Test_BlockchainAPI, init)
{
    EXPECT_TRUE(invalid_nym_.empty());
    EXPECT_FALSE(nym_not_in_wallet_.empty());
    EXPECT_FALSE(alex_.empty());
    EXPECT_FALSE(bob_.empty());
    EXPECT_FALSE(chris_.empty());
    EXPECT_FALSE(daniel_.empty());
    EXPECT_TRUE(empty_id_.empty());
    EXPECT_FALSE(contact_alex_.empty());
    EXPECT_FALSE(contact_bob_.empty());
    EXPECT_FALSE(contact_chris_.empty());
    EXPECT_FALSE(contact_daniel_.empty());
    EXPECT_TRUE(account_1_id_.empty());
    EXPECT_TRUE(account_2_id_.empty());
    EXPECT_TRUE(account_3_id_.empty());
    EXPECT_TRUE(account_4_id_.empty());
    EXPECT_TRUE(account_5_id_.empty());
    EXPECT_TRUE(account_6_id_.empty());
    EXPECT_TRUE(account_7_id_.empty());
    EXPECT_FALSE(fingerprint_a_.empty());
    EXPECT_FALSE(fingerprint_b_.empty());
    EXPECT_FALSE(fingerprint_c_.empty());
}

TEST_F(Test_BlockchainAPI, invalid_nym)
{
    bool loaded(false);

    try {
        api_.Blockchain().Account(invalid_nym_, btc_chain_);
        loaded = true;
    } catch (...) {
    }

    EXPECT_FALSE(loaded);

    auto accountID = api_.Blockchain().NewHDSubaccount(
        invalid_nym_, ot::BlockchainAccountType::BIP44, btc_chain_, reason_);

    EXPECT_TRUE(accountID->empty());

    auto list = api_.Blockchain().AccountList(invalid_nym_, btc_chain_);

    EXPECT_EQ(0, list.size());
    EXPECT_EQ(0, list.count(accountID));

    loaded = false;

    try {
        api_.Blockchain().Account(nym_not_in_wallet_, btc_chain_);
        loaded = true;
    } catch (...) {
    }

    EXPECT_FALSE(loaded);

    accountID = api_.Blockchain().NewHDSubaccount(
        nym_not_in_wallet_,
        ot::BlockchainAccountType::BIP44,
        btc_chain_,
        reason_);

    EXPECT_TRUE(accountID->empty());

    list = api_.Blockchain().AccountList(nym_not_in_wallet_, btc_chain_);

    EXPECT_EQ(0, list.size());
    EXPECT_EQ(0, list.count(accountID));
}

// Test: when you create a nym with seed A, then the root of every HDPath for a
// blockchain account associated with that nym should also be A.
TEST_F(Test_BlockchainAPI, TestSeedRoot)
{
    account_1_id_.Assign(api_.Blockchain().NewHDSubaccount(
        alex_, ot::BlockchainAccountType::BIP32, btc_chain_, reason_));
    account_2_id_.Assign(api_.Blockchain().NewHDSubaccount(
        daniel_, ot::BlockchainAccountType::BIP32, btc_chain_, reason_));

    EXPECT_FALSE(account_1_id_.empty());
    EXPECT_FALSE(account_2_id_.empty());

    auto list = api_.Blockchain().AccountList(alex_, btc_chain_);

    EXPECT_EQ(1, list.size());
    EXPECT_EQ(1, list.count(account_1_id_));

    list = api_.Blockchain().AccountList(daniel_, btc_chain_);

    EXPECT_EQ(1, list.size());
    EXPECT_EQ(1, list.count(account_2_id_));

    // Test difference in index on BIP32 implies a different account
    EXPECT_NE(account_1_id_, account_2_id_);

    try {
        const auto& account1 =
            api_.Blockchain().HDSubaccount(alex_, account_1_id_);
        const auto& account2 =
            api_.Blockchain().HDSubaccount(daniel_, account_2_id_);

        EXPECT_EQ(account1.Path().root(), fingerprint_a_);
        EXPECT_EQ(account2.Path().root(), fingerprint_a_);

    } catch (...) {
        EXPECT_TRUE(false);
    }
}

// Test that one onym creates the same account for the same chain (BIP32 or
// BIP44).
TEST_F(Test_BlockchainAPI, TestNym_AccountIdempotence)
{
    account_3_id_.Assign(api_.Blockchain().NewHDSubaccount(
        chris_, ot::BlockchainAccountType::BIP32, btc_chain_, reason_));
    account_4_id_.Assign(api_.Blockchain().NewHDSubaccount(
        chris_, ot::BlockchainAccountType::BIP44, btc_chain_, reason_));

    EXPECT_FALSE(account_3_id_.empty());
    EXPECT_FALSE(account_4_id_.empty());
    EXPECT_NE(account_3_id_, account_4_id_);

    const auto& before =
        api_.Blockchain().Account(chris_, btc_chain_).GetHD().at(account_4_id_);

    EXPECT_EQ(before.ID(), account_4_id_);

    const auto duplicate = api_.Blockchain().NewHDSubaccount(
        chris_, ot::BlockchainAccountType::BIP44, btc_chain_, reason_);

    EXPECT_EQ(account_4_id_, duplicate);

    const auto& after =
        api_.Blockchain().Account(chris_, btc_chain_).GetHD().at(account_4_id_);

    EXPECT_EQ(after.ID(), account_4_id_);

    auto list = api_.Blockchain().AccountList(chris_, btc_chain_);

    EXPECT_EQ(2, list.size());
    EXPECT_EQ(1, list.count(account_3_id_));
    EXPECT_EQ(1, list.count(account_4_id_));
}

// Test that the same nym creates different accounts for two chains
TEST_F(Test_BlockchainAPI, TestChainDiff)
{
    account_5_id_.Assign(api_.Blockchain().NewHDSubaccount(
        chris_, ot::BlockchainAccountType::BIP44, bch_chain_, reason_));

    EXPECT_NE(account_5_id_, account_4_id_);

    auto list = api_.Blockchain().AccountList(chris_, bch_chain_);

    EXPECT_EQ(1, list.size());
    EXPECT_EQ(1, list.count(account_5_id_));
}

// https://github.com/bitcoin/bips/blob/master/bip-0032.mediawiki#test-vector-1
TEST_F(Test_BlockchainAPI, TestBip32_standard_1)
{
    const std::string empty{};
    auto bytes = ot::Data::Factory(
        "0x000102030405060708090a0b0c0d0e0f", ot::Data::Mode::Hex);
    ot::OTPassword seed{};
    seed.setMemory(bytes->data(), bytes->size());
    const auto fingerprint = api_.Seeds().ImportRaw(seed, reason_);

    ASSERT_FALSE(fingerprint.empty());

    const auto& nymID =
        api_.Wallet()
            .Nym(reason_, "John Doe", {fingerprint, 0}, individual_)
            ->ID();

    ASSERT_FALSE(nymID.empty());

    const auto accountID = api_.Blockchain().NewHDSubaccount(
        nymID, ot::BlockchainAccountType::BIP32, btc_chain_, reason_);

    ASSERT_FALSE(accountID->empty());

    const auto& account =
        api_.Blockchain().Account(nymID, btc_chain_).GetHD().at(0);

    EXPECT_EQ(account.ID(), accountID);

    const auto pRoot = account.RootNode(reason_);

    ASSERT_TRUE(pRoot);

    const auto& root = *pRoot;
    const std::string xpub{
        "xpub68Gmy5EdvgibQVfPdqkBBCHxA5htiqg55crXYuXoQRKfDBFA1WEjWgP6LHhwBZeNK1"
        "VTsfTFUHCdrfp1bgwQ9xv5ski8PX9rL2dZXvgGDnw"};
    const std::string xprv{
        "xprv9uHRZZhk6KAJC1avXpDAp4MDc3sQKNxDiPvvkX8Br5ngLNv1TxvUxt4cV1rGL5hj6K"
        "CesnDYUhd7oWgT11eZG7XnxHrnYeSvkzY7d2bhkJ7"};

    EXPECT_EQ(xpub, root.Xpub(reason_));
    EXPECT_EQ(xprv, root.Xprv(reason_));
}

// https://github.com/bitcoin/bips/blob/master/bip-0032.mediawiki#test-vector-3
TEST_F(Test_BlockchainAPI, TestBip32_standard_3)
{
    const std::string empty{};
    auto bytes = ot::Data::Factory(
        "0x4b381541583be4423346c643850da4b320e46a87ae3d2a4e6da11eba819cd4acba45"
        "d239319ac14f863b8d5ab5a0d0c64d2e8a1e7d1457df2e5a3c51c73235be",
        ot::Data::Mode::Hex);
    ot::OTPassword seed{};
    seed.setMemory(bytes->data(), bytes->size());
    const auto fingerprint = api_.Seeds().ImportRaw(seed, reason_);

    ASSERT_FALSE(fingerprint.empty());

    const auto& nymID =
        api_.Wallet()
            .Nym(reason_, "John Doe", {fingerprint, 0}, individual_)
            ->ID();

    ASSERT_FALSE(nymID.empty());

    const auto accountID = api_.Blockchain().NewHDSubaccount(
        nymID, ot::BlockchainAccountType::BIP32, btc_chain_, reason_);

    ASSERT_FALSE(accountID->empty());

    const auto& account =
        api_.Blockchain().Account(nymID, btc_chain_).GetHD().at(0);

    EXPECT_EQ(account.ID(), accountID);

    const auto pRoot = account.RootNode(reason_);

    ASSERT_TRUE(pRoot);

    const auto& root = *pRoot;
    const std::string xpub{
        "xpub68NZiKmJWnxxS6aaHmn81bvJeTESw724CRDs6HbuccFQN9Ku14VQrADWgqbhhTHBao"
        "hPX4CjNLf9fq9MYo6oDaPPLPxSb7gwQN3ih19Zm4Y"};
    const std::string xprv{
        "xprv9uPDJpEQgRQfDcW7BkF7eTya6RPxXeJCqCJGHuCJ4GiRVLzkTXBAJMu2qaMWPrS7AA"
        "NYqdq6vcBcBUdJCVVFceUvJFjaPdGZ2y9WACViL4L"};

    EXPECT_EQ(xpub, root.Xpub(reason_));
    EXPECT_EQ(xprv, root.Xprv(reason_));
}

TEST_F(Test_BlockchainAPI, testBip32_SeedA)
{
    const auto& account =
        api_.Blockchain().Account(alex_, btc_chain_).GetHD().at(0);

    EXPECT_EQ(account.ID(), account_1_id_);
    EXPECT_FALSE(account.LastUsed(Subchain::External));
    EXPECT_FALSE(account.LastUsed(Subchain::Internal));
    EXPECT_TRUE(account.LastGenerated(Subchain::External));
    EXPECT_TRUE(account.LastGenerated(Subchain::Internal));

    std::optional<ot::Bip32Index> index{};
    std::optional<ot::Bip32Index> last{};
    std::optional<ot::Bip32Index> generated{};

    for (ot::Bip32Index i{0}; i < 10; ++i) {
        const auto label{std::string{"receive "} + std::to_string(i)};
        index =
            account.UseNext(Subchain::External, reason_, contact_bob_, label);
        last = account.LastUsed(Subchain::External);
        generated = account.LastGenerated(Subchain::External);

        ASSERT_TRUE(index);
        EXPECT_EQ(i, index.value());
        ASSERT_TRUE(last);
        EXPECT_EQ(i, last.value());
        ASSERT_TRUE(generated);
        EXPECT_TRUE(generated.value() > last.value());

        const auto& element = account.BalanceElement(Subchain::External, i);
        const auto& target = alex_external_.at(i);

        EXPECT_EQ(element.Address(AddressStyle::P2PKH, reason_), target);

        const auto [bytes, style, chain] =
            api_.Blockchain().DecodeAddress(target);
        const auto encoded =
            api_.Blockchain().EncodeAddress(style, chain, bytes);

        EXPECT_EQ(target, encoded);
        EXPECT_EQ(element.Contact()->str(), contact_bob_.str());
        EXPECT_EQ(element.Label(), label);
    }

    for (ot::Bip32Index i{0}; i < 10; ++i) {
        const auto label{std::string{"change "} + std::to_string(i)};
        index =
            account.UseNext(Subchain::Internal, reason_, contact_bob_, label);
        last = account.LastUsed(Subchain::Internal);
        generated = account.LastGenerated(Subchain::Internal);

        ASSERT_TRUE(index);
        EXPECT_EQ(i, index.value());
        ASSERT_TRUE(last);
        EXPECT_EQ(i, last.value());
        ASSERT_TRUE(generated);
        EXPECT_TRUE(generated.value() > last.value());

        const auto& element = account.BalanceElement(Subchain::Internal, i);

        EXPECT_EQ(
            element.Address(AddressStyle::P2PKH, reason_),
            alex_internal_.at(i));
        EXPECT_EQ(element.Contact()->str(), empty_id_.str());
        EXPECT_EQ(element.Label(), label);
    }
}

TEST_F(Test_BlockchainAPI, testBip32_SeedB)
{
    account_6_id_.Assign(api_.Blockchain().NewHDSubaccount(
        bob_, ot::BlockchainAccountType::BIP32, btc_chain_, reason_));

    ASSERT_FALSE(account_6_id_.empty());

    auto list = api_.Blockchain().AccountList(bob_, btc_chain_);

    EXPECT_EQ(1, list.size());
    EXPECT_EQ(1, list.count(account_6_id_));

    const auto& account =
        api_.Blockchain().Account(bob_, btc_chain_).GetHD().at(0);

    EXPECT_EQ(account.ID(), account_6_id_);
    EXPECT_FALSE(account.LastUsed(Subchain::External));
    EXPECT_FALSE(account.LastUsed(Subchain::Internal));
    EXPECT_TRUE(account.LastGenerated(Subchain::External));
    EXPECT_TRUE(account.LastGenerated(Subchain::Internal));

    std::optional<ot::Bip32Index> index{};
    std::optional<ot::Bip32Index> last{};
    std::optional<ot::Bip32Index> generated{};

    for (ot::Bip32Index i{0}; i < 10; ++i) {
        const auto label{std::string{"receive "} + std::to_string(i)};
        index =
            account.UseNext(Subchain::External, reason_, contact_alex_, label);
        last = account.LastUsed(Subchain::External);
        generated = account.LastGenerated(Subchain::External);

        ASSERT_TRUE(index);
        EXPECT_EQ(i, index.value());
        ASSERT_TRUE(last);
        EXPECT_EQ(i, last.value());
        ASSERT_TRUE(generated);
        EXPECT_TRUE(generated.value() > last.value());

        const auto& element = account.BalanceElement(Subchain::External, i);

        EXPECT_EQ(
            element.Address(AddressStyle::P2PKH, reason_), bob_external_.at(i));
        EXPECT_EQ(element.Contact()->str(), contact_alex_.str());
        EXPECT_EQ(element.Label(), label);
    }

    for (ot::Bip32Index i{0}; i < 10; ++i) {
        const auto label{std::string{"change "} + std::to_string(i)};
        index =
            account.UseNext(Subchain::Internal, reason_, contact_alex_, label);
        last = account.LastUsed(Subchain::Internal);
        generated = account.LastGenerated(Subchain::Internal);

        ASSERT_TRUE(index);
        EXPECT_EQ(i, index.value());
        ASSERT_TRUE(last);
        EXPECT_EQ(i, last.value());
        ASSERT_TRUE(generated);
        EXPECT_TRUE(generated.value() > last.value());

        const auto& element = account.BalanceElement(Subchain::Internal, i);

        EXPECT_EQ(
            element.Address(AddressStyle::P2PKH, reason_), bob_internal_.at(i));
        EXPECT_EQ(element.Contact()->str(), empty_id_.str());
        EXPECT_EQ(element.Label(), label);
    }
}

TEST_F(Test_BlockchainAPI, testBip44_btc)
{
    const auto& account =
        api_.Blockchain().Account(chris_, btc_chain_).GetHD().at(account_4_id_);

    EXPECT_EQ(account.ID(), account_4_id_);
    EXPECT_FALSE(account.LastUsed(Subchain::External));
    EXPECT_FALSE(account.LastUsed(Subchain::Internal));
    EXPECT_TRUE(account.LastGenerated(Subchain::External));
    EXPECT_TRUE(account.LastGenerated(Subchain::Internal));

    std::optional<ot::Bip32Index> index{};
    std::optional<ot::Bip32Index> last{};
    std::optional<ot::Bip32Index> generated{};

    for (ot::Bip32Index i{0}; i < 2; ++i) {
        const auto label{std::string{"receive "} + std::to_string(i)};
        index = account.UseNext(
            Subchain::External, reason_, contact_daniel_, label);
        last = account.LastUsed(Subchain::External);
        generated = account.LastGenerated(Subchain::External);

        ASSERT_TRUE(index);
        EXPECT_EQ(i, index.value());
        ASSERT_TRUE(last);
        EXPECT_EQ(i, last.value());
        ASSERT_TRUE(generated);
        EXPECT_TRUE(generated.value() > last.value());

        const auto& element = account.BalanceElement(Subchain::External, i);

        EXPECT_EQ(
            element.Address(AddressStyle::P2PKH, reason_),
            chris_btc_external_.at(i));
        EXPECT_EQ(element.Contact()->str(), contact_daniel_.str());
        EXPECT_EQ(element.Label(), label);
    }

    for (ot::Bip32Index i{0}; i < 2; ++i) {
        const auto label{std::string{"change "} + std::to_string(i)};
        index = account.UseNext(
            Subchain::Internal, reason_, contact_daniel_, label);
        last = account.LastUsed(Subchain::Internal);
        generated = account.LastGenerated(Subchain::Internal);

        ASSERT_TRUE(index);
        EXPECT_EQ(i, index.value());
        ASSERT_TRUE(last);
        EXPECT_EQ(i, last.value());
        ASSERT_TRUE(generated);
        EXPECT_TRUE(generated.value() > last.value());

        const auto& element = account.BalanceElement(Subchain::Internal, i);

        EXPECT_EQ(
            element.Address(AddressStyle::P2PKH, reason_),
            chris_btc_internal_.at(i));
        EXPECT_EQ(element.Contact()->str(), empty_id_.str());
        EXPECT_EQ(element.Label(), label);
    }
}

TEST_F(Test_BlockchainAPI, testBip44_bch)
{
    const auto& account =
        api_.Blockchain().Account(chris_, bch_chain_).GetHD().at(account_5_id_);

    EXPECT_EQ(account.ID(), account_5_id_);
    EXPECT_FALSE(account.LastUsed(Subchain::External));
    EXPECT_FALSE(account.LastUsed(Subchain::Internal));
    EXPECT_TRUE(account.LastGenerated(Subchain::External));
    EXPECT_TRUE(account.LastGenerated(Subchain::Internal));

    std::optional<ot::Bip32Index> index{};
    std::optional<ot::Bip32Index> last{};
    std::optional<ot::Bip32Index> generated{};

    for (ot::Bip32Index i{0}; i < 2; ++i) {
        const auto label{std::string{"receive "} + std::to_string(i)};
        index = account.UseNext(Subchain::External, reason_, empty_id_, label);
        last = account.LastUsed(Subchain::External);
        generated = account.LastGenerated(Subchain::External);

        ASSERT_TRUE(index);
        EXPECT_EQ(i, index.value());
        ASSERT_TRUE(last);
        EXPECT_EQ(i, last.value());
        ASSERT_TRUE(generated);
        EXPECT_TRUE(generated.value() > last.value());

        const auto& element = account.BalanceElement(Subchain::External, i);

        EXPECT_EQ(
            element.Address(AddressStyle::P2PKH, reason_),
            chris_bch_external_.at(i));
        EXPECT_EQ(element.Contact()->str(), empty_id_.str());
        EXPECT_EQ(element.Label(), label);
    }

    for (ot::Bip32Index i{0}; i < 2; ++i) {
        const auto label{std::string{"change "} + std::to_string(i)};
        index = account.UseNext(Subchain::Internal, reason_, empty_id_, label);
        last = account.LastUsed(Subchain::Internal);
        generated = account.LastGenerated(Subchain::Internal);

        ASSERT_TRUE(index);
        EXPECT_EQ(i, index.value());
        ASSERT_TRUE(last);
        EXPECT_EQ(i, last.value());
        ASSERT_TRUE(generated);
        EXPECT_TRUE(generated.value() > last.value());

        const auto& element = account.BalanceElement(Subchain::Internal, i);

        EXPECT_EQ(
            element.Address(AddressStyle::P2PKH, reason_),
            chris_bch_internal_.at(i));
        EXPECT_EQ(element.Contact()->str(), empty_id_.str());
        EXPECT_EQ(element.Label(), label);
    }
}

TEST_F(Test_BlockchainAPI, testBip44_ltc)
{
    account_7_id_.Assign(api_.Blockchain().NewHDSubaccount(
        chris_, ot::BlockchainAccountType::BIP44, ltc_chain_, reason_));

    ASSERT_FALSE(account_7_id_.empty());

    auto list = api_.Blockchain().AccountList(chris_, ltc_chain_);

    EXPECT_EQ(1, list.size());
    EXPECT_EQ(1, list.count(account_7_id_));

    const auto& account =
        api_.Blockchain().Account(chris_, ltc_chain_).GetHD().at(account_7_id_);

    EXPECT_EQ(account.ID(), account_7_id_);
    EXPECT_FALSE(account.LastUsed(Subchain::External));
    EXPECT_FALSE(account.LastUsed(Subchain::Internal));
    EXPECT_TRUE(account.LastGenerated(Subchain::External));
    EXPECT_TRUE(account.LastGenerated(Subchain::Internal));

    std::optional<ot::Bip32Index> index{};
    std::optional<ot::Bip32Index> last{};
    std::optional<ot::Bip32Index> generated{};

    for (ot::Bip32Index i{0}; i < 2; ++i) {
        const auto label{empty_string_};
        index =
            account.UseNext(Subchain::External, reason_, contact_alex_, label);
        last = account.LastUsed(Subchain::External);
        generated = account.LastGenerated(Subchain::External);

        ASSERT_TRUE(index);
        EXPECT_EQ(i, index.value());
        ASSERT_TRUE(last);
        EXPECT_EQ(i, last.value());
        ASSERT_TRUE(generated);
        EXPECT_TRUE(generated.value() > last.value());

        const auto& element = account.BalanceElement(Subchain::External, i);
        const auto& target = chris_ltc_external_.at(i);

        EXPECT_EQ(element.Address(AddressStyle::P2PKH, reason_), target);

        const auto [bytes, style, chain] =
            api_.Blockchain().DecodeAddress(target);
        const auto encoded =
            api_.Blockchain().EncodeAddress(style, chain, bytes);

        EXPECT_EQ(target, encoded);
        EXPECT_EQ(element.Contact()->str(), contact_alex_.str());
        EXPECT_EQ(element.Label(), label);
    }

    for (ot::Bip32Index i{0}; i < 2; ++i) {
        const auto label{std::string{"change "} + std::to_string(i)};
        index =
            account.UseNext(Subchain::Internal, reason_, contact_alex_, label);
        last = account.LastUsed(Subchain::Internal);
        generated = account.LastGenerated(Subchain::Internal);

        ASSERT_TRUE(index);
        EXPECT_EQ(i, index.value());
        ASSERT_TRUE(last);
        EXPECT_EQ(i, last.value());
        ASSERT_TRUE(generated);
        EXPECT_TRUE(generated.value() > last.value());

        const auto& element = account.BalanceElement(Subchain::Internal, i);

        EXPECT_EQ(
            element.Address(AddressStyle::P2PKH, reason_),
            chris_ltc_internal_.at(i));
        EXPECT_EQ(element.Contact()->str(), empty_id_.str());
        EXPECT_EQ(element.Label(), label);
    }
}

TEST_F(Test_BlockchainAPI, AccountList)
{
    auto list = api_.Blockchain().AccountList(alex_, bch_chain_);

    EXPECT_EQ(0, list.size());

    list = api_.Blockchain().AccountList(alex_, ltc_chain_);

    EXPECT_EQ(0, list.size());

    list = api_.Blockchain().AccountList(bob_, bch_chain_);

    EXPECT_EQ(0, list.size());

    list = api_.Blockchain().AccountList(bob_, ltc_chain_);

    EXPECT_EQ(0, list.size());

    list = api_.Blockchain().AccountList(daniel_, bch_chain_);

    EXPECT_EQ(0, list.size());

    list = api_.Blockchain().AccountList(daniel_, ltc_chain_);

    EXPECT_EQ(0, list.size());
}

TEST_F(Test_BlockchainAPI, AssignContact_no_change)
{
    const auto& account =
        api_.Blockchain().Account(chris_, btc_chain_).GetHD().at(account_4_id_);
    const auto& element = account.BalanceElement(Subchain::External, 0);

    EXPECT_EQ(element.Contact()->str(), contact_daniel_.str());

    const auto assigned = api_.Blockchain().AssignContact(
        chris_, account_4_id_, Subchain::External, 0, contact_daniel_);

    EXPECT_TRUE(assigned);
    EXPECT_EQ(element.Contact()->str(), contact_daniel_.str());
}

TEST_F(Test_BlockchainAPI, Receive_unassigned)
{
    EXPECT_TRUE(transaction_state_0());

    auto tx = ot::proto::BlockchainTransaction{};
    tx.set_version(1);
    tx.set_txid(txid_1_);
    tx.set_chain(ot::proto::CITEMTYPE_BCH);
    tx.set_txversion(1);
    tx.set_fee(14);
    tx.set_time(ot::Clock::to_time_t(ot::Clock::now()));
    tx.set_memo(memo_1_);
    tx.set_confirmations(88);

    {
        auto& input = *tx.add_input();
        input.set_version(1);
        input.set_index(0);
        input.set_script(dummy_script_);
        auto& previous = *input.mutable_previous();
        previous.set_version(1);
        previous.set_txid(txid_0_);
        previous.set_index(0);
    }
    {
        auto& output = *tx.add_output();
        output.set_version(1);
        output.set_index(0);
        output.set_value(100000000);
        output.set_script(dummy_script_);
        auto& key = *output.mutable_key();
        key.set_version(1);
        key.set_chain(ot::proto::CITEMTYPE_BCH);
        key.set_nym(chris_.str());
        key.set_subaccount(account_5_id_.str());
        key.set_subchain(static_cast<std::uint8_t>(Subchain::External));
        key.set_index(0);
    }
    {
        auto& output = *tx.add_output();
        output.set_version(1);
        output.set_index(1);
        output.set_value(200000000);
        output.set_script(dummy_script_);
        auto& external = *output.mutable_external();
        external.set_version(1);
        external.set_type(ot::proto::BTOUTPUT_P2PKH);
        external.add_data(address_1_.str());
    }

    const auto stored =
        api_.Blockchain().StoreTransaction(chris_, bch_chain_, tx, reason_);

    ASSERT_TRUE(stored);
    EXPECT_TRUE(transaction_state_1());
}

TEST_F(Test_BlockchainAPI, Load_Transaction)
{
    const auto pTX = api_.Blockchain().Transaction(txid_1_);

    ASSERT_TRUE(pTX);

    const auto& tx = *pTX;

    EXPECT_EQ(tx.version(), 1);
    EXPECT_EQ(tx.txid(), txid_1_);
    EXPECT_EQ(tx.chain(), ot::proto::CITEMTYPE_BCH);
    EXPECT_EQ(tx.txversion(), 1);
    EXPECT_EQ(tx.fee(), 14);
    EXPECT_EQ(tx.memo(), memo_1_);
    EXPECT_EQ(tx.confirmations(), 88);

    const auto& input = tx.input(0);

    EXPECT_EQ(input.version(), 1);
    EXPECT_EQ(input.index(), 0);
    EXPECT_EQ(input.script(), dummy_script_);

    const auto& previous = input.previous();

    EXPECT_EQ(previous.version(), 1);
    EXPECT_EQ(previous.txid(), txid_0_);
    EXPECT_EQ(previous.index(), 0);

    {
        const auto& output = tx.output(0);

        EXPECT_EQ(output.version(), 1);
        EXPECT_EQ(output.index(), 0);
        EXPECT_EQ(output.value(), 100000000);
        EXPECT_EQ(output.script(), dummy_script_);

        const auto& key = output.key();

        EXPECT_EQ(key.version(), 1);
        EXPECT_EQ(key.chain(), ot::proto::CITEMTYPE_BCH);
        EXPECT_EQ(key.nym(), chris_.str());
        EXPECT_EQ(key.subaccount(), account_5_id_.str());
        EXPECT_EQ(
            key.subchain(), static_cast<std::uint8_t>(Subchain::External));
        EXPECT_EQ(key.index(), 0);
    }
    {
        const auto& output = tx.output(1);

        EXPECT_EQ(output.version(), 1);
        EXPECT_EQ(output.index(), 1);
        EXPECT_EQ(output.value(), 200000000);
        EXPECT_EQ(output.script(), dummy_script_);

        const auto& external = output.external();

        EXPECT_EQ(external.version(), 1);
        EXPECT_EQ(external.type(), ot::proto::BTOUTPUT_P2PKH);
        EXPECT_EQ(external.data(0), address_1_.str());
    }

    EXPECT_TRUE(transaction_state_1());
}

TEST_F(Test_BlockchainAPI, Receive_assigned)
{
    EXPECT_TRUE(transaction_state_1());

    {
        auto tx = ot::proto::BlockchainTransaction{};
        tx.set_version(1);
        tx.set_txid(txid_2_);
        tx.set_chain(ot::proto::CITEMTYPE_BTC);
        tx.set_txversion(1);
        tx.set_fee(14);
        tx.set_time(ot::Clock::to_time_t(ot::Clock::now()));
        tx.set_memo(memo_2_);
        tx.set_confirmations(88);

        {
            auto& input = *tx.add_input();
            input.set_version(1);
            input.set_index(0);
            auto& previous = *input.mutable_previous();
            previous.set_version(1);
            previous.set_txid(txid_0_);
            previous.set_index(1);
            input.set_script(dummy_script_);
        }
        {
            auto& output = *tx.add_output();
            output.set_version(1);
            output.set_index(0);
            output.set_value(100000000);
            output.set_script(dummy_script_);
            auto& key = *output.mutable_key();
            key.set_version(1);
            key.set_chain(ot::proto::CITEMTYPE_BTC);
            key.set_nym(bob_.str());
            key.set_subaccount(account_6_id_.str());
            key.set_subchain(static_cast<std::uint8_t>(Subchain::External));
            key.set_index(0);
        }
        {
            auto& output = *tx.add_output();
            output.set_version(1);
            output.set_index(1);
            output.set_value(200000000);
            output.set_script(dummy_script_);
            auto& external = *output.mutable_external();
            external.set_version(1);
            external.set_type(ot::proto::BTOUTPUT_P2PKH);
            external.add_data(address_1_.str());
        }

        const auto stored =
            api_.Blockchain().StoreTransaction(bob_, btc_chain_, tx, reason_);

        EXPECT_TRUE(stored);
    }
    {
        auto tx = ot::proto::BlockchainTransaction{};
        tx.set_version(1);
        tx.set_txid(txid_3_);
        tx.set_chain(ot::proto::CITEMTYPE_BTC);
        tx.set_txversion(1);
        tx.set_fee(14);
        tx.set_time(ot::Clock::to_time_t(ot::Clock::now()));
        tx.set_memo(memo_3_);
        tx.set_confirmations(88);

        {
            auto& input = *tx.add_input();
            input.set_version(1);
            input.set_index(0);
            input.set_script(dummy_script_);
            auto& previous = *input.mutable_previous();
            previous.set_version(1);
            previous.set_txid(txid_0_);
            previous.set_index(2);
        }
        {
            auto& output = *tx.add_output();
            output.set_version(1);
            output.set_index(0);
            output.set_value(300000000);
            output.set_script(dummy_script_);
            auto& key = *output.mutable_key();
            key.set_version(1);
            key.set_chain(ot::proto::CITEMTYPE_BTC);
            key.set_nym(bob_.str());
            key.set_subaccount(account_1_id_.str());
            key.set_subchain(static_cast<std::uint8_t>(Subchain::External));
            key.set_index(0);
        }
        {
            auto& output = *tx.add_output();
            output.set_version(1);
            output.set_index(1);
            output.set_value(400000000);
            output.set_script(dummy_script_);
            auto& external = *output.mutable_external();
            external.set_version(1);
            external.set_type(ot::proto::BTOUTPUT_P2PKH);
            external.add_data(address_1_.str());
        }

        const auto stored =
            api_.Blockchain().StoreTransaction(alex_, btc_chain_, tx, reason_);

        EXPECT_TRUE(stored);
    }

    EXPECT_TRUE(transaction_state_2());
}

TEST_F(Test_BlockchainAPI, Test_Transaction_Idempotence)
{
    EXPECT_TRUE(transaction_state_2());

    {
        auto tx = ot::proto::BlockchainTransaction{};
        tx.set_version(1);
        tx.set_txid(txid_2_);
        tx.set_chain(ot::proto::CITEMTYPE_BTC);
        tx.set_txversion(1);
        tx.set_fee(14);
        tx.set_time(ot::Clock::to_time_t(ot::Clock::now()));
        tx.set_memo(memo_2_);
        tx.set_confirmations(88);

        {
            auto& input = *tx.add_input();
            input.set_version(1);
            input.set_index(0);
            auto& previous = *input.mutable_previous();
            previous.set_version(1);
            previous.set_txid(txid_0_);
            previous.set_index(1);
            input.set_script(dummy_script_);
        }
        {
            auto& output = *tx.add_output();
            output.set_version(1);
            output.set_index(0);
            output.set_value(100000000);
            output.set_script(dummy_script_);
            auto& key = *output.mutable_key();
            key.set_version(1);
            key.set_chain(ot::proto::CITEMTYPE_BTC);
            key.set_nym(bob_.str());
            key.set_subaccount(account_6_id_.str());
            key.set_subchain(static_cast<std::uint8_t>(Subchain::External));
            key.set_index(0);
        }
        {
            auto& output = *tx.add_output();
            output.set_version(1);
            output.set_index(1);
            output.set_value(200000000);
            output.set_script(dummy_script_);
            auto& external = *output.mutable_external();
            external.set_version(1);
            external.set_type(ot::proto::BTOUTPUT_P2PKH);
            external.add_data(address_1_.str());
        }

        const auto stored =
            api_.Blockchain().StoreTransaction(bob_, btc_chain_, tx, reason_);

        EXPECT_TRUE(stored);
    }

    EXPECT_TRUE(transaction_state_2());
}

TEST_F(Test_BlockchainAPI, AssignContact_assign)
{
    EXPECT_TRUE(transaction_state_2());

    const auto& account =
        api_.Blockchain().Account(chris_, bch_chain_).GetHD().at(account_5_id_);
    const auto& element = account.BalanceElement(Subchain::External, 0);

    EXPECT_EQ(element.Contact()->str(), empty_id_.str());

    const auto assigned = api_.Blockchain().AssignContact(
        chris_, account_5_id_, Subchain::External, 0, contact_bob_);

    EXPECT_TRUE(assigned);
    EXPECT_EQ(element.Contact()->str(), contact_bob_.str());
    EXPECT_TRUE(transaction_state_3());
}

TEST_F(Test_BlockchainAPI, AssignContact_reassign)
{
    EXPECT_TRUE(transaction_state_3());

    const auto& account =
        api_.Blockchain().Account(bob_, btc_chain_).GetHD().at(account_6_id_);
    const auto& element = account.BalanceElement(Subchain::External, 0);

    EXPECT_EQ(element.Contact()->str(), contact_alex_.str());

    const auto assigned = api_.Blockchain().AssignContact(
        bob_, account_6_id_, Subchain::External, 0, contact_chris_);

    EXPECT_TRUE(assigned);
    EXPECT_EQ(element.Contact()->str(), contact_chris_.str());
    EXPECT_TRUE(transaction_state_4());
}

TEST_F(Test_BlockchainAPI, AssignContact_clear)
{
    EXPECT_TRUE(transaction_state_4());
    const auto& account =
        api_.Blockchain().Account(alex_, btc_chain_).GetHD().at(account_1_id_);
    const auto& element = account.BalanceElement(Subchain::External, 0);

    EXPECT_EQ(element.Contact()->str(), contact_bob_.str());

    const auto assigned = api_.Blockchain().AssignContact(
        alex_, account_1_id_, Subchain::External, 0, empty_id_);

    EXPECT_TRUE(assigned);
    EXPECT_EQ(element.Contact()->str(), empty_id_.str());
    EXPECT_TRUE(transaction_state_5());
}

TEST_F(Test_BlockchainAPI, AssignLabel_no_change)
{
    const auto& account =
        api_.Blockchain().Account(chris_, ltc_chain_).GetHD().at(account_7_id_);
    const auto& element = account.BalanceElement(Subchain::Internal, 0);
    const std::string label{"change 0"};

    EXPECT_EQ(element.Label(), label);

    const auto assigned = api_.Blockchain().AssignLabel(
        chris_, account_7_id_, Subchain::Internal, 0, label);

    EXPECT_TRUE(assigned);
    EXPECT_EQ(element.Label(), label);
    EXPECT_TRUE(transaction_state_5());
}

TEST_F(Test_BlockchainAPI, AssignLabel_label)
{
    const auto& account =
        api_.Blockchain().Account(chris_, ltc_chain_).GetHD().at(account_7_id_);
    const auto& element = account.BalanceElement(Subchain::External, 0);
    const std::string label{"foo"};

    EXPECT_EQ(element.Label(), empty_string_);

    const auto assigned = api_.Blockchain().AssignLabel(
        chris_, account_7_id_, Subchain::External, 0, label);

    EXPECT_TRUE(assigned);
    EXPECT_EQ(element.Label(), label);
    EXPECT_TRUE(transaction_state_5());
}

TEST_F(Test_BlockchainAPI, AssignLabel_relabel)
{
    const auto& account =
        api_.Blockchain().Account(chris_, ltc_chain_).GetHD().at(account_7_id_);
    const auto& element = account.BalanceElement(Subchain::External, 0);
    const std::string before{"foo"};
    const std::string after{"bar"};

    EXPECT_EQ(element.Label(), before);

    const auto assigned = api_.Blockchain().AssignLabel(
        chris_, account_7_id_, Subchain::External, 0, after);

    EXPECT_TRUE(assigned);
    EXPECT_EQ(element.Label(), after);
    EXPECT_TRUE(transaction_state_5());
}

TEST_F(Test_BlockchainAPI, AssignLabel_clear)
{
    const auto& account =
        api_.Blockchain().Account(chris_, ltc_chain_).GetHD().at(account_7_id_);
    const auto& element = account.BalanceElement(Subchain::External, 0);
    const std::string before{"bar"};

    EXPECT_EQ(element.Label(), before);

    const auto assigned = api_.Blockchain().AssignLabel(
        chris_, account_7_id_, Subchain::External, 0, empty_string_);

    EXPECT_TRUE(assigned);
    EXPECT_EQ(element.Label(), empty_string_);
    EXPECT_TRUE(transaction_state_5());
}

TEST_F(Test_BlockchainAPI, Send_transaction)
{
    EXPECT_TRUE(transaction_state_5());

    auto address1 = api_.Factory().Data();
    auto address2 = api_.Factory().Data();
    auto address3 = api_.Factory().Data();

    {
        const auto assigned = api_.Blockchain().AssignContact(
            alex_, account_1_id_, Subchain::External, 2, contact_bob_);

        EXPECT_TRUE(assigned);

        const auto& account =
            api_.Blockchain().HDSubaccount(alex_, account_1_id_);
        const auto& element = account.BalanceElement(Subchain::External, 2);
        address1 = element.PubkeyHash(reason_);

        auto contactE = api_.Contacts().mutable_Contact(contact_alex_, reason_);

        ASSERT_TRUE(contactE);

        const auto claimed = contactE->get().AddBlockchainAddress(
            element.Address(AddressStyle::P2PKH, reason_),
            ot::proto::CITEMTYPE_BTC);

        EXPECT_TRUE(claimed);
    }
    {
        const auto assigned = api_.Blockchain().AssignContact(
            daniel_, account_2_id_, Subchain::External, 2, contact_bob_);

        EXPECT_TRUE(assigned);

        const auto& account =
            api_.Blockchain().HDSubaccount(daniel_, account_2_id_);
        const auto& element = account.BalanceElement(Subchain::External, 2);
        address2 = element.PubkeyHash(reason_);

        auto contactE =
            api_.Contacts().mutable_Contact(contact_daniel_, reason_);

        ASSERT_TRUE(contactE);

        const auto claimed = contactE->get().AddBlockchainAddress(
            element.Address(AddressStyle::P2PKH, reason_),
            ot::proto::CITEMTYPE_BTC);

        EXPECT_TRUE(claimed);
    }
    {
        const auto assigned = api_.Blockchain().AssignContact(
            chris_, account_3_id_, Subchain::External, 2, contact_bob_);

        EXPECT_TRUE(assigned);

        const auto& account =
            api_.Blockchain().HDSubaccount(chris_, account_3_id_);
        const auto& element = account.BalanceElement(Subchain::External, 2);
        address3 = element.PubkeyHash(reason_);
    }

    auto tx = ot::proto::BlockchainTransaction{};
    tx.set_version(1);
    tx.set_txid(txid_4_);
    tx.set_chain(ot::proto::CITEMTYPE_BTC);
    tx.set_txversion(1);
    tx.set_fee(0);
    tx.set_time(ot::Clock::to_time_t(ot::Clock::now()));
    tx.set_memo(memo_4_);
    tx.set_confirmations(1);

    {
        auto& input = *tx.add_input();
        input.set_version(1);
        input.set_index(0);
        input.set_script(dummy_script_);
        auto& previous = *input.mutable_previous();
        previous.set_version(1);
        previous.set_txid(txid_2_);
        previous.set_index(0);
    }
    {
        auto& output = *tx.add_output();
        output.set_version(1);
        output.set_index(0);
        output.set_value(10000000);
        output.set_script(dummy_script_);
        auto& external = *output.mutable_external();
        external.set_version(1);
        external.set_type(ot::proto::BTOUTPUT_P2PKH);
        external.add_data(address1->str());
    }
    {
        auto& output = *tx.add_output();
        output.set_version(1);
        output.set_index(1);
        output.set_value(20000000);
        output.set_script(dummy_script_);
        auto& external = *output.mutable_external();
        external.set_version(1);
        external.set_type(ot::proto::BTOUTPUT_P2PKH);
        external.add_data(address2->str());
    }
    {
        auto& output = *tx.add_output();
        output.set_version(1);
        output.set_index(2);
        output.set_value(30000000);
        output.set_script(dummy_script_);
        auto& external = *output.mutable_external();
        external.set_version(1);
        external.set_type(ot::proto::BTOUTPUT_P2PKH);
        external.add_data(address3->str());
    }
    {
        auto& output = *tx.add_output();
        output.set_version(1);
        output.set_index(3);
        output.set_value(40000000);
        output.set_script(dummy_script_);
        auto& key = *output.mutable_key();
        key.set_version(1);
        key.set_chain(ot::proto::CITEMTYPE_BTC);
        key.set_nym(bob_.str());
        key.set_subaccount(account_6_id_.str());
        key.set_subchain(static_cast<std::uint8_t>(Subchain::Internal));
        key.set_index(0);
    }

    const auto stored =
        api_.Blockchain().StoreTransaction(bob_, btc_chain_, tx, reason_);

    ASSERT_TRUE(stored);
    EXPECT_TRUE(transaction_state_6());
}

TEST_F(Test_BlockchainAPI, Assign_External_Address)
{
    {
        const auto& account =
            api_.Blockchain().HDSubaccount(chris_, account_3_id_);
        const auto& element = account.BalanceElement(Subchain::External, 2);
        auto contactE =
            api_.Contacts().mutable_Contact(contact_chris_, reason_);

        ASSERT_TRUE(contactE);

        const auto claimed = contactE->get().AddBlockchainAddress(
            element.Address(AddressStyle::P2PKH, reason_),
            ot::proto::CITEMTYPE_BTC);

        EXPECT_TRUE(claimed);
    }

    EXPECT_TRUE(transaction_state_7());
}
}  // namespace
