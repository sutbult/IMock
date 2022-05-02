#pragma once

#include <exception/MockException.hpp>

namespace IMock::Exception {

/// Thrown when a call was made to a method that has been mocked but the
/// arguments does not match any mocked arguments.
class UnmockedCallException : public MockException {
    public:
        /// Creates an UnmockedCallException.
        ///
        /// @param callString A string describing how the call was made.
        UnmockedCallException(
            std::string callString)
            : MockException(getMessage(std::move(callString))) {
        }

    private:
        /// Creates an exception message.
        ///
        /// @param callString A string describing how the call was made.
        /// @return A generated exception message.
        static std::string getMessage(
            std::string callString) {
            // Create and return a message containing callString.
            return "The call "
                + callString
                + " does not match any mocked case.";
        }
};

}
