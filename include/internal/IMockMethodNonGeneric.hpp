#pragma once

namespace IMock {
namespace Internal {

/// Interface for a method in a mock without any specified argument types or
/// return value type.
class IMockMethodNonGeneric {
    public:
        /// Virtual destructor of IMockMethodNonGeneric.
        virtual ~IMockMethodNonGeneric() {
        }
};

}
}
