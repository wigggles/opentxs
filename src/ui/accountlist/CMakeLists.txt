# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-sources AccountList.cpp AccountListItem.cpp)
set(ot-qobject-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/AccountList.hpp"
)
set(cxx-install-headers
    ${ot-qobject-install-headers}
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/AccountListItem.hpp"
)
set(cxx-headers ${cxx-install-headers} AccountList.hpp AccountListItem.hpp)

if(OT_BLOCKCHAIN_EXPORT)
  list(APPEND cxx-sources BlockchainAccountListItem.cpp)
  list(APPEND cxx-headers BlockchainAccountListItem.hpp)
endif()

if(OT_WITH_QT)
  qt5_wrap_cpp(MOC_SRCS ${ot-qobject-install-headers})
else()
  set(MOC_SRCS "")
endif()

add_library(
  opentxs-ui-accountlist OBJECT ${cxx-sources} ${cxx-headers} ${MOC_SRCS}
)
target_link_libraries(opentxs-ui-accountlist PRIVATE opentxs::messages)
target_include_directories(
  opentxs-ui-accountlist SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)

if(OT_QT_EXPORT)
  target_compile_definitions(
    opentxs-ui-accountlist PRIVATE ${Qt5Core_DEFINITIONS}
  )
  target_compile_definitions(
    opentxs-ui-accountlist PRIVATE ${Qt5Gui_DEFINITIONS}
  )
  target_compile_definitions(
    opentxs-ui-accountlist PRIVATE ${Qt5Widgets_DEFINITIONS}
  )

  if(OT_QML_EXPORT)
    target_compile_definitions(
      opentxs-ui-accountlist PRIVATE ${Qt5Qml_DEFINITIONS}
    )
    target_compile_definitions(opentxs-ui-accountlist PRIVATE -DOT_WITH_QML=1)
  else()
    target_compile_definitions(opentxs-ui-accountlist PRIVATE -DOT_WITH_QML=0)
  endif()
else()
  target_compile_definitions(opentxs-ui-accountlist PRIVATE -DOT_WITH_QML=0)
endif()

add_dependencies(opentxs-ui-accountlist generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/ui
    COMPONENT dev
  )
endif()
