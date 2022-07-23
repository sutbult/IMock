#pragma once

#include <internal/InnerMock.hpp>
#include <internal/MockWithMethodCase.hpp>
#include <Method.hpp>
#include <MockCaseID.hpp>
#include <MockWithArguments.hpp>

namespace IMock {

/// A Mock with an associated method to add a mock case for.
///
/// @tparam TInterface The interface that the method belongs to.
/// @tparam id The MockWithID used to identify the mock case.
/// @tparam TReturn The return type of the method.
/// @tparam TArguments The types of the arguments to the method.
template <typename TInterface, MockCaseID id, typename TReturn,
    typename ...TArguments>
class MockWithMethod {
    private:
        /// The InnerMock to add a mock case to.
        Internal::InnerMock<TInterface>& _mock;

        /// The method to add a mock case to.
        Method<TInterface, TReturn, TArguments...> _method;

        /// A string describing how a call is made to the method being mocked.
        std::string _methodString;

    public:
        /// Creates a MockWithMethod.
        ///
        /// @param mock The InnerMock to add a mock case to.
        /// @param method The method to add a mock case to.
        /// @param methodString A string describing how a call is made to the
        /// method being mocked.
        MockWithMethod(
            Internal::InnerMock<TInterface>& mock,
            Method<TInterface, TReturn, TArguments...> method,
            std::string methodString)
            : _mock(mock)
            , _method(std::move(method))
            , _methodString(std::move(methodString)) {
        }

        /// Creates a MockWithArguments used to add a mock case matching the
        /// provided arguments.
        ///
        /// @param arguments The arguments to match.
        /// @return A MockWithArguments associated with the arguments.
        MockWithArguments<TInterface, id, TReturn, TArguments...> with(
            TArguments... arguments) {
            // Create and return a MockWithArguments with the InnerMock,
            // the method, the call string and the arguments.
            return MockWithArguments<TInterface, id, TReturn, TArguments...>(
                _mock,
                _method,
                _methodString,
                std::tuple<TArguments...>(
                    std::forward<TArguments>(arguments)...));
        }

        /// Adds a fake handling the method call.
        ///
        /// @param fake A callback to call when the method is called.
        /// @return A CallCount that can be queried about the number of calls
        /// done to the added mock case.
        CallCount fake(std::function<TReturn (TArguments...)> fake) {
            // Create a MockWithMethodCase.
            std::unique_ptr<Internal::ICase<TReturn, TArguments...>> mockCase
                = Internal::makeUnique<Internal::MockWithMethodCase<
                    TReturn, TArguments...>>(fake);

            // Add the case to InnerMock.
            return _mock.template addCase<id, TReturn, TArguments...>(
                _method,
                std::move(_methodString),
                std::move(mockCase));
        }
};

}
