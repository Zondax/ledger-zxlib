/*******************************************************************************
 *   Ledger App - Bitcoin Wallet
 *   (c) 2018 - 2024 Zondax AG
 *   (c) 2016-2019 Ledger
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

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

int decode_base58(const char *in, size_t length, unsigned char *out, size_t *outlen);

int encode_base58(const unsigned char *in, size_t length, unsigned char *out, size_t *outlen);

char encode_base58_clip(unsigned char v);

#ifdef __cplusplus
}
#endif
