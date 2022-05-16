#pragma once

#include <internal/InnerMock.hpp>
#include <MockCaseID.hpp>
#include <MockWithID.hpp>

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
///
/// @tparam TInterface The type of interface to be mocked.
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
        /// @tparam id The MockWithID used to identify the mock case to add.
        template <MockCaseID id>
        MockWithID<TInterface, id> withCounter() {
            // Create an return a MockWithID with _innerMock.
            return MockWithID<TInterface, id>(_innerMock);
        }
};

}
