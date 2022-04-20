#pragma once

#include <exception>
#include <functional>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>

namespace IMock {

typedef unsigned int MockCaseID;

}

namespace IMock::Internal {

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
            : _returnValue(std::move(returnValue)) {
        }
        
        virtual TReturn getReturnValue() override {
            return _returnValue;
        }
};

}

namespace IMock::Internal {

template <typename TReturn>
class CaseMatch {
    private:
        std::unique_ptr<IReturnValue<TReturn>> _returnValue;

    public:
        CaseMatch(std::unique_ptr<IReturnValue<TReturn>> returnValue)
            : _returnValue(std::move(returnValue)) {
        }

        CaseMatch(CaseMatch&& other)
            : _returnValue(std::move(other._returnValue)) {
        }

        virtual ~CaseMatch() {
        }

        bool isMatch() {
            return _returnValue.get() != nullptr;
        }

        IReturnValue<TReturn>& getReturnValue() {
            return *_returnValue.get();
        }
};

}

namespace IMock::Internal {

template<typename _Tp, typename... _Args>
std::unique_ptr<_Tp> make_unique(_Args&&... __args)
{
    return std::unique_ptr<_Tp>(
        new _Tp(std::forward<_Args>(__args)...));
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

class UnmockedCallException : public MockException {
    public:
        UnmockedCallException()
            : MockException("A call was made to a method that has been mocked "
                "but the arguments does not match the mocked arguments.") {
        }
};

}

namespace IMock::Internal {

template <typename TReturn, typename ...Arguments>
class ICase {
    public:
        virtual ~ICase() noexcept {
        }

        virtual CaseMatch<TReturn> matches(std::tuple<Arguments...>& arguments)
            = 0;
};

}

namespace IMock::Internal {

class IMockMethodNonGeneric {
    public:
        virtual ~IMockMethodNonGeneric() {
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

namespace IMock::Exception {

class UnknownCallException : public MockException {
    public:
        UnknownCallException()
            : MockException("A call was made to a method that has not been "
                "mocked.") {
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

}

namespace IMock::Internal {

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

}

namespace IMock::Internal {

class VirtualOffsetContext {
    public:
        template <typename TInterface, typename TReturn, typename ...TArguments>
        static VirtualOffset getVirtualOffset(
            TReturn (TInterface::*method)(TArguments...)) {    
            auto referenceMethod = reinterpret_cast<
                VirtualOffset (VirtualOffsetReference::*)()>(method);
            
            static VirtualOffsetReference virtualOffsetReference;

            return (virtualOffsetReference.*referenceMethod)();
        }

        template <typename TInterface>
        static VirtualTableSize getVirtualTableSize() {
            class DerivedInterface : public TInterface {
                public:
                    virtual void lastMethod() = 0;
            };

            VirtualOffset virtualTableSize = getVirtualOffset(
                &DerivedInterface::lastMethod);

            return virtualTableSize;
        }
    
    private:
        VirtualOffsetContext();
};

}

namespace IMock::Internal {

template <typename TInterface>
class VirtualTable {
    private:
        VirtualTableSize _virtualTableSize;
        std::unique_ptr<void*, std::function<void(void**)>> _virtualTable;

    public:
        VirtualTable() 
            : _virtualTableSize(VirtualOffsetContext
                ::getVirtualTableSize<TInterface>())
            , _virtualTable(
                new void*[_virtualTableSize],
                [](void** virtualTable) {
                    delete[] virtualTable;
                }) {
            std::fill(
                _virtualTable.get(),
                _virtualTable.get() + _virtualTableSize,
                reinterpret_cast<void*>(UnknownCall::onUnknownCall));
        }

        void** get() const {
            return _virtualTable.get();
        }
};

}

namespace IMock::Internal {

template <typename TInterface>
class InnerMock {
    private:
        struct MockFake {
            private:
                void** _virtualTable;
                InnerMock* _mock;

            public:
                MockFake(
                    void** virtualTable,
                    InnerMock* mock)
                    : _virtualTable(virtualTable)
                    , _mock(mock) {
                }

                InnerMock* getMock() {
                    return _mock;
                }
        };

        std::map<MockCaseID, VirtualOffset> _virtualOffsets;
        std::map<VirtualOffset, std::unique_ptr<IMockMethodNonGeneric>>
            _mockMethods;

        VirtualTable<TInterface> _virtualTable;

        MockFake _mockFake;

    public:
        InnerMock()
            : _mockFake(_virtualTable.get(), this) {
        }

        virtual ~InnerMock() {
        }

        TInterface& get() {
            return *reinterpret_cast<TInterface*>(&_mockFake);
        }

        template <MockCaseID id, typename TReturn, typename ...TArguments>
        MockCaseCallCount addCase(
            TReturn (TInterface::*method)(TArguments...),
            std::unique_ptr<ICase<TReturn, TArguments...>> mockCase) {
            // TODO: Move the logic to a method that doesn't include id as a
            // template parameter.
            VirtualOffset virtualOffset =
                VirtualOffsetContext::getVirtualOffset(method);

            bool virtualOffsetsNoID = _virtualOffsets.count(id) == 0;
            if(virtualOffsetsNoID) {
                _virtualOffsets[id] = virtualOffset;
            }

            bool methodHasNoMocks = _mockMethods.count(virtualOffset) == 0;
            if(methodHasNoMocks) {
                _mockMethods[virtualOffset] = make_unique<
                    MockMethod<TReturn, TArguments...>>();

                _virtualTable.get()[virtualOffset] =
                    reinterpret_cast<void*>(onCall<id, TReturn, TArguments...>);
            }
            
            return getMockMethod<TReturn, TArguments...>(virtualOffset)
                .addCase(std::move(mockCase));
        }

    private:
        template <MockCaseID id, typename TReturn, typename ...TArguments>
        static TReturn onCall(MockFake* mockFake, TArguments... arguments) {
            // TODO: Move the logic to a method that doesn't include id as a
            // template parameter.
            InnerMock& mock = *mockFake->getMock();

            VirtualOffset virtualOffset = mock._virtualOffsets[id];

            return mock
                .getMockMethod<TReturn, TArguments...>(virtualOffset)
                .onCall(std::move(arguments)...);
        }

        template <typename TReturn, typename ...TArguments>
        MockMethod<TReturn, TArguments...>& getMockMethod(
            VirtualOffset virtualOffset) {
            IMockMethodNonGeneric* mockMethodNonGeneric
                = _mockMethods[virtualOffset].get();

            MockMethod<TReturn, TArguments...>* mockMethod
                = reinterpret_cast<MockMethod<TReturn, TArguments...>*>(
                    mockMethodNonGeneric);

            return *mockMethod;
        }
};

}

namespace IMock::Internal {

class Apply {
    private:
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

        template<int ...S, typename TReturn,
            typename ...TArguments>
        static TReturn applyWithSeq(
            seq<S...>,
            std::function<TReturn (TArguments...)> callback,
            std::tuple<TArguments...> arguments) {
            return callback(std::move(std::get<S>(arguments))...);
        }

        Apply();

    public:
        template<typename TReturn, typename ...TArguments>
        static TReturn apply(
            std::function<TReturn (TArguments...)> callback,
            std::tuple<TArguments...> arguments) {
            return applyWithSeq(typename gens<sizeof...(TArguments)>::type(),
                callback,
                std::move(arguments));
        }
};

}

namespace IMock::Internal {

class CaseMatchFactory {
    public:
        CaseMatchFactory() = delete;

        template <typename TReturn>
        static CaseMatch<TReturn> noMatch() {
            return CaseMatch<TReturn>(std::unique_ptr<IReturnValue<TReturn>>(
                nullptr));
        }

        template <typename TReturn>
        static CaseMatch<TReturn> match(TReturn returnValue) {
            return CaseMatch<TReturn>(make_unique<NonVoidReturnValue<TReturn>>(
                std::move(returnValue)));
        }

        static CaseMatch<void> matchVoid() {
            return CaseMatch<void>(make_unique<VoidReturnValue>());
        }
};

}

namespace IMock::Internal {

template <typename TReturn, typename ...TArguments>
class MockWithArgumentsCase : public ICase<TReturn, TArguments...> {
    private:
        std::tuple<TArguments...> _arguments;
        std::function<CaseMatch<TReturn> (TArguments...)> _fake;

    public:
        MockWithArgumentsCase(
            std::tuple<TArguments...> arguments,
            std::function<CaseMatch<TReturn> (TArguments...)> fake)
            : _arguments(std::move(arguments))
            , _fake(std::move(fake)) {
            }

        CaseMatch<TReturn> matches(std::tuple<TArguments...>& arguments) {
            bool match = arguments == _arguments;
            if(match) {
                return Apply::apply(_fake, std::move(arguments));
            }
            else {
                return CaseMatchFactory::noMatch<TReturn>();
            }
        }
};

}

namespace IMock {

template <typename TInterface, MockCaseID id, typename TReturn,
    typename ...TArguments>
class MockWithArguments {
    private:
        Internal::InnerMock<TInterface>* _mock;
        TReturn (TInterface::*_method)(TArguments...);
        std::tuple<TArguments...> _arguments;

    public:
        MockWithArguments(
            Internal::InnerMock<TInterface>* mock,
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
            typename std::enable_if<!std::is_void<R>::value, TReturn>::type
                returnValue) {
            std::tuple<TReturn> wrappedReturnValue = (returnValue);

            std::function<Internal::CaseMatch<TReturn> (TArguments...)> fake
                = [wrappedReturnValue] (TArguments... arguments) {
                    return Internal::CaseMatchFactory::match<TReturn>(
                        std::get<0>(wrappedReturnValue));
                };

            return fakeGeneral(fake);
        }

        template<typename R = TReturn,
            typename std::enable_if<std::is_void<R>::value, R>::type* = nullptr>
        MockCaseCallCount returns() {
            std::function<Internal::CaseMatch<TReturn> (TArguments...)> fake
                = [] (TArguments... arguments) {
                    return Internal::CaseMatchFactory::matchVoid();
                };

            return fakeGeneral(fake);
        }

    private:
        MockCaseCallCount fakeGeneral(
            std::function<Internal::CaseMatch<TReturn> (TArguments...)> fake) {
            std::unique_ptr<Internal::ICase<TReturn, TArguments...>> mockCase
                = Internal::make_unique<
                Internal::MockWithArgumentsCase<TReturn, TArguments...>>(
                std::move(_arguments),
                fake);

            return _mock->template addCase<id, TReturn, TArguments...>(
                _method,
                std::move(mockCase));
        }
};

}

namespace IMock {

template <typename TInterface, MockCaseID id, typename TReturn,
    typename ...TArguments>
class MockWithMethod {
    private:
        Internal::InnerMock<TInterface>* _mock;
        TReturn (TInterface::*_method)(TArguments...);

    public:
        MockWithMethod(
            Internal::InnerMock<TInterface>* mock,
            TReturn (TInterface::*method)(TArguments...))
            : _mock(mock)
            , _method(method) {
        }

        MockWithArguments<TInterface, id, TReturn, TArguments...> with(
            TArguments... arguments) {
            return MockWithArguments<TInterface, id, TReturn, TArguments...>(
                _mock,
                _method,
                std::tuple<TArguments...>(std::move(arguments)...));
        }
};

}

namespace IMock {

template <typename TInterface, MockCaseID id>
class MockWithID {
    private:
        Internal::InnerMock<TInterface>* _mock;

    public:
        MockWithID(Internal::InnerMock<TInterface>* mock)
            : _mock(mock) {
        }

        template <typename TReturn, typename ...TArguments>
        MockWithMethod<TInterface, id, TReturn, TArguments...> withMethod(
            TReturn (TInterface::*method)(TArguments...)) {
            return MockWithMethod<TInterface, id, TReturn, TArguments...>(
                _mock,
                method);
        }
};

}

namespace IMock {

template <typename TInterface>
class Mock {
    private:
        Internal::InnerMock<TInterface> _innerMock;

    public:
        virtual ~Mock() {
        }

        TInterface& get() {
            return _innerMock.get();
        }

        template <MockCaseID id>
        MockWithID<TInterface, id> withCounter() {
            return MockWithID<TInterface, id>(&_innerMock);
        }
};

}

#define mockType(mock) \
    std::remove_reference<decltype((mock).get())>::type

#define when(mock, method) \
    mock.withCounter<__COUNTER__>().withMethod(&mockType(mock)::method)
