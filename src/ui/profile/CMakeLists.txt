# Copyright (c) 2010-2020 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(cxx-sources
    Profile.cpp
    ProfileItem.cpp
    ProfileSection.cpp
    ProfileSubsection.cpp
)
set(ot-qobject-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/Profile.hpp"
)
set(cxx-install-headers
    ${ot-qobject-install-headers}
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/ProfileItem.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/ProfileSection.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/ui/ProfileSubsection.hpp"
)
set(cxx-headers
    ${cxx-install-headers}
    Profile.hpp
    ProfileItem.hpp
    ProfileSection.hpp
    ProfileSubsection.hpp
)

if(OT_WITH_QT)
  qt5_wrap_cpp(MOC_SRCS ${ot-qobject-install-headers})
else()
  set(MOC_SRCS "")
endif()

add_library(opentxs-ui-profile OBJECT ${cxx-sources} ${cxx-headers} ${MOC_SRCS})
target_link_libraries(opentxs-ui-profile PRIVATE opentxs::messages)
target_include_directories(
  opentxs-ui-profile SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)

if(OT_QT_EXPORT)
  target_compile_definitions(opentxs-ui-profile PRIVATE ${Qt5Core_DEFINITIONS})
  target_compile_definitions(opentxs-ui-profile PRIVATE ${Qt5Gui_DEFINITIONS})
  target_compile_definitions(
    opentxs-ui-profile PRIVATE ${Qt5Widgets_DEFINITIONS}
  )

  if(OT_QML_EXPORT)
    target_compile_definitions(opentxs-ui-profile PRIVATE ${Qt5Qml_DEFINITIONS})
    target_compile_definitions(opentxs-ui-profile PRIVATE -DOT_WITH_QML=1)
  else()
    target_compile_definitions(opentxs-ui-profile PRIVATE -DOT_WITH_QML=0)
  endif()
else()
  target_compile_definitions(opentxs-ui-profile PRIVATE -DOT_WITH_QML=0)
endif()

add_dependencies(opentxs-ui-profile generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/ui
    COMPONENT dev
  )
endif()
