/*******************************************************************************
*   (c) 2018 - 2022 Zondax AG
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

#if defined(TARGET_STAX)

#include "view_internal.h"
#include "ux.h"
#include "app_mode.h"
#include "nbgl_use_case.h"
#include "actions.h"

#include "nbgl_page.h"

#ifdef APP_SECRET_MODE_ENABLED
zxerr_t secret_enabled();
#endif

#ifdef APP_ACCOUNT_MODE_ENABLED
zxerr_t account_enabled();
#endif

#ifdef SHORTCUT_MODE_ENABLED
zxerr_t shortcut_enabled();
#endif

#define APPROVE_LABEL_STAX "Sign transaction?"
#define REJECT_LABEL_STAX "Reject transaction"
#define CANCEL_LABEL "Cancel"
#define HOLD_TO_APPROVE_MSG "Hold to sign"

static const char HOME_TEXT[] = "This application enables\nsigning transactions on the\n" MENU_MAIN_APP_LINE1 " network";

ux_state_t G_ux;
bolos_ux_params_t G_ux_params;
extern unsigned int review_type;

const char *txn_intro_message = NULL;

static nbgl_layoutTagValue_t pairs[NB_MAX_DISPLAYED_PAIRS_IN_REVIEW];

static nbgl_layoutTagValue_t pair;
static nbgl_layoutTagValueList_t pairList;
static nbgl_pageInfoLongPress_t infoLongPress;

static nbgl_layoutSwitch_t settings[4];

static uint8_t total_pages;


typedef enum {
    EXPERT_MODE = 0,
#ifdef APP_ACCOUNT_MODE_ENABLED
    ACCOUNT_MODE,
#endif
#ifdef SHORTCUT_MODE_ENABLED
    SHORTCUT_MODE,
#endif
#ifdef APP_SECRET_MODE_ENABLED
    SECRET_MODE,
#endif
} settings_list_e;


typedef enum {
  EXPERT_MODE_TOKEN = FIRST_USER_TOKEN,
  ACCOUNT_MODE_TOKEN,
  SHORTCUT_MODE_TOKEN,
  SECRET_MODE_TOKEN,
} config_token_e;

void app_quit(void) {
    // exit app here
    os_sched_exit(-1);
}

static void h_reject_internal(void) {
    h_reject(review_type);
}

static void h_approve_internal(void) {
    h_approve(review_type);
}

static void view_idle_show_impl_callback() {
    view_idle_show_impl(0, NULL);
}

static const char* const INFO_KEYS[] = {"Version", "Developed by", "Website", "License"};
static const char* const INFO_VALUES[] = {APPVERSION, "Zondax AG", "https://zondax.ch", "Apache 2.0"};

static const char* txn_choice_message = "Reject transaction?";
static const char* add_choice_message = "Reject address?";
static const char* ui_choice_message = "Reject configuration?";

static const char* txn_verified = "Transaction signed";
static const char* txn_cancelled = "Transaction rejected";

static const char* add_verified = "Address verified";
static const char* add_cancelled = "Address verification\ncancelled";

static void h_expert_toggle() {
    app_mode_set_expert(!app_mode_expert());
}

static void confirm_callback(bool confirm) {
    const char* message = NULL;
    switch (review_type) {
        case REVIEW_ADDRESS:
            message = confirm ? add_verified : add_cancelled;
            break;

        case REVIEW_TXN:
            message = confirm ? txn_verified : txn_cancelled;
            break;

        case REVIEW_UI:
        default:
            confirm ? h_approve(review_type) : h_reject(review_type);
            return;
    }
    nbgl_useCaseStatus(message, confirm, (confirm ? h_approve_internal : h_reject_internal));
}

static void cancel(void) {
    ZEMU_LOGF(50, "Cancelling...\n")
    confirm_callback(false);
}

static void action_callback(bool confirm) {
    ZEMU_LOGF(50, "Check action callback: %d\n", confirm)
    if (confirm) {
        confirm_callback(confirm);
        return;
    }

    const char* message = NULL;
    switch (review_type) {
        case REVIEW_UI:
            message = ui_choice_message;
            break;

        case REVIEW_ADDRESS:
            message = add_choice_message;
            break;

        case REVIEW_TXN:
            message = txn_choice_message;
            break;

        default:
            ZEMU_LOGF(50, "Error unrecognize review option\n")
            view_error_show();
            return;
    }

    nbgl_useCaseConfirm(message,
                        NULL,
                        "Yes, reject",
                        "Go back",
                        cancel);
}

static void check_cancel(void) {
    action_callback(false);
}

static void confirm_setting(bool confirm) {
    if (confirm && viewdata.viewfuncAccept != NULL) {
        viewdata.viewfuncAccept();
        return;
    }
    confirm_callback(confirm);
}

void view_error_show() {
    viewdata.key = viewdata.keys[0];
    viewdata.value = viewdata.values[0];
    MEMZERO(viewdata.key, MAX_CHARS_PER_KEY_LINE);
    MEMZERO(viewdata.value, MAX_CHARS_PER_VALUE1_LINE);
    snprintf(viewdata.key, MAX_CHARS_PER_KEY_LINE, "ERROR");
    snprintf(viewdata.value, MAX_CHARS_PER_VALUE1_LINE, "SHOWING DATA");
    view_error_show_impl();
}

void view_error_show_impl() {
    nbgl_useCaseChoice(&C_round_warning_64px, viewdata.key, viewdata.value, "Ok", NULL, confirm_setting);
}

zxerr_t h_review_update_data() {
    if (viewdata.viewfuncGetNumItems == NULL) {
        ZEMU_LOGF(50, "h_review_update_data - GetNumItems == NULL\n")
        return zxerr_no_data;
    }
    if (viewdata.viewfuncGetItem == NULL) {
        ZEMU_LOGF(50, "h_review_update_data - GetItems == NULL\n")
        return zxerr_no_data;
    }

    if (viewdata.viewfuncAccept == NULL) {
        ZEMU_LOGF(50, "h_review_update_data - Function Accept == NULL\n")
        return zxerr_no_data;
    }

    if (viewdata.key == NULL || viewdata.value == NULL) {
        return zxerr_unknown;
    }

    CHECK_ZXERR(viewdata.viewfuncGetNumItems(&viewdata.itemCount))

    if (viewdata.itemIdx  >= viewdata.itemCount) {
        return zxerr_no_data;
    }

    CHECK_ZXERR(viewdata.viewfuncGetItem(
            viewdata.itemIdx,
            viewdata.key, MAX_CHARS_PER_KEY_LINE,
            viewdata.value, MAX_CHARS_PER_VALUE1_LINE,
            viewdata.pageIdx, &viewdata.pageCount))

    return zxerr_ok;
}

void h_review_update() {
    zxerr_t err = h_review_update_data();
    switch(err) {
        case zxerr_ok:
        case zxerr_no_data:
            break;
        default:
            ZEMU_LOGF(50, "View error show\n")
            view_error_show();
            break;
    }
}

static bool settings_screen_callback(uint8_t page, nbgl_pageContent_t* content) {
    switch ((uint8_t) page)
    {
        case 0: {
            content->type = INFOS_LIST;
            content->infosList.nbInfos = sizeof(INFO_KEYS)/sizeof(INFO_KEYS[0]);
            content->infosList.infoContents = INFO_VALUES;
            content->infosList.infoTypes = INFO_KEYS;
            break;
        }

        case 1: {
            // Config
            content->type = SWITCHES_LIST;
            content->switchesList.nbSwitches = 1;
            content->switchesList.switches = settings;

            settings[0].initState = app_mode_expert();
            settings[0].text = "Expert mode";
            settings[0].tuneId = TUNE_TAP_CASUAL;
            settings[0].token = EXPERT_MODE_TOKEN;

#ifdef APP_ACCOUNT_MODE_ENABLED
            if (app_mode_expert() || app_mode_account()) {
                settings[ACCOUNT_MODE].initState = app_mode_account();
                settings[ACCOUNT_MODE].text = "Crowdloan account";
                settings[ACCOUNT_MODE].tuneId = TUNE_TAP_CASUAL;
                settings[ACCOUNT_MODE].token = ACCOUNT_MODE_TOKEN;
                content->switchesList.nbSwitches++;
            }
#endif

#ifdef SHORTCUT_MODE_ENABLED
            if (app_mode_expert() || app_mode_shortcut()) {
                settings[SHORTCUT_MODE].initState = app_mode_shortcut();
                settings[SHORTCUT_MODE].text = "Shortcut mode";
                settings[SHORTCUT_MODE].tuneId = TUNE_TAP_CASUAL;
                settings[SHORTCUT_MODE].token = SHORTCUT_MODE_TOKEN;
                content->switchesList.nbSwitches++;
            }
#endif

#ifdef APP_SECRET_MODE_ENABLED
            if (app_mode_expert() || app_mode_secret()) {
                settings[SECRET_MODE].initState = app_mode_secret();
                settings[SECRET_MODE].text = "Secret mode";
                settings[SECRET_MODE].tuneId = TUNE_TAP_CASUAL;
                settings[SECRET_MODE].token = SECRET_MODE_TOKEN;
                content->switchesList.nbSwitches++;
            }
#endif
            break;
        }

        default:
            ZEMU_LOGF(50, "Incorrect settings page: %d\n", page)
            return false;
    }

    return true;
}

static void settings_toggle_callback(int token, __Z_UNUSED uint8_t index) {
    switch (token) {
        case EXPERT_MODE_TOKEN:
            h_expert_toggle();
            break;

#ifdef APP_ACCOUNT_MODE_ENABLED
        case ACCOUNT_MODE_TOKEN:
            account_enabled();
            break;
#endif

#ifdef SHORTCUT_MODE_ENABLED
        case SHORTCUT_MODE_TOKEN:
            shortcut_enabled();
            break;
#endif

#ifdef APP_SECRET_MODE_ENABLED
        case SECRET_MODE_TOKEN:
            secret_enabled();
            break;
#endif

        default:
            ZEMU_LOGF(50, "Toggling setting not found\n")
            break;
    }
}

void setting_screen() {
    //Set return button top-left (true) botton-left (false)
    const bool return_button_top_left = true;
    const uint8_t init_page = 0;
    const uint8_t settings_pages = 2;
    nbgl_useCaseSettings(MENU_MAIN_APP_LINE1, init_page, settings_pages, return_button_top_left,
                        view_idle_show_impl_callback, settings_screen_callback, settings_toggle_callback);
}

void view_idle_show_impl(__Z_UNUSED uint8_t item_idx, char *statusString) {
    viewdata.key = viewdata.keys[0];
    const char *home_text = HOME_TEXT;
    if (statusString == NULL ) {
#ifdef APP_SECRET_MODE_ENABLED
        if (app_mode_secret()) {
            snprintf(viewdata.key, MAX_CHARS_PER_KEY_LINE, "%s", MENU_MAIN_APP_LINE2_SECRET);
            home_text = viewdata.key;
        }
#endif
    } else {
        snprintf(viewdata.key, MAX_CHARS_PER_KEY_LINE, "%s", statusString);
    }
    const bool settings_icon = true;
    nbgl_useCaseHome(MENU_MAIN_APP_LINE1, &C_icon_stax_64, home_text, settings_icon, setting_screen, app_quit);
}

static zxerr_t update_data_page(uint8_t page, uint8_t *elementsPerPage) {
    if (elementsPerPage == NULL) {
        return zxerr_unknown;
    }

    *elementsPerPage = 0;
    bool tooLongToFit = false;
    uint8_t pageIdx = 0;
    uint8_t itemIdx = 0;

    // Move until current page
    while (pageIdx < page) {
        const uint8_t fieldsInPage = nbgl_useCaseGetNbTagValuesInPage(pairList.nbPairs, &pairList, itemIdx, &tooLongToFit);
        itemIdx += fieldsInPage;
        pageIdx++;
    }

    // Get printeable items for this page and retrieve them
    const uint8_t fieldsInCurrentPage = nbgl_useCaseGetNbTagValuesInPage(pairList.nbPairs, &pairList, itemIdx, &tooLongToFit);
    for (uint8_t idx = 0; idx < fieldsInCurrentPage; idx++) {
        viewdata.itemIdx = itemIdx + idx;
        viewdata.key = viewdata.keys[idx];
        viewdata.value = viewdata.values[idx];
        CHECK_ZXERR(h_review_update_data())
    }
    *elementsPerPage = fieldsInCurrentPage;

    return zxerr_ok;
}

static bool transaction_screen_callback(uint8_t page, nbgl_pageContent_t *content) {

    const zxerr_t err = (page == total_pages || page == LAST_PAGE_FOR_REVIEW) ? zxerr_no_data : update_data_page(page, &content->tagValueList.nbPairs);

    switch(err) {
        case zxerr_ok: {
            content->type = TAG_VALUE_LIST;
            content->tagValueList.pairs = pairs;
            content->tagValueList.wrapping = false;
            content->tagValueList.nbMaxLinesForValue = NB_MAX_LINES_IN_REVIEW;;

            for (uint8_t i = 0; i < content->tagValueList.nbPairs; i++) {
                pairs[i].item = viewdata.keys[i];
                pairs[i].value = viewdata.values[i];
            }
            break;
        }
        case zxerr_no_data: {
            content->type = INFO_LONG_PRESS;
            content->infoLongPress.icon = &C_icon_stax_64;
            content->infoLongPress.text = APPROVE_LABEL_STAX;
            content->infoLongPress.longPressText = HOLD_TO_APPROVE_MSG;
            break;
        }
        default:
            ZEMU_LOGF(50, "View error show\n")
            view_error_show();
            break;
    }

    return true;
}

static void review_configuration() {
    viewdata.key = viewdata.keys[0];
    viewdata.value = viewdata.values[0];
    const zxerr_t err = h_review_update_data();
    if (err != zxerr_ok) {
        ZEMU_LOGF(50, "Config screen error\n")
        view_error_show();
    }

    nbgl_useCaseChoice(&C_round_warning_64px, viewdata.key, viewdata.value, "Accept", "Reject", confirm_setting);
}

static void review_address() {
    nbgl_layoutTagValueList_t* extraPagesPtr = NULL;

    if (app_mode_expert()) {
        pairs[0].item = viewdata.keys[1];
        pairs[0].value = viewdata.values[1];

        viewdata.itemIdx = 1;
        viewdata.key = viewdata.keys[1];
        viewdata.value = viewdata.values[1];
        h_review_update_data();

        pairList.nbMaxLinesForValue = 0;
        pairList.nbPairs = 1;
        pairList.pairs = pairs;

        extraPagesPtr = &pairList;
    }

    viewdata.itemIdx = 0;
    viewdata.key = viewdata.keys[0];
    viewdata.value = viewdata.values[0];
    h_review_update_data();

    nbgl_useCaseAddressConfirmationExt(viewdata.value, action_callback, extraPagesPtr);
}

static nbgl_layoutTagValue_t* update_item_callback(uint8_t index) {
    uint8_t internalIndex = index % NB_MAX_DISPLAYED_PAIRS_IN_REVIEW;

    viewdata.itemIdx = index;
    viewdata.key = viewdata.keys[internalIndex];
    viewdata.value = viewdata.values[internalIndex];

    h_review_update_data();
    pair.item = viewdata.key;
    pair.value = viewdata.value;
    return &pair;
}

static void review_transaction_shortcut() {
    if (viewdata.viewfuncGetNumItems == NULL) {
        ZEMU_LOGF(50, "GetNumItems==NULL\n")
        view_error_show();
        return;
    }

    infoLongPress.icon = &C_icon_stax_64;
    infoLongPress.text = APPROVE_LABEL_STAX;
    infoLongPress.longPressText = HOLD_TO_APPROVE_MSG;

    pairList.nbMaxLinesForValue = NB_MAX_LINES_IN_REVIEW;
    viewdata.viewfuncGetNumItems(&viewdata.itemCount);

    pairList.nbPairs = viewdata.itemCount;
    pairList.pairs = NULL; // to indicate that callback should be used
    pairList.callback = update_item_callback;
    pairList.startIndex = 0;

    total_pages = nbgl_useCaseGetNbPagesForTagValueList(&pairList);
    nbgl_useCaseForwardOnlyReview(REJECT_LABEL_STAX, NULL, transaction_screen_callback, action_callback);
}

static void review_transaction_static() {
    if (viewdata.viewfuncGetNumItems == NULL) {
        ZEMU_LOGF(50, "GetNumItems==NULL\n")
        view_error_show();
        return;
    }

    infoLongPress.icon = &C_icon_stax_64;
    infoLongPress.text = APPROVE_LABEL_STAX;
    infoLongPress.longPressText = HOLD_TO_APPROVE_MSG;

    pairList.nbMaxLinesForValue = NB_MAX_LINES_IN_REVIEW;
    viewdata.viewfuncGetNumItems(&viewdata.itemCount);

    pairList.nbPairs = viewdata.itemCount;
    pairList.pairs = NULL; // to indicate that callback should be used
    pairList.callback = update_item_callback;
    pairList.startIndex = 0;

    nbgl_useCaseStaticReview(&pairList, &infoLongPress, REJECT_LABEL_STAX, action_callback);
}

void view_review_show_impl(unsigned int requireReply){
    review_type = (review_type_e) requireReply;

    // Retrieve intro text for transaction
    txn_intro_message = NULL;
    viewdata.key = viewdata.keys[0];
    viewdata.value = viewdata.values[0];
    if (viewdata.viewfuncGetItem != NULL) {
        const zxerr_t err = viewdata.viewfuncGetItem(0xFF, viewdata.key, MAX_CHARS_PER_KEY_LINE,
                                                    viewdata.value, MAX_CHARS_PER_VALUE1_LINE,
                                                    0, &viewdata.pageCount);
        if (err == zxerr_ok) {
            txn_intro_message = viewdata.value;
        }
    }
    h_paging_init();

    switch (review_type) {
        case REVIEW_UI:
            nbgl_useCaseReviewStart(&C_icon_stax_64,
                                    "Review configuration",
                                    NULL,
                                    CANCEL_LABEL,
                                    review_configuration,
                                    check_cancel);
            break;
        case REVIEW_ADDRESS:
            nbgl_useCaseReviewStart(&C_icon_stax_64,
                                    "Verify " MENU_MAIN_APP_LINE1 "\naddress",
                                    NULL,
                                    CANCEL_LABEL,
                                    review_address,
                                    check_cancel);
            break;

        case REVIEW_TXN:
        default:
            nbgl_useCaseReviewStart(&C_icon_stax_64,
                                    (txn_intro_message == NULL ? "Review transaction" : txn_intro_message),
                                    NULL,
                                    REJECT_LABEL_STAX,
                                    app_mode_shortcut() ? review_transaction_shortcut : review_transaction_static,
                                    check_cancel);
    }
}

#endif
