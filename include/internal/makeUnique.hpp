#pragma once

#include <memory>

namespace IMock {
namespace Internal {

/// Creates a unique_ptr.
///
/// Named makeUnique using camel casing to not cause any ambiguities with
/// std::make_unique.
///
/// @param arguments Arguments used to create the contained value.
/// @return A created unique_ptr.
/// @tparam TPointer The type of contained value to create.
/// @tparam TArguments The types of the arguments used to create the contained
/// value.
template<typename TPointer, typename... TArguments>
std::unique_ptr<TPointer> makeUnique(TArguments&&... arguments)
{
    // Check if std::make_unique is available (introduced in C++14).
    #ifdef __cpp_lib_make_unique

    // Use std::make_unique to create the unique_ptr since std::make_unique is
    // available.
    return std::make_unique<TPointer, TArguments...>(
        std::forward<TArguments>(arguments)...);
    #else

    // Create an instance manually and then create a unique_ptr using it since
    // std::make_unique is not available.
    return std::unique_ptr<TPointer>(
        new TPointer(std::forward<TArguments>(arguments)...));
    #endif
}

}
}
