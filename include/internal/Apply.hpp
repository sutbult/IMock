#pragma once

#include <tuple>
#include <functional>

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

        template<int ...S, typename TReturn,
            typename ...TArguments>
        static TReturn applyWithSeq(
            seq<S...>,
            std::function<TReturn (TArguments...)> callback,
            std::tuple<TArguments...> arguments) {
            return callback(std::move(std::get<S>(arguments))...);
        }

        Apply();

    public:
        template<typename TReturn, typename ...TArguments>
        static TReturn apply(
            std::function<TReturn (TArguments...)> callback,
            std::tuple<TArguments...> arguments) {
            return applyWithSeq(typename gens<sizeof...(TArguments)>::type(),
                callback,
                std::move(arguments));
        }
};

}
