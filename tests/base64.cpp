/*******************************************************************************
*   (c) 2019 Zondax GmbH
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
#include "base64.h"

namespace {
    TEST(BASE64, basic_case) {
        char out[100];
        uint8_t data[] = {1, 3, 5, 7, 9, 11, 13, 15, 17, 19};

        base64_encode(data, 0, out, sizeof(out));
        EXPECT_STREQ(out, "");

        base64_encode(data, 1, out, sizeof(out));
        EXPECT_STREQ(out, "AQ==");

        base64_encode(data, 2, out, sizeof(out));
        EXPECT_STREQ(out, "AQM=");

        base64_encode(data, 3, out, sizeof(out));
        EXPECT_STREQ(out, "AQMF");

        base64_encode(data, 4, out, sizeof(out));
        EXPECT_STREQ(out, "AQMFBw==");

        base64_encode(data, 5, out, sizeof(out));
        EXPECT_STREQ(out, "AQMFBwk=");

        base64_encode(data, 6, out, sizeof(out));
        EXPECT_STREQ(out, "AQMFBwkL");

        base64_encode(data, 7, out, sizeof(out));
        EXPECT_STREQ(out, "AQMFBwkLDQ==");

        base64_encode(data, 8, out, sizeof(out));
        EXPECT_STREQ(out, "AQMFBwkLDQ8=");

        base64_encode(data, 9, out, sizeof(out));
        EXPECT_STREQ(out, "AQMFBwkLDQ8R");

        base64_encode(data, 10, out, sizeof(out));
        EXPECT_STREQ(out, "AQMFBwkLDQ8REw==");
    };
}
