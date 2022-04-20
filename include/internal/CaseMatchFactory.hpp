#pragma once

#include <memory>

#include <internal/CaseMatch.hpp>
#include <internal/make_unique.hpp>

namespace IMock::Internal {

class CaseMatchFactory {
    public:
        CaseMatchFactory() = delete;

        template <typename TReturn>
        static CaseMatch<TReturn> noMatch() {
            return CaseMatch<TReturn>(std::unique_ptr<IReturnValue<TReturn>>(
                nullptr));
        }

        template <typename TReturn>
        static CaseMatch<TReturn> match(TReturn returnValue) {
            return CaseMatch<TReturn>(make_unique<NonVoidReturnValue<TReturn>>(
                std::move(returnValue)));
        }

        static CaseMatch<void> matchVoid() {
            return CaseMatch<void>(make_unique<VoidReturnValue>());
        }
};

}
