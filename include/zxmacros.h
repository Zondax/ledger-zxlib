// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
#pragma once

#include <memory.h>
#define __INLINE inline __attribute__((always_inline)) static

#define NVCONST

#define LOG(str)
#define LOGSTACK()

#ifdef LEDGER_SPECIFIC
#include "os.h"
#include "cx.h"

void debug_printf(void* buffer);

#undef LOG
#undef LOGSTACK
#define LOG(str) debug_printf(str)

extern unsigned int app_stack_canary;
void __logstack();

#define LOGSTACK() __logstack()
#endif

__INLINE void nvcpy(NVCONST void *dst, void const *src, uint16_t n)
{
#ifdef LEDGER_SPECIFIC
    nvm_write((void*)dst, (void*)src, n);
#else
    memcpy(dst, src, n);
#endif
}
__INLINE void nvset(NVCONST void *dst, uint32_t val)
{
#ifdef LEDGER_SPECIFIC
    uint32_t tmp=val;
    nvm_write((void*) dst, (void *) &tmp, 4);
#else
    *((uint32_t*)dst) = val;
#endif
}

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define __SWAP(v) (((v) & 0x000000FFu) << 24u | ((v) & 0x0000FF00u) << 8u | ((v) & 0x00FF0000u) >> 8u | ((v) & 0xFF000000u) >> 24u)
#define HtoNL(v) __SWAP( v )
#define NtoHL(v) __SWAP( v )
#else
#define HtoNL(x) (x)
#define NtoHL(x) (x)
#endif
