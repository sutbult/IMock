#pragma once

#include <functional>
#include <memory>

#include <internal/UnknownCall.hpp>
#include <internal/VirtualOffset.hpp>
#include <internal/VirtualOffsetContext.hpp>

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
            : _virtualTableSize(VirtualOffsetContext
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
