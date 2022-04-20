#pragma once

#include <exception/UnmockedCallException.hpp>
#include <internal/CaseMatch.hpp>
#include <internal/ICase.hpp>
#include <internal/IMockMethodNonGeneric.hpp>
#include <internal/MockCaseMutableCallCount.hpp>
#include <internal/make_unique.hpp>
#include <MockCaseCallCount.hpp>

namespace IMock::Internal {

template <typename TReturn, typename ...TArguments>
class MockMethod : public IMockMethodNonGeneric {
    private:
        class InnerMockCase {
            public:
                std::unique_ptr<ICase<TReturn, TArguments...>> _mockCase;
                std::shared_ptr<MockCaseMutableCallCount> _callCount;
                std::unique_ptr<InnerMockCase> _next;

                InnerMockCase(
                    std::unique_ptr<ICase<TReturn, TArguments...>> mockCase,
                    std::shared_ptr<MockCaseMutableCallCount> callCount,
                    std::unique_ptr<InnerMockCase> next)
                    : _mockCase(std::move(mockCase))
                    , _callCount(std::move(callCount))
                    , _next(std::move(next)) {
                }
        };

        std::unique_ptr<InnerMockCase> _topMockCase;

    public:
        MockMethod()
            : _topMockCase(std::unique_ptr<InnerMockCase>(nullptr)) {
        }

        MockCaseCallCount addCase(
            std::unique_ptr<ICase<TReturn, TArguments...>> mockCase) {
            std::shared_ptr<MockCaseMutableCallCount> callCountPointer
                = std::make_shared<MockCaseMutableCallCount>();

            _topMockCase = Internal::make_unique<InnerMockCase>(
                std::move(mockCase),
                callCountPointer,
                std::move(_topMockCase));

            MockCaseCallCount callCount(callCountPointer);

            return callCount;
        }

        TReturn onCall(TArguments... arguments) {
            std::tuple<TArguments...> tupleArguments(std::move(arguments)...);

            InnerMockCase* mockCase = _topMockCase.get();

            while(mockCase != nullptr) {
                CaseMatch<TReturn> caseMatch = mockCase->_mockCase->matches(
                    tupleArguments);

                if(caseMatch.isMatch()) {
                    mockCase->_callCount->increase();
                    return caseMatch
                        .getReturnValue()
                        .getReturnValue();
                }
                else {
                    mockCase = mockCase->_next.get();
                }
            }

            throw Exception::UnmockedCallException();
        }
};

}
