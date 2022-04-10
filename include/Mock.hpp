#pragma once

#include <internal/InnerMock.hpp>
#include <MockCaseID.hpp>
#include <MockWithID.hpp>

namespace IMock {

template <typename TInterface>
class Mock {
    private:
        Internal::InnerMock<TInterface> _innerMock;

    public:
        virtual ~Mock() {
        }

        TInterface& get() {
            return _innerMock.get();
        }

        template <MockCaseID id>
        MockWithID<TInterface, id> withCounter() {
            return MockWithID<TInterface, id>(&_innerMock);
        }
};

}
