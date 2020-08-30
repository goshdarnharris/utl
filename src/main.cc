/* vim: set tabstop=4 shiftwidth=4 expandtab filetype=cpp : */
#include "utl/utl.hh"
#include "utl/logger.hh"
#include <stdio.h>
#include <CppUTest/CommandLineTestRunner.h>

struct printf_logger {
    utl::result<void> write(utl::string_view const& s) const {
        auto* substr = new char[s.size()+1];
        strncpy(substr, s.data(), s.size());
        substr[s.size()] = '\0'; //NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        printf("%s",substr);
        delete[] substr;
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
