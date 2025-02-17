/*******************************************************************************
 *   (c) 2018 - 2024 Zondax AG
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

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#include "zxerror.h"
#include "zxmacros.h"

__Z_INLINE const char *getMonth(uint8_t tm_mon) {
    switch (tm_mon) {
        case 1:
            return "Jan";
        case 2:
            return "Feb";
        case 3:
            return "Mar";
        case 4:
            return "Apr";
        case 5:
            return "May";
        case 6:
            return "Jun";
        case 7:
            return "Jul";
        case 8:
            return "Aug";
        case 9:
            return "Sep";
        case 10:
            return "Oct";
        case 11:
            return "Nov";
        case 12:
            return "Dec";
        default:
            return "ERR";
    }
}

typedef struct {
    uint8_t tm_sec;
    uint8_t tm_min;
    uint8_t tm_hour;
    uint16_t tm_day;
    uint8_t tm_mon;
    uint16_t tm_year;
    const char *monthName;
} timedata_t;

zxerr_t printTime(char *out, uint16_t outLen, uint64_t t);
zxerr_t printTimeSpecialFormat(char *out, uint16_t outLen, uint64_t t);
zxerr_t decodeTime(timedata_t *timedata, uint64_t t);

// Convert seconds since epoch to UTC date
zxerr_t extractTime(uint64_t time, timedata_t *date);

#ifdef __cplusplus
}
#endif
