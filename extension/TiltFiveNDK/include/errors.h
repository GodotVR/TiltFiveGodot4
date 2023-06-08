/*
 * Copyright (C) 2020-2022 Tilt Five, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

/// \file
/// \brief C errors for the Tilt Five™ API

#ifndef __cplusplus
#include <stdint.h>
#else
#include <cstdint>
#endif

#ifdef _WIN32
#ifdef BUILDING_T5_NATIVE_DLL
#ifdef __GNUC__
#define T5_EXPORT __attribute__((dllexport))
#else  // !__GNUC__
#define T5_EXPORT __declspec(dllexport)
#endif  // !__GNUC__
#else   // !BUILDING_T5_NATIVE_DLL
#define T5_EXPORT
#endif  // !BUILDING_T5_NATIVE_DLL
#else   // !_WIN32
#define T5_EXPORT __attribute__((visibility("default")))
#endif  // !_WIN32

#ifdef __cplusplus
extern "C" {
#endif

/// \brief Represents an error code that may be returned by the Tilt Five™ API.
typedef uint32_t T5_Result;

/// \defgroup C_Errors Tilt Five™ Error Codes (C)
/// \brief Many exported functions return a ::T5_Result, with a value as defined in this list.
/// \{

/// Success
#define T5_SUCCESS (0x0000)

/// Timeout
#define T5_TIMEOUT (0x0001)

/// \brief No context.
///
/// Some functions require either a ::T5_Context or a ::T5_Glasses.
/// This error is returned if an invalid object is passed.
#define T5_ERROR_NO_CONTEXT (0x1000)

/// No library loaded
#define T5_ERROR_NO_LIBRARY (0x1001)

/// An internal error occurred
#define T5_ERROR_INTERNAL (0x1002)

/// Service isn't connected
#define T5_ERROR_NO_SERVICE (0x1003)

/// Misc IO failure
#define T5_ERROR_IO_FAILURE (0x1004)

/// Service doesn't understand the request
#define T5_ERROR_REQUEST_ID_UNKNOWN (0x1005)

/// Argument(s) are invalid
#define T5_ERROR_INVALID_ARGS (0x1006)

/// Device lost
#define T5_ERROR_DEVICE_LOST (0x1007)

/// Target (wand) not found
#define T5_ERROR_TARGET_NOT_FOUND (0x1008)

/// Incorrect state for the request
#define T5_ERROR_INVALID_STATE (0x1009)

/// The requested param is unknown
#define T5_ERROR_SETTING_UNKNOWN (0x100A)

/// The requested param has a different type to the requested type
#define T5_ERROR_SETTING_WRONG_TYPE (0x100B)

/// Miscellaneous remote error
#define T5_ERROR_MISC_REMOTE (0x100C)

/// Buffer overflow
#define T5_ERROR_OVERFLOW (0x100D)

/// Specified graphics API is unavailable
#define T5_ERROR_GRAPHICS_API_UNAVAILABLE (0x100E)

/// Action is unsupported
#define T5_ERROR_UNSUPPORTED (0x100F)

/// Failed to decode
#define T5_ERROR_DECODE_ERROR (0x1010)

/// Graphics context is invalid
#define T5_ERROR_INVALID_GFX_CONTEXT (0x1011)

/// Failed to initialize graphics context
#define T5_ERROR_GFX_CONTEXT_INIT_FAIL (0x1012)

/// Target is not currently available
#define T5_ERROR_TRY_AGAIN (0x1015)

/// Target is unavailable
#define T5_ERROR_UNAVAILABLE (0x1016)

/// The target is already connected
#define T5_ERROR_ALREADY_CONNECTED (0x1017)

/// The target is not connected
#define T5_ERROR_NOT_CONNECTED (0x1018)

/// Overflow during string conversion operation
#define T5_ERROR_STRING_OVERFLOW (0x1019)

/// Service incompatible
#define T5_ERROR_SERVICE_INCOMPATIBLE (0x101A)

/// Permission denied
#define T5_PERMISSION_DENIED (0x101B)

/// Invalid Buffer Size
#define T5_ERROR_INVALID_BUFFER_SIZE (0x101C)

/// Maximum C error code
#define T5_MAX_ERROR (0x8000)

/// Get a human readable error message
///
/// \param[in] result - A ::T5_Result
T5_EXPORT const char* t5GetResultMessage(T5_Result result);

/// \}

#ifdef __cplusplus
}  // extern "C"
#endif

#undef T5_EXPORT
