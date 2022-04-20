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
            std::tuple<TReturn> wrappedReturnValue = (returnValue);

            std::function<Internal::CaseMatch<TReturn> (TArguments...)> fake
                = [wrappedReturnValue] (TArguments... arguments) {
                    return Internal::CaseMatchFactory::match<TReturn>(
                        std::get<0>(wrappedReturnValue));
                };

            return fakeGeneral(fake);
        }

        template<typename R = TReturn,
            typename std::enable_if<std::is_void<R>::value, R>::type* = nullptr>
        MockCaseCallCount returns() {
            std::function<Internal::CaseMatch<TReturn> (TArguments...)> fake
                = [] (TArguments... arguments) {
                    return Internal::CaseMatchFactory::matchVoid();
                };

            return fakeGeneral(fake);
        }

    private:
        MockCaseCallCount fakeGeneral(
            std::function<Internal::CaseMatch<TReturn> (TArguments...)> fake) {
            std::unique_ptr<Internal::ICase<TReturn, TArguments...>> mockCase
                = Internal::make_unique<
                Internal::MockWithArgumentsCase<TReturn, TArguments...>>(
                std::move(_arguments),
                fake);

            return _mock->template addCase<id, TReturn, TArguments...>(
                _method,
                std::move(mockCase));
        }
};

}
