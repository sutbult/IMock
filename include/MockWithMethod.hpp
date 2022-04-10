#pragma once

#include <internal/InnerMock.hpp>
#include <MockCaseID.hpp>
#include <MockWithArguments.hpp>

namespace IMock {

template <typename TInterface, MockCaseID id, typename TReturn,
    typename ...TArguments>
class MockWithMethod {
    private:
        Internal::InnerMock<TInterface>* _mock;
        TReturn (TInterface::*_method)(TArguments...);

    public:
        MockWithMethod(
            Internal::InnerMock<TInterface>* mock,
            TReturn (TInterface::*method)(TArguments...))
            : _mock(mock)
            , _method(method) {
        }

        MockWithArguments<TInterface, id, TReturn, TArguments...> with(
            TArguments... arguments) {
            return MockWithArguments<TInterface, id, TReturn, TArguments...>(
                _mock,
                _method,
                std::tuple<TArguments...>(std::move(arguments)...));
        }
};

}
