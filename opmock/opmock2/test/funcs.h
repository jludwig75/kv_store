// A collection of function prototypes for testing mocks.
//
// Copyright (c) 2015 SanDisk. All rights reserved.

typedef struct
{
    int a;
    int b;
} a_struct;

int int_func();
void void_func();
void void_func_int(int one);
void void_func_int_char_ptr(int one, char *two);
int int_func_int(int one);
char *char_ptr_func_char_ptr(char *one);
a_struct struct_func();
inline int inline_test(int *me) {
    return *me;
}
