# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(
  cxx-sources
  ConfigLoader.cpp
  MainFile.cpp
  MessageProcessor.cpp
  Notary.cpp
  PayDividendVisitor.cpp
  ReplyMessage.cpp
  Server.cpp
  ServerSettings.cpp
  Transactor.cpp
  UserCommandProcessor.cpp
)
set(cxx-install-headers "")
set(
  cxx-headers
  ${cxx-install-headers}
  ConfigLoader.hpp
  Macros.hpp
  MainFile.hpp
  MessageProcessor.hpp
  Notary.hpp
  PayDividendVisitor.hpp
  ReplyMessage.hpp
  Server.hpp
  ServerSettings.hpp
  Transactor.hpp
  UserCommandProcessor.hpp
)

add_library(opentxs-server OBJECT ${cxx-sources} ${cxx-headers})
target_include_directories(
  opentxs-server SYSTEM
  PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)
set_property(TARGET opentxs-server PROPERTY POSITION_INDEPENDENT_CODE 1)
