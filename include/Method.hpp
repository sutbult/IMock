#pragma once

namespace IMock {

/// A method on TInterface with the return type TReturn with the arguments
/// ...TArguments.
template <typename TInterface, typename TReturn, typename ...TArguments>
using Method = TReturn (TInterface::*)(TArguments...);

}
