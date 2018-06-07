/*******************************************************************************
*   (c) 2018 ZondaX GmbH
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
#include "zxmacros.h"

#ifdef LEDGER_SPECIFIC
typedef uint16_t BUFFER_IDX_TYPE;
#else
typedef size_t BUFFER_IDX_TYPE;
#endif

typedef struct {
  uint8_t* data;
  BUFFER_IDX_TYPE size;
  BUFFER_IDX_TYPE pos_r;
  BUFFER_IDX_TYPE pos_w;
} buffer_state_t;

__INLINE BUFFER_IDX_TYPE CLIP(BUFFER_IDX_TYPE min, BUFFER_IDX_TYPE val, BUFFER_IDX_TYPE max)
{
    if (val<min) return min;
    if (val>max) return max;
    return val;
}

__INLINE bool eof(const buffer_state_t* buffer_state)
{
    return buffer_state->pos_r==buffer_state->pos_w;
}

__INLINE bool full(const buffer_state_t* buffer_state)
{
    return buffer_state->pos_w==buffer_state->size;
}

__INLINE BUFFER_IDX_TYPE space_left(const buffer_state_t* buffer_state)
{
    return (buffer_state->pos_w>buffer_state->size) ?
           (BUFFER_IDX_TYPE) 0 :
           buffer_state->size-buffer_state->pos_w;
}

__INLINE void resetBuffet(buffer_state_t* buffer_state)
{
    buffer_state->pos_r = 0;
    buffer_state->pos_w = 0;
}

__INLINE void set_pos_r(buffer_state_t* buffer_state, BUFFER_IDX_TYPE new_pos_r)
{
    buffer_state->pos_r = CLIP(0, new_pos_r, buffer_state->pos_w);
}

__INLINE void set_pos_w(buffer_state_t* buffer_state, BUFFER_IDX_TYPE new_pos_w)
{
    buffer_state->pos_w = CLIP(0, new_pos_w, buffer_state->size);
    buffer_state->pos_r = CLIP(0, buffer_state->pos_r, buffer_state->pos_w);
}

__INLINE void initBuffer(buffer_state_t* buffer_state, uint8_t* buffer, BUFFER_IDX_TYPE buffer_size)
{
    buffer_state->data = buffer;
    buffer_state->size = buffer_size;
    resetBuffet(buffer_state);
}

__INLINE BUFFER_IDX_TYPE getChunk(buffer_state_t* buffer_state, uint8_t* chunk, BUFFER_IDX_TYPE max_chunk_size)
{
    const BUFFER_IDX_TYPE src_n = CLIP(0, buffer_state->pos_w-buffer_state->pos_r, max_chunk_size);
    const uint8_t* src_p = buffer_state->data+buffer_state->pos_r;

    memcpy((void*) chunk, src_p, src_n);
    buffer_state->pos_r += src_n;

    return src_n;
}

__INLINE BUFFER_IDX_TYPE putChunk(buffer_state_t* buffer_state, const uint8_t* chunk, BUFFER_IDX_TYPE chunk_size)
{
    const BUFFER_IDX_TYPE dst_n = CLIP(0, chunk_size, buffer_state->size-buffer_state->pos_w);
    uint8_t* const dst_p = buffer_state->data+buffer_state->pos_w;
    memcpy(dst_p, chunk, dst_n);
    buffer_state->pos_w += dst_n;

    return dst_n;
}
