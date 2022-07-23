#pragma once

#include <utility>

#include <internal/Apply.hpp>

namespace IMock {
namespace Internal {

/// Interface for retrieving a return value.
///
/// @tparam TReturn The return type of the mocked method.
template <typename TReturn>
class IReturnValue {
    public:
        /// Virtual destructor of IReturnValue.
        virtual ~IReturnValue() {
        }

        /// Gets the return value.
        ///
        /// NOTICE: The method can only be called once for each instance.
        /// Any subsequent calls may result in an exception being thrown.
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
///
/// @tparam TReturn The return type of the mocked method.
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
            : _returnValue(std::forward<TReturn>(returnValue)) {
        }
        
        virtual TReturn getReturnValue() override {
            // Return the stored value.
            return _returnValue;
        }
};

/// Implements FakeReturnValue to make getReturnValue call the provided fake
/// with the provided arguments and return its return value.
///
/// @tparam TReturn The return type of the mocked method.
/// @tparam TArguments The types of the arguments of the mocked method.
template <typename TReturn, typename ...TArguments>
class FakeReturnValue : public IReturnValue<TReturn> {
    private:
        /// The fake to call.
        std::function<TReturn (TArguments...)> _fake;

        /// The arguments to call the fake with.
        std::tuple<TArguments...> _arguments;

    public:
        /// Creates a FakeReturnValue.
        ///
        /// @param fake The fake to call.
        /// @param arguments The arguments to call the fake with.
        FakeReturnValue(
            std::function<TReturn (TArguments...)> fake,
            std::tuple<TArguments...> arguments)
            : _fake(std::move(fake))
            , _arguments(std::move(arguments)) {
        }

        virtual TReturn getReturnValue() override {
            // Forward the call to _fake.
            return Apply::apply(_fake, std::move(_arguments));
        }
};

}
}
