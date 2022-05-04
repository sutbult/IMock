#pragma once

#include <functional>

#include <internal/CaseMatch.hpp>
#include <internal/CaseMatchFactory.hpp>
#include <internal/ICase.hpp>

namespace IMock {
namespace Internal {

/// An ICase checking if calls match provided arguments.
template <typename TReturn, typename ...TArguments>
class MockWithArgumentsCase : public ICase<TReturn, TArguments...> {
    private:
        /// The arguments to check calls with.
        std::tuple<TArguments...> _arguments;

        /// A callback to call if the arguments match.
        std::function<CaseMatch<TReturn> (std::tuple<TArguments...>)> _fake;

    public:
        /// Creates a MockWithArgumentsCase.
        ///
        /// @param arguments The arguments to check calls with.
        /// @param fake A callback to call if the arguments match.
        MockWithArgumentsCase(
            std::tuple<TArguments...> arguments,
            std::function<CaseMatch<TReturn> (std::tuple<TArguments...>)> fake)
            : _arguments(std::move(arguments))
            , _fake(std::move(fake)) {
            }

        CaseMatch<TReturn> matches(std::tuple<TArguments...>& arguments) {
            // Check if the call arguments matches the mock case's arguments.
            if(arguments == _arguments) {
                // Call _fake and return its return value if a match has been
                // made. The arguments are moved to _fake as they will never be
                // read again.
                return _fake(std::move(arguments));
            }
            else {
                // Return a CaseMatch indicating no match has been made.
                return CaseMatchFactory::noMatch<TReturn>();
            }
        }
};

}
}
