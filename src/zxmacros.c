// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#include "../../../ledger-zxlib/include/zxmacros.h"

void __logstack()
{
    uint8_t st;
    uint32_t tmp1 = (uint32_t)&st - (uint32_t)&app_stack_canary;
    uint32_t tmp2 = 0x20002800 - (uint32_t)&st;
    char buffer[30];
    snprintf(buffer, 40, "%d / %d", tmp1, tmp2);
    LOG(buffer);
}
