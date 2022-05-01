#pragma once

#include <memory>

namespace IMock::Internal {

/// Creates a unique_ptr.
///
/// @param __args Arguments used to create the contained value.
/// @return A created unique_ptr.
template<typename _Tp, typename... _Args>
std::unique_ptr<_Tp> make_unique(_Args&&... __args)
{
    // Check if std::make_unique is available (introduced in C++14).
    #ifdef __cpp_lib_make_unique

    // Use std::make_unique to create the unique_ptr since std::make_unique is
    // available.
    return std::make_unique<_Tp, _Args...>(
        std::forward<_Args>(__args)...);
    #else

    // Create an instance manually and then create a unique_ptr using it since
    // std::make_unique is not available.
    return std::unique_ptr<_Tp>(
        new _Tp(std::forward<_Args>(__args)...));
    #endif
}

}
