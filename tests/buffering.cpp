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
#include <gmock/gmock.h>
#include "buffering.h"

namespace {
TEST(BUFFERING, init)
{
    uint8_t buffer[250];
    buffer_state_t buffer_state;

    initBuffer(&buffer_state, buffer, sizeof(buffer));

    EXPECT_EQ(buffer_state.pos, 0);
    EXPECT_EQ(buffer_state.size, 250);
}

TEST(BUFFERING, get_3)
{
    const uint16_t buffer_size = 40;
    std::vector<uint8_t> buffer(buffer_size);
    buffer_state_t buffer_state;

    for(uint8_t i=0; i<buffer_size; i++)
        buffer[i] = i;

    initBuffer(&buffer_state, buffer.data(), buffer_size);

    std::vector<uint8_t> chunk(20);
    getChunk(&buffer_state, chunk.data(), (int16_t)chunk.size());

    //////////////
    // Get chunk 1
    auto header = reinterpret_cast<chunk_header_t*>(chunk.data());
    EXPECT_EQ(header->data_size, 18);
    EXPECT_FALSE(lastChunk(chunk.data()));
    EXPECT_EQ(buffer_state.pos, 18);
    for(uint8_t i=0; i<header->data_size; i++)
        EXPECT_EQ(chunk[sizeof(chunk_header_t)+i], i);

    //////////////
    // Get chunk 2
    getChunk(&buffer_state, chunk.data(), (int16_t)chunk.size());

    header = reinterpret_cast<chunk_header_t*>(chunk.data());
    EXPECT_EQ(header->data_size, 18);
    EXPECT_FALSE(lastChunk(chunk.data()));
    EXPECT_EQ(buffer_state.pos, 36);
    for(uint8_t i=0; i<header->data_size; i++)
        EXPECT_EQ(chunk[sizeof(chunk_header_t)+i], i+18);

    //////////////
    // Get chunk 3
    getChunk(&buffer_state, chunk.data(), (int16_t)chunk.size());

    header = reinterpret_cast<chunk_header_t*>(chunk.data());
    EXPECT_EQ(header->data_size, 4);
    EXPECT_TRUE(lastChunk(chunk.data()));
    EXPECT_EQ(buffer_state.pos, buffer_size);
    for(uint8_t i=0; i<header->data_size; i++)
        EXPECT_EQ(chunk[sizeof(chunk_header_t)+i], i+36);
}

TEST(BUFFERING, put_2)
{
    const uint16_t buffer_size = 40;
    std::vector<uint8_t> buffer(buffer_size);
    buffer_state_t buffer_state;
    initBuffer(&buffer_state, buffer.data(), buffer_size);

    ///////////
    std::vector<uint8_t> chunk(30);
    for(uint8_t i=0; i<28; i++)
        chunk[i+2] = i;
    auto header = reinterpret_cast<chunk_header_t*>(chunk.data());
    header->flags.last_chunk=0;
    header->data_size = 28;

    //////////////
    // Put chunk 1
    EXPECT_FALSE(putChunk(&buffer_state, chunk.data()));
    EXPECT_EQ(buffer_state.pos, 28);

    //////////////
    // Put chunk 2
    header->flags.last_chunk=1;
    header->data_size = 5;
    EXPECT_TRUE(putChunk(&buffer_state, chunk.data()));
    EXPECT_EQ(buffer_state.pos, 33);
}

TEST(BUFFERING, put_2_overflow)
{
    const uint16_t buffer_size = 40;
    std::vector<uint8_t> buffer(buffer_size);
    buffer_state_t buffer_state;
    initBuffer(&buffer_state, buffer.data(), buffer_size);

    ///////////
    std::vector<uint8_t> chunk(30);
    for(uint8_t i=0; i<28; i++)
        chunk[i+2] = i;
    auto header = reinterpret_cast<chunk_header_t*>(chunk.data());
    header->flags.last_chunk=0;
    header->data_size = 28;

    //////////////
    // Put chunk 1
    EXPECT_FALSE(putChunk(&buffer_state, chunk.data()));
    EXPECT_EQ(buffer_state.pos, 28);

    //////////////
    // Put chunk 2
    header->flags.last_chunk=0;
    header->data_size = 28;
    EXPECT_TRUE(putChunk(&buffer_state, chunk.data()));
    EXPECT_EQ(buffer_state.pos, 40);
}

}
