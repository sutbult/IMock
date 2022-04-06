#pragma once

#include <sstream>

#include <exception/MockException.hpp>

namespace IMock::Exception {

class UnknownCallException : public MockException {
    public:
        UnknownCallException();
};

UnknownCallException::UnknownCallException()
    : MockException("A call was made to a method that has not been mocked.") {
}

class UnmockedCallException : public MockException {
    public:
        UnmockedCallException();
};

UnmockedCallException::UnmockedCallException()
    : MockException("A call was made to a method that has been mocked but the "
        "arguments does not match the mocked arguments.") {
}

}
