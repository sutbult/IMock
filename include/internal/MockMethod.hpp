#pragma once

#include <exception/UnmockedCallException.hpp>
#include <internal/Apply.hpp>
#include <internal/CaseMatch.hpp>
#include <internal/ICase.hpp>
#include <internal/IMockMethodNonGeneric.hpp>
#include <internal/JoinStrings.hpp>
#include <internal/MutableCallCount.hpp>
#include <internal/makeUnique.hpp>
#include <internal/ToString.hpp>
#include <CallCount.hpp>

namespace IMock {
namespace Internal {

/// A mocked method containing a number of mock cases.
///
/// @tparam TReturn The return type of the mocked method.
/// @tparam TArguments The types of the arguments of the mocked method.
template <typename TReturn, typename ...TArguments>
class MockMethod : public IMockMethodNonGeneric {
    private:
        /// Contains information about one mock case.
        class InnerMockCase {
            public:
                /// The mock case's ICase.
                std::unique_ptr<ICase<TReturn, TArguments...>> _mockCase;

                /// A MutableCallCount keeping track of how many times the mock
                /// case has been called.
                std::shared_ptr<MutableCallCount> _callCount;

                /// The next mock case.
                std::unique_ptr<InnerMockCase> _next;

                /// Creates a InnerMockCase.
                ///
                /// @param mockCase The mock case's ICase.
                /// @param callCount A MutableCallCount keeping track of how
                /// many times the mock case has been called.
                /// @param next The next mock case.
                InnerMockCase(
                    std::unique_ptr<ICase<TReturn, TArguments...>> mockCase,
                    std::shared_ptr<MutableCallCount> callCount,
                    std::unique_ptr<InnerMockCase> next)
                    : _mockCase(std::move(mockCase))
                    , _callCount(std::move(callCount))
                    , _next(std::move(next)) {
                }
        };

        /// The most recently mock case to have been added.
        std::unique_ptr<InnerMockCase> _topMockCase;

        /// A string describing how a call is made to the method being mocked.
        std::string _methodString;

    public:
        /// Creates a MockMethod without any mock cases.
        ///
        /// @param methodString A string describing how a call is made to the
        /// method being mocked.
        MockMethod(std::string methodString)
            : _topMockCase(std::unique_ptr<InnerMockCase>(nullptr))
            , _methodString(std::move(methodString)) {
        }

        /// Destructs the MockMethod by deleting all InnerMockCase instances
        /// iteratively to not cause any stack overflows.
        ~MockMethod() noexcept {
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
        CallCount addCase(
            std::unique_ptr<ICase<TReturn, TArguments...>> mockCase) {
            // Create a MutableCallCount for the mock case.
            std::shared_ptr<MutableCallCount> callCountPointer
                = std::make_shared<MutableCallCount>();

            // Create a InnerMockCase and assign it to _topMockCase.
            _topMockCase = Internal::makeUnique<InnerMockCase>(
                std::move(mockCase),
                callCountPointer,
                std::move(_topMockCase));

            // Create a CallCount for the mock case.
            CallCount callCount(callCountPointer);

            // Return the CallCount.
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
            std::tuple<TArguments...> tupleArguments(
                std::forward<TArguments>(arguments)...);

            // Declare a pointer for mock cases and initialize it with the top
            // mock case.
            InnerMockCase* mockCase = _topMockCase.get();

            // Iterate while mock cases exist.
            while(mockCase != nullptr) {
                // Check if the current mock case matches the arguments.
                CaseMatch<TReturn> caseMatch = mockCase->_mockCase->matches(
                    tupleArguments);

                // Get if a match happened.
                bool match = caseMatch.isMatch();

                // Check if a match happened.
                if(match) {
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

            // No mock case matches the arguments.

            // Create a call string from the arguments.
            std::string callString = getCallString(std::move(tupleArguments));

            // Throw an UnmockedCallException.
            throw Exception::UnmockedCallException(callString);
        }

    private:
        /// Create a call string from the provided arguments.
        ///
        /// @param arguments The arguments of the call.
        /// @return A string describing how the call was made.
        std::string getCallString(std::tuple<TArguments...> arguments) const {
            // Convert the arguments to strings.
            std::vector<std::string> stringArguments
                = Apply::apply<std::vector<std::string>, TArguments...>(
                    std::function<std::vector<std::string> (TArguments...)>(
                        ToString::toStrings<TArguments...>),
                    std::move(arguments));

            // Join the argument strings.
            std::string argumentsString = JoinStrings::joinStrings(
                ", ",
                std::move(stringArguments));

            // Create a call string.
            std::string callString
                = _methodString
                + "(" + argumentsString + ")";

            // Return the call string.
            return callString;
        }
};

}
}
