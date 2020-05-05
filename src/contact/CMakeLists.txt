# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(
  cxx-sources
  Contact.cpp
  ContactData.cpp
  ContactGroup.cpp
  ContactItem.cpp
  ContactSection.cpp
)
set(
  cxx-install-headers
  "${opentxs_SOURCE_DIR}/include/opentxs/contact/Contact.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/contact/ContactData.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/contact/ContactGroup.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/contact/ContactItem.hpp"
  "${opentxs_SOURCE_DIR}/include/opentxs/contact/ContactSection.hpp"
)
set(cxx-headers ${cxx-install-headers})

add_library(opentxs-contact OBJECT ${cxx-sources} ${cxx-headers})
target_link_libraries(opentxs-contact PRIVATE opentxs::messages)
set_property(TARGET opentxs-contact PROPERTY POSITION_INDEPENDENT_CODE 1)

install(
  FILES ${cxx-install-headers}
  DESTINATION include/opentxs/contact
  COMPONENT dev
)
