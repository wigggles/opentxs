# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(OT_WITH_QT)
  add_definitions(${Qt5Core_DEFINITIONS})
  add_definitions(${Qt5Gui_DEFINITIONS})
  add_definitions(${Qt5Widgets_DEFINITIONS})
  add_definitions(${Qt5Qml_DEFINITIONS})
endif()

set(
  cxx-sources
  AccountActivity.cpp
  AccountList.cpp
  AccountListItem.cpp
  AccountSummary.cpp
  AccountSummaryItem.cpp
  ActivitySummary.cpp
  ActivitySummaryItem.cpp
  ActivityThread.cpp
  ActivityThreadItem.cpp
  BalanceItem.cpp
  BlockchainAccountListItem.cpp
  ChequeBalanceItem.cpp
  Contact.cpp
  ContactItem.cpp
  ContactList.cpp
  ContactListItem.cpp
  ContactSection.cpp
  ContactSubsection.cpp
  IssuerItem.cpp
  MailItem.cpp
  MessagableList.cpp
  PayableList.cpp
  PayableListItem.cpp
  PaymentItem.cpp
  PendingSend.cpp
  Profile.cpp
  ProfileItem.cpp
  ProfileSection.cpp
  ProfileSubsection.cpp
  TransferBalanceItem.cpp
  UI.cpp
  UnitList.cpp
  UnitListItem.cpp
  Widget.cpp
)
set(
  ot-qobject-install-headers
  "${opentxs_SOURCE_DIR}/include/opentxs/ui/AccountActivity.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/ui/AccountList.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/ui/AccountSummary.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/ui/ActivitySummary.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/ui/ActivityThread.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/ui/Contact.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/ui/ContactList.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/ui/List.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/ui/MessagableList.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/ui/PayableList.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/ui/Profile.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/ui/UnitList.hpp"
)
set(
  cxx-install-headers
  ${ot-qobject-install-headers}
  "${opentxs_SOURCE_DIR}/include/opentxs/ui/AccountListItem.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/ui/AccountSummaryItem.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/ui/ActivitySummaryItem.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/ui/ActivityThreadItem.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/ui/BalanceItem.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/ui/ContactItem.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/ui/ContactListItem.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/ui/ContactSection.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/ui/ContactSubsection.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/ui/IssuerItem.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/ui/ListRow.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/ui/PayableListItem.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/ui/ProfileItem.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/ui/ProfileSection.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/ui/ProfileSubsection.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/ui/UnitListItem.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/ui/Widget.hpp"
)
set(
  cxx-headers
  ${cxx-install-headers}
  "${opentxs_SOURCE_DIR}/src/internal/ui/UI.hpp"
  AccountActivity.hpp
  AccountList.hpp
  AccountListItem.hpp
  AccountSummary.hpp
  AccountSummaryItem.hpp
  ActivitySummary.hpp
  ActivitySummaryItem.hpp
  ActivityThread.hpp
  ActivityThreadItem.hpp
  BalanceItem.hpp
  Combined.hpp
  Contact.hpp
  ContactItem.hpp
  ContactList.hpp
  ContactListItem.hpp
  ContactSection.hpp
  ContactSubsection.hpp
  IssuerItem.hpp
  List.hpp
  MailItem.hpp
  MessagableList.hpp
  PayableList.hpp
  PayableListItem.hpp
  PendingSend.hpp
  Profile.hpp
  ProfileItem.hpp
  ProfileSection.hpp
  ProfileSubsection.hpp
  Row.hpp
  RowType.hpp
  UnitList.hpp
  UnitListItem.hpp
  Widget.hpp
)

if(OT_WITH_QT)
  qt5_wrap_cpp(MOC_SRCS ${ot-qobject-install-headers})
else()
  set(MOC_SRCS "")
endif()

add_library(opentxs-ui OBJECT ${cxx-sources} ${cxx-headers} ${MOC_SRCS})
target_link_libraries(opentxs-ui PRIVATE opentxs::messages)
set_property(TARGET opentxs-ui PROPERTY POSITION_INDEPENDENT_CODE 1)

install(
  FILES ${cxx-install-headers}
  DESTINATION include/opentxs/ui
  COMPONENT dev
)
