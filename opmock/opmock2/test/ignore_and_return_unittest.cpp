// Test the IgnoreAndReturn feature of Opmock.
//
// Copyright (c) 2015 SanDisk. All rights reserved.

#include <CppUTest/CommandLineTestRunner.h>

#include "funcs_stub.hpp"

TEST_GROUP(empty_fixture) {

    void setup() {
        opmock_test_reset();
    }

    void teardown() {
        opmock_test_verify();

        if (opmock_get_number_of_errors() != 0) {
            opmock_print_error_messages();
            FAIL("Opmock detected the error(s) above!");
        }
    }
};

TEST(empty_fixture, ignore_int_mock) {

    int_func_IgnoreAndReturn(0x55);
    CHECK(int_func() == 0x55);
}

TEST(empty_fixture, expect_after_ignore_int_mock) {

    int_func_IgnoreAndReturn(0x55);
    int_func_ExpectAndReturn(0x55);
    CHECK(int_func() == 0x55);
}

TEST(empty_fixture, ignore_void_mock) {

    void_func_IgnoreAndReturn();
    void_func();
}

TEST(empty_fixture, expect_after_ignore_void_mock) {

    void_func_IgnoreAndReturn();
    void_func_ExpectAndReturn();
    void_func();
}

TEST(empty_fixture, ignore_void_mock_with_param) {

    void_func_int_IgnoreAndReturn();
    void_func_int(0x55);
}

TEST(empty_fixture, ignore_void_mock_with_params) {

    char str[] = "test";

    void_func_int_char_ptr_IgnoreAndReturn();
    void_func_int_char_ptr(0x55, str);
}

TEST(empty_fixture, ignore_int_mock_with_param) {

    int_func_int_IgnoreAndReturn(0x55);
    CHECK(int_func_int(0xAA) == 0x55);
}

TEST(empty_fixture, ignore_char_ptr_mock_with_param) {

    char str_out[] = "test";
    char str_in[] = "not";

    char_ptr_func_char_ptr_IgnoreAndReturn(str_out);
    STRCMP_EQUAL(str_out, char_ptr_func_char_ptr(str_in));
}

int main(int ac, char **av) {

    return CommandLineTestRunner::RunAllTests(ac, av);
}
