// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_SHAREDPIMPL_HPP
#define OPENTXS_SHAREDPIMPL_HPP

#include <cassert>
#include <memory>

#ifdef SWIG
%ignore opentxs::SharedPimpl::SharedPimpl(const std::shared_ptr<const C>&);
%ignore opentxs::SharedPimpl::SharedPimpl(SharedPimpl&&);
%ignore opentxs::SharedPimpl::operator const C&();
%rename(assign) opentxs::SharedPimpl::operator=(const SharedPimpl&);
%rename(move) opentxs::SharedPimpl::operator=(SharedPimpl&&);
#endif

namespace opentxs
{
template <class C>
class SharedPimpl
{
public:
    explicit SharedPimpl(const std::shared_ptr<const C>& in) noexcept
        : pimpl_(in)
    {
        assert(pimpl_);
    }
    SharedPimpl(const SharedPimpl& rhs) noexcept = default;
    SharedPimpl(SharedPimpl&& rhs) noexcept = default;
    SharedPimpl& operator=(const SharedPimpl& rhs) noexcept = default;
    SharedPimpl& operator=(SharedPimpl&& rhs) noexcept = default;

#ifndef SWIG
    operator const C&() const noexcept { return *pimpl_; }
#endif

    const C* operator->() const { return pimpl_.get(); }

    const C& get() const noexcept { return *pimpl_; }

    ~SharedPimpl() = default;

#ifdef SWIG_VERSION
    SharedPimpl() = default;
#endif

private:
    std::shared_ptr<const C> pimpl_{nullptr};

#ifndef SWIG_VERSION
    SharedPimpl() = delete;
#endif
};  // class SharedPimpl
}  // namespace opentxs

#endif
