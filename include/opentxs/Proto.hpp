// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_PROTO_HPP
#define OPENTXS_PROTO_HPP

// IWYU pragma: begin_exports
#include <opentxs-proto/Types.hpp>
#include <opentxs-proto/Check.hpp>
// IWYU pragma: end_exports

#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Data.hpp"

#include <iostream>

namespace opentxs
{
static const proto::HashType StandardHash{proto::HASHTYPE_BLAKE2B256};

namespace proto
{
template <class T>
OTData ProtoAsData(const T& serialized)
{
    auto size = serialized.ByteSize();
    char* protoArray = new char[size];

    OT_ASSERT_MSG(
        nullptr != protoArray, "protoArray failed to dynamically allocate.");

    serialized.SerializeToArray(protoArray, size);
    auto serializedData = Data::Factory(protoArray, size);
    delete[] protoArray;

    return serializedData;
}

template <class T>
std::string ProtoAsString(const T& serialized)
{
    auto size = serialized.ByteSize();
    char* protoArray = new char[size];

    OT_ASSERT_MSG(
        nullptr != protoArray, "protoArray failed to dynamically allocate.");

    serialized.SerializeToArray(protoArray, size);
    std::string serializedData(protoArray, size);
    delete[] protoArray;

    return serializedData;
}

template <class T>
OTString ProtoAsArmored(const T& serialized, const String& header)
{
    auto data = ProtoAsData<T>(serialized);
    Armored armored(data);
    auto output = String::Factory();
    armored.WriteArmoredString(output, header.Get());

    return output;
}

template <class T>
T RawToProto(const void* input, const std::size_t size)
{
    T serialized;
    serialized.ParseFromArray(input, size);

    return serialized;
}

template <class T>
T TextToProto(const std::string& input)
{
    return RawToProto<T>(input.data(), input.size());
}

template <class T>
T DataToProto(const Data& input)
{
    return RawToProto<T>(static_cast<const char*>(input.data()), input.size());
}

template <class T>
T StringToProto(const String& input)
{
    Armored armored;
    OTString unconstInput = String::Factory(input.Get());

    if (!armored.LoadFromString(unconstInput)) {
        std::cerr << __FUNCTION__ << "Failed to decode armored protobuf."
                  << std::endl;

        return T();
    } else {

        return DataToProto<T>(Data::Factory((armored)));
    }
}
}  // namespace proto
}  // namespace opentxs
#endif
