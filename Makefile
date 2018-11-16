CC=gcc
CXX=g++
CFLAGS=-D_GNU_SOURCE -I./kv_store -I./test_director -Wno-write-strings -Wall

KV_STOR_OBJS = $(patsubst %.c,%.o,$(wildcard kv_store/*.c))
TEST_DIRECTOR_OBJS = $(patsubst %.c,%.o,$(wildcard test_director/*.c))

KV_HARNESS_OBJS = kv_harness.o
KV_STRESS_OBJS = kv_stress.o

GTEST_DIR=googletest/googletest
GMOCK_DIR=googletest/googlemock

all: kv_harness kv_stress

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) -g

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CFLAGS) -std=c++11 -isystem ${GTEST_DIR}/include -isystem ${GMOCK_DIR}/include -pthread -Igmock-global/include -g

libgtest.a: ${GTEST_DIR}/src/gtest-all.cc ${GTEST_DIR}/src/gtest_main.cc $(GMOCK_DIR)/src/gmock-all.cc
	g++ -std=c++11 -isystem ${GTEST_DIR}/include -I${GTEST_DIR} -pthread -c ${GTEST_DIR}/src/gtest-all.cc
	g++ -std=c++11 -isystem ${GTEST_DIR}/include -I${GTEST_DIR} -pthread -c ${GTEST_DIR}/src/gtest_main.cc
	g++ -std=c++11 -isystem ${GTEST_DIR}/include -I${GTEST_DIR} -I${GMOCK_DIR} -I${GMOCK_DIR}/include -pthread -c ${GMOCK_DIR}/src/gmock-all.cc
	ar -rv libgtest.a gtest-all.o gtest_main.o gmock-all.o

unittest: kv_block_allocator_unittest kv_directory_unittest kv_append_point_unittest kv_store_unittest kv_store_write_block_unittest

kv_append_point_unittest: libgtest.a test/kv_append_point_unittest.o kv_store/kv_append_point.o
	$(CXX) -o $@ $^ $(CFLAGS) -L. -lgtest -lpthread -g
	./kv_append_point_unittest

kv_block_allocator_unittest: libgtest.a test/kv_block_allocator_unittest.o kv_store/kv_block_allocator.o
	$(CXX) -o $@ $^ $(CFLAGS) -L. -lgtest -lpthread -g
	./kv_block_allocator_unittest

kv_directory_unittest: libgtest.a test/kv_directory_unittest.o kv_store/kv_directory.o
	$(CXX) -o $@ $^ $(CFLAGS) -L. -lgtest -lpthread -g
	./kv_directory_unittest

kv_store_unittest: libgtest.a test/kv_store_unittest.o kv_store/kv_store.o
	$(CXX) -o $@ $^ $(CFLAGS) -L. -lgtest -lpthread -std=c++11 -g
	./kv_store_unittest

kv_store_write_block_unittest: libgtest.a test/kv_store_write_block_unittest.o kv_store/kv_store_write_block.o
	$(CXX) -o $@ $^ $(CFLAGS) -L. -lgtest -lpthread -std=c++11 -g
	./kv_store_write_block_unittest

kv_harness: $(KV_HARNESS_OBJS) $(KV_STOR_OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

kv_stress: $(KV_STRESS_OBJS) $(KV_STOR_OBJS) $(TEST_DIRECTOR_OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm $(KV_STOR_OBJS) $(TEST_DIRECTOR_OBJS) test/*.o *.o kv_harness kv_stress *_unittest libgtest.a

test: all
	./kv_harness delme.kv
	rm delme.kv
	./kv_stress delme.kv
	rm delme.kv

test_all: unittest test
