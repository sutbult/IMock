#pragma once

#include <memory>

#include <exception/WrongCallCountException.hpp>
#include <internal/MockCaseMutableCallCount.hpp>

namespace IMock {

class MockCaseCallCount {
    private:
        std::shared_ptr<Internal::MockCaseMutableCallCount> _callCount;

    public:
        MockCaseCallCount(
            std::shared_ptr<Internal::MockCaseMutableCallCount> callCount) 
            : _callCount(std::move(callCount)) {
        }
    
        int getCallCount() {
            return _callCount->getCallCount();
        }

        void verifyCallCount(int expectedCallCount) {
            int actualCallCount = getCallCount();

            if(actualCallCount != expectedCallCount) {
                throw Exception::WrongCallCountException(
                    expectedCallCount,
                    actualCallCount);
            }
        }

        void verifyCalledOnce() {
            verifyCallCount(1);
        }

        void verifyNeverCalled() {
            verifyCallCount(0);
        }
};

}
