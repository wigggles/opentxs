// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_BLOCKCHAINSELECTION_HPP
#define OPENTXS_UI_BLOCKCHAINSELECTION_HPP

#ifndef Q_MOC_RUN

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/ui/List.hpp"

#ifdef SWIG
// clang-format off
%rename(UIBlockchainSelection) opentxs::ui::BlockchainSelection;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace ui
{
namespace implementation
{
class BlockchainSelection;
}  // namespace implementation

class BlockchainSelection;
class BlockchainSelectionItem;

#if OT_QT
class BlockchainSelectionQt;
#endif
}  // namespace ui
}  // namespace opentxs

namespace opentxs
{
namespace ui
{
class BlockchainSelection : virtual public List
{
public:
    OPENTXS_EXPORT virtual opentxs::SharedPimpl<
        opentxs::ui::BlockchainSelectionItem>
    First() const noexcept = 0;
    OPENTXS_EXPORT virtual opentxs::SharedPimpl<
        opentxs::ui::BlockchainSelectionItem>
    Next() const noexcept = 0;

    OPENTXS_EXPORT virtual bool Disable(
        const blockchain::Type type) const noexcept = 0;
    OPENTXS_EXPORT virtual bool Enable(
        const blockchain::Type type) const noexcept = 0;
    OPENTXS_EXPORT virtual bool Toggle(
        const blockchain::Type type) const noexcept = 0;

    OPENTXS_EXPORT ~BlockchainSelection() override = default;

protected:
    BlockchainSelection() noexcept = default;

private:
    BlockchainSelection(const BlockchainSelection&) = delete;
    BlockchainSelection(BlockchainSelection&&) = delete;
    BlockchainSelection& operator=(const BlockchainSelection&) = delete;
    BlockchainSelection& operator=(BlockchainSelection&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

#if OT_QT || defined(Q_MOC_RUN)
class opentxs::ui::BlockchainSelectionQt final : public QIdentityProxyModel
{
    Q_OBJECT

signals:
    void updated() const;

public:
    // User roles return the same data for all columns
    //
    // TypeRole: int (blockchain::Type)
    //
    // Qt::DisplayRole, NameColumn: QString
    // Qt::DisplayRole, EnabledColumn: bool
    // Qt::DisplayRole, TestnetColumn: bool

    enum Columns {
        NameColumn = 0,
        EnabledColumn = 1,
        TestnetColumn = 2,
    };
    enum Roles {
        TypeRole = Qt::UserRole + 0,
    };

    OPENTXS_EXPORT Q_INVOKABLE bool disableChain(
        const int chain) const noexcept;
    OPENTXS_EXPORT Q_INVOKABLE bool enableChain(const int chain) const noexcept;
    OPENTXS_EXPORT Q_INVOKABLE bool toggleChain(const int chain) const noexcept;

    BlockchainSelectionQt(implementation::BlockchainSelection& parent) noexcept;

    ~BlockchainSelectionQt() final = default;

private:
    implementation::BlockchainSelection& parent_;

    void notify() const noexcept;

    BlockchainSelectionQt(const BlockchainSelectionQt&) = delete;
    BlockchainSelectionQt(BlockchainSelectionQt&&) = delete;
    BlockchainSelectionQt& operator=(const BlockchainSelectionQt&) = delete;
    BlockchainSelectionQt& operator=(BlockchainSelectionQt&&) = delete;
};
#endif
#endif
