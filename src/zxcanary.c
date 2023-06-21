/*******************************************************************************
*   (c) 2018 - 2023 Zondax AG
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
#include "zxcanary.h"
#include <stdint.h>
#include <stdbool.h>
#include "zxmacros.h"

// This symbol is defined by the link script to be at the start of the stack area.
extern unsigned int app_stack_canary;
#define ZONDAX_CANARY (*((volatile uint32_t*) (&app_stack_canary + sizeof(uint32_t))))

#if defined(HAVE_ZONDAX_CANARY)
#include "errors.h"
#include "os_random.h"

static uint32_t dynamicCanary = 0;
static bool initialized = false;
#endif

void init_zondax_canary() {
#if defined(HAVE_ZONDAX_CANARY)
    if (initialized) return;

    uint8_t randomNum[10] = {0};
    if (cx_get_random_bytes(randomNum, sizeof(randomNum)) != CX_OK) {
        handle_stack_overflow();
    }

    for(uint8_t i = 0; i < sizeof(uint32_t); i++) {
        ZEMU_LOGF(50, "BYTE %d is: %x\n", i, randomNum[i])
        dynamicCanary += randomNum[i] << (8 * i);
    }

    ZONDAX_CANARY = dynamicCanary;
    initialized = true;
    ZEMU_LOGF(50, "RANDOM CANARY: %d\n", dynamicCanary)
#endif
}

void check_zondax_canary() {
#if defined(HAVE_ZONDAX_CANARY)
    if (!initialized || ZONDAX_CANARY != dynamicCanary) {
        ZEMU_LOGF(50, "ZONDAX CANARY TRIGGERED!!!!!\n")
        handle_stack_overflow();
    }
#endif
}
