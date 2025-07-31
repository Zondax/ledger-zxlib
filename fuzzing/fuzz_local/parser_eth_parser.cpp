#include <cassert>
#include <cstdint>
#include <cstdio>

#include "evm_def.h"
#include "evm_erc20.h"
#include "parser_evm.h"
#include "evm_utils.h"
#include "zxformat.h"

#ifdef NDEBUG
#error "This fuzz target won't work correctly with NDEBUG defined, which will cause asserts to be eliminated"
#endif

using std::size_t;

namespace {
static char PARSER_KEY[16384];
static char PARSER_VALUE[16384];
}  // namespace

// External variables and functions required by the EVM parser
extern "C" {

    // Network support variables
    const uint64_t supported_networks_evm[1] = {1};
    const uint8_t supported_networks_evm_len = 1;

    // Token support variables
    const uint8_t supportedTokensSize = 1;
    const erc20_tokens_t supportedTokens[] = {
        {{0x60, 0xE1, 0x77, 0x36, 0x36, 0xCF, 0x5E, 0x4A, 0x22, 0x7d,
          0x9A, 0xC2, 0x4F, 0x20, 0xfE, 0xca, 0x03, 0x4e, 0xe2, 0x5A},
         "WFIL ",
         18},
    };
    
    // Mock implementation for app_mode functions
    void app_mode_skip_blindsign_ui() {
        // Empty mock implementation for fuzzing
    }
    
    bool app_mode_blindsign() {
        // Return true to allow blindsigning in fuzzing
        return true;
    }
    
    parser_evm_error_t getNumItemsEthAppSpecific(eth_tx_t * /*ethTxObj*/, uint8_t *num_items) {
        *num_items = 1;  // Return at least 1 item for fuzzing
        return parser_evm_ok;
    }
    
    parser_evm_error_t printGenericAppSpecific(const parser_evm_context_t * /*ctx*/, const eth_tx_t * /*ethTxObj*/,
                                               uint8_t /*displayIdx*/, char *outKey, uint16_t outKeyLen,
                                               char *outVal, uint16_t outValLen,
                                               uint8_t /*pageIdx*/, uint8_t *pageCount) {
        if (outKey && outKeyLen > 0) {
            (void)snprintf(outKey, outKeyLen, "Key");
        }
        if (outVal && outValLen > 0) {
            (void)snprintf(outVal, outValLen, "Value");
        }
        if (pageCount) {
            *pageCount = 1;
        }
        return parser_evm_ok;
    }
    
    parser_evm_error_t printERC20TransferAppSpecific(const parser_evm_context_t * /*ctx*/, const eth_tx_t * /*ethTxObj*/,
                                                    uint8_t /*displayIdx*/, char *outKey, uint16_t outKeyLen,
                                                    char *outVal, uint16_t outValLen,
                                                    uint8_t /*pageIdx*/, uint8_t *pageCount) {
        if (outKey && outKeyLen > 0) {
            (void)snprintf(outKey, outKeyLen, "ERC20");
        }
        if (outVal && outValLen > 0) {
            (void)snprintf(outVal, outValLen, "Transfer");
        }
        if (pageCount) {
            *pageCount = 1;
        }
        return parser_evm_ok;
    }
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    parser_evm_context_t ctx;
    parser_evm_error_t rc;

    char buffer[10000];
    array_to_hexstr(buffer, sizeof(buffer), data, size);
    // fprintf(stderr, "input blob: %s\n", buffer);

    rc = parser_parse_eth(&ctx, data, size);

    if (rc != parser_evm_ok) {
        // fprintf(stderr, "parser error: %s\n", parser_getEvmErrorDescription(rc));
        return 0;
    }

    /* assert(size <= UINT16_MAX && "too big!"); */

    rc = parser_validate_eth(&ctx);
    if (rc != parser_evm_ok) {
        // fprintf(stderr, "validation error: %s\n", parser_getEvmErrorDescription(rc));
        return 0;
    }

    uint8_t num_items;
    rc = parser_getNumItemsEth(&ctx, &num_items);
    if (rc != parser_evm_ok) {
        (void)fprintf(stderr, "error in parser_getNumItems: %s\n", parser_getEvmErrorDescription(rc));
        assert(false);
    }

    for (uint8_t i = 0; i < num_items; i += 1) {
        uint8_t page_idx = 0;
        uint8_t page_count = 1;
        while (page_idx < page_count) {
            rc = parser_getItemEth(&ctx, i, PARSER_KEY, sizeof(PARSER_KEY), PARSER_VALUE, sizeof(PARSER_VALUE),
                                   page_idx, &page_count);
            if (rc != parser_evm_ok) {
                assert(fprintf(stderr, "error getting item %u at page index %u: %s\n", (unsigned)i, (unsigned)page_idx,
                               parser_getEvmErrorDescription(rc)) != 0);
                assert(false);
            }

            page_idx += 1;
        }
    }

    return 0;
}
