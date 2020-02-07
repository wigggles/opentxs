// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_CONTACT_HPP
#define OPENTXS_UI_CONTACT_HPP

#ifndef Q_MOC_RUN

#include "opentxs/Forward.hpp"

#include "opentxs/ui/List.hpp"
#include "opentxs/Proto.hpp"

#include <string>

#ifdef SWIG
// clang-format off
%rename(UIContact) opentxs::ui::Contact;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace ui
{
namespace implementation
{
class Contact;
}  // namespace implementation

class Contact : virtual public List
{
public:
    OPENTXS_EXPORT virtual std::string ContactID() const noexcept = 0;
    OPENTXS_EXPORT virtual std::string DisplayName() const noexcept = 0;
    OPENTXS_EXPORT virtual opentxs::SharedPimpl<opentxs::ui::ContactSection>
    First() const noexcept = 0;
    OPENTXS_EXPORT virtual opentxs::SharedPimpl<opentxs::ui::ContactSection>
    Next() const noexcept = 0;
    OPENTXS_EXPORT virtual std::string PaymentCode() const noexcept = 0;

    OPENTXS_EXPORT ~Contact() override = default;

protected:
    Contact() noexcept = default;

private:
    Contact(const Contact&) = delete;
    Contact(Contact&&) = delete;
    Contact& operator=(const Contact&) = delete;
    Contact& operator=(Contact&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif

#if OT_QT || defined(Q_MOC_RUN)
class opentxs::ui::ContactQt final : public QIdentityProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString displayName READ displayName NOTIFY updated)
    Q_PROPERTY(QString contactID READ contactID NOTIFY updated)
    Q_PROPERTY(QString paymentCode READ paymentCode NOTIFY updated)

signals:
    void updated() const;

public:
    // Tree layout
    OPENTXS_EXPORT QString displayName() const noexcept;
    OPENTXS_EXPORT QString contactID() const noexcept;
    OPENTXS_EXPORT QString paymentCode() const noexcept;

    ~ContactQt() final = default;

private:
    friend opentxs::Factory;

    implementation::Contact& parent_;

    void notify() const noexcept;

    ContactQt(implementation::Contact& parent) noexcept;
    ContactQt() = delete;
    ContactQt(const ContactQt&) = delete;
    ContactQt(ContactQt&&) = delete;
    ContactQt& operator=(const ContactQt&) = delete;
    ContactQt& operator=(ContactQt&&) = delete;
};
#endif
#endif
