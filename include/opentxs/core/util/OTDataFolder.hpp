// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_OTDATAFOLDER_HPP
#define OPENTXS_CORE_OTDATAFOLDER_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/String.hpp"

namespace opentxs
{

// cppcheck-suppress noConstructor
class OTDataFolder
{
private:
    static OTDataFolder* pDataFolder;

    bool m_bInitialized;

    String m_strDataFolderPath;
    String m_strDataConfigFilePath;

public:
    EXPORT static bool Init(const String& strThreadContext);

    EXPORT static bool IsInitialized();

    EXPORT static bool Cleanup();

    EXPORT static String Get();
    EXPORT static bool Get(String& strDataFolder);

    EXPORT static bool GetConfigFilePath(String& strConfigFilePath);
};

}  // namespace opentxs

#endif  // OPENTXS_CORE_OTDATAFOLDER_HPP
