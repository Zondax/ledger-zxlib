/*******************************************************************************
*   (c) 2018 ZondaX GmbH
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
********************************************************************************/
#include <gmock/gmock.h>
#include <zxmacros.h>
#include <zxio.h>

namespace {
TEST(MACROS, array_to_hexstr) {
    uint8_t array1[] = {1, 3, 5};

    char output[20];
    memset(output, 1, 20);

    array_to_hexstr(output, array1, sizeof(array1));
    EXPECT_EQ(memcmp(output, "010305", 2*sizeof(array1)), 0);
    EXPECT_EQ(output[2*sizeof(array1)], 0);
}
}

namespace {
TEST(MACROS, fpuint64_to_str) {
    char output[100];
    printf("\n");

    fpuint64_to_str(output, 123, 5);
    printf("%10s\n", output);
    EXPECT_EQ(std::string(output), "0.00123");

    fpuint64_to_str(output, 1234, 5);
    printf("%10s\n", output);
    EXPECT_EQ(std::string(output), "0.01234");

    fpuint64_to_str(output, 12345, 5);
    printf("%10s\n", output);
    EXPECT_EQ(std::string(output), "0.12345");

    fpuint64_to_str(output, 123456, 5);
    printf("%10s\n", output);
    EXPECT_EQ(std::string(output), "1.23456");

    fpuint64_to_str(output, 1234567, 5);
    printf("%10s\n", output);
    EXPECT_EQ(std::string(output), "12.34567");
}
}
