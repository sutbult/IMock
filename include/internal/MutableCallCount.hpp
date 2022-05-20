#pragma once

namespace IMock {
namespace Internal {

/// Contains a call count that can be increased and retrieved.
class MutableCallCount {
    private:
        // The call count.
        int _callCount;

    public:
        /// Creates a MutableCallCount by initializing the call count with zero.
        MutableCallCount()
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
}
