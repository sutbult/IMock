#pragma once

#include <functional>
#include <memory>

#include <internal/UnknownCall.hpp>
#include <internal/VirtualOffset.hpp>

namespace IMock::Internal {

template <typename TInterface>
class VirtualTable {
    private:
        VirtualTableSize _virtualTableSize;
        std::unique_ptr<void*, std::function<void(void**)>> _virtualTable;

    public:
        VirtualTable() 
            : _virtualTableSize(Internal::getVirtualTableSize<TInterface>())
            , _virtualTable(
                new void*[_virtualTableSize],
                [](void** virtualTable) {
                    delete[] virtualTable;
                }) {
            std::fill(
                _virtualTable.get(),
                _virtualTable.get() + _virtualTableSize,
                reinterpret_cast<void*>(Internal::UnknownCall::onUnknownCall));
        }

        void** get() const {
            return _virtualTable.get();
        }
};

}
