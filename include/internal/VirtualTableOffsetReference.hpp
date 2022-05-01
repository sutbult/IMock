#pragma once

#include <internal/VirtualTableOffset.hpp>

namespace IMock::Internal {

// Define a macro creating a virtual method returning its own virtual table
// offset.
#define offset0(id) \
    virtual VirtualTableOffset offset ## id() {\
        return id;\
    }\

// Calls offset0 16 times, creating 16 methods.
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

// Calls offset1 16 times, creating 256 methods.
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

// Calls offset2 16 times, creating 4096 methods.
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

/// A reference struct used by VirtualTableOffsetContext to calculate virtual
/// table offsets and sizes.
struct VirtualTableOffsetReference {
    // Call offset3 to create 4096 offset methods.
    offset3(0x)
};

// Undefine the used macros.
#undef offset1
#undef offset2
#undef offset3

}
