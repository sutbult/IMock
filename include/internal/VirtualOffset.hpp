#pragma once

namespace IMock::Internal {

typedef unsigned int VirtualOffset;

typedef VirtualOffset VirtualTableSize;

#define offset0(id) \
    virtual VirtualOffset offset ## id() {\
        return id;\
    }\

#define offset1(id) \
    offset0(id ## 0) \
    offset0(id ## 1) \
    offset0(id ## 2) \
    offset0(id ## 3) \
    offset0(id ## 4) \
    offset0(id ## 5) \
    offset0(id ## 6) \
    offset0(id ## 7) \
    offset0(id ## 8) \
    offset0(id ## 9) \
    offset0(id ## A) \
    offset0(id ## B) \
    offset0(id ## C) \
    offset0(id ## D) \
    offset0(id ## E) \
    offset0(id ## F) \

#define offset2(id) \
    offset1(id ## 0) \
    offset1(id ## 1) \
    offset1(id ## 2) \
    offset1(id ## 3) \
    offset1(id ## 4) \
    offset1(id ## 5) \
    offset1(id ## 6) \
    offset1(id ## 7) \
    offset1(id ## 8) \
    offset1(id ## 9) \
    offset1(id ## A) \
    offset1(id ## B) \
    offset1(id ## C) \
    offset1(id ## D) \
    offset1(id ## E) \
    offset1(id ## F) \

#define offset3(id) \
    offset2(id ## 0) \
    offset2(id ## 1) \
    offset2(id ## 2) \
    offset2(id ## 3) \
    offset2(id ## 4) \
    offset2(id ## 5) \
    offset2(id ## 6) \
    offset2(id ## 7) \
    offset2(id ## 8) \
    offset2(id ## 9) \
    offset2(id ## A) \
    offset2(id ## B) \
    offset2(id ## C) \
    offset2(id ## D) \
    offset2(id ## E) \
    offset2(id ## F) \

struct VirtualOffsetReference {
    offset3(0x)
};

#undef offset1
#undef offset2
#undef offset3

template <typename TInterface, typename TReturn, typename ...TArguments>
VirtualOffset getVirtualOffset(TReturn (TInterface::*method)(TArguments...)) {    
    auto referenceMethod = 
        reinterpret_cast<VirtualOffset (VirtualOffsetReference::*)()>(method);
    
    static VirtualOffsetReference virtualOffsetReference;

    return (virtualOffsetReference.*referenceMethod)();
}

template <typename TInterface>
VirtualTableSize getVirtualTableSize() {
    class DerivedInterface : public TInterface {
        public:
            virtual void lastMethod() = 0;
    };

    VirtualOffset virtualTableSize = getVirtualOffset(
        &DerivedInterface::lastMethod);

    return virtualTableSize;
}

}
