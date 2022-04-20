#pragma once

#include <functional>
#include <map>

#include <MockCaseID.hpp>
#include <internal/CaseMatch.hpp>
#include <internal/make_unique.hpp>
#include <internal/MockMethod.hpp>
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
