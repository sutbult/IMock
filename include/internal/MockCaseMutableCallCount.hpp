#pragma once

namespace IMock::Internal {

class MockCaseMutableCallCount {
    private:
        int _callCount;

    public:
        MockCaseMutableCallCount()
            : _callCount(0) {
        }

        void increase() {
            _callCount++;
        }

        int getCallCount() {
            return _callCount;
        }
};

}
