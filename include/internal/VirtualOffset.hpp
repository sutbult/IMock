#pragma once

namespace IMock::Internal {

// TODO: Rename VirtualOffset and all its references to VirtualTableOffset.

/// Specifies an method offset within a virtual table.
typedef unsigned int VirtualOffset;

/// Specifies the size of a virtual table.
typedef VirtualOffset VirtualTableSize;

}
