// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Output.hpp"

#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"

#define OT_METHOD " opentxs::blockchain::transaction::bitcoin::Output::"

namespace opentxs::blockchain::transaction::bitcoin
{

OTData ScriptElement::Encode() const noexcept
{
    try {
        if (hasOpcode()) {
            OpcodeField raw_opcode(static_cast<std::uint8_t>(getOpcode()));

            return Data::Factory(&raw_opcode, sizeof(raw_opcode));
        } else if (hasData()) {

            return Data::Factory(getData().data(), getData().size());
        }
        return Data::Factory();
    } catch (...) {

        return Data::Factory();
    }
}

bool ScriptElement::DecodeOpcodeFromPayload(
    const std::byte*& it,
    std::size_t& expectedSize,
    const std::size_t size,
    ScriptElement& result) noexcept
{
    OpcodeField raw_opcode{};
    // -----------------------------------------------
    expectedSize += sizeof(raw_opcode);

    if (expectedSize > size) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Size below minimum for script opcode")
            .Flush();

        return false;
    }
    // ----------------------------------------------------------
    OTPassword::safe_memcpy(
        &raw_opcode, sizeof(raw_opcode), it, sizeof(raw_opcode));
    it += sizeof(raw_opcode);

    const Opcode opcode(static_cast<Opcode>(raw_opcode.value()));
    // -----------------------------------------------
    result = ScriptElement(opcode);

    return true;
}

bool ScriptElement::DecodeDataFromPayload(
    const std::byte*& it,
    std::size_t& expectedSize,
    const std::size_t size,
    const std::size_t data_size,
    ScriptElement& result) noexcept
{
    expectedSize += data_size;

    if (expectedSize > size) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Size below minimum for script data element")
            .Flush();

        return false;
    }
    // ----------------------------------------------------------
    result = ScriptElement(Data::Factory(it, data_size));
    it += data_size;

    return true;
}

// Based on the current opcode, this tells us how many bytes
// (if any) that are expected to be read for the next script element.
// If zero, then presumably the next element is another opcode, and not data.
std::size_t ScriptElement::expectedDataByteCount() const noexcept
{
    try {
        Opcode opcode = getOpcode();

        if (false == hasOpcode()) { return 0; }
        if (Opcode::OP_0 == opcode || Opcode::OP_FALSE == opcode) { return 0; }
        if (opcode >= Opcode::OP_DATA_MIN && opcode <= Opcode::OP_DATA_MAX) {
            return static_cast<std::size_t>(opcode);
        }
        if (Opcode::OP_PUSHDATA1 == opcode) { return 1; }
        if (Opcode::OP_PUSHDATA2 == opcode) { return 2; }
        if (Opcode::OP_PUSHDATA4 == opcode) { return 4; }

        //        if (Opcode::OP_1NEGATE == opcode) { return 0; }
        //        if (Opcode::OP_1 == opcode) { return 0; }
        //        if (Opcode::OP_TRUE == opcode) { return 0; }
        //        if (opcode >= Opcode::OP_2 &&
        //            opcode <= Opcode::OP_16) { return 0; }
        //
        //        if (   Opcode::OP_NOP == opcode
        //            || Opcode::OP_IF == opcode
        //            || Opcode::OP_NOTIF == opcode
        //            || Opcode::OP_ELSE == opcode
        //            || Opcode::OP_ENDIF == opcode
        //            || Opcode::OP_VERIFY == opcode
        //            || Opcode::OP_RETURN == opcode
        //            ) { return 0; }
        //
        //        if (   Opcode::OP_TOALTSTACK == opcode
        //            || Opcode::OP_FROMALTSTACK == opcode
        //            || Opcode::OP_IFDUP == opcode
        //            || Opcode::OP_DEPTH == opcode
        //            || Opcode::OP_DROP == opcode
        //            || Opcode::OP_DUP == opcode
        //            || Opcode::OP_NIP == opcode
        //            || Opcode::OP_OVER == opcode
        //            || Opcode::OP_PICK == opcode
        //            || Opcode::OP_ROLL == opcode
        //            || Opcode::OP_ROT == opcode
        //            || Opcode::OP_SWAP == opcode
        //            || Opcode::OP_TUCK == opcode
        //            || Opcode::OP_2DROP == opcode
        //            || Opcode::OP_2DUP == opcode
        //            || Opcode::OP_3DUP == opcode
        //            || Opcode::OP_2OVER == opcode
        //            || Opcode::OP_2ROT == opcode
        //            || Opcode::OP_2SWAP == opcode
        //            ) { return 0; }
        //
        //        if (   Opcode::OP_CAT == opcode
        //            || Opcode::OP_SUBSTR == opcode
        //            || Opcode::OP_LEFT == opcode
        //            || Opcode::OP_RIGHT == opcode
        //            || Opcode::OP_SIZE == opcode
        //            ) { return 0; }
        //
        //        if (   Opcode::OP_INVERT == opcode
        //            || Opcode::OP_AND == opcode
        //            || Opcode::OP_OR == opcode
        //            || Opcode::OP_XOR == opcode
        //            || Opcode::OP_EQUAL == opcode
        //            || Opcode::OP_EQUALVERIFY == opcode
        //            ) { return 0; }
        //
        //        if (   Opcode::OP_1ADD == opcode
        //            || Opcode::OP_1SUB == opcode
        //            || Opcode::OP_2MUL == opcode
        //            || Opcode::OP_2MUL == opcode
        //            || Opcode::OP_2DIV == opcode
        //            || Opcode::OP_NEGATE == opcode
        //            || Opcode::OP_ABS == opcode
        //            || Opcode::OP_NOT == opcode
        //            || Opcode::OP_0NOTEQUAL == opcode
        //            || Opcode::OP_ADD == opcode
        //            || Opcode::OP_SUB == opcode
        //            || Opcode::OP_MUL == opcode
        //            || Opcode::OP_DIV == opcode
        //            || Opcode::OP_MOD == opcode
        //            || Opcode::OP_LSHIFT == opcode
        //            || Opcode::OP_RSHIFT == opcode
        //            || Opcode::OP_BOOLAND == opcode
        //            || Opcode::OP_BOOLOR == opcode
        //            || Opcode::OP_NUMEQUAL == opcode
        //            || Opcode::OP_NUMEQUALVERIFY == opcode
        //            || Opcode::OP_NUMNOTEQUAL == opcode
        //            || Opcode::OP_LESSTHAN == opcode
        //            || Opcode::OP_GREATERTHAN == opcode
        //            || Opcode::OP_LESSTHANOREQUAL == opcode
        //            || Opcode::OP_GREATERTHANOREQUAL == opcode
        //            || Opcode::OP_MIN == opcode
        //            || Opcode::OP_MAX == opcode
        //            || Opcode::OP_WITHIN == opcode
        //            ) { return 0; }
        //
        //        if (   Opcode::OP_RIPEMD160 == opcode
        //            || Opcode::OP_SHA1 == opcode
        //            || Opcode::OP_SHA256 == opcode
        //            || Opcode::OP_HASH160 == opcode
        //            || Opcode::OP_HASH256 == opcode
        //            || Opcode::OP_CODESEPARATOR == opcode
        //            || Opcode::OP_CHECKSIG == opcode
        //            || Opcode::OP_CHECKSIGVERIFY == opcode
        //            || Opcode::OP_CHECKMULTISIG == opcode
        //            || Opcode::OP_CHECKMULTISIGVERIFY == opcode
        //            ) { return 0; }
        //
        //        if (   Opcode::OP_CHECKLOCKTIMEVERIFY == opcode
        //            || Opcode::OP_CHECKSEQUENCEVERIFY == opcode
        //            ) { return 0; }
        //
        //        if (   Opcode::OP_PUBKEYHASH == opcode
        //            || Opcode::OP_PUBKEY == opcode
        //            || Opcode::OP_INVALIDOPCODE == opcode
        //            ) { return 0; }
        //
        //        if (   Opcode::OP_RESERVED == opcode
        //            || Opcode::OP_VER == opcode
        //            || Opcode::OP_VERIF == opcode
        //            || Opcode::OP_VERNOTIF == opcode
        //            || Opcode::OP_RESERVED1 == opcode
        //            || Opcode::OP_RESERVED2 == opcode
        //            ) { return 0; }
        //
        //       if (   Opcode::OP_NOP1 == opcode
        //            || Opcode::OP_NOP4 == opcode
        //            || Opcode::OP_NOP5 == opcode
        //            || Opcode::OP_NOP6 == opcode
        //            || Opcode::OP_NOP7 == opcode
        //            || Opcode::OP_NOP9 == opcode
        //            || Opcode::OP_NOP10 == opcode
        //            ) { return 0; }

        return 0;
    } catch (...) {
        return 0;
    }
}

void ScriptElement::setOpcode(Opcode opcode) noexcept
{
    try {
        has_opcode_ = true;
        has_data_ = false;
        if (!data_->empty()) { data_->Release(); }
        opcode_ = opcode;
    } catch (...) {
        return;
    }
}

void ScriptElement::setData(const Data& newData) noexcept
{
    try {
        has_opcode_ = false;
        has_data_ = true;
        opcode_ = Opcode::OP_INVALIDOPCODE;
        data_ = Data::Factory(newData);
    } catch (...) {
        return;
    }
}

ScriptElement::ScriptElement()
    : has_opcode_(false)
    , opcode_(Opcode::OP_INVALIDOPCODE)
    , has_data_(false)
    , data_(Data::Factory())
{
}

ScriptElement::ScriptElement(Opcode opcode)
    : has_opcode_(true)
    , opcode_(opcode)
    , has_data_(false)
    , data_(Data::Factory())
{
}

ScriptElement::ScriptElement(const Data& data)
    : has_opcode_(false)
    , opcode_(Opcode::OP_INVALIDOPCODE)
    , has_data_(true)
    , data_(Data::Factory(data))
{
}

ScriptElement::ScriptElement(const ScriptElement& other)
    : has_opcode_(other.hasOpcode())
    , opcode_(other.getOpcode())
    , has_data_(other.hasData())
    , data_(has_data_ ? Data::Factory(other.getData()) : Data::Factory())
{
}

ScriptElement::ScriptElement(ScriptElement&& other)
    : has_opcode_(false)
    , opcode_(Opcode::OP_INVALIDOPCODE)
    , has_data_(false)
    , data_(Data::Factory(other.data_))
{
    has_opcode_ = other.has_opcode_;
    opcode_ = other.opcode_;
    has_data_ = other.has_data_;
}

ScriptElement& ScriptElement::operator=(const ScriptElement& rhs)
{
    if (this != &rhs) {
        has_opcode_ = rhs.hasOpcode();
        opcode_ = rhs.getOpcode();
        has_data_ = rhs.hasData();

        if (has_data_) {
            data_ = Data::Factory(rhs.getData());
        } else {
            data_->Release();
        }
    }

    return *this;
}

ScriptElement& ScriptElement::operator=(ScriptElement&& rhs)
{
    if (this != &rhs) {
        has_opcode_ = rhs.hasOpcode();
        opcode_ = rhs.getOpcode();
        has_data_ = rhs.hasData();

        if (has_data_) {
            data_ = Data::Factory(rhs.getData());
        } else {
            data_->Release();
        }
    }
    return *this;
}

OTData Output::Encode() const noexcept
{
    try {
        ValueField raw_value(static_cast<Value>(value_));

        auto output = Data::Factory(&raw_value, sizeof(raw_value));

        const auto scriptLength =
            blockchain::bitcoin::CompactSize(pk_script_->size()).Encode();
        output->Concatenate(scriptLength.data(), scriptLength.size());

        output->Concatenate(pk_script_->data(), pk_script_->size());

        return output;
    } catch (...) {
        return Data::Factory();
    }
}

Output::Output()
    : value_(0)
    , pk_script_(Data::Factory())
{
}

Output::Output(const Value& value, const Data& pk_script) noexcept
    : value_(value)
    , pk_script_(Data::Factory(pk_script))
{
}

bool Output::DecodeScriptFromPayload(
    const std::byte*& it,
    std::size_t& expectedSize,
    const std::size_t size,
    bitcoin::Script& result) noexcept
{
    ScriptElement opcode{};
    const bool got_opcode =
        ScriptElement::DecodeOpcodeFromPayload(it, expectedSize, size, opcode);

    if (false == got_opcode) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Opcode missing or incomplete")
            .Flush();

        return false;
    }
    result.push_back(opcode);
    // -----------------------------------------------
    const std::size_t data_size = opcode.expectedDataByteCount();

    if (data_size > 0) {
        ScriptElement data{};
        const bool got_data = ScriptElement::DecodeDataFromPayload(
            it, expectedSize, size, data_size, data);

        if (false == got_data) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Data missing or incomplete")
                .Flush();

            return false;
        }
        result.push_back(data);
    }

    return true;
}

Amount Output::Amount() const { return value_; }

Script Output::getScript() const
{
    auto* it{static_cast<const std::byte*>(pk_script_->data())};
    const std::size_t script_length = pk_script_->size();
    std::size_t expectedSize{};

    bitcoin::Script output{};
    const bool script_decoded = Output::DecodeScriptFromPayload(
        it, expectedSize, script_length, output);
    if (script_decoded) { return output; }

    return bitcoin::Script{};
}

bool Output::DecodeFromPayload(
    const std::byte*& it,
    std::size_t& expectedSize,
    const std::size_t size,
    Output& result) noexcept
{
    ValueField raw_value{};

    expectedSize += sizeof(raw_value);

    if (expectedSize > size) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Value field missing or incomplete")
            .Flush();

        return false;
    }
    // -----------------------------------------------
    OTPassword::safe_memcpy(
        &raw_value, sizeof(raw_value), it, sizeof(raw_value));
    it += sizeof(raw_value);

    const Value value = raw_value.value();
    // -----------------------------------------------
    expectedSize += sizeof(std::byte);

    if (expectedSize > size) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Size below minimum for pk script length field")
            .Flush();

        return false;
    }
    // ----------------------------------------------------------
    auto scriptLength = std::size_t{0};
    const bool decodedScriptLength =
        blockchain::bitcoin::DecodeCompactSizeFromPayload(
            it, expectedSize, size, scriptLength);

    if (!decodedScriptLength) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": CompactSize incomplete for pk script length field")
            .Flush();

        return false;
    }
    if (scriptLength < 1) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": pk script length field cannot be zero")
            .Flush();

        return false;
    }
    // -----------------------------------------------
    expectedSize += scriptLength;

    if (expectedSize > size) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": pk script field missing or incomplete")
            .Flush();

        return false;
    }

    const auto pk_script =
        Data::Factory(reinterpret_cast<const unsigned char*>(it), scriptLength);
    it += scriptLength;
    // -----------------------------------------------
    result = Output(value, pk_script);

    return true;
}

}  // namespace opentxs::blockchain::transaction::bitcoin
