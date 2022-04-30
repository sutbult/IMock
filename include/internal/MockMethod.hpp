#pragma once

#include <exception/UnmockedCallException.hpp>
#include <internal/CaseMatch.hpp>
#include <internal/ICase.hpp>
#include <internal/IMockMethodNonGeneric.hpp>
#include <internal/MockCaseMutableCallCount.hpp>
#include <internal/make_unique.hpp>
#include <MockCaseCallCount.hpp>

namespace IMock::Internal {

/// A mocked method containing a number of mock cases.
template <typename TReturn, typename ...TArguments>
class MockMethod : public IMockMethodNonGeneric {
    private:
        /// Contains information about one mock case.
        class InnerMockCase {
            public:
                /// The mock case's ICase.
                std::unique_ptr<ICase<TReturn, TArguments...>> _mockCase;

                /// A MockCaseMutableCallCount keeping track of how many times
                /// the mock case has been called.
                std::shared_ptr<MockCaseMutableCallCount> _callCount;

                /// The next mock case.
                std::unique_ptr<InnerMockCase> _next;

                /// Creates a InnerMockCase.
                ///
                /// @param mockCase The mock case's ICase.
                /// @param callCount A MockCaseMutableCallCount keeping track of
                /// how many times the mock case has been called.
                /// @param next The next mock case.
                InnerMockCase(
                    std::unique_ptr<ICase<TReturn, TArguments...>> mockCase,
                    std::shared_ptr<MockCaseMutableCallCount> callCount,
                    std::unique_ptr<InnerMockCase> next)
                    : _mockCase(std::move(mockCase))
                    , _callCount(std::move(callCount))
                    , _next(std::move(next)) {
                }
        };

        /// The most recently mock case to have been added.
        std::unique_ptr<InnerMockCase> _topMockCase;

    public:
        /// Creates a MockMethod without any mock cases.
        MockMethod()
            : _topMockCase(std::unique_ptr<InnerMockCase>(nullptr)) {
        }

        /// Destructs the MockMethod by deleting all InnerMockCase instances
        /// iteratively to not cause any stack overflows.
        virtual ~MockMethod() {
            // Declare a pointer for mock cases and initialize it with the top
            // mock case while releasing its unique_ptr.
            InnerMockCase* mockCase = _topMockCase.release();

            // Iterate while mock cases exist.
            while(mockCase != nullptr) {
                // Get the next mock case while releasing its unique_ptr.
                InnerMockCase* nextMockCase = mockCase->_next.release();

                // Delete the mock case.
                delete mockCase;

                // Assign the next mock case to mockCase to continue with it.
                mockCase = nextMockCase;
            }
        }

        /// Adds a new mock case.
        ///
        /// @param mockCase A mock case to add.
        MockCaseCallCount addCase(
            std::unique_ptr<ICase<TReturn, TArguments...>> mockCase) {
            // Create a MockCaseMutableCallCount for the mock case.
            std::shared_ptr<MockCaseMutableCallCount> callCountPointer
                = std::make_shared<MockCaseMutableCallCount>();

            // Create a InnerMockCase and assign it to _topMockCase.
            _topMockCase = Internal::make_unique<InnerMockCase>(
                std::move(mockCase),
                callCountPointer,
                std::move(_topMockCase));

            // Create a MockCaseCallCount for the mock case.
            MockCaseCallCount callCount(callCountPointer);

            // Return the MockCaseCallCount.
            return callCount;
        }

        /// Call this when the method to mock is called.
        ///
        /// @param arguments The arguments of the call.
        /// @return The return value from the first matching mock case.
        /// @throws Throws an UnmockedCallException if the arguments does not
        /// match any mock case.
        TReturn onCall(TArguments... arguments) {
            // Create a tuple from the arguments.
            std::tuple<TArguments...> tupleArguments(std::move(arguments)...);

            // Declare a pointer for mock cases and initialize it with the top
            // mock case.
            InnerMockCase* mockCase = _topMockCase.get();

            // Iterate while mock cases exist.
            while(mockCase != nullptr) {
                // Check if the current mock case matches the arguments.
                CaseMatch<TReturn> caseMatch = mockCase->_mockCase->matches(
                    tupleArguments);

                // Check if a match happened.
                if(caseMatch.isMatch()) {
                    // If so, increase the call count.
                    mockCase->_callCount->increase();

                    // And then, return the return value.
                    return caseMatch
                        .getReturnValue()
                        .getReturnValue();
                }
                else {
                    // Otherwise, continue with the next mock case.
                    mockCase = mockCase->_next.get();
                }
            }

            // Throw an UnmockedCallException if no mock case matches the
            // arguments.
            throw Exception::UnmockedCallException();
        }
};

}
