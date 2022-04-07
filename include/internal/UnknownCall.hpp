#pragma once

#include <exception/UnknownCallException.hpp>

namespace IMock::Internal {

class UnknownCall {
    public:
        static void onUnknownCall(void*) {
            throw Exception::UnknownCallException();
        }

    private:
        UnknownCall();
};

}
