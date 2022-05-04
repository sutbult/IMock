#pragma once

#include <tuple>

#include <internal/CaseMatch.hpp>

namespace IMock {
namespace Internal {

/// Interface for a mocked case.
template <typename TReturn, typename ...Arguments>
class ICase {
    public:
        /// Virtual destructor of ICase.
        virtual ~ICase() noexcept {
        }

        /// Checks if the provided arguments matches the case.
        ///
        /// @param arguments The arguments the mocked method was called with.
        /// The arguments will never be used again if the function indicates a
        /// match, which means the values can safely be moved.
        /// @return A CaseMatch indicating if the arguments resulted in a match.
        virtual CaseMatch<TReturn> matches(std::tuple<Arguments...>& arguments)
            = 0;
};

}
}
