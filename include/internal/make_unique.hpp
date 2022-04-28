#pragma once

#include <memory>

namespace IMock::Internal {

// TODO: The corresponding method exists in the standard library in C++14.
// Make it so this method is used when C++14 is used.

/// Creates a unique_ptr.
///
/// @param __args Arguments used to create the contained value.
/// @return A created unique_ptr.
template<typename _Tp, typename... _Args>
std::unique_ptr<_Tp> make_unique(_Args&&... __args)
{
    return std::unique_ptr<_Tp>(
        new _Tp(std::forward<_Args>(__args)...));
}

}
