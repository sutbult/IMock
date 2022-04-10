#pragma once

#include <internal/InnerMock.hpp>
#include <MockCaseID.hpp>
#include <MockWithMethod.hpp>

namespace IMock {

template <typename TInterface, MockCaseID id>
class MockWithID {
    private:
        Internal::InnerMock<TInterface>* _mock;

    public:
        MockWithID(Internal::InnerMock<TInterface>* mock)
            : _mock(mock) {
        }

        template <typename TReturn, typename ...TArguments>
        MockWithMethod<TInterface, id, TReturn, TArguments...> withMethod(
            TReturn (TInterface::*method)(TArguments...)) {
            return MockWithMethod<TInterface, id, TReturn, TArguments...>(
                _mock,
                method);
        }
};

}
