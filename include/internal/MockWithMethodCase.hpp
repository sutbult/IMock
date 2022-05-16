#pragma once

#include <functional>

#include <internal/CaseMatch.hpp>
#include <internal/CaseMatchFactory.hpp>
#include <internal/ICase.hpp>

namespace IMock {
namespace Internal {

/// An ICase always calling a provided callback.
///
/// @tparam TReturn The return type of the mocked method.
/// @tparam TArguments The types of the arguments of the mocked method.
template <typename TReturn, typename ...TArguments>
class MockWithMethodCase : public ICase<TReturn, TArguments...> {
    private:
        /// A callback to be called.
        std::function<TReturn (TArguments...)> _fake;

    public:
        /// Creates a MockWithMethodCase.
        ///
        /// @param fake A callback to be called.
        MockWithMethodCase(
            std::function<TReturn (TArguments...)> fake)
            : _fake(std::move(fake)) {
            }

        /// Always matches the arguments.
        ///
        /// @param arguments The arguments the mocked method was called with,
        /// which will be moved to _fake.
        /// @return A CaseMatch indicating a match using the return value from
        /// _fake.
        CaseMatch<TReturn> matches(std::tuple<TArguments...>& arguments) {
            // Return a CaseMatch for the fake.
            return Internal::CaseMatchFactory::matchFake(
                _fake,
                std::move(arguments));
        }
};

}
}
