#pragma once

#include <memory>

#include <internal/CaseMatch.hpp>
#include <internal/makeUnique.hpp>

namespace IMock {
namespace Internal {

/// Utility to create CaseMatch instances.
class CaseMatchFactory {
    public:
        /// CaseMatchFactory is not supposed to be instantiated since it only
        /// contains static methods.
        CaseMatchFactory() = delete;

        /// Creates a CaseMatch indicating no match has been made.
        /// @tparam TReturn The return type of the mocked method.
        template <typename TReturn>
        static CaseMatch<TReturn> noMatch() {
            /// Create and return a CaseMatch without a return value.
            return CaseMatch<TReturn>(std::unique_ptr<IReturnValue<TReturn>>(
                nullptr));
        }

        /// Creates a CaseMatch indicating a match has been made with a method
        /// that has a return value.
        ///
        /// @param returnValue The call's return value.
        /// @return A CaseMatch indicating a match with the provided return
        /// value.
        /// @tparam TReturn The return type of the mocked method.
        template <typename TReturn>
        static CaseMatch<TReturn> match(TReturn returnValue) {
            // Create and return a CaseMatch with the provided return value.
            return CaseMatch<TReturn>(makeUnique<NonVoidReturnValue<TReturn>>(
                std::forward<TReturn>(returnValue)));
        }

        /// Creates a CaseMatch indicating a match mas been made with a method
        /// with the return type void.
        ///
        /// @return A CaseMatch indicating a match.
        static CaseMatch<void> matchVoid() {
            // Create and return a CaseMatch with a VoidReturnValue.
            return CaseMatch<void>(makeUnique<VoidReturnValue>());
        }

        /// Creates a CaseMatch indicating a match has been made and handle the
        /// match with a fake.
        ///
        /// @param fake The fake to call.
        /// @param arguments The arguments to call the fake with.
        /// @return A CaseMatch indicating a match with the provided fake.
        /// @tparam TReturn The return type of the mocked method.
        /// @tparam TArguments The types of the arguments to the method.
        template <typename TReturn, typename ...TArguments>
        static CaseMatch<TReturn> matchFake(
            std::function<TReturn (TArguments...)> fake,
            std::tuple<TArguments...> arguments) {
            // Create and return a CaseMatch with the fake and the arguments.
            return CaseMatch<TReturn>(
                makeUnique<FakeReturnValue<TReturn, TArguments...>>(
                    std::move(fake),
                    std::move(arguments)));
        }
};

}
}
