#pragma once

#include <exception/MockException.hpp>

namespace IMock::Exception {

/// Thrown when a MockWithArguments is used twice, since the arguments are
/// moved.
class MockWithArgumentsUsedTwiceException : public MockException {
    public:
        /// Creates a MockWithArgumentsUsedTwiceException.
        MockWithArgumentsUsedTwiceException()
            : MockException("A MockWithArguments instance was reused."
                " This is not possible since the arguments are moved when"
                " adding a case.") {
        }
};

}
