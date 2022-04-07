#pragma once

#include <exception>
#include <functional>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>

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

class UnknownCallException : public MockException {
    public:
        UnknownCallException()
            : MockException("A call was made to a method that has not been "
                "mocked.") {
        }
};

}

namespace IMock::Exception {

class UnmockedCallException : public MockException {
    public:
        UnmockedCallException()
            : MockException("A call was made to a method that has been mocked "
                "but the arguments does not match the mocked arguments.") {
        }
};

}

namespace IMock::Exception {

class WrongCallCountException : public MockException {
    public:
        WrongCallCountException(
            int expectedCallCount,
            int actualCallCount) 
            : MockException(getMessage(
                expectedCallCount,
                actualCallCount)) {
        }

    private:
        static std::string getMessage(
            int expectedCallCount,
            int actualCallCount) {
            std::stringstream out;

            out << "Expected the method to be called "
                << expectedCallCount
                << " time";
            
            if(expectedCallCount != 1) {
                out << "s";
            }

            out << " but it was called "
                << actualCallCount
                << " time";

            if(actualCallCount != 1) {
                out << "s";
            }

            out << ".";

            return out.str();
        }
};

}

namespace IMock::Internal {

class UnknownCall {
    public:
        static void onUnknownCall(void*) {
            throw Exception::UnknownCallException();
        }

    private:
        UnknownCall();
};

}

namespace IMock::Internal {

typedef unsigned int VirtualOffset;

typedef VirtualOffset VirtualTableSize;

#define offset0(id) \
    virtual VirtualOffset offset ## id() {\
        return id;\
    }\

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

struct VirtualOffsetReference {
    offset3(0x)
};

#undef offset1
#undef offset2
#undef offset3

template <typename TInterface, typename TReturn, typename ...TArguments>
VirtualOffset getVirtualOffset(TReturn (TInterface::*method)(TArguments...)) {    
    auto referenceMethod = 
        reinterpret_cast<VirtualOffset (VirtualOffsetReference::*)()>(method);
    
    static VirtualOffsetReference virtualOffsetReference;

    return (virtualOffsetReference.*referenceMethod)();
}

template <typename TInterface>
VirtualTableSize getVirtualTableSize() {
    class DerivedInterface : public TInterface {
        public:
            virtual void lastMethod() = 0;
    };

    VirtualOffset virtualTableSize = getVirtualOffset(
        &DerivedInterface::lastMethod);

    return virtualTableSize;
}

}

namespace IMock::internal {

template <typename TReturn>
class IReturnValue {
    public:
        virtual ~IReturnValue() {
        }

        virtual TReturn getReturnValue() = 0;
};

class VoidReturnValue : public IReturnValue<void> {
    public:
        virtual void getReturnValue() override {
        }
};

template <typename TReturn>
class NonVoidReturnValue : public IReturnValue<TReturn> {
    private:
        TReturn _returnValue;

    public:
        NonVoidReturnValue(TReturn returnValue)
            : _returnValue(returnValue) {
        }
        
        virtual TReturn getReturnValue() override {
            return _returnValue;
        }
};

}

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

namespace IMock {

namespace internal {

template<typename _Tp, typename... _Args>
inline std::unique_ptr<_Tp> make_unique(_Args&&... __args)
{
    return std::unique_ptr<_Tp>(
        new _Tp(std::forward<_Args>(__args)...));
}

// Trick to statically produce a list of integers. Solution taken from:
// https://stackoverflow.com/a/7858971/6188897
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

template<int ...S, typename TClass, typename TReturn, typename ...TArguments>
TReturn applyWithSeq(
    seq<S...>,
    TReturn (TClass::*callback)(TArguments...),
    TClass& self,
    std::tuple<TArguments...> arguments) {
    return (self.*callback)(std::move(std::get<S>(arguments))...);
}

template<typename TClass, typename TReturn, typename ...TArguments>
TReturn apply(
    TReturn (TClass::*callback)(TArguments...),
    TClass& self,
    std::tuple<TArguments...> arguments) {
    return applyWithSeq(typename gens<sizeof...(TArguments)>::type(),
        callback,
        self,
        std::move(arguments));
}

class ICaseNonGeneric {
    public:
        virtual ~ICaseNonGeneric() {
        }
};

template <typename TReturn, typename ...TArguments> 
class ICase : public ICaseNonGeneric {
    public:
        virtual TReturn call(TArguments... arguments) = 0;
};

template <typename TReturn, typename ...TArguments>
class UnmockedCase : public ICase<TReturn, TArguments...> {
    public:
        virtual TReturn call(TArguments... arguments) override {
            throw Exception::UnmockedCallException();
        }
};

template <typename TReturn, typename ...TArguments>
class MockedCase : public ICase<TReturn, TArguments...> {
    private:
        std::unique_ptr<ICase<TReturn, TArguments...>> _previousCase;
        std::shared_ptr<Internal::MockCaseMutableCallCount> _callCount;
        std::unique_ptr<IReturnValue<TReturn>> _returnValue;
        std::tuple<TArguments...> _arguments;

    public:
        MockedCase(
            std::unique_ptr<ICase<TReturn, TArguments...>> previousCase,
            std::shared_ptr<Internal::MockCaseMutableCallCount> callCount,
            std::unique_ptr<IReturnValue<TReturn>> returnValue,
            std::tuple<TArguments...> arguments)
            : _previousCase(std::move(previousCase))
            , _callCount(std::move(callCount))
            , _returnValue(std::move(returnValue))
            , _arguments(std::move(arguments)) {
        }

        virtual TReturn call(TArguments... arguments) override {
            std::tuple<TArguments...> tupleArguments(std::move(arguments)...);
            bool match = tupleArguments == _arguments;
            if(match) {
                _callCount->increase();
                return _returnValue->getReturnValue();
            }
            else {
                return apply(
                    &ICase<TReturn, TArguments...>::call,
                    *_previousCase,
                    std::move(tupleArguments));
            }
        }
};

}

typedef unsigned int ID;

template <typename TInterface>
class Mock {
    private:
        struct MockFake {
            private:
                void** _virtualTable;
                Mock* _mock;

            public:
                MockFake(
                    void** virtualTable,
                    Mock* mock)
                    : _virtualTable(virtualTable)
                    , _mock(mock) {
                }

                Mock* getMock() {
                    return _mock;
                }
        };

        template <ID id, typename TReturn, typename ...TArguments>
        class MockWithMethod;

        template <ID id>
        class MockWithCounter {
            private:
                Mock* _mock;

            public:
                MockWithCounter(Mock* mock)
                    : _mock(mock) {
                }

                template <typename TReturn, typename ...TArguments>
                MockWithMethod<id, TReturn, TArguments...> withMethod(
                    TReturn (TInterface::*method)(TArguments...)) {
                    return MockWithMethod<id, TReturn, TArguments...>(
                        _mock,
                        method);
                }
        };

        template <ID id, typename TReturn, typename ...TArguments>
        class MockWithArguments;

        template <ID id, typename TReturn, typename ...TArguments>
        class MockWithMethod {
            private:
                Mock* _mock;
                TReturn (TInterface::*_method)(TArguments...);

            public:
                MockWithMethod(
                    Mock* mock,
                    TReturn (TInterface::*method)(TArguments...))
                    : _mock(mock)
                    , _method(method) {
                }

                MockWithArguments<id, TReturn, TArguments...> with(
                    TArguments... arguments) {
                    return MockWithArguments<id, TReturn, TArguments...>(
                        _mock,
                        _method,
                        std::tuple<TArguments...>(std::move(arguments)...));
                }
        };

        template <ID id, typename TReturn, typename ...TArguments>
        class MockWithArguments {
            private:
                Mock* _mock;
                TReturn (TInterface::*_method)(TArguments...);
                std::tuple<TArguments...> _arguments;

            public:
                MockWithArguments(
                    Mock* mock,
                    TReturn (TInterface::*method)(TArguments...),
                    std::tuple<TArguments...> arguments)
                    : _mock(std::move(mock))
                    , _method(std::move(method))
                    , _arguments(std::move(arguments)) {
                }

                // Solution taken from:
                // https://eli.thegreenplace.net/2014/sfinae-and-enable_if/
                template<typename R = TReturn>
                MockCaseCallCount returns(
                    typename std::enable_if<!std::is_void<R>::value,
                    TReturn>::type returnValue) {
                    std::unique_ptr<internal::IReturnValue<TReturn>>
                        wrappedReturnValue = internal::make_unique<
                        internal::NonVoidReturnValue<TReturn>>(returnValue);

                    return _mock->addCase<id, TReturn, TArguments...>(
                        _method,
                        std::move(wrappedReturnValue),
                        std::move(_arguments));
                }

                template<typename R = TReturn,
                    typename std::enable_if<std::is_void<R>::value, R>::type* =
                    nullptr>
                MockCaseCallCount returns() {
                    std::unique_ptr<internal::IReturnValue<TReturn>>
                        wrappedReturnValue = internal::make_unique<
                        internal::VoidReturnValue>();

                    return _mock->addCase<id, TReturn, TArguments...>(
                        _method,
                        std::move(wrappedReturnValue),
                        std::move(_arguments));
                }
        };

        std::map<ID, Internal::VirtualOffset> _virtualOffsets;
        std::map<
            Internal::VirtualOffset,
            std::unique_ptr<internal::ICaseNonGeneric>
        > _cases;

        Internal::VirtualTableSize _virtualTableSize;
        std::unique_ptr<void*, std::function<void(void**)>> _virtualTable;

        MockFake _mockFake;

    public:
        Mock() 
            : _virtualTableSize(Internal::getVirtualTableSize<TInterface>())
            , _virtualTable(
                new void*[_virtualTableSize],
                [](void** virtualTable) {
                    delete[] virtualTable;
                })
            , _mockFake(_virtualTable.get(), this) {
            std::fill(
                _virtualTable.get(),
                _virtualTable.get() + _virtualTableSize,
                reinterpret_cast<void*>(Internal::UnknownCall::onUnknownCall));
        }

        virtual ~Mock() {
        }

        TInterface& get() {
            return *reinterpret_cast<TInterface*>(&_mockFake);
        }

        template <ID id, typename TReturn, typename ...TArguments>
        MockCaseCallCount addCase(
            TReturn (TInterface::*method)(TArguments...),
            std::unique_ptr<internal::IReturnValue<TReturn>> returnValue,
            std::tuple<TArguments...> arguments) {
            Internal::VirtualOffset virtualOffset =
                Internal::getVirtualOffset(method);

            bool virtualOffsetsNoID = _virtualOffsets.count(id) == 0;
            if(virtualOffsetsNoID) {
                _virtualOffsets[id] = virtualOffset;
            }

            bool methodHasNoMocks = _cases.count(virtualOffset) == 0;
            if(methodHasNoMocks) {
                _cases[virtualOffset] = internal::make_unique<
                    internal::UnmockedCase<TReturn, TArguments...>>();
                _virtualTable.get()[virtualOffset] =
                    reinterpret_cast<void*>(onCall<id, TReturn, TArguments...>);
            }
            std::unique_ptr<internal::ICase<TReturn, TArguments...>>*
                previousCase = reinterpret_cast<std::unique_ptr<internal::ICase<
                    TReturn, TArguments...>>*>(&_cases.at(virtualOffset));
            
            std::shared_ptr<Internal::MockCaseMutableCallCount> callCountPointer 
                = std::make_shared<Internal::MockCaseMutableCallCount>();
            
            _cases[virtualOffset] = internal::make_unique<internal::MockedCase<
                TReturn, TArguments...>>(
                    std::move(*previousCase),
                    callCountPointer,
                    std::move(returnValue),
                    std::move(arguments));
            
            MockCaseCallCount callCount(callCountPointer);

            return callCount;
        }

        template <ID id>
        MockWithCounter<id> withCounter() {
            return MockWithCounter<id>(this);
        }

    private:
        template <ID id, typename TReturn, typename ...TArguments>
        static TReturn onCall(MockFake* mockFake, TArguments... arguments) {
            Mock* mock = mockFake->getMock();
            Internal::VirtualOffset virtualOffset = mock->_virtualOffsets[id];
            internal::ICaseNonGeneric* caseNonGeneric =
                mock->_cases[virtualOffset].get();
            internal::ICase<TReturn, TArguments...>* _case =
                reinterpret_cast<internal::ICase<TReturn, TArguments...>*>(
                    caseNonGeneric);
            return _case->call(std::move(arguments)...);
        }
};

}

#define mockType(mock) \
    std::remove_reference<decltype((mock).get())>::type

#define when(mock, method) \
    mock.withCounter<__COUNTER__>().withMethod(&mockType(mock)::method)
