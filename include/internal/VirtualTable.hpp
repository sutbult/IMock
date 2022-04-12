#pragma once

#include <functional>
#include <memory>

#include <internal/UnknownCall.hpp>
#include <internal/VirtualOffset.hpp>
#include <internal/VirtualOffsetContext.hpp>

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
