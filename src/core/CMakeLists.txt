# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(contract)
add_subdirectory(cron)
add_subdirectory(crypto)
add_subdirectory(identifier)
add_subdirectory(otprotob)
add_subdirectory(recurring)
add_subdirectory(script)
add_subdirectory(trade)
add_subdirectory(transaction)
add_subdirectory(util)

set(cxx-sources
    Account.cpp
    AccountList.cpp
    AccountVisitor.cpp
    Armored.cpp
    Cheque.cpp
    Contract.cpp
    Data.cpp
    Flag.cpp
    Identifier.cpp
    Instrument.cpp
    Item.cpp
    Ledger.cpp
    Log.cpp
    LogSource.cpp
    Message.cpp
    NumList.cpp
    NymFile.cpp
    OTStorage.cpp
    OTTrackable.cpp
    OTTransaction.cpp
    OTTransactionType.cpp
    PasswordPrompt.cpp
    Secret.cpp
    Shutdown.cpp
    StateMachine.cpp
    String.cpp
    StringXML.cpp
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/core/Account.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/AccountList.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/AccountVisitor.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/Armored.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/Cheque.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/Contract.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/Data.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/Flag.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/Helpers.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/Identifier.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/Instrument.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/Item.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/Ledger.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/Lockable.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/Log.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/LogSource.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/Message.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/NumList.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/NymFile.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/OTStorage.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/OTStoragePB.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/OTTrackable.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/OTTransaction.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/OTTransactionType.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/PasswordPrompt.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/Secret.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/String.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/StringXML.hpp"
)
set(cxx-headers
    "${cxx-install-headers}"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/UniqueQueue.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/identifier/Identifier.hpp"
    "${opentxs_SOURCE_DIR}/src/internal/core/Core.hpp"
    "Armored.hpp"
    "Data.hpp"
    "Flag.hpp"
    "Identifier.hpp"
    "NymFile.hpp"
    "Secret.hpp"
    "Shutdown.hpp"
    "StateMachine.hpp"
    "String.hpp"
    "StringXML.hpp"
    "Worker.hpp"
)

add_library(opentxs-core OBJECT ${cxx-sources} ${cxx-headers})
target_include_directories(
  opentxs-core SYSTEM
  PRIVATE "${ZLIB_INCLUDE_DIRS}" "${CMAKE_CURRENT_BINARY_DIR}/otprotob"
          "${opentxs_SOURCE_DIR}/deps/"
)
target_link_libraries(
  opentxs-core PRIVATE Boost::headers opentxs::messages
                       unofficial-sodium::sodium
)
add_dependencies(opentxs-core otprotob)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/core
    COMPONENT dev
  )
endif()
