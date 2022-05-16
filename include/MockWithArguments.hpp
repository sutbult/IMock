#pragma once

#include <tuple>
#include <utility>

#include <exception/MockWithArgumentsUsedTwiceException.hpp>
#include <internal/Apply.hpp>
#include <internal/CaseMatchFactory.hpp>
#include <internal/InnerMock.hpp>
#include <internal/makeUnique.hpp>
#include <internal/MockWithArgumentsCase.hpp>
#include <Method.hpp>
#include <MockCaseCallCount.hpp>
#include <MockCaseID.hpp>

namespace IMock {

/// A Mock with an associated method and arguments to add a mock case for.
template <typename TInterface, MockCaseID id, typename TReturn,
    typename ...TArguments>
class MockWithArguments {
    private:
        /// The InnerMock to add a mock case to.
        Internal::InnerMock<TInterface>& _mock;

        /// The method to add a mock case to.
        Method<TInterface, TReturn, TArguments...> _method;

        /// A string describing how a call is made to the method being mocked.
        std::string _methodString;

        /// The arguments to match calls with.
        std::tuple<TArguments...> _arguments;

        /// Describes if the instance already has been used.
        bool _used;

    public:
        /// Creates a MockWithArguments.
        ///
        /// @param mock The InnerMock to add a mock case to.
        /// @param method The method to add a mock case to.
        /// @param arguments The arguments to match calls with.
        /// @param methodString A string describing how a call is made to the
        /// method being mocked.
        MockWithArguments(
            Internal::InnerMock<TInterface>& mock,
            Method<TInterface, TReturn, TArguments...> method,
            std::string methodString,
            std::tuple<TArguments...> arguments)
            : _mock(mock)
            , _method(std::move(method))
            , _methodString(std::move(methodString))
            , _arguments(std::move(arguments))
            , _used(false) {
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
            std::function<Internal::CaseMatch<TReturn>
                (std::tuple<TArguments...>)> fake
                = [wrappedReturnValue] (std::tuple<TArguments...> arguments) {
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
            std::function<Internal::CaseMatch<TReturn>
                (std::tuple<TArguments...>)> fake
                = [] (std::tuple<TArguments...> arguments) {
                    // Return a CaseMatch for void.
                    return Internal::CaseMatchFactory::matchVoid();
                };

            // Forward the call to fakeGeneral.
            return fakeGeneral(fake);
        }

        /// Adds a fake handling the method call when called with the associated
        /// arguments.
        ///
        /// @param fake A callback to call when the method is called and a match
        /// happens.
        /// @return A MockCaseCallCount that can be queried about the number of
        /// calls done to the added mock case.
        MockCaseCallCount fake(std::function<TReturn (TArguments...)> fake) {
            // Call fakeGeneral with a fake.
            return fakeGeneral([fake](std::tuple<TArguments...> arguments) {
                // Return a CaseMatch for the fake.
                return Internal::CaseMatchFactory::matchFake(
                    fake,
                    std::move(arguments));
            });
        }

    private:
        /// Adds a mock case making calls to the associated method be forwarded
        /// to fake when called with the associated arguments.
        ///
        /// @param fake A callback to be called when a match happens.
        /// @return A MockCaseCallCount that can be queried about the number of
        /// calls done to the added mock case.
        MockCaseCallCount fakeGeneral(
            std::function<Internal::CaseMatch<TReturn>
                (std::tuple<TArguments...>)> fake) {
            // Check if the instance already has been used.
            if(_used) {
                // Throw a MockWithArgumentsUsedTwiceException since the
                // instance cannot be used again as the arguments has been
                // moved.
                throw Exception::MockWithArgumentsUsedTwiceException();
            }
            else {
                // Raise the _used flag to mark that the instance has been used.
                _used = true;
            }

            // Create a MockWithArgumentsCase.
            std::unique_ptr<Internal::ICase<TReturn, TArguments...>> mockCase
                = Internal::makeUnique<
                Internal::MockWithArgumentsCase<TReturn, TArguments...>>(

                // Move the arguments, which means the instance cannot be used
                // again.
                std::move(_arguments),
                fake);

            // Add the case to InnerMock.
            return _mock.template addCase<id, TReturn, TArguments...>(
                _method,
                std::move(_methodString),
                std::move(mockCase));
        }
};

}
