#pragma once

#include <internal/InnerMock.hpp>
#include <MockCaseID.hpp>
#include <MockWithMethod.hpp>

namespace IMock {

/// A Mock with an associated MockCaseID used to add a mock case.
template <typename TInterface, MockCaseID id>
class MockWithID {
    private:
        /// The InnerMock to add a mock case to.
        Internal::InnerMock<TInterface>& _mock;

    public:
        /// Creates a MockWithID.
        ///
        /// @param mock The InnerMock to add a mock case to.
        MockWithID(Internal::InnerMock<TInterface>& mock)
            : _mock(mock) {
        }

        /// Creates a MockWithMethod used to add a mock case to the provided
        /// method.
        ///
        /// @param method The method to add mock cases for.
        /// @param methodString A string describing how a call is made to the
        /// method being mocked.
        /// @return A MockWithMethod associated with the method.
        template <typename TReturn, typename ...TArguments>
        MockWithMethod<TInterface, id, TReturn, TArguments...> withMethod(
            TReturn (TInterface::*method)(TArguments...),
            std::string methodString) {
            // Create and return a MockWithMethod with the InnerMock,
            // the method and the call string.
            return MockWithMethod<TInterface, id, TReturn, TArguments...>(
                _mock,
                method,
                std::move(methodString));
        }
};

}
