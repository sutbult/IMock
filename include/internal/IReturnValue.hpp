#pragma once

#include <utility>

namespace IMock::Internal {

/// Interface for retrieving a return value.
template <typename TReturn>
class IReturnValue {
    public:
        /// Virtual destructor of IReturnValue.
        virtual ~IReturnValue() {
        }

        /// Gets the return value.
        ///
        /// @return The return value.
        virtual TReturn getReturnValue() = 0;
};

/// Implements IReturnValue to make getReturnValue return void.
class VoidReturnValue : public IReturnValue<void> {
    public:
        virtual void getReturnValue() override {
        }
};

/// Implements IReturnValue to make getReturnValue return the provided value.
template <typename TReturn>
class NonVoidReturnValue : public IReturnValue<TReturn> {
    private:
        /// The value to return.
        TReturn _returnValue;

    public:
        /// Creates a NonVoidReturnValue.
        ///
        /// @param returnValue The value to return.
        NonVoidReturnValue(TReturn returnValue)
            : _returnValue(std::move(returnValue)) {
        }
        
        virtual TReturn getReturnValue() override {
            // Return the stored value.
            return _returnValue;
        }
};

}
