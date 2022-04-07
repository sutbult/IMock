#pragma once

#include <exception/MockException.hpp>

namespace IMock::Exception {

class UnknownCallException : public MockException {
    public:
        UnknownCallException();
};

UnknownCallException::UnknownCallException()
    : MockException("A call was made to a method that has not been mocked.") {
}

}
