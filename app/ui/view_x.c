/*******************************************************************************
 *   (c) 2018 - 2023 Zondax AG
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

#include "bolos_target.h"

#if defined(TARGET_NANOX) || defined(TARGET_NANOS2)

#include "actions.h"
#include "apdu_codes.h"
#include "app_mode.h"
#include "bagl.h"
#include "glyphs.h"
#include "tx.h"
#include "view.h"
#include "view_internal.h"
#include "view_nano.h"
#include "view_nano_inspect.h"
#include "view_templates.h"
#include "zxmacros.h"

#ifdef APP_SECRET_MODE_ENABLED
#include "secret.h"
#endif

#include <stdio.h>
#include <string.h>

static bool custom_callback_active = false;
// Add global variable to store original callback at the top with other globals
static unsigned int (*original_button_callback)(unsigned int button_mask, unsigned int button_mask_counter) = NULL;

void account_enabled();
void shortcut_enabled();

static void h_expert_toggle();
static void h_expert_update();
static void h_review_loop_start();
static void h_review_loop_inside();
static void h_review_loop_end();
static unsigned int handle_button_push(unsigned int button_mask, unsigned int button_mask_counter);
static void set_button_callback(void);
static void reset_button_callback(void) ;

#ifdef APP_SECRET_MODE_ENABLED
static void h_secret_click();
#endif

#ifdef APP_ACCOUNT_MODE_ENABLED
static void h_account_toggle();
static void h_account_update();
#endif

#ifdef SHORTCUT_MODE_ENABLED
static void h_shortcut_toggle();
static void h_shortcut_update();
#endif

#ifdef APP_BLINDSIGN_MODE_ENABLED
static void h_blindsign_toggle();
static void h_blindsign_update();
#endif

static void h_shortcut(unsigned int);
static void run_ux_review_flow(review_type_e reviewType, const ux_flow_step_t *const start_step);
const ux_flow_step_t *ux_review_flow[MAX_REVIEW_UX_SCREENS];

#include "ux.h"
ux_state_t G_ux;
bolos_ux_params_t G_ux_params;
uint8_t flow_inside_loop;
extern unsigned int review_type;

UX_STEP_NOCB(ux_idle_flow_1_step, pbb,
             {
                 &C_icon_app,
                 MENU_MAIN_APP_LINE1,
                 viewdata.key,
             });
UX_STEP_CB_INIT(ux_idle_flow_2_step, bn, h_expert_update(), h_expert_toggle(),
                {
                    "Expert mode:",
                    viewdata.value,
                });
UX_STEP_NOCB(ux_idle_flow_3_step, bn,
             {
                 APPVERSION_LINE1,
                 APPVERSION_LINE2,
             });

UX_STEP_INIT(
    ux_review_dummy_pre,
    NULL,
    NULL,
    {
      set_button_callback();
      ux_flow_next();
    });

UX_STEP_NOCB(
    ux_review_skip_step,
    nn,
    {
      "Press right to read",
      "Double-press to skip"
    });
UX_STEP_NOCB(ux_review_skip_page, bnnn_paging, { .title = viewdata.key, .text = viewdata.value, });

#ifdef APP_SECRET_MODE_ENABLED
UX_STEP_CB(ux_idle_flow_4_step, bn, h_secret_click(),
           {
               "Developed by:",
               "Zondax.ch",
           });
#else
UX_STEP_NOCB(ux_idle_flow_4_step, bn,
             {
                 "Developed by:",
                 "Zondax.ch",
             });
#endif

UX_STEP_NOCB(ux_idle_flow_5_step, bn,
             {
                 "License:",
                 "Apache 2.0",
             });
UX_STEP_CB(ux_idle_flow_6_step, pb, os_sched_exit(-1),
           {
               &C_icon_dashboard,
               "Quit",
           });

#ifdef APP_ACCOUNT_MODE_ENABLED
UX_STEP_CB_INIT(ux_idle_flow_7_step, bn, h_account_update(), h_account_toggle(),
                {
                    "Account:",
                    viewdata.value,
                });
#endif

#ifdef SHORTCUT_MODE_ENABLED
UX_STEP_CB_INIT(ux_idle_flow_8_step, bn, h_shortcut_update(), h_shortcut_toggle(),
                {
                    "Shortcut mode:",
                    viewdata.value,
                });
#endif

#ifdef APP_BLINDSIGN_MODE_ENABLED
UX_STEP_CB_INIT(ux_idle_flow_9_step, bn, h_blindsign_update(), h_blindsign_toggle(),
                {
                    "Blind sign:",
                    viewdata.value,
                });
#endif

const ux_flow_step_t *const ux_idle_flow[] = {
    &ux_idle_flow_1_step, &ux_idle_flow_2_step,
#ifdef APP_BLINDSIGN_MODE_ENABLED
    &ux_idle_flow_9_step,
#endif
#ifdef APP_ACCOUNT_MODE_ENABLED
    &ux_idle_flow_7_step,
#endif
#ifdef SHORTCUT_MODE_ENABLED
    &ux_idle_flow_8_step,
#endif
    &ux_idle_flow_3_step, &ux_idle_flow_4_step, &ux_idle_flow_5_step, &ux_idle_flow_6_step, FLOW_END_STEP,
};

///////////
UX_STEP_CB_INIT(ux_menu_init_flow_2_step, bn, NULL, h_initialize(),
                {
                    "Click to",
                    "Initialize",
                });
UX_STEP_NOCB(ux_menu_init_flow_4_step, bn,
             {
                 "Developed by:",
                 "Zondax.ch",
             });

const ux_flow_step_t *const ux_menu_initialize[] = {
    &ux_idle_flow_1_step, &ux_menu_init_flow_2_step, &ux_idle_flow_3_step, &ux_menu_init_flow_4_step,
    &ux_idle_flow_5_step, &ux_idle_flow_6_step,

    FLOW_END_STEP,
};

///////////

UX_STEP_NOCB(ux_message_flow_1_step, pbb,
             {
                 &C_icon_app,
                 viewdata.key,
                 viewdata.value,
             });

UX_FLOW(ux_message_flow, &ux_message_flow_1_step);

///////////

UX_STEP_NOCB(ux_error_flow_1_step, bnnn_paging,
             {
                 .title = viewdata.key,
                 .text = viewdata.value,
             });
UX_STEP_VALID(ux_error_flow_2_step, pb, h_error_accept(0), {&C_icon_validate_14, "Ok"});

UX_FLOW(ux_error_flow, &ux_error_flow_1_step, &ux_error_flow_2_step);

///////////

UX_STEP_NOCB(ux_custom_error_flow_1_step, pbb,
             {
                 &C_icon_warning,
                 viewdata.key,
                 viewdata.value,
             });
UX_STEP_VALID(ux_custom_error_flow_2_step, pb, h_error_accept(0), {&C_icon_validate_14, "Ok"});

UX_FLOW(ux_custom_error_flow, &ux_custom_error_flow_1_step, &ux_custom_error_flow_2_step);

///////////
UX_STEP_CB(ux_warning_blind_sign_step, pnn, h_error_accept(0),
           {
               &C_icon_crossmark,
               "Blind signing must be",
               "enabled in Settings",
           });
UX_FLOW(ux_warning_blind_sign_flow, &ux_warning_blind_sign_step);

UX_STEP_NOCB(ux_approval_blind_signing_warning_step, pbb,
             {
                 &C_icon_warning,
                 "Blind",
                 "Signing",
             });

#ifdef APP_BLINDSIGN_MODE_ENABLED
UX_STEP_NOCB(ux_approval_blind_signing_message_step, bnnn_paging,
             {
                 REVIEW_BLINDSIGN_MESSAGE_TITLE,
                 REVIEW_BLINDSIGN_MESSAGE_VALUE,
             });
#endif
///////////

UX_FLOW_DEF_NOCB(ux_review_flow_1_review_title, pbb,
                 {
                     &C_icon_app,
                     REVIEW_SCREEN_TITLE,
                     REVIEW_SCREEN_TXN_VALUE,
                 });
UX_FLOW_DEF_NOCB(ux_review_flow_2_review_title, pbb,
                 {
                     &C_icon_app,
                     REVIEW_SCREEN_TITLE,
                     REVIEW_SCREEN_ADDR_VALUE,
                 });
UX_FLOW_DEF_NOCB(ux_review_flow_3_review_title, pbb,
                 {
                     &C_icon_app,
                     "Review",
                     "configuration",
                 });
UX_FLOW_DEF_NOCB(ux_review_flow_4_review_title, pbb,
                 {
                     &C_icon_app,
                     REVIEW_MSG_TITLE,
                     REVIEW_MSG_VALUE,
                 });

UX_STEP_INIT(ux_review_flow_2_start_step, NULL, NULL, { h_review_loop_start(); });
#ifdef HAVE_INSPECT
UX_STEP_CB_INIT(ux_review_flow_2_step, bnnn_paging, h_review_loop_inside(), inspect_init(),
                {
                    .title = viewdata.key,
                    .text = viewdata.value,
                });
#else
UX_STEP_NOCB_INIT(ux_review_flow_2_step, bnnn_paging, { h_review_loop_inside(); },
                  {
                      .title = viewdata.key,
                      .text = viewdata.value,
                  });
#endif
UX_STEP_INIT(ux_review_flow_2_end_step, NULL, NULL, { h_review_loop_end(); });
#if defined(APP_BLINDSIGN_MODE_ENABLED)
UX_STEP_VALID(ux_review_flow_3_step_blindsign, pnn, h_approve(0),
              {&C_icon_validate_14, APPROVE_LABEL_1, APPROVE_LABEL_2});
#endif
UX_STEP_VALID(ux_review_flow_3_step, pb, h_approve(0), {&C_icon_validate_14, APPROVE_LABEL});

UX_STEP_VALID(ux_review_flow_4_step, pb, h_reject(review_type), {&C_icon_crossmark, REJECT_LABEL});
UX_STEP_VALID(ux_review_flow_6_step, pb, h_approve(0), {&C_icon_validate_14, "Ok"});

UX_STEP_CB_INIT(ux_review_flow_5_step, pb, NULL, h_shortcut(0), {&C_icon_eye, SHORTCUT_STR});

//////////////////////////
//////////////////////////
//////////////////////////
//////////////////////////
//////////////////////////

void h_review_update() {
    zxerr_t err = h_review_update_data();
    switch (err) {
        case zxerr_ok:
        case zxerr_no_data:
            break;
        default:
            view_error_show();
            break;
    }
}

void h_review_loop_start() {
    if (flow_inside_loop) {
        // coming from right

        if (!h_paging_can_decrease()) {
            // exit to the left
            flow_inside_loop = 0;
            ux_flow_prev();
            return;
        }

        h_paging_decrease();
    } else {
        // coming from left
        h_paging_init();
    }

    h_review_update();

    ux_flow_next();
}

void h_review_loop_inside() { flow_inside_loop = 1; }

void h_review_loop_end() {
    if (flow_inside_loop) {
        // coming from left
        h_paging_increase();
        zxerr_t err = h_review_update_data();

        switch (err) {
            case zxerr_ok:
                ux_layout_bnnn_paging_reset();
                // If we're at the end of current item and there's more to show
                if (viewdata.with_confirmation &&
                    (review_type == REVIEW_TXN || review_type == REVIEW_MSG) &&
                    viewdata.pageIdx == viewdata.pageCount - 1 &&
                    viewdata.itemIdx < viewdata.itemCount - 1) {

                    // Show skip screen and enable button handler
                    uint8_t index = 0;
                    ux_review_flow[index++] = &ux_review_dummy_pre;
                    ux_review_flow[index++] = &ux_review_skip_step;
                    ux_review_flow[index++] = FLOW_END_STEP;
                    ux_flow_init(0, ux_review_flow, NULL);
                    set_button_callback();
                    return;
                }
                break;

            case zxerr_no_data: {
                flow_inside_loop = 0;
                ux_flow_next();
                return;
            }
            default:
                view_error_show();
                break;
        }
    } else {
        // coming from right
        h_paging_decrease();
        h_review_update();
    }

    // move to prev flow but trick paging to show first page
    CUR_FLOW.prev_index = CUR_FLOW.index - 2;
    CUR_FLOW.index--;
    ux_flow_relayout();
}

void splitValueField() {
    uint16_t vlen = strlen(viewdata.value);
    if (vlen == 0) {
        snprintf(viewdata.value, MAX_CHARS_PER_VALUE1_LINE, " ");
    }
}

void splitValueAddress() { splitValueField(); }

max_char_display get_max_char_per_line() { return MAX_CHARS_PER_VALUE1_LINE; }

void h_expert_toggle() {
    app_mode_set_expert(!app_mode_expert());
    ux_flow_init(0, ux_idle_flow, &ux_idle_flow_2_step);
}

void h_expert_update() {
    snprintf(viewdata.value, MAX_CHARS_PER_VALUE1_LINE, "disabled");
    if (app_mode_expert()) {
        snprintf(viewdata.value, MAX_CHARS_PER_VALUE1_LINE, "enabled");
    }
}

#ifdef APP_BLINDSIGN_MODE_ENABLED
void h_blindsign_toggle() {
    app_mode_set_blindsign(!app_mode_blindsign());
    ux_flow_init(0, ux_idle_flow, &ux_idle_flow_9_step);
}

void h_blindsign_update() {
    snprintf(viewdata.value, MAX_CHARS_PER_VALUE1_LINE, "disabled");
    if (app_mode_blindsign()) {
        snprintf(viewdata.value, MAX_CHARS_PER_VALUE1_LINE, "enabled");
    }
}
#endif

#ifdef APP_ACCOUNT_MODE_ENABLED
void h_account_toggle() {
    if (app_mode_expert()) {
        account_enabled();
    } else {
        ux_flow_init(0, ux_idle_flow, &ux_idle_flow_7_step);
    }
}

void h_account_update() {
    snprintf(viewdata.value, MAX_CHARS_PER_VALUE1_LINE, ACCOUNT_DEFAULT);
    if (app_mode_account()) {
        snprintf(viewdata.value, MAX_CHARS_PER_VALUE1_LINE, ACCOUNT_SECONDARY);
    }
}
#endif

#ifdef SHORTCUT_MODE_ENABLED
void h_shortcut_toggle() {
    if (app_mode_expert() && !app_mode_shortcut()) {
        shortcut_enabled();
        return;
    }
    app_mode_set_shortcut(0);
    ux_flow_init(0, ux_idle_flow, &ux_idle_flow_8_step);
}

void h_shortcut_update() {
    snprintf(viewdata.value, MAX_CHARS_PER_VALUE1_LINE, "disabled");
    if (app_mode_shortcut()) {
        snprintf(viewdata.value, MAX_CHARS_PER_VALUE1_LINE, "enabled");
    }
}
#endif

#ifdef APP_SECRET_MODE_ENABLED
void h_secret_click() {
    if (COIN_SECRET_REQUIRED_CLICKS == 0) {
        // There is no secret mode
        return;
    }

    viewdata.secret_click_count++;

    char buffer[50];
    snprintf(buffer, sizeof(buffer), "secret click %d\n", viewdata.secret_click_count);
    zemu_log(buffer);

    if (viewdata.secret_click_count >= COIN_SECRET_REQUIRED_CLICKS) {
        secret_enabled();
        viewdata.secret_click_count = 0;
        return;
    }

    ux_flow_init(0, ux_idle_flow, &ux_idle_flow_4_step);
}
#endif

static void h_shortcut(__Z_UNUSED unsigned int _) { run_ux_review_flow(REVIEW_TXN, &ux_review_flow_3_step); }

//////////////////////////
//////////////////////////
//////////////////////////
//////////////////////////
//////////////////////////

void view_idle_show_impl(__Z_UNUSED uint8_t item_idx, const char *statusString) {
    if (statusString == NULL) {
        snprintf(viewdata.key, MAX_CHARS_PER_KEY_LINE, "%s", MENU_MAIN_APP_LINE2);
#ifdef APP_SECRET_MODE_ENABLED
        if (app_mode_secret()) {
            snprintf(viewdata.key, MAX_CHARS_PER_KEY_LINE, "%s", MENU_MAIN_APP_LINE2_SECRET);
        }
#endif
    } else {
        snprintf(viewdata.key, MAX_CHARS_PER_KEY_LINE, "%s", statusString);
    }

    if (G_ux.stack_count == 0) {
        ux_stack_push();
    }
    ux_flow_init(0, ux_idle_flow, NULL);
}

void view_initialize_show_impl(__Z_UNUSED uint8_t item_idx, const char *statusString) {
    if (statusString == NULL) {
        snprintf(viewdata.key, MAX_CHARS_PER_KEY_LINE, "%s", "Not Ready");
    } else {
        snprintf(viewdata.key, MAX_CHARS_PER_KEY_LINE, "%s", statusString);
    }

    if (G_ux.stack_count == 0) {
        ux_stack_push();
    }
    ux_flow_init(0, ux_menu_initialize, NULL);
}

void view_review_show_impl(unsigned int requireReply, const char *title, const char *validate) {
    UNUSED(title);
    UNUSED(validate);
    review_type = requireReply;
    h_paging_init();
    h_paging_decrease();
    ////
    flow_inside_loop = 0;
    if (G_ux.stack_count == 0) {
        ux_stack_push();
    }

    run_ux_review_flow((review_type_e)review_type, NULL);
}

void run_root_txn_flow() { run_ux_review_flow(review_type, &ux_review_flow_2_start_step); }

// Build review UX flow and run it
void run_ux_review_flow(review_type_e reviewType, const ux_flow_step_t *const start_step) {
    uint8_t index = 0;

    switch (reviewType) {
        case REVIEW_UI:
            ux_review_flow[index++] = &ux_review_flow_3_review_title;
            break;

        case REVIEW_ADDRESS:
            ux_review_flow[index++] = &ux_review_flow_2_review_title;
            break;

        case REVIEW_MSG:
            ux_review_flow[index++] = &ux_review_flow_4_review_title;
            break;

        case REVIEW_GENERIC:
        case REVIEW_TXN:
        default:
#ifdef APP_BLINDSIGN_MODE_ENABLED
            if (app_mode_blindsign_required()) {
                ux_review_flow[index++] = &ux_approval_blind_signing_warning_step;
                ux_review_flow[index++] = &ux_approval_blind_signing_message_step;
            }
#endif
            ux_review_flow[index++] = &ux_review_flow_1_review_title;
            if (app_mode_shortcut()) {
                ux_review_flow[index++] = &ux_review_flow_5_step;
            }
            break;
    }

    ux_review_flow[index++] = &ux_review_flow_2_start_step;
    ux_review_flow[index++] = &ux_review_flow_2_step;
    ux_review_flow[index++] = &ux_review_flow_2_end_step;

    if (reviewType == REVIEW_MSG) {
        ux_review_flow[index++] = &ux_review_flow_6_step;
    } else {
#ifdef APP_BLINDSIGN_MODE_ENABLED
        if (app_mode_blindsign_required() && reviewType == REVIEW_TXN) {
            ux_review_flow[index++] = &ux_review_flow_3_step_blindsign;
        } else {
            ux_review_flow[index++] = &ux_review_flow_3_step;
        }
#else
        ux_review_flow[index++] = &ux_review_flow_3_step;
#endif
        ux_review_flow[index++] = &ux_review_flow_4_step;
    }
    ux_review_flow[index++] = FLOW_END_STEP;

    ux_flow_init(0, ux_review_flow, start_step);
}

void view_message_impl(const char *title, const char *message) {
    snprintf(viewdata.key, MAX_CHARS_PER_KEY_LINE, "%s", title);
    snprintf(viewdata.value, MAX_CHARS_PER_VALUE1_LINE, "%s", message);
    ux_layout_bnnn_paging_reset();
    if (G_ux.stack_count == 0) {
        ux_stack_push();
    }
    ux_flow_init(0, ux_message_flow, NULL);
}

void view_error_show_impl() {
    ux_layout_bnnn_paging_reset();
    if (G_ux.stack_count == 0) {
        ux_stack_push();
    }
    ux_flow_init(0, ux_error_flow, NULL);
}

void view_custom_error_show_impl() {
    ux_layout_bnnn_paging_reset();
    if (G_ux.stack_count == 0) {
        ux_stack_push();
    }
    ux_flow_init(0, ux_custom_error_flow, NULL);
}

void view_blindsign_error_show_impl() { ux_flow_init(0, ux_warning_blind_sign_flow, NULL); }

static unsigned int handle_button_push(unsigned int button_mask, unsigned int button_mask_counter) {
    UNUSED(button_mask_counter);

    if (!custom_callback_active) {
        if (original_button_callback != NULL) {
            return original_button_callback(button_mask, button_mask_counter);
        }
        return 0;
    }

    switch (button_mask) {
        // Handle skip to approve
        case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT:
            G_ux.stack[0].button_push_callback = NULL;
            if (review_type == REVIEW_MSG) {
                run_ux_review_flow((review_type_e)review_type, &ux_review_flow_6_step);
            } else {
                run_ux_review_flow((review_type_e)review_type, &ux_review_flow_3_step);
            }
            reset_button_callback();
            return 1;

        // Handle continue review
        case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
            viewdata.itemIdx++;
            // Temporarily disable callback to prevent double processing
            G_ux.stack[0].button_push_callback = NULL;
            run_ux_review_flow((review_type_e)review_type, &ux_review_flow_2_start_step);
            reset_button_callback();
            return 1;
    }

    return 0;
}

static void set_button_callback(void) {
    custom_callback_active = true;
    // Store default callback to restablish later
    original_button_callback = G_ux.stack[0].button_push_callback;
    G_ux.stack[0].button_push_callback = handle_button_push;
}

static void reset_button_callback(void) {
    custom_callback_active = false;
    G_ux.stack[0].button_push_callback = original_button_callback;
    // original_button_callback = NULL;
}
#endif
