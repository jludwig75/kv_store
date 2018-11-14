// Test the IgnoreOtherCalls feature of Opmock.
//
// Copyright (c) 2015 SanDisk. All rights reserved.

#include <CppUTest/CommandLineTestRunner.h>

#include "funcs_stub.hpp"
#include "funcs_other_stub.hpp"

void opmock_verify()
{
    opmock_test_verify();

    if (opmock_get_number_of_errors() != 0) {
        opmock_print_error_messages();
        FAIL("Opmock detected the error(s) above!");
    }
}

TEST_GROUP(empty_fixture) {

    void setup() {
        opmock_test_reset();
    }

    void teardown() {
    }
};

TEST(empty_fixture, ignore_all) {

    int i = 0x55;
    char str[] = "test";

    OPMOCK_IGNORE_OTHER_CALLS();

    int_func();
    void_func();
    void_func_int(i);
    void_func_int_char_ptr(i, str);
    int_func_int(i);
    char_ptr_func_char_ptr(str);

    opmock_verify();
}

TEST(empty_fixture, ignore_all_but_one) {

    int i = 0x55;
    char str[] = "test";

    void_func_ExpectAndReturn();
    
    OPMOCK_IGNORE_OTHER_CALLS();

    int_func();
    void_func();
    void_func_int(i);
    void_func_int_char_ptr(i, str);
    int_func_int(i);
    char_ptr_func_char_ptr(str);

    opmock_verify();
}

TEST(empty_fixture, ignore_all_but_one_and_dont_call) {

    int i = 0x55;
    char str[] = "test";

    void_func_ExpectAndReturn();
    
    OPMOCK_IGNORE_OTHER_CALLS();

    int_func();
    void_func_int(i);
    void_func_int_char_ptr(i, str);
    int_func_int(i);
    char_ptr_func_char_ptr(str);

    opmock_test_verify();
    CHECK(opmock_get_number_of_errors() == 1);
}

TEST(empty_fixture, ignore_all_but_one_and_call_too_many) {

    int i = 0x55;
    char str[] = "test";

    void_func_ExpectAndReturn();
    
    OPMOCK_IGNORE_OTHER_CALLS();

    int_func();
    void_func();
    void_func_int(i);
    void_func_int_char_ptr(i, str);
    int_func_int(i);
    char_ptr_func_char_ptr(str);

    void_func();

    opmock_test_verify();
    CHECK(opmock_get_number_of_errors() == 1);
}

TEST(empty_fixture, ignore_none) {

    int int_param = 0x55;
    int int_return = 0xAA;
    char str_param[] = "test param";
    char str_return[] = "test return";

    int_func_ExpectAndReturn(int_return);
    void_func_ExpectAndReturn();
    void_func_int_ExpectAndReturn(int_param, cmp_int);
    void_func_int_char_ptr_ExpectAndReturn(int_param, str_param, cmp_int, cmp_cstr);
    int_func_int_ExpectAndReturn(int_param, int_return, cmp_int);
    char_ptr_func_char_ptr_ExpectAndReturn(str_param, str_return, cmp_cstr);

    OPMOCK_IGNORE_OTHER_CALLS();

    CHECK(int_func() == int_return);
    void_func();
    void_func_int(int_param);
    void_func_int_char_ptr(int_param, str_param);
    CHECK(int_func_int(int_param) == int_return);
    STRCMP_EQUAL(str_return, char_ptr_func_char_ptr(str_param));

    opmock_verify();
}

TEST(empty_fixture, ignore_all_multiple_headers) {

    int i = 0x55;
    char str[] = "test";

    OPMOCK_IGNORE_OTHER_CALLS();

    int_func();
    void_func();
    void_func_int(i);
    void_func_int_char_ptr(i, str);
    int_func_int(i);
    char_ptr_func_char_ptr(str);

    int_func_other();
    void_func_other();
    void_func_int_other(i);
    void_func_int_char_ptr_other(i, str);
    int_func_int_other(i);
    char_ptr_func_char_ptr_other(str);

    opmock_verify();
}

TEST(empty_fixture, ignore_all_but_one_multiple_headers) {

    int i = 0x55;
    char str[] = "test";

    void_func_other_ExpectAndReturn();
    
    OPMOCK_IGNORE_OTHER_CALLS();

    int_func();
    void_func();
    void_func_int(i);
    void_func_int_char_ptr(i, str);
    int_func_int(i);
    char_ptr_func_char_ptr(str);

    int_func_other();
    void_func_other();
    void_func_int_other(i);
    void_func_int_char_ptr_other(i, str);
    int_func_int_other(i);
    char_ptr_func_char_ptr_other(str);

    opmock_verify();
}

TEST(empty_fixture, ignore_none_multiple_headers) {

    int int_param = 0x55;
    int int_return = 0xAA;
    char str_param[] = "test param";
    char str_return[] = "test return";

    int_func_ExpectAndReturn(int_return);
    void_func_ExpectAndReturn();
    void_func_int_ExpectAndReturn(int_param, cmp_int);
    void_func_int_char_ptr_ExpectAndReturn(int_param, str_param, cmp_int, cmp_cstr);
    int_func_int_ExpectAndReturn(int_param, int_return, cmp_int);
    char_ptr_func_char_ptr_ExpectAndReturn(str_param, str_return, cmp_cstr);

    int_func_other_ExpectAndReturn(int_return);
    void_func_other_ExpectAndReturn();
    void_func_int_other_ExpectAndReturn(int_param, cmp_int);
    void_func_int_char_ptr_other_ExpectAndReturn(int_param, str_param, cmp_int, cmp_cstr);
    int_func_int_other_ExpectAndReturn(int_param, int_return, cmp_int);
    char_ptr_func_char_ptr_other_ExpectAndReturn(str_param, str_return, cmp_cstr);

    OPMOCK_IGNORE_OTHER_CALLS();

    CHECK(int_func() == int_return);
    void_func();
    void_func_int(int_param);
    void_func_int_char_ptr(int_param, str_param);
    CHECK(int_func_int(int_param) == int_return);
    STRCMP_EQUAL(str_return, char_ptr_func_char_ptr(str_param));

    CHECK(int_func_other() == int_return);
    void_func_other();
    void_func_int_other(int_param);
    void_func_int_char_ptr_other(int_param, str_param);
    CHECK(int_func_int_other(int_param) == int_return);
    STRCMP_EQUAL(str_return, char_ptr_func_char_ptr_other(str_param));

    opmock_verify();
}

TEST(empty_fixture, ignored_functions_return_zero) {

    char str_param[] = "test";
    a_struct struct_return = {0x55, 0xAA};
    char *ptr;

    OPMOCK_IGNORE_OTHER_CALLS();

    CHECK(int_func() == 0);
    CHECK(!char_ptr_func_char_ptr(str_param));

    struct_return = struct_func();
    ptr = (char *)&struct_return;
    for(int i = sizeof(struct_return); i; --i) {
        CHECK(*ptr++ == '\0');
    }
}

TEST(empty_fixture, ignore_all_but_one_reverse_order) {

    int i = 0x55;
    char str[] = "test";

    OPMOCK_IGNORE_OTHER_CALLS();

    void_func_ExpectAndReturn();
    
    int_func();
    void_func();
    void_func_int(i);
    void_func_int_char_ptr(i, str);
    int_func_int(i);
    char_ptr_func_char_ptr(str);

    opmock_verify();
}

TEST(empty_fixture, ignore_and_return) {

    int int_return = 0x55;

    OPMOCK_IGNORE_OTHER_CALLS();

    int_func_IgnoreAndReturn(int_return);
    
    CHECK(int_return == int_func());

    opmock_verify();
}

TEST(empty_fixture, ignore_and_return_reverse_order) {

    int int_return = 0x55;

    int_func_IgnoreAndReturn(int_return);
    
    OPMOCK_IGNORE_OTHER_CALLS();

    CHECK(int_return == int_func());

    opmock_verify();
}


TEST(empty_fixture, ignore_then_reset_and_ignore) {

    int int_return = 0x55;

    // Set some expectations and ignore the rest.
    int_func_ExpectAndReturn(int_return);
    OPMOCK_IGNORE_OTHER_CALLS();

    void_func();
    CHECK(int_return == int_func());

    opmock_verify();

    // Reset and set some different expectations.
    opmock_test_reset();
    void_func_ExpectAndReturn();
    OPMOCK_IGNORE_OTHER_CALLS();

    void_func();
    CHECK(0 == int_func());

    opmock_verify();

    // Ensure that we still fail on expected calls.
    void_func();

    opmock_test_verify();
    CHECK(opmock_get_number_of_errors() == 1);
}


int main(int ac, char **av) {

    return CommandLineTestRunner::RunAllTests(ac, av);
}
