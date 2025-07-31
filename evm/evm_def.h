/*******************************************************************************
 *   (c) 2018 - 2025 Zondax AG
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

#include <stdint.h>
typedef enum {
    // Evm errors
    parser_evm_ok = 0,
    parser_evm_no_data,
    parser_evm_init_context_empty,
    parser_evm_display_idx_out_of_range,
    parser_evm_display_page_out_of_range,
    parser_evm_unexpected_error,
    parser_evm_unexpected_type,
    parser_evm_unexpected_buffer_end,
    parser_evm_unexpected_value,
    parser_evm_value_out_of_range,
    parser_evm_unsupported_tx,
    parser_evm_invalid_chain_id,
    parser_evm_invalid_rs_values,
    parser_evm_blindsign_mode_required,
} parser_evm_error_t;

typedef struct {
    const uint8_t *buffer;
    uint16_t bufferLen;
    uint16_t offset;
} parser_evm_context_t;

#ifdef __cplusplus
}
#endif
