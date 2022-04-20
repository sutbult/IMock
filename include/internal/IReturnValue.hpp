#pragma once

#include <utility>

namespace IMock::Internal {

template <typename TReturn>
class IReturnValue {
    public:
        virtual ~IReturnValue() {
        }

        virtual TReturn getReturnValue() = 0;
};

class VoidReturnValue : public IReturnValue<void> {
    public:
        virtual void getReturnValue() override {
        }
};

template <typename TReturn>
class NonVoidReturnValue : public IReturnValue<TReturn> {
    private:
        TReturn _returnValue;

    public:
        NonVoidReturnValue(TReturn returnValue)
            : _returnValue(std::move(returnValue)) {
        }
        
        virtual TReturn getReturnValue() override {
            return _returnValue;
        }
};

}
