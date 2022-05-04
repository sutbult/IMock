#pragma once

#include <functional>

#include <internal/CaseMatch.hpp>
#include <internal/CaseMatchFactory.hpp>
#include <internal/ICase.hpp>

namespace IMock {
namespace Internal {

/// An ICase always calling a provided callback.
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

        CaseMatch<TReturn> matches(std::tuple<TArguments...>& arguments) {
            // Return a CaseMatch for the fake.
            return Internal::CaseMatchFactory::matchFake(
                _fake,
                std::move(arguments));
        }
};

}
}
