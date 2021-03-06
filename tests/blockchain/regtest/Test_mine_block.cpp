// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Helpers.hpp"  // IWYU pragma: associated

#include <gtest/gtest-message.h>
#include <gtest/gtest-test-part.h>
#include <gtest/gtest.h>
#include <algorithm>
#include <vector>

#include "opentxs/Bytes.hpp"
#include "opentxs/api/client/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/block/bitcoin/Header.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"
#include "opentxs/blockchain/block/bitcoin/Inputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/blockchain/client/HeaderOracle.hpp"

namespace
{
TEST_F(Regtest_fixture, init_opentxs) {}

TEST_F(Regtest_fixture, start_chains) { EXPECT_TRUE(Start()); }

TEST_F(Regtest_fixture, generate_block)
{
    const auto& network = miner_.Blockchain().GetChain(chain_);
    const auto& headerOracle = network.HeaderOracle();
    auto previousHeader = [&] {
        const auto genesis = headerOracle.LoadHeader(
            ot::blockchain::client::HeaderOracle::GenesisBlockHash(chain_));

        return genesis->as_Bitcoin();
    }();

    ASSERT_TRUE(previousHeader);

    using OutputBuilder = ot::api::Factory::OutputBuilder;
    auto tx = miner_.Factory().BitcoinGenerationTransaction(
        chain_,
        previousHeader->Height() + 1,
        [&] {
            auto output = std::vector<OutputBuilder>{};
            const auto text = std::string{"null"};
            const auto keys = std::set<ot::api::client::blockchain::Key>{};
            output.emplace_back(
                5000000000,
                miner_.Factory().BitcoinScriptNullData(chain_, {text}),
                keys);

            return output;
        }(),
        "The Industrial Revolution and its consequences have been a disaster "
        "for the human race.");

    ASSERT_TRUE(tx);

    {
        const auto& inputs = tx->Inputs();

        ASSERT_EQ(inputs.size(), 1);

        const auto& input = inputs.at(0);

        EXPECT_EQ(input.Coinbase().size(), 91);
    }

    {
        const auto serialized = [&] {
            auto output = miner_.Factory().Data();
            tx->Serialize(output->WriteInto());

            return output;
        }();

        ASSERT_GT(serialized->size(), 0);

        const auto recovered = miner_.Factory().BitcoinTransaction(
            chain_, serialized->Bytes(), true);

        ASSERT_TRUE(recovered);

        const auto serialized2 = [&] {
            auto output = miner_.Factory().Data();
            recovered->Serialize(output->WriteInto());

            return output;
        }();

        EXPECT_EQ(recovered->ID(), tx->ID());
        EXPECT_EQ(serialized.get(), serialized2.get());
    }

    auto block = miner_.Factory().BitcoinBlock(
        *previousHeader,
        tx,
        previousHeader->nBits(),
        {},
        previousHeader->Version(),
        [start{ot::Clock::now()}] {
            return (ot::Clock::now() - start) > std::chrono::minutes(1);
        });

    ASSERT_TRUE(block);

    const auto serialized = [&] {
        auto output = miner_.Factory().Data();
        block->Serialize(output->WriteInto());

        return output;
    }();

    ASSERT_GT(serialized->size(), 0);

    const auto recovered =
        miner_.Factory().BitcoinBlock(chain_, serialized->Bytes());

    EXPECT_TRUE(recovered);
}

TEST_F(Regtest_fixture, shutdown) { Shutdown(); }
}  // namespace
