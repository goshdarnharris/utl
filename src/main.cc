// Copyright 2021 George Harris
//
// All portions of UTL (github.com/goshdarnharris/utl) are licensed
// under the Apache License, Version 2.0 (the "License"). 
// 
//     http://www.apache.org/licenses/LICENSE-2.0

/* vim: set tabstop=4 shiftwidth=4 expandtab filetype=cpp : */
#include "utl/utl.hh"
#include "utl/logger.hh"
#include <stdio.h>
#include <CppUTest/CommandLineTestRunner.h>

struct printf_logger {
    static utl::result<void> write(utl::string_view const& s) {
        auto* substr = new char[s.size()+1]; //NOLINT(cppcoreguidelines-owning-memory)
        strncpy(substr, s.data(), s.size());
        substr[s.size()] = '\0'; //NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        printf("%s",substr);
        delete[] substr; //NOLINT(cppcoreguidelines-owning-memory)
        return utl::success();
    }
};

extern "C" int main(int argc, char* argv[])
{
    //FIXME: logger ergonomics aren't great.
    const auto plog = printf_logger{};
    const auto output = utl::logger::output<printf_logger>{plog};
    const auto log_config = utl::logger::push_output{&output};

    utl::log("UTL test binary - starting tests");

    RUN_ALL_TESTS(argc,argv);
    return 0x0;
}
