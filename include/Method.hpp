#pragma once

namespace IMock {

/// A method on TInterface with the return type TReturn with the arguments
/// ...TArguments.
///
/// @tparam TInterface The interface that the method belongs to.
/// @tparam TReturn The return type of the method.
/// @tparam TArguments The types of the arguments to the method.
template <typename TInterface, typename TReturn, typename ...TArguments>
using Method = TReturn (TInterface::*)(TArguments...);

}
