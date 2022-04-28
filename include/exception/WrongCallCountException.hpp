#pragma once

#include <sstream>

#include <exception/MockException.hpp>

namespace IMock::Exception {

/// Thrown when a method is expected to have been called a certain number of
/// times but actually was called a different number of times.
class WrongCallCountException : public MockException {
    public:
        /// Creates a WrongCallCountException.
        ///
        /// @param expectedCallCount The number of calls expected to have been
        /// made.
        /// @param actualCallCount The number of calls that has actually been
        /// made.
        WrongCallCountException(
            int expectedCallCount,
            int actualCallCount)
            : MockException(getMessage(
                expectedCallCount,
                actualCallCount)) {
        }

    private:
        /// Creates an exception message.
        ///
        /// @param expectedCallCount The number of calls expected to have been
        /// made.
        /// @param actualCallCount The number of calls that has actually been
        /// made.
        /// @return A generated exception message.
        static std::string getMessage(
            int expectedCallCount,
            int actualCallCount) {
            // Create a stringstream.
            std::stringstream out;

            // Append the expected number of calls.
            out << "Expected the method to be called "
                << expectedCallCount
                << " time";

            // Check if the expected number of calls should be referred to in
            // plural.
            if(expectedCallCount != 1) {
                // Append an "s" if that's the case.
                out << "s";
            }

            // Append the actual number of calls.
            out << " but it was called "
                << actualCallCount
                << " time";

            // Check if the actual number of calls should be referred to in
            // plural.
            if(actualCallCount != 1) {
                // Append an "s" if that's the case.
                out << "s";
            }

            // Append a period.
            out << ".";

            // Create a string and return it.
            return out.str();
        }
};

}
