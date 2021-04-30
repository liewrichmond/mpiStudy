#include <stdio.h>
#include <stdlib.h>
#include <check.h>
#include "utils.h"

START_TEST(test_test)
{
    ck_assert_int_eq(1,1);
}
END_TEST

START_TEST(testGetNeighbors)
{
    struct coord **buffer = malloc(8 * sizeof(*buffer));
    struct coord **expected = malloc(8 * sizeof(*expected));

    for (int i = 0; i < 8; i++) {
        buffer[i] = malloc(sizeof(**buffer));
        expected[i] = malloc(sizeof(**expected));
    }

    (expected[0])->row = -1;
    (expected[1])->row = -1;
    (expected[2])->row = -1;
    (expected[3])->row =  0;
    (expected[4])->row =  0;
    (expected[5])->row =  1;
    (expected[6])->row =  1;
    (expected[7])->row =  1;

    (expected[0])->col = -1;
    (expected[1])->col =  0;
    (expected[2])->col =  1;
    (expected[3])->col = -1;
    (expected[4])->col =  1;
    (expected[5])->col = -1;
    (expected[6])->col =  0;
    (expected[7])->col =  1;

    getNeighbors(0,0, buffer);

    for(int i=0; i < 8; i++) {
        ck_assert_int_eq((buffer[i])->row, (expected[i])->row);
        ck_assert_int_eq((buffer[i])->col, (expected[i])->col);
    }

    for(int i = 0; i < 8; i++) {
        free(buffer[i]);
        free(expected[i]);
    }
    free(buffer);
    free(expected);
}
END_TEST

START_TEST(testGetNeighborsNoWrap)
{
    struct coord **buffer = malloc(8 * sizeof(*buffer));
    struct coord **expected = malloc(8 * sizeof(*expected));

    for (int i = 0; i < 8; i++) {
        buffer[i] = malloc(sizeof(**buffer));
        expected[i] = malloc(sizeof(**expected));
    }

    (expected[0])->row = 1;
    (expected[1])->row = 1;
    (expected[2])->row = 1;
    (expected[3])->row = 2;
    (expected[4])->row = 2;
    (expected[5])->row = 3;
    (expected[6])->row = 3;
    (expected[7])->row = 3;

    (expected[0])->col = 0;
    (expected[1])->col = 1;
    (expected[2])->col = 2;
    (expected[3])->col = 0;
    (expected[4])->col = 2;
    (expected[5])->col = 0;
    (expected[6])->col = 1;
    (expected[7])->col = 2;

    getNeighbors(2, 1, buffer);

    for(int i=0; i < 8; i++) {
        ck_assert_int_eq((buffer[i])->row, (expected[i])->row);
        ck_assert_int_eq((buffer[i])->col, (expected[i])->col);
    }

    for(int i = 0; i < 8; i++) {
        free(buffer[i]);
        free(expected[i]);
    }
    free(buffer);
    free(expected);
}
END_TEST

Suite * makeSuite() {
    Suite *suite = suite_create("suite");
    TCase *utilsTests = tcase_create("testUtils");

    tcase_add_test(utilsTests, test_test);
    tcase_add_test(utilsTests, testGetNeighbors);
    tcase_add_test(utilsTests, testGetNeighborsNoWrap);
    suite_add_tcase(suite, utilsTests);

    return suite;
}

int main() {
    Suite *s = makeSuite();
    SRunner *sr = srunner_create(s);
    
    srunner_run_all(sr, CK_NORMAL);
}
