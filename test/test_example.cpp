#include <Arduino.h>
#include <unity.h>

int int_a = 1;
int int_b = 1;
//
const char *str_a = "hello";
const char *str_b = "olleh";

/**
 * @brief Compare 2 integers
 * 
 */
void test_int_equal(void) {
    TEST_ASSERT_EQUAL(int_a, int_a);
}

/**
 * @brief verify the length of 2 strings
 * 
 */
void test_str_length(void) {
    TEST_ASSERT_EQUAL_STRING_LEN(str_a, str_a, sizeof(str_a));
}

void setup() {
    // Delay 2secs. before begin
    delay(2000);
    // Start tests
    UNITY_BEGIN();
}

void loop(){
    // test_01: integer equal value
    RUN_TEST(test_int_equal);

    // test_02: string equal length
    RUN_TEST(test_str_length);

    // End tests.
    UNITY_END();
}