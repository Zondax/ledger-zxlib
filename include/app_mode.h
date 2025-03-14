/*******************************************************************************
 *   (c) 2016 Ledger
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
#include "stdbool.h"
#include "zxmacros.h"

#ifdef __cplusplus
extern "C" {
#endif

void app_mode_reset();

bool app_mode_expert();

void app_mode_set_expert(uint8_t val);

bool app_mode_account();

void app_mode_set_account(uint8_t val);

bool app_mode_secret();

void app_mode_set_secret(uint8_t val);

bool app_mode_shortcut();

void app_mode_set_shortcut(uint8_t val);

bool app_mode_blindsign();

void app_mode_set_blindsign(uint8_t val);

bool app_mode_blindsign_required();

void app_mode_skip_blindsign_ui();
#ifdef __cplusplus
}
#endif
