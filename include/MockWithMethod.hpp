#pragma once

#include <internal/InnerMock.hpp>
#include <MockCaseID.hpp>
#include <MockWithArguments.hpp>

namespace IMock {

/// A Mock with an associated method to add a mock case for.
template <typename TInterface, MockCaseID id, typename TReturn,
    typename ...TArguments>
class MockWithMethod {
    private:
        /// The InnerMock to add a mock case to.
        Internal::InnerMock<TInterface>& _mock;

        /// The method to add a mock case to.
        TReturn (TInterface::*_method)(TArguments...);

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
            TReturn (TInterface::*method)(TArguments...),
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
                std::tuple<TArguments...>(std::move(arguments)...));
        }

        // TODO: Add a method for adding a mock case with a fake.
};

}
