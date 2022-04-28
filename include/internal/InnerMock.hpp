#pragma once

#include <functional>
#include <map>

#include <MockCaseID.hpp>
#include <internal/CaseMatch.hpp>
#include <internal/make_unique.hpp>
#include <internal/MockMethod.hpp>
#include <internal/VirtualTable.hpp>

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

                /// A pointer to the InnerMock that did the mocking.
                InnerMock* _mock;

            public:
                /// Creates a MockFake.
                ///
                /// @param virtualTable The raw virtual table of the mocked
                /// interface.
                /// @param mock A pointer to the InnerMock that did the mocking.
                MockFake(
                    void** virtualTable,
                    InnerMock* mock)
                    : _virtualTable(std::move(virtualTable))
                    , _mock(std::move(mock)) {
                }

                /// Gets the contained InnerMock.
                ///
                /// @return The contained InnerMock.
                InnerMock* getMock() {
                    // Return the contained InnerMock.
                    return _mock;
                }
        };

        /// Maps the MockCaseID's of all mock cases to the virtual offsets of
        /// their mocked methods.
        std::map<MockCaseID, VirtualOffset> _virtualOffsets;

        /// Maps the virtual offsets of the mocked methods to MockMethod
        /// instances dealing with calls to respective method.
        std::map<VirtualOffset, std::unique_ptr<IMockMethodNonGeneric>>
            _mockMethods;

        /// A VirtualTable to add mocked methods to.
        VirtualTable<TInterface> _virtualTable;

        /// A MockFake used by the InnerMock.
        MockFake _mockFake;

    public:
        /// Creates an InnerMock.
        InnerMock()
            : _mockFake(_virtualTable.get(), this) {
        }

        /// Virtual destructor of InnerMock.
        virtual ~InnerMock() {
        }

        /// Gets a reference to an object used in place of an instance of the
        /// interface.
        TInterface& get() {
            // Cast the MockFake to a TInterface and return a reference to it.
            return *reinterpret_cast<TInterface*>(&_mockFake);
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
            std::unique_ptr<ICase<TReturn, TArguments...>> mockCase) {
            // TODO: Move the logic to a method that doesn't include id as a
            // template parameter.

            // TODO: The virtual table offset can potentially be retrieved from
            // _virtualOffsets without having to call getVirtualOffset each
            // time.

            // Get the virtual table offset of the method.
            VirtualOffset virtualOffset =
                VirtualOffsetContext::getVirtualOffset(method);

            // Check if a virtual table offset has been stired in
            // _virtualOffsets for the provided MockCaseID.
            bool virtualOffsetsNoID = _virtualOffsets.count(id) == 0;
            if(virtualOffsetsNoID) {
                // Store the virtual table offset.
                _virtualOffsets[id] = virtualOffset;
            }

            // Check if the method has any existing mock cases.
            bool methodHasNoMocks = _mockMethods.count(virtualOffset) == 0;
            if(methodHasNoMocks) {
                // Create and store a MockMethod if the method has no existing
                // mock cases.
                _mockMethods[virtualOffset] = make_unique<
                    MockMethod<TReturn, TArguments...>>();

                // Store a pointer to onCall in the virtual table.
                _virtualTable.get()[virtualOffset] =
                    reinterpret_cast<void*>(onCall<id, TReturn, TArguments...>);
            }
            
            // Get the MockMethod for the method and add a mock case to it.
            return getMockMethod<TReturn, TArguments...>(virtualOffset)
                .addCase(std::move(mockCase));
        }

    private:
        /// Called when a call to a method in the interface is called.
        ///
        /// @param arguments The arguments the method was called with.
        template <MockCaseID id, typename TReturn, typename ...TArguments>
        static TReturn onCall(MockFake* mockFake, TArguments... arguments) {
            // TODO: Move the logic to a method that doesn't include id as a
            // template parameter.

            // Get the InnerMock.
            InnerMock& mock = *mockFake->getMock();

            // Get the virtual table offset for the MockCaseID.
            VirtualOffset virtualOffset = mock._virtualOffsets[id];

            // Get the MockMethod for the called method and forward the call to
            // onCall.
            return mock
                .getMockMethod<TReturn, TArguments...>(virtualOffset)
                .onCall(std::move(arguments)...);
        }

        /// Gets the MockMethod for the method with the provided virtual table
        /// offset.
        ///
        /// @param virtualOffset The method's virtual table offset.
        /// @return The method's MockMethod.
        template <typename TReturn, typename ...TArguments>
        MockMethod<TReturn, TArguments...>& getMockMethod(
            VirtualOffset virtualOffset) {
            // Get the MockMethod from _mockMethods.
            IMockMethodNonGeneric* mockMethodNonGeneric
                = _mockMethods[virtualOffset].get();

            // Cast mockMethodNonGeneric to its correct type.
            MockMethod<TReturn, TArguments...>* mockMethod
                = reinterpret_cast<MockMethod<TReturn, TArguments...>*>(
                    mockMethodNonGeneric);

            // Return the MockMethod.
            return *mockMethod;
        }
};

}
