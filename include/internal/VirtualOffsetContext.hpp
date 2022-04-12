#pragma once

#include <internal/VirtualOffset.hpp>
#include <internal/VirtualOffsetReference.hpp>

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
