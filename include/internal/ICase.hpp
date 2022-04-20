#pragma once

#include <tuple>

#include <internal/CaseMatch.hpp>

namespace IMock::Internal {

template <typename TReturn, typename ...Arguments>
class ICase {
    public:
        virtual ~ICase() noexcept {
        }

        virtual CaseMatch<TReturn> matches(std::tuple<Arguments...>& arguments)
            = 0;
};

}
