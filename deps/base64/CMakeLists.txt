# Copyright (c) 2010-2019 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(c-sources base64.c)
set(c-headers base64.h)

add_library(base64 OBJECT ${c-sources} ${c-headers})

set_property(TARGET base64 PROPERTY POSITION_INDEPENDENT_CODE 1)
