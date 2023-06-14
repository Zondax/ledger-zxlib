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

typedef enum {
    EXPERT_MODE = 0,
#ifdef APP_ACCOUNT_MODE_ENABLED
    ACCOUNT_MODE,
#endif
#ifdef APP_SECRET_MODE_ENABLED
    SECRET_MODE,
#endif
} settings_list_e;


typedef enum {
  EXPERT_MODE_TOKEN = FIRST_USER_TOKEN,
  ACCOUNT_MODE_TOKEN,
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

static const char* txn_verified = "TRANSACTION\nSIGNED";
static const char* txn_cancelled = "Transaction rejected";

static const char* add_verified = "ADDRESS\nVERIFIED";
static const char* add_cancelled = "Address verification\ncancelled";

static void h_expert_toggle() {
    app_mode_set_expert(!app_mode_expert());
}

static void confirm_error(__Z_UNUSED bool confirm) {
    h_error_accept(0);
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

void view_custom_error_show(const char *upper, const char *lower) {
    viewdata.key = viewdata.keys[0];
    viewdata.value = viewdata.values[0];
    MEMZERO(viewdata.key, MAX_CHARS_PER_KEY_LINE);
    MEMZERO(viewdata.value, MAX_CHARS_PER_VALUE1_LINE);
    snprintf(viewdata.key, MAX_CHARS_PER_KEY_LINE, "%s", upper);
    snprintf(viewdata.value, MAX_CHARS_PER_VALUE1_LINE, "%s", lower);

    nbgl_useCaseChoice(&C_round_warning_64px, viewdata.key, viewdata.value, "Ok", "", confirm_error);
}

void view_error_show_impl() {
    nbgl_useCaseChoice(&C_round_warning_64px, viewdata.key, viewdata.value, "Ok", NULL, confirm_setting);
}

static uint8_t get_pair_number() {
    uint8_t numItems = 0;
    uint8_t numPairs = 0;
    viewdata.viewfuncGetNumItems(&numItems);
    for (uint8_t i = 0; i < numItems; i++) {
        viewdata.viewfuncGetItem(
            i,
            viewdata.key, MAX_CHARS_PER_KEY_LINE,
            viewdata.value, MAX_CHARS_PER_VALUE1_LINE,
            0, &viewdata.pageCount);
        numPairs += viewdata.pageCount;
    }
    return numPairs;
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

    uint8_t accPages = 0;
    for (uint8_t i = 0; i < viewdata.itemCount; i++) {
        CHECK_ZXERR(viewdata.viewfuncGetItem(
                i,
                viewdata.key, MAX_CHARS_PER_KEY_LINE,
                viewdata.value, MAX_CHARS_PER_VALUE1_LINE,
                0, &viewdata.pageCount))
        if (viewdata.pageCount == 0) {
            ZEMU_LOGF(50, "pageCount is 0!")
            return zxerr_no_data;
        }

        if (accPages + viewdata.pageCount > viewdata.itemIdx) {
            const uint8_t innerIdx = viewdata.itemIdx - accPages;
            CHECK_ZXERR(viewdata.viewfuncGetItem(
                    i,
                    viewdata.key, MAX_CHARS_PER_KEY_LINE,
                    viewdata.value, MAX_CHARS_PER_VALUE1_LINE,
                    innerIdx, &viewdata.pageCount))
            if (viewdata.pageCount > 1) {
                const uint8_t titleLen = strnlen(viewdata.key, MAX_CHARS_PER_KEY_LINE);
                snprintf(viewdata.key + titleLen, MAX_CHARS_PER_KEY_LINE - titleLen, " (%d/%d)", innerIdx + 1, viewdata.pageCount);
            }
            return zxerr_ok;
        }
        accPages += viewdata.pageCount;
    }

    return zxerr_no_data;
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
    switch (page)
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
    const bool return_button_top_left = false;
    const uint8_t init_page = 0;
    const uint8_t settings_pages = 2;
    nbgl_useCaseSettings(MENU_MAIN_APP_LINE1, init_page, settings_pages, return_button_top_left,
                        view_idle_show_impl_callback, settings_screen_callback, settings_toggle_callback);
}

void view_idle_show_impl(__Z_UNUSED uint8_t item_idx, const char *statusString) {
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

void view_message_impl(const char *title, const char *message) {
    viewdata.value = viewdata.values[0];
    uint32_t titleLen = 0;
    if (title != NULL) {
        snprintf(viewdata.value, MAX_CHARS_PER_VALUE1_LINE, "%s", title);
        titleLen = strnlen(title, MAX_CHARS_PER_VALUE1_LINE);
    }

    if (message != NULL) {
        const char sep = (titleLen > 0) ? 0x0A : 0x00;
        snprintf(viewdata.value, MAX_CHARS_PER_VALUE1_LINE - titleLen, "%c%s", sep, message);
    }

    nbgl_useCaseSpinner(viewdata.value);
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

    uint8_t numItems = 0;
    if (viewdata.viewfuncGetNumItems == NULL ||
        viewdata.viewfuncGetNumItems(&numItems) != zxerr_ok ||
        numItems > NB_MAX_DISPLAYED_PAIRS_IN_REVIEW) {
        ZEMU_LOGF(50, "Show address error\n")
        view_error_show();
    }

    for (uint8_t idx = 1; idx < numItems; idx++) {
        pairs[idx - 1].item = viewdata.keys[idx];
        pairs[idx - 1].value = viewdata.values[idx];

        viewdata.itemIdx = idx;
        viewdata.key = viewdata.keys[idx];
        viewdata.value = viewdata.values[idx];
        h_review_update_data();

        pairList.nbMaxLinesForValue = 0;
        pairList.nbPairs = idx;
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
    pairList.nbPairs = get_pair_number();
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
                                    cancel);
            break;
        case REVIEW_ADDRESS: {
            #if defined(CUSTOM_ADDRESS_TEXT)
                const char ADDRESS_TEXT[] = CUSTOM_ADDRESS_TEXT;
            #else
                const char ADDRESS_TEXT[] = "Verify " MENU_MAIN_APP_LINE1 "\naddress";
            #endif
            nbgl_useCaseReviewStart(&C_icon_stax_64,
                                    ADDRESS_TEXT,
                                    NULL,
                                    CANCEL_LABEL,
                                    review_address,
                                    cancel);
            break;
        }
        case REVIEW_TXN:
        default:
            nbgl_useCaseReviewStart(&C_icon_stax_64,
                                    (txn_intro_message == NULL ? "Review transaction" : txn_intro_message),
                                    NULL,
                                    REJECT_LABEL_STAX,
                                    review_transaction_static,
                                    check_cancel);
    }
}

#endif
