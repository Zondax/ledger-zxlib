/*******************************************************************************
*   (c) 2018 - 2022 Zondax AG
*   (c) 2016 Ledger
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
#include <stdint.h>
#include "coin.h"
#include "zxerror.h"

#if defined(LEDGER_SPECIFIC)
#include "bolos_target.h"
#if defined(BOLOS_SDK)
#include "os.h"
#include "cx.h"
#endif
#endif

typedef struct {
    uint8_t displayIdx;
    char *outKey;
    uint16_t outKeyLen;
    char *outVal;
    uint16_t outValLen;
    uint8_t pageIdx;
    uint8_t *pageCount;
} ui_field_t;

typedef struct {
    uint8_t itemIdx;
    uint8_t itemCount;
    uint8_t pageIdx;
    uint8_t pageCount;
} paging_t;

#define MAX_DEPTH   10
typedef struct {
    uint8_t level;
    uint8_t trace[MAX_DEPTH];
    paging_t paging;
} inner_state_t;

typedef zxerr_t (*viewfunc_getNumItems_t)(uint8_t *num_items);

typedef zxerr_t (*viewfunc_getItem_t)(int8_t displayIdx,
                                      char *outKey, uint16_t outKeyLen,
                                      char *outVal, uint16_t outValLen,
                                      uint8_t pageIdx, uint8_t *pageCount);

typedef zxerr_t (*viewfunc_getInnerItem_t)(uint8_t depth_level, uint8_t *trace, ui_field_t *ui_field);

typedef bool (*viewfunc_canInspectItem_t)(uint8_t depth_level, uint8_t *trace, uint8_t innerItemIdx);

typedef void (*viewfunc_accept_t)();

typedef zxerr_t (*viewfunc_initialize_t)();

typedef enum {
  REVIEW_UI = 0,
  REVIEW_ADDRESS,
  REVIEW_TXN,
} review_type_e;

#ifdef APP_SECRET_MODE_ENABLED
zxerr_t secret_enabled();
#endif

/// view_init (initializes UI)
void view_init();

/// view_initialize_show (idle view - main menu + status)
void view_initialize_init(viewfunc_initialize_t viewFuncInit);
void view_initialize_show(uint8_t item_idx, const char *statusString);

/// view_idle_show (idle view - main menu + status)
void view_idle_show(uint8_t item_idx, const char *statusString);

void view_message_show(const char *title, const char *message);

/// view_error (error view)
void view_error_show();

void view_custom_error_show(const char *upper, const char *lower);

void view_review_init(viewfunc_getItem_t viewfuncGetItem,
                      viewfunc_getNumItems_t viewfuncGetNumItems,
                      viewfunc_accept_t viewfuncAccept);

void view_inspect_init(viewfunc_getInnerItem_t view_funcGetInnerItem,
                      viewfunc_getNumItems_t view_funcGetInnerNumItems,
                      viewfunc_canInspectItem_t view_funcCanInspectItem);

void view_review_show(review_type_e reviewKind);
