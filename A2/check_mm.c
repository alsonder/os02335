/**
 * @file   check_mm.c
 * @Author 02335 team
 * @date   September, 2024
 * @brief  Unit tests and suite for the memory management subsystem.
 *
 * This file implements a suite of unit tests using the Check
 * unit testing framework.
 *
 * Add your new tests here. Feel free to organize
 * your tests in multiple suites.
 *
 * The Check framework includes a series of assert
 * functions. See them all at:
 * http://check.sourceforge.net/doc/check_html/check_4.html#Convenience-Test-Functions
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <check.h>
#include "mm.h"

/* Choose which malloc/free to test */
#define MALLOC simple_malloc
#define FREE   simple_free

/**
 * @name    sum_block
 * @brief   Utility function to calculate XOR checksum of a memory block.
 * @param   data Pointer to the start of the memory block.
 * @param   size Size of the memory block.
 * @return  Checksum value of the memory block.
 */
uint32_t sum_block(uint32_t *data, uint32_t size) {
    uint32_t sum = 0;
    for (uint32_t n = 0; n < (size >> 2); n++) {
        sum ^= data[n];
    }
    return sum;
}

/**
 * @name   test_simple_allocation
 * @brief  Tests whether a simple memory allocation works correctly.
 */
START_TEST(test_simple_allocation) {
    int *ptr1;

    ptr1 = MALLOC(10 * sizeof(int));

    /* Test whether the pointer is not NULL */
    ck_assert(ptr1 != NULL);

    FREE(ptr1);
}
END_TEST

/**
 * @name   test_simple_unique_addresses
 * @brief  Tests whether two separate allocations return unique addresses.
 */
START_TEST(test_simple_unique_addresses) {
    int *ptr1, *ptr2;

    ptr1 = MALLOC(10 * sizeof(int));
    ptr2 = MALLOC(10 * sizeof(int));

    /* Test whether each pointer has a unique address */
    ck_assert(ptr1 + 10 <= ptr2 || ptr2 + 10 <= ptr1);

    FREE(ptr1);
    FREE(ptr2);
}
END_TEST

/**
 * @name   test_next_fit_strategy
 * @brief  Test case to verify that the memory management system uses the next-fit strategy.
 */
START_TEST(test_next_fit_strategy) {
    int *ptr1, *ptr2, *ptr3;

    // Allocate two blocks
    ptr1 = MALLOC(100 * sizeof(int));
    ptr2 = MALLOC(50 * sizeof(int));
    ck_assert(ptr1 != NULL);
    ck_assert(ptr2 != NULL);

    // Free the first block to create a gap at the beginning of memory
    FREE(ptr1);

    // Allocate a new block that is smaller than ptr1 but larger than ptr2
    ptr3 = MALLOC(30 * sizeof(int));
    ck_assert(ptr3 != NULL);

    // Check that ptr3 is not placed in the same space as ptr1 (indicating next-fit strategy)
    ck_assert(ptr3 != ptr1);

    // Free remaining allocated blocks
    FREE(ptr2);
    FREE(ptr3);
}
END_TEST

/**
 * @name   test_memory_exerciser
 * @brief  Allocates and deallocates varying sizes of memory blocks to check for alignment and corruption.
 */
START_TEST(test_memory_exerciser) {
    uint32_t iterations = 1000;

    /* Struct to keep track of allocations */
    struct {
        void *addr;
        uint32_t *data;
        uint32_t size;
        uint32_t crc;
    } blocks[16];

    uint32_t total_memory_size = 0;
    uint32_t clock = 0;

    // Initialize blocks to zero
    for (clock = 0; clock < 16; clock++) {
        blocks[clock].addr = 0;
    }

    clock = 0;
    while (iterations--) {
        char *addr;

        // Randomize the size of a block
        blocks[clock].size = (24 * 1024 * 1024 - total_memory_size) * (rand() & (1024 * 1024 - 1)) / (1024 * 8);

        // Sanity check the block size
        if ((blocks[clock].size > 0) && (blocks[clock].size < (24 * 1024 * 1024))) {
            // Allocate memory
            addr = MALLOC(blocks[clock].size);
            ck_assert_msg(addr != NULL, "Memory allocation failed!");

            // Verify that the address is 8-byte aligned
            ck_assert_msg(((uintptr_t)addr & 0x07) == 0, "Unaligned address %p returned!", addr);

            blocks[clock].data = (uint32_t *)addr;

            // Fill memory with data for verification
            uint32_t sum = 0;
            for (uint32_t n = 0; n < (blocks[clock].size) >> 2; n++) {
                uint32_t x = (uint32_t)rand();
                blocks[clock].data[n] = x;
                sum ^= x;
            }
            blocks[clock].crc = sum;

            total_memory_size += blocks[clock].size;
            blocks[clock].addr = addr;
        } else {
            blocks[clock].addr = 0;
        }

        // Move on to the next block
        clock = (clock + 1) & 15;

        // Verify all existing blocks before free
        for (uint32_t n = 0; n < 16; n++) {
            if (blocks[n].addr != NULL) {
                uint32_t sum = sum_block(blocks[n].data, blocks[n].size);
                ck_assert_msg(blocks[n].crc == sum, "Checksum failed for block %d at addr=%p: %08x != %08x",
                              n, blocks[n].addr, blocks[n].crc, sum);
            }
        }

        // Try to free one block
        if (blocks[clock].addr != 0) {
            FREE(blocks[clock].addr);
            total_memory_size -= blocks[clock].size;
            blocks[clock].addr = NULL;

            // Verify all existing blocks after free
            for (uint32_t n = 0; n < 16; n++) {
                if (blocks[n].addr != NULL) {
                    uint32_t sum = sum_block(blocks[n].data, blocks[n].size);
                    ck_assert_msg(blocks[n].crc == sum, "Checksum failed for block %d at addr=%p: %08x != %08x",
                                  n, blocks[n].addr, blocks[n].crc, sum);
                }
            }
        }
    }

    // Free final blocks
    for (clock = 0; clock < 16; clock++) {
        if (blocks[clock].addr != NULL) {
            uint32_t sum = sum_block(blocks[clock].data, blocks[clock].size);
            ck_assert(blocks[clock].crc == sum);
            FREE(blocks[clock].addr);
        }
    }
}
END_TEST

/**
 * @name   simple_malloc_suite
 * @brief  Creates a test suite for the memory management system.
 * @return Suite pointer containing all tests.
 */
Suite *simple_malloc_suite() {
    Suite *s = suite_create("simple_malloc");
    TCase *tc_core = tcase_create("Core tests");
    tcase_set_timeout(tc_core, 120);
    tcase_add_test(tc_core, test_simple_allocation);
    tcase_add_test(tc_core, test_simple_unique_addresses);
    tcase_add_test(tc_core, test_memory_exerciser);
    tcase_add_test(tc_core, test_next_fit_strategy);

    suite_add_tcase(s, tc_core);
    return s;
}

/**
 * @name   main
 * @brief  Test runner that runs the test suite and reports results.
 * @return 0 if all tests pass, 1 otherwise.
 */
int main() {
    int number_failed;
    Suite *s = simple_malloc_suite();
    SRunner *sr = srunner_create(s);
    srunner_set_fork_status(sr, CK_NOFORK);
    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? 0 : 1;
}
