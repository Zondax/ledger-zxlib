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
#include <buffering.h>
#include "buffering.h"

namespace {
TEST(BUFFERING, init)
{
    uint8_t buffer[250];
    buffer_state_t buffer_state;

    initBuffer(&buffer_state, buffer, sizeof(buffer));

    EXPECT_EQ(buffer_state.pos_r, 0);
    EXPECT_EQ(buffer_state.pos_w, 0);
    EXPECT_EQ(buffer_state.size, 250);

    std::vector<uint8_t> chunk(20);
    auto bytes_read = getChunk(&buffer_state, chunk.data(), (uint16_t) chunk.size());

    EXPECT_EQ(0, bytes_read);
}

TEST(BUFFERING, put_chunks_4)
{
    std::vector<uint8_t> buffer(40);

    buffer_state_t buffer_state;
    initBuffer(&buffer_state, buffer.data(), buffer.size());

    ///////////
    std::vector<uint8_t> chunk(30);
    for (uint8_t i = 0; i<28; i++)
        chunk[i+2] = i;

    //////////////
    // Put chunk 1
    EXPECT_EQ(30, putChunk(&buffer_state, chunk.data(), chunk.size()));
    EXPECT_EQ(0, buffer_state.pos_r);
    EXPECT_EQ(30, buffer_state.pos_w);
    EXPECT_FALSE(eof(&buffer_state));
    EXPECT_FALSE(full(&buffer_state));

    //////////////
    // Put chunk 2
    EXPECT_EQ(5, putChunk(&buffer_state, chunk.data(), 5));
    EXPECT_EQ(0, buffer_state.pos_r);
    EXPECT_EQ(35, buffer_state.pos_w);
    EXPECT_FALSE(eof(&buffer_state));
    EXPECT_FALSE(full(&buffer_state));

    //////////////
    // Put chunk 3
    EXPECT_EQ(5, putChunk(&buffer_state, chunk.data(), 5));
    EXPECT_EQ(0, buffer_state.pos_r);
    EXPECT_EQ(40, buffer_state.pos_w);
    EXPECT_FALSE(eof(&buffer_state));
    EXPECT_TRUE(full(&buffer_state));

    //////////////
    // Put chunk 4
    EXPECT_EQ(0, putChunk(&buffer_state, chunk.data(), 10));
    EXPECT_EQ(0, buffer_state.pos_r);
    EXPECT_EQ(40, buffer_state.pos_w);
    EXPECT_FALSE(eof(&buffer_state));
    EXPECT_TRUE(full(&buffer_state));

}

TEST(BUFFERING, get_chunks_2)
{
    std::vector<uint8_t> buffer(40);

    buffer_state_t buffer_state;
    initBuffer(&buffer_state, buffer.data(), (uint16_t) buffer.size());
    set_pos_w(&buffer_state, 50);

    EXPECT_EQ(buffer_state.size, buffer_state.pos_w);
    EXPECT_EQ(buffer.size(), buffer_state.size);

    std::vector<uint8_t> chunk(25);
    auto bytes_read = getChunk(&buffer_state, chunk.data(), (int16_t) chunk.size());
    EXPECT_EQ(chunk.size(), bytes_read);

    bytes_read = getChunk(&buffer_state, chunk.data(), (int16_t) chunk.size());
    EXPECT_EQ(15, bytes_read);
}

}
