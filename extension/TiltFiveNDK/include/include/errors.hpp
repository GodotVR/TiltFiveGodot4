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
/// \brief C++ errors for the Tilt Five™ API

#include "errors.h"

#include <system_error>

namespace tiltfive {

///////////////////////////////////////////////////////////////////////////////
//                                   Error                                   //
///////////////////////////////////////////////////////////////////////////////

/// \defgroup Cpp_Errors Tilt Five™ Error Codes (C++)
/// \brief Common error codes for all C++ functions
/// \{

/// Error codes returned by most functions of return type Result
enum class Error {
    // --------- C Errors ---------
    /// Success
    kSuccess = T5_SUCCESS,

    /// Timeout
    kTimeout = T5_TIMEOUT,

    /// No context
    //
    /// Some functions require either a T5_Context or a T5_Glasses.
    /// This error is returned if an invalid object is passed.
    kNoContext = T5_ERROR_NO_CONTEXT,

    /// Library Unavailable
    kLibraryUnavailable = T5_ERROR_NO_LIBRARY,

    /// An internal error occurred
    kInternalError = T5_ERROR_INTERNAL,

    /// Service isn't connected
    kNoService = T5_ERROR_NO_SERVICE,

    /// Misc IO failure
    kIoFailure = T5_ERROR_IO_FAILURE,

    /// Service doesn't understand the request
    kRequestIdUnknown = T5_ERROR_REQUEST_ID_UNKNOWN,

    /// Argument(s) are invalid
    kInvalidArgument = T5_ERROR_INVALID_ARGS,

    /// Device lost
    kDeviceLost = T5_ERROR_DEVICE_LOST,

    /// Target (wand) not found
    kTargetNotFound = T5_ERROR_TARGET_NOT_FOUND,

    /// Incorrect state for the request
    kInvalidState = T5_ERROR_INVALID_STATE,

    /// The requested param is unknown
    kSettingUnknown = T5_ERROR_SETTING_UNKNOWN,

    /// The requested param has a different type to the requested type
    kSettingWrongType = T5_ERROR_SETTING_WRONG_TYPE,

    /// Miscellaneous remote error
    kMiscRemote = T5_ERROR_MISC_REMOTE,

    /// Buffer overflow
    kOverflow = T5_ERROR_OVERFLOW,

    /// Specified graphics API is unavailable
    kGfxApiUnavailable = T5_ERROR_GRAPHICS_API_UNAVAILABLE,

    /// Action is unsupported
    kUnsupported = T5_ERROR_UNSUPPORTED,

    /// Failed to decode
    kDecodeError = T5_ERROR_DECODE_ERROR,

    /// Graphics context is invalid
    kInvalidGfxContext = T5_ERROR_INVALID_GFX_CONTEXT,

    /// Failed to initialize graphics context
    kGfxContextInitFail = T5_ERROR_GFX_CONTEXT_INIT_FAIL,

    /// Target is temporarily unavailable
    kTryAgain = T5_ERROR_TRY_AGAIN,

    /// Target is unavailable
    kUnavailable = T5_ERROR_UNAVAILABLE,

    /// Target is already connected
    kAlreadyConnected = T5_ERROR_ALREADY_CONNECTED,

    /// Target is not connected
    kNotConnected = T5_ERROR_NOT_CONNECTED,

    /// String overflow
    kStringOverflow = T5_ERROR_STRING_OVERFLOW,

    /// Service incompatible
    kServiceIncompatible = T5_ERROR_SERVICE_INCOMPATIBLE,

    /// Permission denied
    kPermissionDenied = T5_PERMISSION_DENIED,

    /// Invalid buffer size
    kInvalidBuffer = T5_ERROR_INVALID_BUFFER_SIZE,
};

/// \}

namespace details {

// Work around the lack of inline variable support.
template <typename Dummy>
struct ErrorCategory : std::error_category {
    [[nodiscard]] auto name() const noexcept -> const char* override {
        return "Tilt Five Error";
    }

    [[nodiscard]] auto message(int ev) const -> std::string override {
        return ::t5GetResultMessage(static_cast<T5_Result>(ev));
    }

    static const ErrorCategory kSingleton;
};

template <typename Dummy>
const ErrorCategory<Dummy> ErrorCategory<Dummy>::kSingleton;

}  // namespace details

using ErrorCategory = details::ErrorCategory<void>;

inline std::error_code make_error_code(Error e) noexcept {
    return {static_cast<int>(e), ErrorCategory::kSingleton};
}

}  // namespace tiltfive

namespace std {

template <>
struct is_error_code_enum<tiltfive::Error> : true_type {};

}  // namespace std
