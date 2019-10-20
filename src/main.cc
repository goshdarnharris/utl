/* vim: set tabstop=4 shiftwidth=4 expandtab filetype=cpp : */
#include "utl.hh"
#include "logger.hh"
#include <stdio.h>
#include <CppUTest/CommandLineTestRunner.h>

extern "C" int main(int argc, char* argv[]);

struct printf_logger {
    utl::result<void> write(utl::string_view const& s) const {
        printf("%s\n",s.data());
        return utl::success();
    }
};

int main(int argc, char* argv[])
{
    //FIXME: logger ergonomics aren't great.
    const auto plog = printf_logger{};
    const auto output = utl::logger::output<printf_logger>{plog};
    const auto log_config = utl::logger::push_output{&output};

    utl::log("UTL test binary - starting tests");

    RUN_ALL_TESTS(argc,argv);
    return 0x0;
}
