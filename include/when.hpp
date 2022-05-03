#pragma once

/// Gets the interface type the provided Mock mocks.
#define mockType(mock) \
    std::remove_reference<decltype((mock).get())>::type

/// Call this with a Mock and a method on the mocked interface to get a
/// MockWithMethod to use to add a mock case.
#define when(mock, method) \
    mock.withCounter<__COUNTER__>().withMethod( \
        &mockType(mock)::method, \
        #mock ".get()." #method)
