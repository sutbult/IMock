#pragma once

#include <memory>

#include <exception/WrongCallCountException.hpp>
#include <internal/MutableCallCount.hpp>

namespace IMock {

/// Is used to access the number of times a mock case has been called or to
/// verify that a mock case has been called a certain number of times.
class CallCount {
    private:
        /// A pointer to a MutableCallCount containing the call count.
        std::shared_ptr<Internal::MutableCallCount> _callCount;

    public:
        /// Creates a CallCount.
        ///
        /// @param callCount A MutableCallCount to get the call count from.
        CallCount(
            std::shared_ptr<Internal::MutableCallCount> callCount) 
            : _callCount(std::move(callCount)) {
        }

        /// Gets the number of times the underlying mock case currently has been
        /// called.
        ///
        /// @return The number of times the underlying mock case currently has
        /// been called.
        int getCallCount() {
            // Call getCallCount to get the call count and then return it.
            return _callCount->getCallCount();
        }

        /// Verify the underlying mock case has been called a certain number of
        /// times.
        ///
        /// @param expectedCallCount The number of times the mock case should
        /// have been called.
        /// @throws Throws a WrongCallCountException if the actual call count
        /// differs from the expected call count.
        void verifyCallCount(int expectedCallCount) {
            // Get the actual call count.
            int actualCallCount = getCallCount();

            // Check if the call counts are equal.
            if(actualCallCount != expectedCallCount) {
                // Throw a WrongCallCountException if that's not the case.
                throw Exception::WrongCallCountException(
                    expectedCallCount,
                    actualCallCount);
            }
        }

        /// Verify the underlying mock case has been called exactly once.
        ///
        /// @throws Throws a WrongCallCountException if the mock case has not
        /// been called exactly once.
        void verifyCalledOnce() {
            // Call verifyCallCount with one.
            verifyCallCount(1);
        }

        /// Verify the underlying mock case has never been called.
        ///
        /// @throws Throws a WrongCallCountException if the mock case has been
        /// called at least once.
        void verifyNeverCalled() {
            // Call verifyCallCount with zero.
            verifyCallCount(0);
        }
};

}
