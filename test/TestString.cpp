#include <Arduino.h>
#include <cstdint>
#include <sstream>

#include "unity.h"

#include "SerializablePOD.h"
#include "TinyString.h"

/**
 * @brief Test the String class constructors
 * 
 */
void test_construct_string(void) {
    auto test_string_1 = tinystring::String("LOCHA.IO");
    TEST_ASSERT_EQUAL_STRING("LOCHA.IO", test_string_1.c_str());

    auto test_string_2 = tinystring::String();
    TEST_ASSERT_EQUAL_PTR(nullptr, test_string_2.c_str());
}

/**
 * @brief Test the String class length() function
 * 
 */
void test_length_string(void) {
    auto test_string_1 = tinystring::String("LOCHA.IO");
    TEST_ASSERT_EQUAL_INT(8, test_string_1.length());

    auto test_string_2 = tinystring::String();
    TEST_ASSERT_EQUAL_PTR(nullptr, test_string_2.c_str());
}

/**
 * @brief Serialize and deserialize a String
 * 
 */
void test_serialize_deserialize_string(void) {
    auto test_string = tinystring::String("WIFI_AP_SSID_EXAMPLE");

    std::ostringstream stream_out;
    test_string.serialize(stream_out);


    auto test_string_deserialized = tinystring::String();
    auto stream_in = std::istringstream(stream_out.str());
    test_string_deserialized.deserialize(stream_in);

    TEST_ASSERT_EQUAL_STRING(test_string.c_str(), test_string_deserialized.c_str());
}
/**
 * @brief Serialize and deserialize a String
 * 
 */
void test_serialize_deserialize_string_with_operators(void) {
    auto test_string = tinystring::String("WIFI_AP_SSID_EXAMPLE");

    std::ostringstream stream_out;
    stream_out << test_string; 


    auto test_string_deserialized = tinystring::String();
    auto stream_in = std::istringstream(stream_out.str());
    stream_in >> test_string_deserialized;

    TEST_ASSERT_EQUAL_STRING(test_string.c_str(), test_string_deserialized.c_str());
}

extern "C" void app_main() {
    // 2 s delay before starting
    delay(2000);

    UNITY_BEGIN();
    RUN_TEST(test_construct_string);
    RUN_TEST(test_length_string);
    RUN_TEST(test_serialize_deserialize_string);
    RUN_TEST(test_serialize_deserialize_string_with_operators);
    UNITY_END();
}