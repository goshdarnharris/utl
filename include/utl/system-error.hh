// Copyright 2021 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0

#ifndef UTL_SYSTEM_ERROR_HH_
#define UTL_SYSTEM_ERROR_HH_

#include "utl/error.hh"

namespace utl {
    
enum class system_error {
    OK,
    UNKNOWN
};

//Is there any better way to do this? I don't like introducing
//a namespace in this way.
using namespace utl::literals;

template <>
struct is_error_code_enum<system_error> : std::true_type {};

//these should all be overloads
error_code make_error_code(system_error code);

} //namespace utl

#endif //UTL_SYSTEM_ERROR_HH_
