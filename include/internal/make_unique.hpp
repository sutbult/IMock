#pragma once

#include <memory>

namespace IMock::Internal {

template<typename _Tp, typename... _Args>
std::unique_ptr<_Tp> make_unique(_Args&&... __args)
{
    return std::unique_ptr<_Tp>(
        new _Tp(std::forward<_Args>(__args)...));
}

}
