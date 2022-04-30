#pragma once

#include <memory>

#include <internal/CaseMatch.hpp>
#include <internal/make_unique.hpp>

namespace IMock::Internal {

/// Utility to create CaseMatch instances.
class CaseMatchFactory {
    public:
        /// CaseMatchFactory is not supposed to be instantiated since it only
        /// contains static methods.
        CaseMatchFactory() = delete;

        /// Creates a CaseMatch indicating no match has been made.
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
        template <typename TReturn>
        static CaseMatch<TReturn> match(TReturn returnValue) {
            // Create and return a CaseMatch with the provided return value.
            return CaseMatch<TReturn>(make_unique<NonVoidReturnValue<TReturn>>(
                std::move(returnValue)));
        }

        /// Creates a CaseMatch indicating a match mas been made with a method
        /// with the return type void.
        ///
        /// @return A CaseMatch indicating a match.
        static CaseMatch<void> matchVoid() {
            // Create and return a CaseMatch with a VoidReturnValue.
            return CaseMatch<void>(make_unique<VoidReturnValue>());
        }
};

}