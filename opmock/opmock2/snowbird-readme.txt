Snowbird Opmock Readme File
===========================

Note
----

This version of Opmock 2 has been modified.  In order to get Opmock 2 to build
under Windows, we ported it to LLVM/Clang 3.8.  We also added support for
MSVC versions prior to VS 2015 by using _snprintf because these versions don't
ship with snprintf.  Another modification was to eliminate casting a struct to
itself because MSVC doesn't handle this:

struct my_struct s;
(struct my_struct)s; // MSVC doesn't like this!

We have extended Opmock 2 to include support for 'ignored" functions like
CppUTest.  A mock can be put into ignore mode, in which parameters are not
checked and calls are not recorded.  This is useful if you only want to
stub out an interface to allow unit testing and you do not care about
verify the specifics of how the interface is used.  See the section below on
using Opmock 2 for details on using this feature.


Using Opmock 2
==============

This section documents how we use Opmock 2 in the Snowbird code base.
Please refer to "opmock2.pdf" in the "documentation" folder for general
usage information.

Opmock has been integrated into the build system.  Mocks are automatically
generated on-demand at build time and have a dependency link to the file
being mocked.  Mocking an interface is a two-step process.  First, the
mocked header must be included in the unit test source code to use the
mocks and the supporting expectation-recording functions.  Second, the
mocked implementation must be added to the source files for the project.
An example of both of these operations are shown below.


Including the Mocked Header
---------------------------

Opmock creates a header file with the "_stub.hpp" suffix.  Include this file
in any source file that needs to use or set up the mocks.  The mocked header
will be generated in the build directory, but the tree structure will be a
mirror of the source code tree.  Therefore, you must qualify the path to the
generated header file in the same manner you would qualify the path to the
header being mocked.

Also include the Snowbird Opmock wrapper header "sb_opmock.h".  This file
includes some conveniences for working with Opmock under the Snowbird system.

Example:
--------

If you need to mock the file "ftl/axi.h", add the following to your unit test.

    #include "sb_opmock.h"
    #include "ftl/axi_stub.hpp"


Adding the Mocks to the Build
-----------------------------

Opmock creates a source file containing the implementation of the mocks and
other supporting functions.  This file contains the "_stub.cpp" suffix.  Add
this file to the unit test build by incuding it in the "MOCK_SRC" list in
the CMakelists.txt file that create the unit test.  The same comment above
regarding the path to the mocked header applies.  You must qualify the path
in the same manner you would qualify the path to the header being mocked.

Example:
--------

If you are mocking the "ftl/axi.h" interface, add the following to your
CMakelists.txt.

    set(MOCK_SRC
	    ftl/axi_stub.cpp
    )

Make sure that this list is passed to the "create_unit_test" call.

    create_unit_test(dq_unittest "${TEST_SRC}" "${MOCK_SRC}" "${TEST_LIBS}")


Mock Setup and Verification
---------------------------

CppUTest provides for "setup" and "teardown" functions.  We can use these
to reset and verify mocked functions.  Add a call to "opmock_test_reset"
to the "setup" function to make sure the mocks are ready before each test.
If you want to verify call expectations, add a call to "opmock_verify" to
the "teardown" function.  This step is optional.  If you only want to stub
the interface and do not care about how often or when the mocks are called,
simply don't call "opmock_verify".

Example:
--------

    TEST_GROUP(empty_fixture) {
    
        void setup() {
            opmock_test_reset();
        }
    
        void teardown() {
            opmock_verify();
        }
    };


Ignoring Mocks
---------------------------

We have added the capability for mocks to ignore calls and parameter
checking, but still provide stubs and return values.  If you do not care
about the number of times or when or with what parameters a mock is called,
you can put that mock into "ignore" mode by calling the
foo_IgnoreAndReturn() function.  This function has the following signature:

void <name>_IgnoreAndReturn( <return type of mocked function> );

The parameters of the mocked function are not included, as they will not be
checked in this mode.  For example, if you were mocking the following
functions:

    void func_1(void);
    int func_2(long a, char *b);

The corresponding IgnoreAndReturn signatures would be:

    void func_1_IgnoreAndReturn();
    void func_2_IgnoreAndReturn(int to_return);

An analog to CppUTest's IgnoreOtherCalls is also available.  The macro
OPMOCK_IGNORE_OTHER_CALLS() goes through all mocks in the included headers
and puts any mock that does not have expectations on it (i.e. the mock's
ExpectAndReturn() function has not been called) into ingore mode.  You use
the OPMOCK_IGNORE_OTHER_CALLS feature by calling ExpectAndReturn on any
mock that you want to perform thorough checking on, calling IgnoreAndReturn
on any mock that you need to specify return values for, and calling
OPMOCK_IGNORE_OTHER_CALLS to put the remaining mocks into ignore mode.  The
order in which you do these actions does not matter.  An example follows.

Mocked Header:

    int func_1(void);
    int func_2(void);
    int func_3(void);
    void func_4(long a, char* b);

Unit Test:

    func_1_ExpectAndReturn(0xAA55);
    func_2_IgnoreAndReturn(0x55AA);
    OPMOCK_IGNORE_OTHER_CALLS();

Note that an ignored function will return zero unless IgnoreAndReturn has
been called for it (each byte of the return type will be set to zero).

Building the Opmock Binaries
============================

The build system uses precompiled binaries of Opmock2, so you do not have
to build it to use it.  Use the procedure below in the event that you need
or wish to build Opmock 2 from source.

The original version of Opmock 2 used a Makefile.  To support both Linux and
Windows, we created a cmake project file to generate the appropriate build
files.  Both LLVM and Clang are also built using cmake.

Obtaining LLVM and Clang
-------------------------
We used version 3.8 of both.  Be careful.  The Clang API is notoriously
unstable.  If you use a different version, you will likely have to port
Opmock 2 again.  Because 3.8 was the latest version at the time, we grabbed
the trunk from SVN as below.

> svn co http://llvm.org/svn/llvm-project/llvm/trunk llvm
> cd llvm/tools
> svn co http://llvm.org/svn/llvm-project/cfe/trunk clang

Building LLVM + Clang
---------------------
For Windows builds, ensure that the only native Windows compiler in your path
is MSVC.  Also, ensure you use an explicit install prefix that does not
contain spaces (such as C:/LLVM).  The default install prefix for Windows
contains spaces.

> cd llvm
> mkdir build
> cd build
> cmake -DCMAKE_INSTALL_PREFIX=<where to install> -DCMAKE_BUILD_TYPE=Release ..

Linux:
> cmake --build .
> cmake --build . --target install

Windows:
> cmake --build . --config Release
> cmake --build . --config Release --target install

Building Opmock 2
-----------------
Make sure that LLVM is in your path

Linux:
> cd opmock2
> mkdir build
> cd build
> mkdir Release
> cd Release
> cmake -DCMAKE_BUILD_TYPE=Release ../..
> cmake --build .

Windows:
> cd opmock2
> mkdir build
> cd build
> cmake -DCMAKE_BUILD_TYPE=Release ..
> cmake --build . --config Release

The resulting opmock2 executable will be under build/Release.  Note, our
build of opmock2 ellicited 149 warnings and 0 errors under VS Express 2013.
