#pragma once

#include <functional>
#include <map>

#include <internal/CaseMatch.hpp>
#include <internal/makeUnique.hpp>
#include <internal/MockMethod.hpp>
#include <internal/union_cast.hpp>
#include <internal/VirtualTable.hpp>
#include <Method.hpp>
#include <MockCaseID.hpp>

namespace IMock {
namespace Internal {

/// Mocks a provided interface to perform wanted actions and return certain
/// values when its virtual methods are called.
///
/// @tparam TInterface The type of interface to be mocked.
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
                
                /// Called when a call to a method in the interface is called.
                ///
                /// @param arguments The arguments the method was called with.
                /// @return The return value from the first matching mock case.
                /// @throws Throws an UnmockedCallException if the arguments
                /// does not match any mock case.
                ///
                /// @return A MockWithID associated with the MockCaseID.
                /// @tparam id The MockWithID used to identify the mock case
                /// that first added a mock case to the method.
                /// @tparam TReturn The return type of the mocked method.
                /// @tparam TArguments The types of the arguments of the mocked
                /// method.
                template <MockCaseID id, typename TReturn,
                    typename ...TArguments>
                TReturn onCall(TArguments... arguments) {
                    // Forward the call to _mock.onCall.
                    return _mock.onCall<TReturn, TArguments...>(
                        id,
                        std::forward<TArguments>(arguments)...);
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
        /// @param methodString A string describing how a call is made to the
        /// method being mocked.
        /// @param mockCase The mock case to add.
        /// @return A CallCount that can be queried about the number of calls
        /// done to the added mock case.
        /// @tparam id The MockWithID used to identify the mock case to add.
        /// @tparam TReturn The return type of the method being mocked.
        /// @tparam TArguments The types of the arguments to the method being
        /// mocked.
        template <MockCaseID id, typename TReturn, typename ...TArguments>
        CallCount addCase(
            Method<TInterface, TReturn, TArguments...> method,
            std::string methodString,
            std::unique_ptr<ICase<TReturn, TArguments...>> mockCase) {
            // Forward the call to addCaseWithOnCall.
            return addCaseWithOnCall<TReturn, TArguments...>(
                id,
                &MockFake::template onCall<id, TReturn, TArguments...>,
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
        /// @param methodString A string describing how a call is made to the
        /// method being mocked.
        /// @param mockCase The mock case to add.
        /// @return A CallCount that can be queried about the number of calls
        /// done to the added mock case.
        /// @tparam TReturn The return type of the method being mocked.
        /// @tparam TArguments The types of the arguments to the method being
        /// mocked.
        template <typename TReturn, typename ...TArguments>
        CallCount addCaseWithOnCall(
            MockCaseID id,
            Method<MockFake, TReturn, TArguments...> onCall,
            Method<TInterface, TReturn, TArguments...> method,
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
                    = union_cast<void*>(onCall);
            }
            
            // Get the MockMethod for the method and add a mock case to it.
            return getMockMethod<TReturn, TArguments...>(virtualTableOffset)
                .addCase(std::move(mockCase));
        }


        /// Called when a call to a method in the interface is called.
        ///
        /// @param id The MockCaseID of the onCall method that made the internal
        /// call.
        /// @param mockFake The MockFake instance this call was made on.
        /// @param arguments The arguments the method was called with.
        /// @return The return value from the first matching mock case.
        /// @throws Throws an UnmockedCallException if the arguments does not
        /// match any mock case.
        /// @tparam id The MockWithID used to identify the mock case that first
        /// added a mock case to the method.
        /// @tparam TReturn The return type of the mocked method.
        /// @tparam TArguments The types of the arguments to the mocked method.
        template <typename TReturn, typename ...TArguments>
        TReturn onCall(
            MockCaseID id,
            TArguments... arguments) {
            // Get the virtual table offset for the MockCaseID.
            VirtualTableOffset virtualTableOffset = _virtualTableOffsets[id];

            // Get the MockMethod for the called method and forward the call to
            // onCall.
            return getMockMethod<TReturn, TArguments...>(virtualTableOffset)
                .onCall(std::forward<TArguments>(arguments)...);
        }

        /// Gets the MockMethod for the method with the provided virtual table
        /// offset.
        ///
        /// @param virtualTableOffset The method's virtual table offset.
        /// @return The method's MockMethod.
        /// @tparam TReturn The return type of the mocked method.
        /// @tparam TArguments The types of the arguments to the mocked method.
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
}
