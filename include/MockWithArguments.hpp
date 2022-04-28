#pragma once

#include <tuple>
#include <utility>

#include <internal/Apply.hpp>
#include <internal/CaseMatchFactory.hpp>
#include <internal/InnerMock.hpp>
#include <internal/MockWithArguments.hpp>
#include <internal/make_unique.hpp>
#include <MockCaseCallCount.hpp>
#include <MockCaseID.hpp>

namespace IMock {

/// A Mock with an associated method and arguments to add a mock case for.
template <typename TInterface, MockCaseID id, typename TReturn,
    typename ...TArguments>
class MockWithArguments {
    private:
        /// The InnerMock to add a mock case to.
        Internal::InnerMock<TInterface>* _mock;

        /// The method to add a mock case to.
        TReturn (TInterface::*_method)(TArguments...);

        /// The arguments to match calls with.
        std::tuple<TArguments...> _arguments;

    public:
        /// Creates a MockWithArguments.
        ///
        /// @param mock The InnerMock to add a mock case to.
        /// @param method The method to add a mock case to.
        /// @param arguments The arguments to match calls with.
        MockWithArguments(
            Internal::InnerMock<TInterface>* mock,
            TReturn (TInterface::*method)(TArguments...),
            std::tuple<TArguments...> arguments)
            : _mock(std::move(mock))
            , _method(std::move(method))
            , _arguments(std::move(arguments)) {
        }

        // The solution for dealing with void has been taken from:
        // https://eli.thegreenplace.net/2014/sfinae-and-enable_if/

        /// Adds a mock case making the associated method return the provided
        /// value when called with the associated arguments.
        ///
        /// The method is available unless the return type is void. 
        ///
        /// @param returnValue The value to return when a match happens.
        /// @return A MockCaseCallCount that can be queried about the number of
        /// calls done to the added mock case.
        template<typename R = TReturn>
        MockCaseCallCount returns(
            typename std::enable_if<!std::is_void<R>::value, TReturn>::type
                returnValue) {
            // Wrap the return value in a tuple. Otherwise, reference values
            // would not work properly when capturing them.
            std::tuple<TReturn> wrappedReturnValue = (returnValue);

            // Create a fake.
            std::function<Internal::CaseMatch<TReturn> (TArguments...)> fake
                = [wrappedReturnValue] (TArguments... arguments) {
                    // Return a CaseMatch containing the return value.
                    return Internal::CaseMatchFactory::match<TReturn>(
                        std::get<0>(wrappedReturnValue));
                };

            // Forward the call to fakeGeneral.
            return fakeGeneral(fake);
        }

        /// Adds a mock case making the associated method callable when called
        /// with the associated arguments.
        ///
        /// The method is only available if the return type is void.
        ///
        /// @return A MockCaseCallCount that can be queried about the number of
        /// calls done to the added mock case.
        template<typename R = TReturn,
            typename std::enable_if<std::is_void<R>::value, R>::type* = nullptr>
        MockCaseCallCount returns() {
            // Create a fake.
            std::function<Internal::CaseMatch<TReturn> (TArguments...)> fake
                = [] (TArguments... arguments) {
                    // Return a CaseMatch for void.
                    return Internal::CaseMatchFactory::matchVoid();
                };

            // Forward the call to fakeGeneral.
            return fakeGeneral(fake);
        }

        // TODO: Add a method for adding a mock case with a fake.

    private:
        /// Adds a mock case making calls to the associated method be forwarded
        /// to fake when called with the associated arguments.
        ///
        /// @param fake A callback to be called when a match happens.
        /// @return A MockCaseCallCount that can be queried about the number of
        /// calls done to the added mock case.
        MockCaseCallCount fakeGeneral(
            std::function<Internal::CaseMatch<TReturn> (TArguments...)> fake) {
            // Create a MockWithArgumentsCase.
            std::unique_ptr<Internal::ICase<TReturn, TArguments...>> mockCase
                = Internal::make_unique<
                Internal::MockWithArgumentsCase<TReturn, TArguments...>>(
                std::move(_arguments),
                fake);

            // Add the case to InnerMock.
            return _mock->template addCase<id, TReturn, TArguments...>(
                _method,
                std::move(mockCase));
        }
};

}
