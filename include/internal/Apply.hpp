#pragma once

#include <tuple>

namespace IMock::Internal {

class Apply {
    private:
        // Trick to statically produce a list of integers. Solution taken from:
        // https://stackoverflow.com/a/7858971/6188897
        template<int ...>
        struct seq {
        };

        template<int N, int ...S>
        struct gens : gens<N-1, N-1, S...> {
        };

        template<int ...S>
        struct gens<0, S...>{
            typedef seq<S...> type;
        };

        template<int ...S, typename TClass, typename TReturn,
            typename ...TArguments>
        static TReturn applyWithSeq(
            seq<S...>,
            TReturn (TClass::*callback)(TArguments...),
            TClass& self,
            std::tuple<TArguments...> arguments) {
            return (self.*callback)(std::move(std::get<S>(arguments))...);
        }

        Apply();

    public:
        template<typename TClass, typename TReturn, typename ...TArguments>
        static TReturn apply(
            TReturn (TClass::*callback)(TArguments...),
            TClass& self,
            std::tuple<TArguments...> arguments) {
            return applyWithSeq(typename gens<sizeof...(TArguments)>::type(),
                callback,
                self,
                std::move(arguments));
        }
};

}
