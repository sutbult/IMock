#pragma once

#include <exception/UnknownCallException.hpp>

namespace IMock::Internal {

/// Contains a static function to call if a call has been made to a method
/// without any mock cases.
class UnknownCall {
    public:
        /// UnknownCall is not supposed to be instantiated since it only
        /// contains a static method.
        UnknownCall() = delete;

        /// Call this if a call has been made to a method without any mock
        /// cases.
        static void onUnknownCall(void*) {
            // Throw an UnknownCallException.
            throw Exception::UnknownCallException();
        }
};

}
