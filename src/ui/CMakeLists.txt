set(cxx-sources
  ActivityThread.cpp
  ActivityThreadItem.cpp
  ActivitySummary.cpp
  ActivitySummaryItem.cpp
  ContactList.cpp
  ContactListItem.cpp
  MailItem.cpp
  MessagableList.cpp
  PayableList.cpp
  PayableListItem.cpp
  Widget.cpp
)

set(cxx-install-headers
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/ui/ActivityThread.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/ui/ActivityThreadItem.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/ui/ActivitySummary.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/ui/ActivitySummaryItem.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/ui/ContactList.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/ui/ContactListItem.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/ui/ListRow.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/ui/MessagableList.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/ui/PayableList.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/ui/PayableListItem.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/opentxs/ui/Widget.hpp"
)

set(cxx-headers
  ${cxx-install-headers}
  "${CMAKE_CURRENT_SOURCE_DIR}/ActivityThread.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ActivityThreadItem.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ActivityThreadItemBlank.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ActivitySummary.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ActivitySummaryItem.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ActivitySummaryItemBlank.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ContactList.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ContactListItem.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/ContactListItemBlank.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/MailItem.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/MessagableList.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/PayableList.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/PayableListItem.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Widget.hpp"
)

set(MODULE_NAME opentxs-ui)

if(WIN32)
  # suppress warnings about exported internal symbols (global log stream objects)
  add_definitions(-DOTLOG_IMPORT)

  # suppress linker warnings about locallly defined imported symbols
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4049 /ignore:4217")

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/windll.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/module.rc
    @ONLY
  )

  # we have an extra Windows-specific file to compile
  add_library(${MODULE_NAME} OBJECT
    ${cxx-sources}
    ${cxx-headers}
    ${CMAKE_CURRENT_BINARY_DIR}/module.rc
  )
else()
  add_library(${MODULE_NAME} OBJECT
    ${cxx-sources}
    ${cxx-headers}
  )
endif()

install(FILES ${cxx-install-headers}
        DESTINATION include/opentxs/ui
        COMPONENT dev)

set_property(TARGET ${MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)
set_lib_property(${MODULE_NAME})
