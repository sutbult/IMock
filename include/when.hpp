#pragma once

#define mockType(mock) \
    std::remove_reference<decltype((mock).get())>::type

#define when(mock, method) \
    mock.withCounter<__COUNTER__>().withMethod(&mockType(mock)::method)
