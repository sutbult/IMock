#pragma once

#include <exception>
#include <functional>
#include <map>
#include <memory>
#include <sstream>
#include <type_traits>

#include <exception/UnknownCallException.hpp>
#include <exception/WrongCallCountException.hpp>
#include <internal/Case.hpp>
#include <internal/UnknownCall.hpp>
#include <internal/VirtualOffset.hpp>
#include <internal/make_unique.hpp>
#include <IReturnValue.hpp>
#include <MockCaseCallCount.hpp>

namespace IMock {

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
                    std::unique_ptr<Internal::IReturnValue<TReturn>>
                        wrappedReturnValue = Internal::make_unique<
                        Internal::NonVoidReturnValue<TReturn>>(returnValue);

                    return _mock->addCase<id, TReturn, TArguments...>(
                        _method,
                        std::move(wrappedReturnValue),
                        std::move(_arguments));
                }

                template<typename R = TReturn,
                    typename std::enable_if<std::is_void<R>::value, R>::type* =
                    nullptr>
                MockCaseCallCount returns() {
                    std::unique_ptr<Internal::IReturnValue<TReturn>>
                        wrappedReturnValue = Internal::make_unique<
                        Internal::VoidReturnValue>();

                    return _mock->addCase<id, TReturn, TArguments...>(
                        _method,
                        std::move(wrappedReturnValue),
                        std::move(_arguments));
                }
        };

        std::map<ID, Internal::VirtualOffset> _virtualOffsets;
        std::map<
            Internal::VirtualOffset,
            std::unique_ptr<Internal::ICaseNonGeneric>
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
            std::unique_ptr<Internal::IReturnValue<TReturn>> returnValue,
            std::tuple<TArguments...> arguments) {
            Internal::VirtualOffset virtualOffset =
                Internal::getVirtualOffset(method);

            bool virtualOffsetsNoID = _virtualOffsets.count(id) == 0;
            if(virtualOffsetsNoID) {
                _virtualOffsets[id] = virtualOffset;
            }

            bool methodHasNoMocks = _cases.count(virtualOffset) == 0;
            if(methodHasNoMocks) {
                _cases[virtualOffset] = Internal::make_unique<
                    Internal::UnmockedCase<TReturn, TArguments...>>();
                _virtualTable.get()[virtualOffset] =
                    reinterpret_cast<void*>(onCall<id, TReturn, TArguments...>);
            }
            std::unique_ptr<Internal::ICase<TReturn, TArguments...>>*
                previousCase = reinterpret_cast<std::unique_ptr<Internal::ICase<
                    TReturn, TArguments...>>*>(&_cases.at(virtualOffset));
            
            std::shared_ptr<Internal::MockCaseMutableCallCount> callCountPointer 
                = std::make_shared<Internal::MockCaseMutableCallCount>();
            
            _cases[virtualOffset] = Internal::make_unique<Internal::MockedCase<
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
            Internal::ICaseNonGeneric* caseNonGeneric =
                mock->_cases[virtualOffset].get();
            Internal::ICase<TReturn, TArguments...>* _case =
                reinterpret_cast<Internal::ICase<TReturn, TArguments...>*>(
                    caseNonGeneric);
            return _case->call(std::move(arguments)...);
        }
};

}

#define mockType(mock) \
    std::remove_reference<decltype((mock).get())>::type

#define when(mock, method) \
    mock.withCounter<__COUNTER__>().withMethod(&mockType(mock)::method)
