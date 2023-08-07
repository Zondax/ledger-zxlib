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
#include "bolos_target.h"

#if defined(TARGET_NANOS)
#include "view_internal.h"
#include "view_nano_inspect.h"
#include "view_templates.h"
#include "ux.h"
#include "bagl.h"

static void h_inspect_button_left();
static void h_inspect_button_right();
static void h_inspect_button_both();

static void h_inspect();
static void h_back();
static void h_rootTxn();

const bagl_element_t *view_prepro(const bagl_element_t *element);

static const bagl_element_t view_inspect[] = {
    UI_BACKGROUND_LEFT_RIGHT_ICONS,
    UI_LabelLine(UIID_LABEL + 0, 0, 8, UI_SCREEN_WIDTH, UI_11PX, UI_WHITE, UI_BLACK, viewdata.key),
    UI_LabelLine(UIID_LABEL + 1, 0, 19, UI_SCREEN_WIDTH, UI_11PX, UI_WHITE, UI_BLACK, viewdata.value),
    UI_LabelLine(UIID_LABEL + 2, 0, 30, UI_SCREEN_WIDTH, UI_11PX, UI_WHITE, UI_BLACK, viewdata.value2),
};


static unsigned int view_inspect_button(unsigned int button_mask, __Z_UNUSED unsigned int button_mask_counter) {
    switch (button_mask) {
        case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT:
            h_inspect_button_both();
            break;
        case BUTTON_EVT_RELEASED | BUTTON_LEFT:
            // Press left to progress to the previous element
            h_inspect_button_left();
            break;

        case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
            // Press right to progress to the next element
            h_inspect_button_right();
            break;
    }
    return 0;
}

void h_inspect_button_left() {
    ZEMU_LOGF(50, "h_inspect_button_left\n");
    h_decrease(&viewdata.innerField.paging);
    h_inspect_update_data();
    view_inspect_show_impl();
}

void h_inspect_button_right() {
    ZEMU_LOGF(50, "h_inspect_button_right\n");
    h_increase(&viewdata.innerField.paging, 0);
    h_inspect_update_data();
    view_inspect_show_impl();
}

void h_inspect_button_both() {
    ZEMU_LOGF(50, "h_inspect_button_both\n")
    paging_t page = viewdata.innerField.paging;

    if (page.itemIdx == 0) {
        h_rootTxn();
    } else if (page.itemIdx == (page.itemCount - 1)) {
        h_back();
    } else {
        h_inspect();
    }
}

void view_inspect_show_impl() {
    ZEMU_LOGF(50, "view_inspect_show_impl\n")
    UX_DISPLAY(view_inspect, view_prepro)
}

void h_inspect() {
    if (viewdata.viewfuncCanInspectItem == NULL) {
        view_error_show();
        return;
    }

    const uint8_t idxOffset = (viewdata.innerField.paging.itemIdx > 0) ? 1 : 0;
    // Check if we can inspect this element
    if (viewdata.innerField.level >= MAX_DEPTH ||
        !viewdata.viewfuncCanInspectItem(viewdata.innerField.level+1,
                                        viewdata.innerField.trace,
                                        viewdata.innerField.paging.itemIdx - idxOffset )) {
        h_inspect_update_data();
        view_inspect_show_impl();
        return;
    }

    // NanoS counts Go to root screen as an item, adjust offset
    viewdata.innerField.level++;
    viewdata.innerField.trace[viewdata.innerField.level] = viewdata.innerField.paging.itemIdx- idxOffset;    viewdata.innerField.paging.itemIdx = 1;
    viewdata.innerField.paging.pageCount = 1;
    viewdata.innerField.paging.itemCount = 0xFF;

    h_inspect_update_data();
    view_inspect_show_impl();
}

void h_rootTxn() {
    viewdata.innerField.level = 0;
    viewdata.itemIdx = viewdata.innerField.trace[viewdata.innerField.level];
    viewdata.innerField.trace[0] = 0;
    h_review_update();
}

void h_back() {
    if (viewdata.innerField.level > 1) {
        // For NanoS we need to bump the previous itemIdx because of the first screen
        viewdata.innerField.paging.itemIdx = viewdata.innerField.trace[viewdata.innerField.level] + 1;
        viewdata.innerField.level--;
        viewdata.innerField.paging.pageCount = 1;
        viewdata.innerField.paging.itemCount = 0xFF;

        h_inspect_update_data();
        view_inspect_show_impl();
        return;
    }

    // Go back to root txn review
    h_rootTxn();
}

void inspect_init() {
    #ifdef HAVE_INSPECT
        h_inspect_init();
        if (!viewdata.viewfuncCanInspectItem(viewdata.innerField.level,
                                            viewdata.innerField.trace,
                                            viewdata.innerField.paging.itemIdx)) {
            h_rootTxn();
            return;
        }
        // Skip first screen for NanoS
        viewdata.innerField.paging.itemIdx = 1;
        const zxerr_t err = h_inspect_update_data();
        if (err == zxerr_no_data) {
            return;
        }
        view_inspect_show_impl();
    #endif
}

#endif
