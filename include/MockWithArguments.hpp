#pragma once

#include <internal/InnerMock.hpp>
#include <MockCaseCallCount.hpp>
#include <MockCaseID.hpp>

namespace IMock {

template <typename TInterface, MockCaseID id, typename TReturn,
    typename ...TArguments>
class MockWithArguments {
    private:
        Internal::InnerMock<TInterface>* _mock;
        TReturn (TInterface::*_method)(TArguments...);
        std::tuple<TArguments...> _arguments;

    public:
        MockWithArguments(
            Internal::InnerMock<TInterface>* mock,
            TReturn (TInterface::*method)(TArguments...),
            std::tuple<TArguments...> arguments)
            : _mock(std::move(mock))
            , _method(std::move(method))
            , _arguments(std::move(arguments)) {
        }

        // Solution taken from:
        // https://eli.thegreenplace.net/2014/sfinae-and-enable_if/
        template<typename R = TReturn>
        MockCaseCallCount returns(
            typename std::enable_if<!std::is_void<R>::value, TReturn>::type
                returnValue) {
            std::unique_ptr<Internal::IReturnValue<TReturn>> wrappedReturnValue
                = Internal::make_unique<Internal::NonVoidReturnValue<TReturn>>(
                returnValue);

            return _mock->template addCase<id, TReturn, TArguments...>(
                _method,
                std::move(wrappedReturnValue),
                std::move(_arguments));
        }

        template<typename R = TReturn,
            typename std::enable_if<std::is_void<R>::value, R>::type* = nullptr>
        MockCaseCallCount returns() {
            std::unique_ptr<Internal::IReturnValue<TReturn>> wrappedReturnValue
                = Internal::make_unique<Internal::VoidReturnValue>();

            return _mock->template addCase<id, TReturn, TArguments...>(
                _method,
                std::move(wrappedReturnValue),
                std::move(_arguments));
        }
};

}
