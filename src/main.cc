/* vim: set tabstop=4 shiftwidth=4 expandtab filetype=cpp : */
#include "utl.hh"
#include <CppUTest/CommandLineTestRunner.h>

extern "C" int main(int argc, char* argv[]);

int main(int argc, char* argv[])
{
    RUN_ALL_TESTS(argc,argv);
    return 0x0;
}
