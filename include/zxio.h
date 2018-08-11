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

#pragma once
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

inline void fpuint64_to_str(char *dst, const uint64_t value, uint8_t decimals) {
    char buffer[100];

    snprintf(buffer, sizeof(buffer), "%" PRId64, value);
    size_t digits = strlen(buffer);

    if (digits <= decimals) {
        *dst++ = '0';
        *dst++ = '.';
        for (int i = 0; i < decimals - digits; i++, dst++)
            *dst = '0';
        strcpy(dst, buffer);
    } else {
        strcpy(dst, buffer);
        const size_t shift = digits - decimals;
        dst = dst + shift;
        *dst++ = '.';

        char *p = buffer + shift;
        strcpy(dst, p);
    }
}
