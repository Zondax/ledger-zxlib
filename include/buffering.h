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

typedef struct {
  struct {
    uint8_t last_chunk: 1;
  } flags;
  uint8_t data_size;
} chunk_header_t;

typedef struct {
  uint8_t* data;
  uint16_t size;
  uint16_t pos;
} buffer_state_t;

__INLINE bool lastChunk(const uint8_t* chunk)
{
    chunk_header_t* header = (chunk_header_t*) chunk;
    return header->flags.last_chunk;
}

__INLINE bool eofBuffer(const buffer_state_t* buffer_state)
{
    return buffer_state->pos==buffer_state->size;
}

__INLINE void resetBuffet(buffer_state_t* buffer_state)
{
    buffer_state->pos = 0;
}

__INLINE void initBuffer(buffer_state_t* buffer_state, uint8_t* buffer, uint16_t buffer_size)
{
    buffer_state->data = buffer;
    buffer_state->size = buffer_size;
    resetBuffet(buffer_state);
}

__INLINE bool getChunk(buffer_state_t* buffer_state, uint8_t* chunk, int16_t max_chunk_size)
{
    max_chunk_size -= sizeof(chunk_header_t);
    const uint16_t data_left = buffer_state->size-buffer_state->pos;
    const uint16_t n = data_left<=max_chunk_size ? data_left : max_chunk_size;

    chunk_header_t* header = (chunk_header_t*) chunk;
    header->data_size = n;
    chunk += sizeof(chunk_header_t);

    const uint8_t* p = buffer_state->data+buffer_state->pos;
    memcpy(chunk, p, n);
    buffer_state->pos += n;

    // return if this is the last chunk
    header->flags.last_chunk = eofBuffer(buffer_state);
    return header->flags.last_chunk;
}

__INLINE bool putChunk(buffer_state_t* buffer_state, const uint8_t* chunk)
{
    chunk_header_t* header = (chunk_header_t*) chunk;
    const uint16_t data_left = buffer_state->size - buffer_state->pos;
    const uint16_t n = data_left<=header->data_size ? data_left : header->data_size;

    uint8_t* const p = buffer_state->data+buffer_state->pos;
    memcpy(p, chunk, n);
    buffer_state->pos += n;

    // return true if it is the last chunk or the buffer is full
    return lastChunk(chunk) || eofBuffer(buffer_state);
}
