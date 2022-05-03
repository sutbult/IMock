#pragma once

#include <exception>
#include <functional>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace IMock {

/// An ID used to distinguish mock cases from each other within a Mock.
typedef unsigned int MockCaseID;

}

namespace IMock::Internal {

/// Utility making it possible to call a callback using arguments contained in
/// a tuple.
class Apply {
    private:
        // A trick to statically produce a list of integers. The solution has
        // been taken from: https://stackoverflow.com/a/7858971/6188897
        template<int ...>
        struct seq {
        };

        template<int N, int ...S>
        struct gens : gens<N-1, N-1, S...> {
        };

        template<int ...S>
        struct gens<0, S...>{
            typedef seq<S...> type;
        };

        /// Calls the provided callback with the arguments in the provided
        /// tuple.
        ///
        /// Also includes a "seq" making it possible to extract the arguments
        /// from the tuple.
        ///
        /// @param callback The function to call.
        /// @param arguments The arguments to call the callback with.
        /// @return The return value from the callback.
        template<int ...S, typename TReturn,
            typename ...TArguments>
        static TReturn applyWithSeq(
            seq<S...>,
            std::function<TReturn (TArguments...)> callback,
            std::tuple<TArguments...> arguments) {
            // Call callback with the extracted arguments.
            return callback(std::move(std::get<S>(arguments))...);
        }

    public:
        /// Apply is not supposed to be instantiated since it only contains
        /// static methods.
        Apply() = delete;

        /// Calls the provided callback with the arguments in the provided
        /// tuple.
        ///
        /// @param callback The function to call.
        /// @param arguments The arguments to call the callback with.
        /// @return The return value from the callback.
        template<typename TReturn, typename ...TArguments>
        static TReturn apply(
            std::function<TReturn (TArguments...)> callback,
            std::tuple<TArguments...> arguments) {
            // Create a "gens" with the number of arguments.
            return applyWithSeq(typename gens<sizeof...(TArguments)>::type(),
                callback,
                std::move(arguments));
        }
};

}

namespace IMock::Internal {

/// Interface for retrieving a return value.
template <typename TReturn>
class IReturnValue {
    public:
        /// Virtual destructor of IReturnValue.
        virtual ~IReturnValue() {
        }

        /// Gets the return value.
        ///
        /// NOTICE: The method can only be called once for each instance.
        /// Any subsequent calls may result in an exception being thrown.
        ///
        /// @return The return value.
        virtual TReturn getReturnValue() = 0;
};

/// Implements IReturnValue to make getReturnValue return void.
class VoidReturnValue : public IReturnValue<void> {
    public:
        virtual void getReturnValue() override {
        }
};

/// Implements IReturnValue to make getReturnValue return the provided value.
template <typename TReturn>
class NonVoidReturnValue : public IReturnValue<TReturn> {
    private:
        /// The value to return.
        TReturn _returnValue;

    public:
        /// Creates a NonVoidReturnValue.
        ///
        /// @param returnValue The value to return.
        NonVoidReturnValue(TReturn returnValue)
            : _returnValue(std::move(returnValue)) {
        }
        
        virtual TReturn getReturnValue() override {
            // Return the stored value.
            return _returnValue;
        }
};

/// Implements FakeReturnValue to make getReturnValue call the provided fake
/// with the provided arguments and return its return value.
template <typename TReturn, typename ...TArguments>
class FakeReturnValue : public IReturnValue<TReturn> {
    private:
        /// The fake to call.
        std::function<TReturn (TArguments...)> _fake;

        /// The arguments to call the fake with.
        std::tuple<TArguments...> _arguments;

    public:
        /// Creates a FakeReturnValue.
        ///
        /// @param fake The fake to call.
        /// @param arguments The arguments to call the fake with.
        FakeReturnValue(
            std::function<TReturn (TArguments...)> fake,
            std::tuple<TArguments...> arguments)
            : _fake(std::move(fake))
            , _arguments(std::move(arguments)) {
        }

        virtual TReturn getReturnValue() override {
            // Forward the call to _fake.
            return Apply::apply(_fake, std::move(_arguments));
        }
};

}

namespace IMock::Internal {

/// Indicates if a call to a mock case resulted in a match with a return value
/// or if it resulted in no match.
template <typename TReturn>
class CaseMatch {
    private:
        /// A possible return value.
        std::unique_ptr<IReturnValue<TReturn>> _returnValue;

    public:
        /// Creates a CaseMatch.
        ///
        /// @param returnValue A possible return value.
        CaseMatch(std::unique_ptr<IReturnValue<TReturn>> returnValue)
            : _returnValue(std::move(returnValue)) {
        }

        /// Move constructor for CaseMatch.
        CaseMatch(CaseMatch&& other)
            : _returnValue(std::move(other._returnValue)) {
        }

        /// Indicates if the call resulted in a match.
        ///
        /// @return True if the call resulted in a match and false otherwise.
        bool isMatch() const {
            // A match happened if the a return value exists.
            return _returnValue.get() != nullptr;
        }

        /// Gets the return value. Calling this on an instance where isMatch()
        /// returns false results in a segmentation fault.
        ///
        /// @return The return value.
        IReturnValue<TReturn>& getReturnValue() const {
            // Get and return the return value.
            return *_returnValue.get();
        }
};

}

namespace IMock::Internal {

/// Creates a unique_ptr.
///
/// Named makeUnique using camel casing to not cause any ambiguities with
/// std::make_unique.
///
/// @param arguments Arguments used to create the contained value.
/// @return A created unique_ptr.
template<typename TPointer, typename... TArguments>
std::unique_ptr<TPointer> makeUnique(TArguments&&... arguments)
{
    // Check if std::make_unique is available (introduced in C++14).
    #ifdef __cpp_lib_make_unique

    // Use std::make_unique to create the unique_ptr since std::make_unique is
    // available.
    return std::make_unique<TPointer, TArguments...>(
        std::forward<TArguments>(arguments)...);
    #else

    // Create an instance manually and then create a unique_ptr using it since
    // std::make_unique is not available.
    return std::unique_ptr<TPointer>(
        new TPointer(std::forward<TArguments>(arguments)...));
    #endif
}

}

namespace IMock::Exception {

/// A class implementing std::exception thrown by IMock when something is wrong.
class MockException : public std::exception {
    private:
        /// An explanation of what went wrong.
        std::string message;
    
    public:
        /// Creates a MockException.
        ///
        /// @param message An explanation of what went wrong.
        MockException(std::string message)
            : message(message) {
        }

        /// An override of std::exception::what() that returns the message.
        ///
        /// @return A constant pointer to the message.
        const char* what() const noexcept override {
            // Return a constant pointer to the message.
            return message.c_str();
        }
};

}

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

namespace IMock::Internal {

/// Interface for a mocked case.
template <typename TReturn, typename ...Arguments>
class ICase {
    public:
        /// Virtual destructor of ICase.
        virtual ~ICase() noexcept {
        }

        /// Checks if the provided arguments matches the case.
        ///
        /// @param arguments The arguments the mocked method was called with.
        /// The arguments will never be used again if the function indicates a
        /// match, which means the values can safely be moved.
        /// @return A CaseMatch indicating if the arguments resulted in a match.
        virtual CaseMatch<TReturn> matches(std::tuple<Arguments...>& arguments)
            = 0;
};

}

namespace IMock::Internal {

/// Interface for a method in a mock without any specified argument types or
/// return value type.
class IMockMethodNonGeneric {
    public:
        /// Virtual destructor of IMockMethodNonGeneric.
        virtual ~IMockMethodNonGeneric() {
        }
};

}

namespace IMock::Internal {

/// Joins vectors of strings using a provided delimiter.
class JoinStrings {
    public:
        /// JoinStrings is not supposed to be instantiated since it only
        /// contains static methods.
        JoinStrings() = delete;

        /// Joins a vector of strings using a provided delimiter.
        ///
        /// @param delimiter The string to insert between each string.
        /// @param strings The vector of strings to be joined.
        static std::string joinStrings(
            std::string delimiter,
            std::vector<std::string> strings) {
            // Create an empty list to store the result.
            std::string result = "";

            // Process each string.
            for(std::vector<std::string>::iterator iterator = strings.begin();
                iterator < strings.end();
                iterator++) {
                // Check if the current string is a non-initial string.
                if(iterator > strings.begin()) {
                    // Append the delimiter if that's the case.
                    result.append(delimiter);
                }

                // Append the current string.
                result.append(*iterator);
            }

            // Return the joined string.
            return result;
        }
};

}

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

namespace IMock::Internal {

/// Converts values to strings.
class ToString {
    private:
        // A trick to see if two types can use the insertion operator.
        // The solution has been taken from:
        // https://stackoverflow.com/a/22759544/6188897
        template<typename S, typename T>
        class is_streamable
        {
            private:
                template<typename SS, typename TT>
                static auto test(int) -> decltype(
                    std::declval<SS&>() << std::declval<TT>(),
                    std::true_type());

                template<typename, typename>
                static auto test(...) -> std::false_type;

            public:
                static const bool value = decltype(test<S,T>(0))::value;
        };

    public:
        /// ToString is not supposed to be instantiated since it only contains
        /// static methods.
        ToString() = delete;

        /// Converts a value to a string.
        ///
        /// Uses the insertion operator since it's available.
        ///
        /// @param value The value to be converted.
        /// @return A string representation of the value.
        template <typename TValue>
        static std::string toString(
            typename std::enable_if<
                is_streamable<std::stringstream, TValue>::value,
                TValue>::type value) {
            // Create a stringstream.
            std::stringstream out;

            // Append the value.
            out << value;

            // Create a string and return it.
            return out.str();
        }

        /// Converts a value to a string.
        ///
        /// Returns a question mark since the insertion operator is unavailable.
        ///
        /// @param value The value to be converted.
        /// @return A string representation of the value.
        template <typename TValue>
        static std::string toString(
            typename std::enable_if<
                !is_streamable<std::stringstream, TValue>::value,
                TValue>::type value) {
            // Return a question mark.
            return "?";
        }

        /// Converts a number of values to strings.
        /// Uses ToString::toString internally.
        ///
        /// @param values The values to be converted.
        /// @return String representations of the values.
        template <typename ...TValue>
        static std::vector<std::string> toStrings(TValue... values) {
            // Call toString for each value.
            std::vector<std::string> strings = {
                toString<TValue>(std::move(values))...
            };

            // Return the strings.
            return strings;
        }
};

}

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

namespace IMock {

/// Is used to access the number of times a mock case has been called or to
/// verify that a mock case has been called a certain number of times.
class MockCaseCallCount {
    private:
        /// A pointer to a MockCaseMutableCallCount containing the call count.
        std::shared_ptr<Internal::MockCaseMutableCallCount> _callCount;

    public:
        /// Creates a MockCaseCallCount.
        ///
        /// @param callCount A MockCaseMutableCallCount to get the call count
        /// from.
        MockCaseCallCount(
            std::shared_ptr<Internal::MockCaseMutableCallCount> callCount) 
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
        ~MockMethod() {
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
            _topMockCase = Internal::makeUnique<InnerMockCase>(
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
        std::string getCallString(std::tuple<TArguments...> arguments) {
            // Convert the arguments to strings.
            std::vector<std::string> stringArguments
                = Apply::apply<std::vector<std::string>, TArguments...>(
                    ToString::toStrings<TArguments...>,
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

namespace IMock::Exception {

/// Thrown when a method is called that has not been mocked.
class UnknownCallException : public MockException {
    public:
        /// Creates an UnknownCallException.
        UnknownCallException()
            : MockException("A call was made to a method that has not been "
                "mocked.") {
        }
};

}

namespace IMock::Internal {

/// Contains a static function to call if a call has been made to a method
/// without any mock cases.
class UnknownCall {
    public:
        /// UnknownCall is not supposed to be instantiated since it only
        /// contains a static method.
        UnknownCall() = delete;

        /// Call this if a call has been made to a method without any mock
        /// cases.
        static void onUnknownCall(void*) {
            // Throw an UnknownCallException.
            throw Exception::UnknownCallException();
        }
};

}

namespace IMock::Internal {

/// Specifies an method offset within a virtual table.
typedef unsigned int VirtualTableOffset;

/// Specifies the size of a virtual table.
typedef VirtualTableOffset VirtualTableSize;

}

namespace IMock::Internal {

// Define a macro creating a virtual method returning its own virtual table
// offset.
#define offset0(id) \
    virtual VirtualTableOffset offset ## id() {\
        return id;\
    }\

// Calls offset0 16 times, creating 16 methods.
#define offset1(id) \
    offset0(id ## 0) \
    offset0(id ## 1) \
    offset0(id ## 2) \
    offset0(id ## 3) \
    offset0(id ## 4) \
    offset0(id ## 5) \
    offset0(id ## 6) \
    offset0(id ## 7) \
    offset0(id ## 8) \
    offset0(id ## 9) \
    offset0(id ## A) \
    offset0(id ## B) \
    offset0(id ## C) \
    offset0(id ## D) \
    offset0(id ## E) \
    offset0(id ## F) \

// Calls offset1 16 times, creating 256 methods.
#define offset2(id) \
    offset1(id ## 0) \
    offset1(id ## 1) \
    offset1(id ## 2) \
    offset1(id ## 3) \
    offset1(id ## 4) \
    offset1(id ## 5) \
    offset1(id ## 6) \
    offset1(id ## 7) \
    offset1(id ## 8) \
    offset1(id ## 9) \
    offset1(id ## A) \
    offset1(id ## B) \
    offset1(id ## C) \
    offset1(id ## D) \
    offset1(id ## E) \
    offset1(id ## F) \

// Calls offset2 16 times, creating 4096 methods.
#define offset3(id) \
    offset2(id ## 0) \
    offset2(id ## 1) \
    offset2(id ## 2) \
    offset2(id ## 3) \
    offset2(id ## 4) \
    offset2(id ## 5) \
    offset2(id ## 6) \
    offset2(id ## 7) \
    offset2(id ## 8) \
    offset2(id ## 9) \
    offset2(id ## A) \
    offset2(id ## B) \
    offset2(id ## C) \
    offset2(id ## D) \
    offset2(id ## E) \
    offset2(id ## F) \

/// A reference struct used by VirtualTableOffsetContext to calculate virtual
/// table offsets and sizes.
struct VirtualTableOffsetReference {
    // Call offset3 to create 4096 offset methods.
    offset3(0x)
};

// Undefine the used macros.
#undef offset1
#undef offset2
#undef offset3

}

namespace IMock::Internal {

/// Contains static methods used to get information about virtual table sizes
/// and offsets within them.
class VirtualTableOffsetContext {
    public:
        /// VirtualTableOffsetContext is not supposed to be instantiated since
        /// it only contains static methods.
        VirtualTableOffsetContext() = delete;

        /// Gets the offset in the virtual table of a provided method in the
        /// interface.
        ///
        /// @param method The method to look up.
        /// @return The virtual table offset of the method.
        template <typename TInterface, typename TReturn, typename ...TArguments>
        static VirtualTableOffset getVirtualTableOffset(
            TReturn (TInterface::*method)(TArguments...)) {    
            // Cast the provided method to a reference method with the same
            // offset in a reference class.
            auto referenceMethod = reinterpret_cast<
                VirtualTableOffset (VirtualTableOffsetReference::*)()>(method);
            
            // Create a VirtualTableOffsetReference to call the reference method
            // on.
            static VirtualTableOffsetReference virtualTableOffsetReference;

            // Call the reference method, whose return value is its virtual
            // table offset.
            return (virtualTableOffsetReference.*referenceMethod)();
        }

        /// Gets the size of the virtual table of an interface.
        ///
        /// @return The size of the virtual table of the interface.
        template <typename TInterface>
        static VirtualTableSize getVirtualTableSize() {
            // Create an interface inheriting the provided interface.
            class DerivedInterface : public TInterface {
                public:
                    /// This method will come after any other methods.
                    virtual void lastMethod() = 0;
            };

            // Get the virtual table offset of the last method, which will be
            // the size of the provided interface's virtual table.
            VirtualTableOffset virtualTableSize = getVirtualTableOffset(
                &DerivedInterface::lastMethod);

            // Return the virtual table size.
            return virtualTableSize;
        }
};

}

namespace IMock::Internal {

/// Stores a raw virtual table for an interface.
template <typename TInterface>
class VirtualTable {
    private:
        // The size of the virtual table.
        VirtualTableSize _virtualTableSize;

        // A pointer to the raw virtual table.
        std::unique_ptr<void*, std::function<void(void**)>> _virtualTable;

    public:
        /// Creates a VirtualTable.
        ///
        /// All methods will initially point to a method throwing an exception
        /// explaining that the method in question has not been mocked.
        VirtualTable()
            : _virtualTableSize(VirtualTableOffsetContext
                ::getVirtualTableSize<TInterface>())
            , _virtualTable(
                new void*[_virtualTableSize],
                [](void** virtualTable) {
                    delete[] virtualTable;
                }) {
            // Fill the virtual table with pointers to
            // UnknownCall::onUnknownCall.
            std::fill(
                _virtualTable.get(),
                _virtualTable.get() + _virtualTableSize,
                reinterpret_cast<void*>(UnknownCall::onUnknownCall));
        }

        /// Gets the raw virtual table.
        ///
        /// @return The raw virtual table.
        void** get() const {
            // Return the raw virtual table.
            return _virtualTable.get();
        }
};

}

namespace IMock::Internal {

/// Mocks a provided interface to perform wanted actions and return certain
/// values when its virtual methods are called.
template <typename TInterface>
class InnerMock {
    private:
        /// A struct used to create objects to use in place of actual instances
        /// of the interface.
        struct MockFake {
            private:
                /// The raw virtual table of the mocked interface.
                void** _virtualTable;

                /// A reference to the InnerMock that did the mocking.
                InnerMock& _mock;

            public:
                /// Creates a MockFake.
                ///
                /// @param virtualTable The raw virtual table of the mocked
                /// interface.
                /// @param mock A reference to the InnerMock that did the
                /// mocking.
                MockFake(
                    void** virtualTable,
                    InnerMock& mock)
                    : _virtualTable(std::move(virtualTable))
                    , _mock(mock) {
                }

                /// Gets the contained InnerMock.
                ///
                /// @return The contained InnerMock.
                InnerMock& getMock() {
                    // Return the contained InnerMock.
                    return _mock;
                }
        };

        /// Maps the MockCaseID's of all mock cases to the virtual table offsets
        /// of their mocked methods.
        std::map<MockCaseID, VirtualTableOffset> _virtualTableOffsets;

        /// Maps the virtual table offsets of the mocked methods to MockMethod
        /// instances dealing with calls to respective method.
        std::map<VirtualTableOffset, std::unique_ptr<IMockMethodNonGeneric>>
            _mockMethods;

        /// A VirtualTable to add mocked methods to.
        VirtualTable<TInterface> _virtualTable;

        /// A MockFake used by the InnerMock.
        MockFake _mockFake;

    public:
        /// Creates an InnerMock.
        InnerMock()
            : _mockFake(_virtualTable.get(), *this) {
        }

        /// Gets a reference to an object used in place of an instance of the
        /// interface.
        TInterface& get() {
            // Cast the MockFake to a TInterface and return a reference to it.
            return reinterpret_cast<TInterface&>(_mockFake);
        }

        /// Adds a mock case to the provided method.
        ///
        /// @param method The method to add a mock case to.
        /// @param mockCase The mock case to add.
        /// @return A MockCaseCallCount that can be queried about the number of
        /// calls done to the added mock case.
        template <MockCaseID id, typename TReturn, typename ...TArguments>
        MockCaseCallCount addCase(
            TReturn (TInterface::*method)(TArguments...),
            std::string methodString,
            std::unique_ptr<ICase<TReturn, TArguments...>> mockCase) {
            // Forward the call to addCaseWithOnCall.
            return addCaseWithOnCall<TReturn, TArguments...>(
                id,
                onCall<id, TReturn, TArguments...>,
                method,
                std::move(methodString),
                std::move(mockCase));
        }

    private:
        /// Adds a mock case to the provided method.
        ///
        /// @param id The MockCaseID of the addCase method that made the
        /// internal call.
        /// @param onCall The onCall method to insert into the virtual table if
        /// required.
        /// @param method The method to add a mock case to.
        /// @param mockCase The mock case to add.
        /// @return A MockCaseCallCount that can be queried about the number of
        /// calls done to the added mock case.
        template <typename TReturn, typename ...TArguments>
        MockCaseCallCount addCaseWithOnCall(
            MockCaseID id,
            TReturn (*onCall)(MockFake*, TArguments...),
            TReturn (TInterface::*method)(TArguments...),
            std::string methodString,
            std::unique_ptr<ICase<TReturn, TArguments...>> mockCase) {
            // Check if a virtual table offset has been stored in
            // _virtualTableOffsets for the provided MockCaseID.
            bool virtualTableOffsetsNoID = _virtualTableOffsets.count(id) == 0;
            if(virtualTableOffsetsNoID) {
                // Get the virtual table offset of the method and store it.
                _virtualTableOffsets[id] = VirtualTableOffsetContext
                    ::getVirtualTableOffset(method);
            }

            // Get the virtual table offset of the provided MockCaseID.
            VirtualTableOffset virtualTableOffset = _virtualTableOffsets[id];

            // Check if the method has any existing mock cases.
            bool methodHasNoMocks = _mockMethods.count(virtualTableOffset) == 0;
            if(methodHasNoMocks) {
                // Create and store a MockMethod if the method has no existing
                // mock cases.
                _mockMethods[virtualTableOffset] = makeUnique<
                    MockMethod<TReturn, TArguments...>>(
                        std::move(methodString));

                // Store a pointer to onCall in the virtual table.
                _virtualTable.get()[virtualTableOffset]
                    = reinterpret_cast<void*>(onCall);
            }
            
            // Get the MockMethod for the method and add a mock case to it.
            return getMockMethod<TReturn, TArguments...>(virtualTableOffset)
                .addCase(std::move(mockCase));
        }

        /// Called when a call to a method in the interface is called.
        ///
        /// @param mockFake The MockFake instance this call was made on.
        /// @param arguments The arguments the method was called with.
        template <MockCaseID id, typename TReturn, typename ...TArguments>
        static TReturn onCall(MockFake* mockFake, TArguments... arguments) {
            // Forward the call to onCallWithMockCaseID.
            return onCallWithMockCaseID<TReturn, TArguments...>(
                id,
                mockFake,
                std::move(arguments)...);
        }

        /// Called when a call to a method in the interface is called.
        ///
        /// @param id The MockCaseID of the onCall method that made the internal
        /// call.
        /// @param mockFake The MockFake instance this call was made on.
        /// @param arguments The arguments the method was called with.
        template <typename TReturn, typename ...TArguments>
        static TReturn onCallWithMockCaseID(
            MockCaseID id,
            MockFake* mockFake,
            TArguments... arguments) {
            // Get the InnerMock.
            InnerMock& mock = mockFake->getMock();

            // Get the virtual table offset for the MockCaseID.
            VirtualTableOffset virtualTableOffset
                = mock._virtualTableOffsets[id];

            // Get the MockMethod for the called method and forward the call to
            // onCall.
            return mock
                .getMockMethod<TReturn, TArguments...>(virtualTableOffset)
                .onCall(std::move(arguments)...);
        }

        /// Gets the MockMethod for the method with the provided virtual table
        /// offset.
        ///
        /// @param virtualTableOffset The method's virtual table offset.
        /// @return The method's MockMethod.
        template <typename TReturn, typename ...TArguments>
        MockMethod<TReturn, TArguments...>& getMockMethod(
            VirtualTableOffset virtualTableOffset) {
            // Get the MockMethod from _mockMethods.
            IMockMethodNonGeneric& mockMethodNonGeneric
                = *_mockMethods[virtualTableOffset].get();

            // Cast mockMethodNonGeneric to its correct type.
            MockMethod<TReturn, TArguments...>& mockMethod
                = reinterpret_cast<MockMethod<TReturn, TArguments...>&>(
                    mockMethodNonGeneric);

            // Return the MockMethod.
            return mockMethod;
        }
};

}

namespace IMock::Internal {

/// Utility to create CaseMatch instances.
class CaseMatchFactory {
    public:
        /// CaseMatchFactory is not supposed to be instantiated since it only
        /// contains static methods.
        CaseMatchFactory() = delete;

        /// Creates a CaseMatch indicating no match has been made.
        template <typename TReturn>
        static CaseMatch<TReturn> noMatch() {
            /// Create and return a CaseMatch without a return value.
            return CaseMatch<TReturn>(std::unique_ptr<IReturnValue<TReturn>>(
                nullptr));
        }

        /// Creates a CaseMatch indicating a match has been made with a method
        /// that has a return value.
        ///
        /// @param returnValue The call's return value.
        /// @return A CaseMatch indicating a match with the provided return
        /// value.
        template <typename TReturn>
        static CaseMatch<TReturn> match(TReturn returnValue) {
            // Create and return a CaseMatch with the provided return value.
            return CaseMatch<TReturn>(makeUnique<NonVoidReturnValue<TReturn>>(
                std::move(returnValue)));
        }

        /// Creates a CaseMatch indicating a match mas been made with a method
        /// with the return type void.
        ///
        /// @return A CaseMatch indicating a match.
        static CaseMatch<void> matchVoid() {
            // Create and return a CaseMatch with a VoidReturnValue.
            return CaseMatch<void>(makeUnique<VoidReturnValue>());
        }

        /// Creates a CaseMatch indicating a match has been made and handle the
        /// match with a fake.
        ///
        /// @param fake The fake to call.
        /// @param arguments The arguments to call the fake with.
        /// @return A CaseMatch indicating a match with the provided fake.
        template <typename TReturn, typename ...TArguments>
        static CaseMatch<TReturn> matchFake(
            std::function<TReturn (TArguments...)> fake,
            std::tuple<TArguments...> arguments) {
            // Create and return a CaseMatch with the fake and the arguments.
            return CaseMatch<TReturn>(
                makeUnique<FakeReturnValue<TReturn, TArguments...>>(
                    std::move(fake),
                    std::move(arguments)));
        }
};

}

namespace IMock::Internal {

/// An ICase always calling a provided callback.
template <typename TReturn, typename ...TArguments>
class MockWithMethodCase : public ICase<TReturn, TArguments...> {
    private:
        /// A callback to be called.
        std::function<TReturn (TArguments...)> _fake;

    public:
        /// Creates a MockWithMethodCase.
        ///
        /// @param fake A callback to be called.
        MockWithMethodCase(
            std::function<TReturn (TArguments...)> fake)
            : _fake(std::move(fake)) {
            }

        CaseMatch<TReturn> matches(std::tuple<TArguments...>& arguments) {
            // Return a CaseMatch for the fake.
            return Internal::CaseMatchFactory::matchFake(
                _fake,
                std::move(arguments));
        }
};

}

namespace IMock::Exception {

/// Thrown when a MockWithArguments is used twice, since the arguments are
/// moved.
class MockWithArgumentsUsedTwiceException : public MockException {
    public:
        /// Creates a MockWithArgumentsUsedTwiceException.
        MockWithArgumentsUsedTwiceException()
            : MockException("A MockWithArguments instance was reused."
                " This is not possible since the arguments are moved when"
                " adding a case.") {
        }
};

}

namespace IMock::Internal {

/// An ICase checking if calls match provided arguments.
template <typename TReturn, typename ...TArguments>
class MockWithArgumentsCase : public ICase<TReturn, TArguments...> {
    private:
        /// The arguments to check calls with.
        std::tuple<TArguments...> _arguments;

        /// A callback to call if the arguments match.
        std::function<CaseMatch<TReturn> (std::tuple<TArguments...>)> _fake;

    public:
        /// Creates a MockWithArgumentsCase.
        ///
        /// @param arguments The arguments to check calls with.
        /// @param fake A callback to call if the arguments match.
        MockWithArgumentsCase(
            std::tuple<TArguments...> arguments,
            std::function<CaseMatch<TReturn> (std::tuple<TArguments...>)> fake)
            : _arguments(std::move(arguments))
            , _fake(std::move(fake)) {
            }

        CaseMatch<TReturn> matches(std::tuple<TArguments...>& arguments) {
            // Check if the call arguments matches the mock case's arguments.
            if(arguments == _arguments) {
                // Call _fake and return its return value if a match has been
                // made. The arguments are moved to _fake as they will never be
                // read again.
                return _fake(std::move(arguments));
            }
            else {
                // Return a CaseMatch indicating no match has been made.
                return CaseMatchFactory::noMatch<TReturn>();
            }
        }
};

}

namespace IMock {

/// A Mock with an associated method and arguments to add a mock case for.
template <typename TInterface, MockCaseID id, typename TReturn,
    typename ...TArguments>
class MockWithArguments {
    private:
        /// The InnerMock to add a mock case to.
        Internal::InnerMock<TInterface>& _mock;

        /// The method to add a mock case to.
        TReturn (TInterface::*_method)(TArguments...);

        /// The arguments to match calls with.
        std::tuple<TArguments...> _arguments;

        /// Describes if the instance already has been used.
        bool _used;

        /// A string describing how a call is made to the method being mocked.
        std::string _methodString;

    public:
        /// Creates a MockWithArguments.
        ///
        /// @param mock The InnerMock to add a mock case to.
        /// @param method The method to add a mock case to.
        /// @param arguments The arguments to match calls with.
        /// @param methodString A string describing how a call is made to the
        /// method being mocked.
        MockWithArguments(
            Internal::InnerMock<TInterface>& mock,
            TReturn (TInterface::*method)(TArguments...),
            std::string methodString,
            std::tuple<TArguments...> arguments)
            : _mock(mock)
            , _method(std::move(method))
            , _methodString(std::move(methodString))
            , _arguments(std::move(arguments))
            , _used(false) {
        }

        // The solution for dealing with void has been taken from:
        // https://eli.thegreenplace.net/2014/sfinae-and-enable_if/

        /// Adds a mock case making the associated method return the provided
        /// value when called with the associated arguments.
        ///
        /// The method is available unless the return type is void. 
        ///
        /// @param returnValue The value to return when a match happens.
        /// @return A MockCaseCallCount that can be queried about the number of
        /// calls done to the added mock case.
        template<typename R = TReturn>
        MockCaseCallCount returns(
            typename std::enable_if<!std::is_void<R>::value, TReturn>::type
                returnValue) {
            // Wrap the return value in a tuple. Otherwise, reference values
            // would not work properly when capturing them.
            std::tuple<TReturn> wrappedReturnValue = (returnValue);

            // Create a fake.
            std::function<Internal::CaseMatch<TReturn>
                (std::tuple<TArguments...>)> fake
                = [wrappedReturnValue] (std::tuple<TArguments...> arguments) {
                    // Return a CaseMatch containing the return value.
                    return Internal::CaseMatchFactory::match<TReturn>(
                        std::get<0>(wrappedReturnValue));
                };

            // Forward the call to fakeGeneral.
            return fakeGeneral(fake);
        }

        /// Adds a mock case making the associated method callable when called
        /// with the associated arguments.
        ///
        /// The method is only available if the return type is void.
        ///
        /// @return A MockCaseCallCount that can be queried about the number of
        /// calls done to the added mock case.
        template<typename R = TReturn,
            typename std::enable_if<std::is_void<R>::value, R>::type* = nullptr>
        MockCaseCallCount returns() {
            // Create a fake.
            std::function<Internal::CaseMatch<TReturn>
                (std::tuple<TArguments...>)> fake
                = [] (std::tuple<TArguments...> arguments) {
                    // Return a CaseMatch for void.
                    return Internal::CaseMatchFactory::matchVoid();
                };

            // Forward the call to fakeGeneral.
            return fakeGeneral(fake);
        }

        /// Adds a fake handling the method call when called with the associated
        /// arguments.
        ///
        /// @param fake A callback to call when the method is called and a match
        /// happens.
        /// @return A MockCaseCallCount that can be queried about the number of
        /// calls done to the added mock case.
        MockCaseCallCount fake(std::function<TReturn (TArguments...)> fake) {
            // Call fakeGeneral with a fake.
            return fakeGeneral([fake](std::tuple<TArguments...> arguments) {
                // Return a CaseMatch for the fake.
                return Internal::CaseMatchFactory::matchFake(
                    fake,
                    std::move(arguments));
            });
        }

    private:
        /// Adds a mock case making calls to the associated method be forwarded
        /// to fake when called with the associated arguments.
        ///
        /// @param fake A callback to be called when a match happens.
        /// @return A MockCaseCallCount that can be queried about the number of
        /// calls done to the added mock case.
        MockCaseCallCount fakeGeneral(
            std::function<Internal::CaseMatch<TReturn>
                (std::tuple<TArguments...>)> fake) {
            // Check if the instance already has been used.
            if(_used) {
                // Throw a MockWithArgumentsUsedTwiceException since the
                // instance cannot be used again as the arguments has been
                // moved.
                throw Exception::MockWithArgumentsUsedTwiceException();
            }
            else {
                // Raise the _used flag to mark that the instance has been used.
                _used = true;
            }

            // Create a MockWithArgumentsCase.
            std::unique_ptr<Internal::ICase<TReturn, TArguments...>> mockCase
                = Internal::makeUnique<
                Internal::MockWithArgumentsCase<TReturn, TArguments...>>(

                // Move the arguments, which means the instance cannot be used
                // again.
                std::move(_arguments),
                fake);

            // Add the case to InnerMock.
            return _mock.template addCase<id, TReturn, TArguments...>(
                _method,
                std::move(_methodString),
                std::move(mockCase));
        }
};

}

namespace IMock {

/// A Mock with an associated method to add a mock case for.
template <typename TInterface, MockCaseID id, typename TReturn,
    typename ...TArguments>
class MockWithMethod {
    private:
        /// The InnerMock to add a mock case to.
        Internal::InnerMock<TInterface>& _mock;

        /// The method to add a mock case to.
        TReturn (TInterface::*_method)(TArguments...);

        /// A string describing how a call is made to the method being mocked.
        std::string _methodString;

    public:
        /// Creates a MockWithMethod.
        ///
        /// @param mock The InnerMock to add a mock case to.
        /// @param method The method to add a mock case to.
        /// @param methodString A string describing how a call is made to the
        /// method being mocked.
        MockWithMethod(
            Internal::InnerMock<TInterface>& mock,
            TReturn (TInterface::*method)(TArguments...),
            std::string methodString)
            : _mock(mock)
            , _method(std::move(method))
            , _methodString(std::move(methodString)) {
        }

        /// Creates a MockWithArguments used to add a mock case matching the
        /// provided arguments.
        ///
        /// @param arguments The arguments to match.
        /// @return A MockWithArguments associated with the arguments.
        MockWithArguments<TInterface, id, TReturn, TArguments...> with(
            TArguments... arguments) {
            // Create and return a MockWithArguments with the InnerMock,
            // the method, the call string and the arguments.
            return MockWithArguments<TInterface, id, TReturn, TArguments...>(
                _mock,
                _method,
                _methodString,
                std::tuple<TArguments...>(std::move(arguments)...));
        }

        /// Adds a fake handling the method call.
        ///
        /// @param fake A callback to call when the method is called.
        /// @return A MockCaseCallCount that can be queried about the number of
        /// calls done to the added mock case.
        MockCaseCallCount fake(std::function<TReturn (TArguments...)> fake) {
            // Create a MockWithMethodCase.
            std::unique_ptr<Internal::ICase<TReturn, TArguments...>> mockCase
                = Internal::makeUnique<Internal::MockWithMethodCase<
                    TReturn, TArguments...>>(fake);

            // Add the case to InnerMock.
            return _mock.template addCase<id, TReturn, TArguments...>(
                _method,
                std::move(_methodString),
                std::move(mockCase));
        }
};

}

namespace IMock {

/// A Mock with an associated MockCaseID used to add a mock case.
template <typename TInterface, MockCaseID id>
class MockWithID {
    private:
        /// The InnerMock to add a mock case to.
        Internal::InnerMock<TInterface>& _mock;

    public:
        /// Creates a MockWithID.
        ///
        /// @param mock The InnerMock to add a mock case to.
        MockWithID(Internal::InnerMock<TInterface>& mock)
            : _mock(mock) {
        }

        /// Creates a MockWithMethod used to add a mock case to the provided
        /// method.
        ///
        /// @param method The method to add mock cases for.
        /// @param methodString A string describing how a call is made to the
        /// method being mocked.
        /// @return A MockWithMethod associated with the method.
        template <typename TReturn, typename ...TArguments>
        MockWithMethod<TInterface, id, TReturn, TArguments...> withMethod(
            TReturn (TInterface::*method)(TArguments...),
            std::string methodString) {
            // Create and return a MockWithMethod with the InnerMock,
            // the method and the call string.
            return MockWithMethod<TInterface, id, TReturn, TArguments...>(
                _mock,
                method,
                std::move(methodString));
        }
};

}

namespace IMock {

/// Mocks a provided interface to perform wanted actions and return certain
/// values when its virtual methods are called.
///
/// The method of this is done is as follows: Every object created from a class
/// or struct with virtual methods have a hidden field that appears first in
/// memory. This field contains a pointer to a buffer containing pointers to the
/// virtual methods in the same order as they are declared.
/// Normally, the compiler fills this buffer. However, IMock instead creates an
/// object with a pointer to a buffer filled with custom methods that can be
/// mocked or faked.
template <typename TInterface>
class Mock {
    private:
        /// An InnerMock containing further logic.
        Internal::InnerMock<TInterface> _innerMock;

    public:
        /// Gets an instance of the interface where the virtual methods have
        /// been mocked.
        TInterface& get() {
            // Call _innerMock.get to get the instance.
            return _innerMock.get();
        }

        /// Creates a MockWithID used to add a mock case.
        /// The MockCaseID must differ from other mock cases used with the same
        /// Mock.
        /// @return A MockWithID associated with the MockCaseID.
        template <MockCaseID id>
        MockWithID<TInterface, id> withCounter() {
            // Create an return a MockWithID with _innerMock.
            return MockWithID<TInterface, id>(_innerMock);
        }
};

}

/// Gets the interface type the provided Mock mocks.
#define mockType(mock) \
    std::remove_reference<decltype((mock).get())>::type

/// Call this with a Mock and a method on the mocked interface to get a
/// MockWithMethod to use to add a mock case.
#define when(mock, method) \
    mock.withCounter<__COUNTER__>().withMethod( \
        &mockType(mock)::method, \
        #mock ".get()." #method)
