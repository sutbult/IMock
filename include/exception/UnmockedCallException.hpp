#pragma once

#include <exception/MockException.hpp>

namespace IMock::Exception {

/// Thrown when a call was made to a method that has been mocked but the
/// arguments does not match any mocked arguments.
class UnmockedCallException : public MockException {
    public:
        /// Creates an UnmockedCallException.
        UnmockedCallException()
            : MockException("A call was made to a method that has been mocked "
                "but the arguments does not match any mocked arguments.") {
        }
};

}
