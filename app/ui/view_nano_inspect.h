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
#pragma once
#include <stdbool.h>

#include "view.h"

bool h_can_increase(paging_t *paging, uint8_t actionsCount);

void h_increase(paging_t *paging, uint8_t actionsCount);

bool h_can_decrease(paging_t *paging);

void h_decrease(paging_t *paging);

void inspect_init();

bool h_paging_inspect_go_to_root_screen();

bool h_paging_inspect_back_screen();
