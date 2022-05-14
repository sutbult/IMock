#pragma once

namespace IMock {
namespace Internal {

/// Casts a value of any type to a value of any other type with regards to the
/// raw memory using a union. Use with caution.
/// @param source The value to cast from.
/// @return The casted value.
template <typename TTarget, typename TSource>
#if defined(__GNUG__) && !defined(__clang__)
[[gnu::optimize("no-devirtualize")]]
#endif
TTarget union_cast(TSource source) {
    // Create a union with a member each for the source and the target.
    union
    {
        TSource source;
        TTarget target;
    } castUnion;

    // Store the source in the union.
    castUnion.source = source;

    // Retrieve the target from the union and return it.
    return castUnion.target;
}

}
}
