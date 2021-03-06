#pragma once

#include <internal/InnerMock.hpp>
#include <Method.hpp>
#include <MockCaseID.hpp>
#include <MockWithMethod.hpp>

namespace IMock {

/// A Mock with an associated MockCaseID used to add a mock case.
///
/// @tparam TInterface The interface that the mocked method belongs to.
/// @tparam id The MockWithID used to identify the mock case to add.
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
        /// @tparam TReturn The return type of the method.
        /// @tparam TArguments The types of the arguments to the method.
        template <typename TReturn, typename ...TArguments>
        MockWithMethod<TInterface, id, TReturn, TArguments...> withMethod(
            Method<TInterface, TReturn, TArguments...> method,
            std::string methodString) const {
            // Create and return a MockWithMethod with the InnerMock,
            // the method and the call string.
            return MockWithMethod<TInterface, id, TReturn, TArguments...>(
                _mock,
                method,
                std::move(methodString));
        }

        /// Creates a MockWithMethod used to add a mock case to the provided
        /// constant method.
        ///
        /// @param method The constant method to add mock cases for.
        /// @param methodString A string describing how a call is made to the
        /// constant method being mocked.
        /// @return A MockWithMethod associated with the constant method.
        /// @tparam TReturn The return type of the constant method.
        /// @tparam TArguments The types of the arguments to the constant
        /// method.
        template <typename TReturn, typename ...TArguments>
        MockWithMethod<TInterface, id, TReturn, TArguments...> withMethod(
            TReturn (TInterface::*method)(TArguments...) const,
            std::string methodString) const {
            // Cast the constant method to a regular method and forward the call
            // to the regular withMethod.
            return withMethod(
                reinterpret_cast<Method<TInterface, TReturn, TArguments...>>(
                    method),
                std::move(methodString));
        }
};

}
