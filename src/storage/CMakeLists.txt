# Copyright (c) 2019 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_subdirectory(drivers)
add_subdirectory(tree)

set(cxx-sources Plugin.cpp)
set(cxx-install-headers "")
set(cxx-header ${cxx-install-headers} Plugin.hpp StorageConfig.hpp)

add_library(opentxs-storage OBJECT ${cxx-sources} ${cxx-headers})
set_property(TARGET opentxs-storage PROPERTY POSITION_INDEPENDENT_CODE 1)
