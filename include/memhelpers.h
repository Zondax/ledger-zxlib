/*******************************************************************************
*   (c) 2019 ZondaX GmbH
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

#if defined(TARGET_NANOX) || defined (TARGET_NANOS)
#include "os.h"
#define MEMMOVE os_memmove
#define MEMCPY os_memcpy
#define MEMCPY_NV nvm_write
#else
#include <string.h>
#define MEMMOVE memmove
#define MEMCPY memcpy
#define MEMCPY_NV memcpy
#endif
