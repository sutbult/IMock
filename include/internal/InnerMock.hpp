#pragma once

#include <functional>
#include <map>

#include <MockCaseID.hpp>
#include <internal/Case.hpp>
#include <internal/make_unique.hpp>
#include <internal/VirtualTable.hpp>

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

        std::map<MockCaseID, Internal::VirtualOffset> _virtualOffsets;
        std::map<
            Internal::VirtualOffset,
            std::unique_ptr<Internal::ICaseNonGeneric>
        > _cases;

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

    private:
        template <MockCaseID id, typename TReturn, typename ...TArguments>
        static TReturn onCall(MockFake* mockFake, TArguments... arguments) {
            InnerMock* mock = mockFake->getMock();
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
