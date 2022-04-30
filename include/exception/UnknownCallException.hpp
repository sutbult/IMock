#pragma once

#include <exception/MockException.hpp>

namespace IMock::Exception {

/// Thrown when a method is called that has not been mocked.
class UnknownCallException : public MockException {
    public:
        /// Creates an UnknownCallException.
        UnknownCallException()
            : MockException("A call was made to a method that has not been "
                "mocked.") {
        }
};

}
