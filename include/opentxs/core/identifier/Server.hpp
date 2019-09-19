// Copyright (c) 2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_IDENTIFIER_SERVER_HPP
#define OPENTXS_CORE_IDENTIFIER_SERVER_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/Identifier.hpp"

#ifdef SWIG
// clang-format off
%ignore opentxs::identifier::Server::Factory;
%extend opentxs::identifier::Server {
    static OTServerID Factory()
    {
        return opentxs::identifier::Server::Factory();
    }
    static OTServerID Factory(
        const std::string& rhs)
    {
        return opentxs::identifier::Server::Factory(rhs);
    }
}
%rename (ServerID) opentxs::identifier::Server;
%template(OTServerID) opentxs::Pimpl<opentxs::identifier::Server>;
// clang-format on
#endif

namespace opentxs
{
#ifndef SWIG
bool operator==(
    const opentxs::Pimpl<opentxs::identifier::Server>& lhs,
    const opentxs::Identifier& rhs);
bool operator!=(
    const opentxs::Pimpl<opentxs::identifier::Server>& lhs,
    const opentxs::Identifier& rhs);
bool operator<(
    const opentxs::Pimpl<opentxs::identifier::Server>& lhs,
    const opentxs::Identifier& rhs);
bool operator>(
    const opentxs::Pimpl<opentxs::identifier::Server>& lhs,
    const opentxs::Identifier& rhs);
bool operator<=(
    const opentxs::Pimpl<opentxs::identifier::Server>& lhs,
    const opentxs::Identifier& rhs);
bool operator>=(
    const opentxs::Pimpl<opentxs::identifier::Server>& lhs,
    const opentxs::Identifier& rhs);
#endif

namespace identifier
{
class Server : virtual public opentxs::Identifier
{
public:
#ifndef SWIG
    EXPORT static OTServerID Factory();
    EXPORT static OTServerID Factory(const std::string& rhs);
    EXPORT static OTServerID Factory(const String& rhs);
#endif

    EXPORT virtual ~Server() = default;

protected:
    Server() = default;

private:
    friend OTServerID;

    virtual Server* clone() const = 0;

    Server(const Server&) = delete;
    Server(Server&&) = delete;
    Server& operator=(const Server&) = delete;
    Server& operator=(Server&&) = delete;
};
}  // namespace identifier
}  // namespace opentxs
#endif
