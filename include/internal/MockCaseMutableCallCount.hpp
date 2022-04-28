#pragma once

namespace IMock::Internal {

/// Contains a call count that can be increased and retrieved.
class MockCaseMutableCallCount {
    private:
        // The call count.
        int _callCount;

    public:
        /// Creates a MockCaseMutableCallCount by initializing the call count
        /// with zero.
        MockCaseMutableCallCount()
            : _callCount(0) {
        }

        /// Increases the call count by one.
        void increase() {
            _callCount++;
        }

        /// Gets the call count.
        ///
        /// @return The call count.
        int getCallCount() {
            // Return the call count.
            return _callCount;
        }
};

}
