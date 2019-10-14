// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_QT_HPP
#define OPENTXS_QT_HPP

#include "opentxs/Forward.hpp"

#if OT_QT
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#pragma GCC diagnostic ignored "-Wdeprecated-copy"

#ifndef Q_MOC_RUN

#include <QtCore/QObject>
#include <QtCore/QAbstractItemModel>
#include <QtCore/QDateTime>
#include <QtQml/QQmlEngine>

#endif

#pragma GCC diagnostic pop
#endif
#endif
