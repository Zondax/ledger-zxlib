/* Copyright (c) 2017 Pieter Wuille
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "segwit_addr.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

uint32_t bech32_polymod_step(uint32_t pre) {
    uint8_t b = pre >> 25u;
    return ((pre & 0x1FFFFFFu) << 5u) ^ (-((b >> 0u) & 1u) & 0x3b6a57b2UL) ^ (-((b >> 1u) & 1u) & 0x26508e6dUL) ^
           (-((b >> 2u) & 1u) & 0x1ea119faUL) ^ (-((b >> 3u) & 1u) & 0x3d4233ddUL) ^ (-((b >> 4u) & 1u) & 0x2a1462b3UL);
}

static uint32_t bech32_final_constant(bech32_encoding enc) {
    if (enc == BECH32_ENCODING_BECH32) return 1;
    if (enc == BECH32_ENCODING_BECH32M) return 0x2bc830a3;
    return 0;
}

static const char *charset = "qpzry9x8gf2tvdw0s3jn54khce6mua7l";

static const int8_t charset_rev[128] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 15, -1, 10, 17,
    21, 20, 26, 30, 7,  5,  -1, -1, -1, -1, -1, -1, -1, 29, -1, 24, 13, 25, 9,  8,  23, -1, 18, 22, 31, 27,
    19, -1, 1,  0,  3,  16, 11, 28, 12, 14, 6,  4,  2,  -1, -1, -1, -1, -1, -1, 29, -1, 24, 13, 25, 9,  8,
    23, -1, 18, 22, 31, 27, 19, -1, 1,  0,  3,  16, 11, 28, 12, 14, 6,  4,  2,  -1, -1, -1, -1, -1};

int bech32_encode(char *output, const char *hrp, const uint8_t *data, size_t data_len, bech32_encoding enc) {
    uint32_t chk = 1;
    size_t i = 0;
    while (hrp[i] != 0) {
        char ch = hrp[i];
        if (ch < 33 || ch > 126) {
            return 0;
        }

        if (ch >= 'A' && ch <= 'Z') return 0;
        chk = bech32_polymod_step(chk) ^ (ch >> 5u);
        ++i;
    }
    if (i + 7 + data_len > 90) return 0;
    chk = bech32_polymod_step(chk);
    while (*hrp != 0) {
        chk = bech32_polymod_step(chk) ^ (*hrp & 0x1fu);
        *(output++) = *(hrp++);
    }
    *(output++) = '1';
    for (i = 0; i < data_len; ++i) {
        if (*data >> 5u) return 0;
        chk = bech32_polymod_step(chk) ^ (*data);
        *(output++) = charset[*(data++)];
    }
    for (i = 0; i < 6; ++i) {
        chk = bech32_polymod_step(chk);
    }
    chk ^= bech32_final_constant(enc);
    for (i = 0; i < 6; ++i) {
        *(output++) = charset[(chk >> ((5u - i) * 5u)) & 0x1fu];
    }
    *output = 0;
    return 1;
}

bech32_encoding bech32_decode(char *hrp, uint8_t *data, size_t *data_len, const char *input) {
    uint32_t chk = 1;
    size_t i;
    size_t input_len = strlen(input);
    size_t hrp_len;
    int have_lower = 0, have_upper = 0;
    if (input_len < 8 || input_len > 90) {
        return 0;
    }
    *data_len = 0;
    while (*data_len < input_len && input[(input_len - 1) - *data_len] != '1') {
        ++(*data_len);
    }
    hrp_len = input_len - (1 + *data_len);
    if (1 + *data_len >= input_len || *data_len < 6) {
        return 0;
    }
    *(data_len) -= 6;
    for (i = 0; i < hrp_len; ++i) {
        char ch = input[i];
        if (ch < 33 || ch > 126) {
            return 0;
        }
        if (ch >= 'a' && ch <= 'z') {
            have_lower = 1;
        } else if (ch >= 'A' && ch <= 'Z') {
            have_upper = 1;
            ch = (ch - 'A') + 'a';
        }
        hrp[i] = ch;
        chk = bech32_polymod_step(chk) ^ (ch >> 5u);
    }
    hrp[i] = 0;
    chk = bech32_polymod_step(chk);
    for (i = 0; i < hrp_len; ++i) {
        chk = bech32_polymod_step(chk) ^ (input[i] & 0x1fu);
    }
    ++i;
    while (i < input_len) {
        int v = (input[i] & 0x80u) ? -1 : charset_rev[(int)input[i]];
        if (input[i] >= 'a' && input[i] <= 'z') have_lower = 1;
        if (input[i] >= 'A' && input[i] <= 'Z') have_upper = 1;
        if (v == -1) {
            return 0;
        }
        chk = bech32_polymod_step(chk) ^ v;
        if (i + 6 < input_len) {
            data[i - (1 + hrp_len)] = v;
        }
        ++i;
    }
    if (have_lower && have_upper) {
        return BECH32_ENCODING_NONE;
    }
    if (chk == bech32_final_constant(BECH32_ENCODING_BECH32)) {
        return BECH32_ENCODING_BECH32;
    } else if (chk == bech32_final_constant(BECH32_ENCODING_BECH32M)) {
        return BECH32_ENCODING_BECH32M;
    } else {
        return BECH32_ENCODING_NONE;
    }
}

int convert_bits(uint8_t *out, size_t *outlen, int outBits, const uint8_t *in, size_t inLen, int inBits, int pad) {
    uint32_t val = 0;
    int bits = 0;
    uint32_t maxv = (((uint32_t)1u) << outBits) - 1u;
    while (inLen--) {
        val = (val << inBits) | *(in++);
        bits += inBits;
        while (bits >= outBits) {
            bits -= outBits;
            out[(*outlen)++] = (val >> bits) & maxv;
        }
    }
    if (pad) {
        if (bits) {
            out[(*outlen)++] = (val << (outBits - bits)) & maxv;
        }
    } else if (((val << (outBits - bits)) & maxv) || bits >= inBits) {
        return 0;
    }
    return 1;
}

int segwit_addr_encode(char *output, const char *hrp, int witver, const uint8_t *witprog, size_t witprog_len) {
    uint8_t data[65];
    size_t datalen = 0;
    bech32_encoding enc = BECH32_ENCODING_BECH32;
    if (witver > 16) return 0;
    if (witver == 0 && witprog_len != 20 && witprog_len != 32) return 0;
    if (witprog_len < 2 || witprog_len > 40) return 0;
    if (witver > 0) enc = BECH32_ENCODING_BECH32M;
    data[0] = witver;
    convert_bits(data + 1, &datalen, 5, witprog, witprog_len, 8, 1);
    ++datalen;
    return bech32_encode(output, hrp, data, datalen, enc);
}

int segwit_addr_decode(int *witver, uint8_t *witdata, size_t *witdata_len, const char *hrp, const char *addr) {
    uint8_t data[84];
    char hrp_actual[84];
    size_t data_len;
    bech32_encoding enc = bech32_decode(hrp_actual, data, &data_len, addr);
    if (enc == BECH32_ENCODING_NONE) return 0;
    if (data_len == 0 || data_len > 65) return 0;
    if (strncmp(hrp, hrp_actual, 84) != 0) return 0;
    if (data[0] > 16) return 0;
    if (data[0] == 0 && enc != BECH32_ENCODING_BECH32) return 0;
    if (data[0] > 0 && enc != BECH32_ENCODING_BECH32M) return 0;
    *witdata_len = 0;
    if (!convert_bits(witdata, witdata_len, 8, data + 1, data_len - 1, 5, 0)) return 0;
    if (*witdata_len < 2 || *witdata_len > 40) return 0;
    if (data[0] == 0 && *witdata_len != 20 && *witdata_len != 32) return 0;
    *witver = data[0];
    return 1;
}
