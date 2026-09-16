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

#include "actions.h"
#include "app_mode.h"
#include "view_internal.h"
#include "zxmacros.h"

#define DEFAULT_SPINNER_TEXT "Processing..."

view_t viewdata;
unsigned int review_type = 0;

// Set for exactly as long as a screen owns the device: armed where a review or
// an error modal is drawn, released by every path that answers it. Apps gate
// their APDU dispatcher on view_review_is_pending() so the request on screen is
// the one that gets answered -- its tx buffer, derivation path and parsed
// context cannot be replaced between the moment it is drawn and the moment the
// user answers it.
//
// The error modals are included on purpose. A handler that raises one throws,
// so the host already has a status word, but the modal still owes a reply
// through h_error_accept() -> app_reply_error(). Leaving it unlocked lets the
// host push a review on top of the modal and have the user approve it with the
// button press they meant as "dismiss".
//
// Deliberately NOT armed for IO_ASYNCH_REPLY at large: chunked transfers and
// the EVM plugin / EIP-712 flows are asynchronous without putting anything on
// screen and must keep streaming.
static volatile bool review_pending = false;

void h_review_mark_pending(void) { review_pending = true; }

bool view_review_is_pending(void) { return review_pending; }

void view_review_clear_pending(void) { review_pending = false; }

///////////////////////////////////
// Paging related

bool h_review_is_skippable() {
#ifdef APP_BLINDSIGN_MODE_ENABLED
    // Only a blind-signed review may be skipped. On an ordinary review the shortcut would let a
    // user approve without seeing the details -- blind signing in all but name, reached without
    // the warning that names it and without the setting they would have had to turn on first.
    if (!app_mode_blindsign_required()) {
        return false;
    }

    uint8_t numItems = 0;
    if (viewdata.viewfuncGetNumItems == NULL || viewdata.viewfuncGetNumItems(&numItems) != zxerr_ok) {
        return false;
    }
    return numItems >= REVIEW_SKIP_MIN_ITEMS;
#else
    // No blind-signing mode means no opted-in state to key the shortcut off, so there is none.
    return false;
#endif
}

void h_paging_init() {
    zemu_log_stack("h_paging_init");

    viewdata.itemIdx = 0;
    viewdata.pageIdx = 0;
    viewdata.pageCount = 1;
    viewdata.itemCount = 0xFF;
}

///////////////////////////////////
// General
void view_init(void) {
    UX_INIT();
    review_pending = false;
#ifdef APP_SECRET_MODE_ENABLED
    viewdata.secret_click_count = 0;
#endif
}

void view_idle_show(uint8_t item_idx, const char *statusString) {
    // The main menu means nothing owns the device, so this is the backstop that
    // releases the lock. It is what recovers an IO reset taken mid-review: the
    // SDK unwinds into app_init(), which comes back through here rather than
    // through view_init() or any of the h_* handlers below.
    review_pending = false;
    view_idle_show_impl(item_idx, statusString);
}

void view_message_show(const char *title, const char *message) { view_message_impl(title, message); }

void view_spinner_show(const char *text) { view_spinner_impl(text ? text : DEFAULT_SPINNER_TEXT); }

void view_review_init(viewfunc_getItem_t viewfuncGetItem, viewfunc_getNumItems_t viewfuncGetNumItems,
                      viewfunc_accept_t viewfuncAccept) {
    viewdata.viewfuncGetItem = viewfuncGetItem;
    viewdata.viewfuncGetNumItems = viewfuncGetNumItems;
    viewdata.viewfuncAccept = viewfuncAccept;

#if defined(TARGET_NANOS) || defined(TARGET_NANOS2) || defined(TARGET_NANOX)
    viewdata.with_confirmation = false;
#endif
}

void view_review_init_progressive(viewfunc_getItem_t viewfuncGetItem, viewfunc_getNumItems_t viewfuncGetNumItems,
                                  viewfunc_accept_t viewfuncAccept) {
    view_review_init(viewfuncGetItem, viewfuncGetNumItems, viewfuncAccept);

#if defined(TARGET_NANOS) || defined(TARGET_NANOS2) || defined(TARGET_NANOX)
    viewdata.with_confirmation = true;
#endif
}

void view_initialize_init(viewfunc_initialize_t viewFuncInit) { viewdata.viewfuncInitialize = viewFuncInit; }

void view_review_show(review_type_e reviewKind) {
    review_pending = true;
    // Set > 0 to reply apdu message
    view_review_show_impl((unsigned int)reviewKind, NULL, NULL);
}

void view_review_show_with_intent(review_type_e reviewKind, const char *intent) {
    review_pending = true;
    // New function that explicitly handles intent
    view_review_show_with_intent_impl((unsigned int)reviewKind, intent);
}

void view_review_show_generic(review_type_e reviewKind, const char *title, const char *validate) {
    review_pending = true;
    view_review_show_impl((unsigned int)reviewKind, title, validate);
}

void view_initialize_show(uint8_t item_idx, const char *statusString) {
    view_initialize_show_impl(item_idx, statusString);
}

void h_approve(__Z_UNUSED unsigned int _) {
    zemu_log_stack("h_approve");

    review_pending = false;
    view_idle_show(0, NULL);
    UX_WAIT();
    if (viewdata.viewfuncAccept != NULL) {
        viewdata.viewfuncAccept();
    }
}

void h_reject(unsigned int requireReply) {
    zemu_log_stack("h_reject");

    review_pending = false;
    view_idle_show(0, NULL);
    UX_WAIT();

    if (requireReply != REVIEW_UI) {
        app_reject();
    }
}

void h_error_accept(__Z_UNUSED unsigned int _) {
    review_pending = false;
    view_idle_show(0, NULL);
    UX_WAIT();
    app_reply_error();
}
