/*
 * Copyright (C) 2020-2023 Tilt Five, Inc.
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
/// \brief C interface definition for the Tilt Five™ API

#include "errors.h"
#include "types.h"

#ifndef __cplusplus
#include <stddef.h>
#else
#include <cstddef>
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

//////////////////////////////////////////////////////////
////                Glasses Interface                 ////
//////////////////////////////////////////////////////////

/// \defgroup C_Glasses_Interface Tilt Five™ Native Interface (C)
/// Functions for managing glasses and wands
/// \{

/// \defgroup C_Ctx Context object management
/// \{

/// \brief Create a context object
///
/// Internally, this starts a client connection to the Tilt Five™ service, which is
/// required for other library functions.
///
/// This function can succeed even if it can't connect to the service, but will
/// continue to attempt to connect. If the service connection goes away at any time,
/// the client will reconnect automatically when it returns.
///
/// Calling t5CreateContext() multiple times is valid, but likely unnecessary. It
/// will result in multiple separate connections to the service. Note that you
/// _do not_ need multiple separate connections to connect to multiple glasses.
///
/// Contexts should be destroyed with t5DestroyContext() after a client is done.
/// Failure to do so will keep the client connected and leak memory.
///
/// Some platforms may require additional parameters to create the context.
/// For example, Android uses JNI, which requires information about the JVM
/// in the form of a NativePlatformContext passed in from the Java layer.
///
/// \attention Ensure that the lifetimes of the ::T5_ClientInfo members remain valid
/// for the duration of the context.
///
/// \par Threading
/// Exclusivity group 1 - Functions in this group must not be called concurrently from different
/// threads.
///
/// \param[out] context         - Pointer to a ::T5_Context to be created
/// \param[in]  clientInfo      - ::T5_ClientInfo filled by client to detail client information.
/// \param[in]  platformContext - Platform specific context.
///                                 Platform | Value
///                                 ---------|------------------------------------------
///                                 Android  | Pointer to a jobject/NativePlatformContext
///                                 Linux    | Should be set to 0
///                                 Windows  | Should be set to 0
///
/// \retval ::T5_SUCCESS               Context object was written to `context`
/// \retval ::T5_ERROR_INVALID_ARGS    Nullptr was supplied for `clientInfo`.
/// \retval ::T5_ERROR_STRING_OVERFLOW One or more of the provided strings is too long.
T5_EXPORT T5_Result t5CreateContext(T5_Context* context,
                                    const T5_ClientInfo* clientInfo,
                                    void* platformContext);

/// \brief Destroy a context object
///
/// This function destroys a ::T5_Context context object created with
/// t5CreateContext() and closes it's connection to the Tilt Five™ service.
///
/// \par Threading
/// Exclusivity group 1 - Functions in this group must not be called concurrently from different
/// threads.
///
/// Attempting to use a ::T5_Context after it has been destroyed will result in a
/// ::T5_ERROR_NO_CONTEXT error.
///
/// \param[in, out]  context - ::T5_Context returned by t5CreateContext(). Invalidated on return.
T5_EXPORT void t5DestroyContext(T5_Context* context);

/// \}
// C_Ctx

/// \defgroup C_Sys System-wide functions
/// Require a ::T5_Context for invocation
/// \{

/// \brief Enumerate all glasses
///
/// An entry in this list does not mean that it's available for use by the client.
/// This list includes glasses that are locked for use by other clients as well as
/// glasses that are not in the 'ready' state (E.g. Rebooting).
///
/// t5CreateGlasses() can be called on any entry, even if the glasses aren't currently
/// available.
///
/// The result is encoded as a sequence of null-terminated strings, with the final
/// entry being an empty string (IE The last entry is terminated with two nulls).
///
/// \par Threading
/// Exclusivity group 1 - Functions in this group must not be called concurrently from different
/// threads.
///
/// \param[in]     context       - ::T5_Context returned by t5CreateContext().
/// \param[out]    buffer        - Buffer to receive the list of devices as a series of null
///                                terminated strings. The last element will be an empty string
///                                (I.E. the list is terminated with two null chars in sequence).
/// \param[in,out] bufferSize    - <b>On Call</b>: Size of buffer.<br/>&nbsp;
///                                <b>On Return</b>: Size of written data. Note that this may
///                                be larger than the buffer, in which case ::T5_ERROR_OVERFLOW
///                                is returned, and this value represents the size of the buffer
///                                needed to avoid overflow.
///
/// \retval ::T5_SUCCESS                     List successfully written to buffer.
/// \retval ::T5_ERROR_INVALID_ARGS          Nullptr was supplied for `buffer`.
///                                           <span class='altMeaning'>or</span>
///                                          Nullptr was supplied for `count`.
/// \retval ::T5_ERROR_OVERFLOW              Provided buffer is too small to contain glasses list.
/// \retval ::T5_ERROR_IO_FAILURE            Failed to communicate with the service.
/// \retval ::T5_ERROR_NO_SERVICE            Service is unavailable.
/// \retval ::T5_ERROR_NO_CONTEXT            `context` is invalid.
/// \retval ::T5_ERROR_INTERNAL              Internal (Not correctable): Generic error.
/// \retval ::T5_ERROR_STRING_OVERFLOW       Internal (Not correctable): String conversion overflow.
/// \retval ::T5_ERROR_SERVICE_INCOMPATIBLE  Service is incompatible; context cannot be used.
///                                          Need driver upgrade.
T5_EXPORT T5_Result t5ListGlasses(T5_Context context, char* buffer, size_t* bufferSize);

/// \brief Create a glasses access object
///
/// Use t5ListGlasses() to obtain a list of ids usable for the `id` parameter.
///
/// Destroy with t5DestroyGlasses()
///
/// \attention Ensure that the lifetime of the graphics context remains valid for the duration of
/// the glasses connection.
///
/// \par Threading
/// Exclusivity group 1 - Functions in this group must not be called concurrently from different
/// threads. The calling thread must be the thread that provided the graphics context.
///
/// \param[in]  context         - ::T5_Context returned by t5CreateContext().
/// \param[in]  id              - Null terminated C string specifying the target glasses.
/// \param[out] glasses         - ::T5_Glasses for use with glasses related functions.
///
/// \retval ::T5_SUCCESS                     Obtained handle to glasses.
/// \retval ::T5_ERROR_INVALID_ARGS          Nullptr was supplied for `id`.
///                                           <span class='altMeaning'>or</span>
///                                          Nullptr was supplied for `glasses`.
/// \retval ::T5_ERROR_NO_CONTEXT            `context` is invalid.
/// \retval ::T5_ERROR_STRING_OVERFLOW       One or more of the provided strings is too long.
/// \retval ::T5_ERROR_INTERNAL              Internal (Not correctable): Generic error.
/// \retval ::T5_ERROR_SERVICE_INCOMPATIBLE  Service is incompatible; context cannot be used.
///                                          Need driver upgrade.
T5_EXPORT T5_Result t5CreateGlasses(T5_Context context, const char* id, T5_Glasses* glasses);

/// \brief Destroy a glasses object
///
/// Destroys a ::T5_Glasses returned by t5CreateGlasses() and frees any
/// associated resources.
///
/// \par Threading
/// Exclusivity group 1 - Functions in this group must not be called concurrently from different
/// threads.
///
/// Attempting to use a ::T5_Glasses after it has been destroyed will result in a
/// ::T5_ERROR_NO_CONTEXT error.
///
/// \param[in, out]  glasses - ::T5_Glasses returned by t5CreateGlasses(). Invalidated on return.
T5_EXPORT void t5DestroyGlasses(T5_Glasses* glasses);

/// \defgroup sys_getParam System-wide parameters

/// \brief Get a system-wide integer parameter
/// \ingroup sys_getParam
///
/// See ::T5_ParamSys for a list of possible parameters to retrieve.
///
/// \par Threading
/// Exclusivity group 1 - Functions in this group must not be called concurrently from different
/// threads.
///
/// \param[in]  context - ::T5_Context returned by t5CreateContext()
/// \param[in]  param   - ::T5_ParamSys to get value for.
/// \param[out] value   - Pointer to a int64_t to be set to the current value.
///
/// \retval ::T5_SUCCESS                     Got the parameter
/// \retval ::T5_ERROR_INVALID_ARGS          `param` was not a valid enumerant
///                                          <span class='altMeaning'>or</span>
///                                          NULL was supplied for `value`
/// \retval ::T5_TIMEOUT                     Timeout waiting to read value.
/// \retval ::T5_ERROR_IO_FAILURE            Failed to communicate with the service.
/// \retval ::T5_ERROR_NO_SERVICE            Service is unavailable.
/// \retval ::T5_ERROR_NO_CONTEXT            `context` is invalid.
/// \retval ::T5_ERROR_SETTING_WRONG_TYPE    The requested parameter is not an integer value.
/// \retval ::T5_ERROR_SERVICE_INCOMPATIBLE  Service is incompatible; context cannot be used.
///                                          Need driver upgrade.
///
/// The following are internal errors that should be discarded and/or logged:
/// \retval ::T5_ERROR_SETTING_UNKNOWN    Internal (Not correctable): Setting is unknown.
/// \retval ::T5_ERROR_INTERNAL           Internal (Not correctable): Generic error.
/// \retval ::T5_ERROR_MISC_REMOTE        Internal (Not correctable): Generic service error.
/// \retval ::T5_ERROR_OVERFLOW           Internal (Not correctable): Buffer overflow.
T5_EXPORT T5_Result t5GetSystemIntegerParam(T5_Context context, T5_ParamSys param, int64_t* value);

/// \brief Get a system-wide floating point parameter
/// \ingroup sys_getParam
///
/// See ::T5_ParamSys for a list of possible parameters to retrieve.
///
/// \par Threading
/// Exclusivity group 1 - Functions in this group must not be called concurrently from different
/// threads.
///
/// \param[in]  context - ::T5_Context returned by t5CreateContext()
/// \param[in]  param   - ::T5_ParamSys to get value for.
/// \param[out] value   - Pointer to a double to be set to the current value.
///
/// \retval ::T5_SUCCESS                     Got the parameter
/// \retval ::T5_ERROR_INVALID_ARGS          `param` was not a valid enumerant
///                                          <span class='altMeaning'>or</span>
///                                          NULL was supplied for `value`
/// \retval ::T5_TIMEOUT                     Timeout waiting to read value.
/// \retval ::T5_ERROR_IO_FAILURE            Failed to communicate with the service.
/// \retval ::T5_ERROR_NO_SERVICE            Service is unavailable.
/// \retval ::T5_ERROR_NO_CONTEXT            `context` is invalid.
/// \retval ::T5_ERROR_SETTING_WRONG_TYPE    The requested parameter is not a floating point value.
/// \retval ::T5_ERROR_SERVICE_INCOMPATIBLE  Service is incompatible; context cannot be used.
///                                          Need driver upgrade.
///
/// The following are internal errors that should be discarded and/or logged:
/// \retval ::T5_ERROR_SETTING_UNKNOWN    Internal (Not correctable): Setting is unknown.
/// \retval ::T5_ERROR_INTERNAL           Internal (Not correctable): Generic error.
/// \retval ::T5_ERROR_MISC_REMOTE        Internal (Not correctable): Generic service error.
/// \retval ::T5_ERROR_OVERFLOW           Internal (Not correctable): Buffer overflow.
T5_EXPORT T5_Result t5GetSystemFloatParam(T5_Context context, T5_ParamSys param, double* value);

/// \brief Get a system-wide UTF-8 encoded string parameter
/// \ingroup sys_getParam
///
/// See ::T5_ParamSys for a list of possible parameters to retrieve.
///
/// \par Threading
/// Exclusivity group 1 - Functions in this group must not be called concurrently from different
/// threads.
///
/// \param[in]  context       - ::T5_Context returned by t5CreateContext()
/// \param[in]  param         - ::T5_ParamSys to get value for.
/// \param[out] buffer        - Pointer to a buffer into which the current value is to be
///                             written as a null-terminated string of UTF-8 code points.
/// \param[in,out] bufferSize - <b>On Call</b>: Size of the buffer pointed to by `buffer`.
///                             <br/>&nbsp;
///                             <b>On Return</b>: Size of the parameter value. Note that this
///                             may be larger than the buffer, in which case ::T5_ERROR_OVERFLOW
///                             is returned, and this value represents the size of the buffer
///                             needed to avoid overflow.
///
/// \retval ::T5_SUCCESS                     Got the parameter
/// \retval ::T5_ERROR_INVALID_ARGS          `param` was not a valid enumerant
///                                          <span class='altMeaning'>or</span>
///                                          NULL was supplied for `buffer`
///                                          <span class='altMeaning'>or</span>
///                                          NULL was supplied for `bufferSize`
/// \retval ::T5_TIMEOUT                     Timeout waiting to read value.
/// \retval ::T5_ERROR_IO_FAILURE            Failed to communicate with the service.
/// \retval ::T5_ERROR_NO_SERVICE            Service is unavailable.
/// \retval ::T5_ERROR_NO_CONTEXT            `context` is invalid.
/// \retval ::T5_ERROR_SETTING_WRONG_TYPE    The requested parameter is not a UTF-8 string value.
/// \retval ::T5_ERROR_SERVICE_INCOMPATIBLE  Service is incompatible; context cannot be used.
///                                          Need driver upgrade.
/// \retval ::T5_ERROR_OVERFLOW              The provided buffer was insufficient to store the
///                                          UTF-8 string value.
///
/// The following are internal errors that should be discarded and/or logged:
/// \retval ::T5_ERROR_SETTING_UNKNOWN    Internal (Not correctable): Setting is unknown.
/// \retval ::T5_ERROR_INTERNAL           Internal (Not correctable): Generic error.
/// \retval ::T5_ERROR_MISC_REMOTE        Internal (Not correctable): Generic service error.
T5_EXPORT T5_Result t5GetSystemUtf8Param(T5_Context context,
                                         T5_ParamSys param,
                                         char* buffer,
                                         size_t* bufferSize);

/// \brief Get a system-wide list of changed parameters
/// \ingroup sys_getParam
///
/// This function doesn't return the values of the changed parameters, but an
/// unordered list of the parameters that have changed since this function was
/// last called. Note that as a result, the first call to this function will
/// always result in a count of 0.
///
/// \par Threading
/// Exclusivity group 1 - Functions in this group must not be called concurrently from different
/// threads.
///
/// \param[in]  context  - ::T5_Context returned by t5CreateContext().
/// \param[out] buffer   - ::T5_ParamSys buffer to receive list of change parameters.
/// \param[in,out] count - <b>On Call</b>: Size of buffer in elements.<br/>&nbsp;
///                        <b>On Return</b>: Number of changed parameters in butter.
///
/// \retval ::T5_SUCCESS                     Changed parameter list written to buffer.
/// \retval ::T5_ERROR_INVALID_ARGS          Nullptr was supplied for `buffer`.
///                                           <span class='altMeaning'>or</span>
///                                          Nullptr was supplied for `count`.
/// \retval ::T5_ERROR_OVERFLOW              Buffer too small to contain parameter list.
/// \retval ::T5_ERROR_NO_CONTEXT            `context` is invalid.
/// \retval ::T5_ERROR_SERVICE_INCOMPATIBLE  Service is incompatible; context cannot be used.
///                                          Need driver upgrade.
T5_EXPORT T5_Result t5GetChangedSystemParams(T5_Context context,
                                             T5_ParamSys* buffer,
                                             uint16_t* count);

/// \brief Get the gameboard dimensions
///
/// \par Threading
/// Thread safe - may be called by any thread at any time.
///
/// \param[in]  context       - ::T5_Context returned by t5CreateContext()
/// \param[in]  gameboardType - ::T5_GameboardType we're interested in
/// \param[out] gameboardSize - Resulting gameboard dimensions
///
/// \retval ::T5_SUCCESS            Changed parameter list written to buffer.
/// \retval ::T5_ERROR_INVALID_ARGS Gameboard type invalid or nullptr supplied for
/// gameboardSize.
T5_EXPORT T5_Result t5GetGameboardSize(T5_Context context,
                                       T5_GameboardType gameboardType,
                                       T5_GameboardSize* gameboardSize);
/// \}
// C_Sys

/// \defgroup C_Gls Glasses functions
/// Require a ::T5_Glasses for invocation
/// \{

/// \defgroup glassesStateFns Glasses state management
/// \brief Functions related to the management of glasses state and exclusivity

/// \brief Reserve glasses for exclusive operations by the client
/// \ingroup glassesStateFns
///
/// Although several operations can be performed without acquiring an exclusive lock on glasses,
/// there are a few for which an exclusive lock is required. Primarily, the ability to get poses
/// (t5GetGlassesPose()) and send frames (t5SendFrameToGlasses()). To reserve glasses for
/// exclusive use, use this function.
///
/// Clients may request glasses that aren't fully available yet (e.g. a device that isn't fully
/// booted, or needs to be rebooted to be compatible with the client). That is why there's a
/// two-step approach here requiring a request for exclusive access first.  To finish preparing
/// for exclusive operations, use t5EnsureGlassesReady().
///
/// Repeated calls to this function should be made until reserve is successful or an error
/// occurs.
///
/// \par Threading
/// Exclusivity group 1 - Functions in this group must not be called concurrently from different
/// threads.
///
/// \param[in]  glasses         - ::T5_Glasses returned by t5CreateGlasses()
/// \param[in]  displayName     - Null terminated C string to display in control panel (localized)
///                               E.g. "Awesome Game (Player 1)"
///
/// \retval ::T5_SUCCESS                 Glasses reserved for this client.
/// \retval ::T5_ERROR_UNAVAILABLE       Glasses already reserved by another client.
/// \retval ::T5_ERROR_ALREADY_CONNECTED Glasses already reserved and connected for this client.
/// \retval ::T5_ERROR_DEVICE_LOST       Glasses have disconnected - destroy glasses with
///                                      t5DestroyGlasses(), re-obtain with
///                                      t5CreateGlasses() and try again.
/// \retval ::T5_ERROR_INVALID_ARGS      Nullptr or invalid input supplied for `displayName`.
/// \retval ::T5_ERROR_NO_CONTEXT        `glasses` is invalid.
/// \retval ::T5_ERROR_INTERNAL          Internal error - not correctable.
/// \retval ::T5_ERROR_STRING_OVERFLOW   `displayName` is too long.
T5_EXPORT T5_Result t5ReserveGlasses(T5_Glasses glasses, const char* displayName);

/// \brief Set the display name for glasses that were previously reserved for exclusive operations.
/// \ingroup glassesStateFns
///
/// \par Threading
/// Exclusivity group 1 - Functions in this group must not be called concurrently from different
/// threads.
///
/// \param[in]  glasses         - ::T5_Glasses returned by t5CreateGlasses()
/// \param[in]  displayName     - Null terminated C string to display in control panel (localized)
///                               E.g. "Awesome Game (Player 2)"
///
/// \retval ::T5_SUCCESS               Glasses reserved for this client.
/// \retval ::T5_ERROR_NO_CONTEXT      `glasses` is invalid.
/// \retval ::T5_ERROR_INVALID_ARGS    Nullptr or invalid input supplied for `displayName`.
/// \retval ::T5_ERROR_INVALID_STATE   Glasses weren't reserved first or client is invalid.
/// \retval ::T5_ERROR_STRING_OVERFLOW `displayName` is too long.
T5_EXPORT T5_Result t5SetGlassesDisplayName(T5_Glasses glasses, const char* displayName);

/// \brief Ensure that reserved glasses are ready for exclusive operations.
/// \ingroup glassesStateFns
///
/// Ensure that reserved glasses are ready for exclusive operations, such as the ability to get
/// poses (t5GetGlassesPose()) and send frames (t5SendFrameToGlasses()).  To reserve glasses for
/// exclusive use t5ReserveGlasses() .  This *must* be checked for success prior to exclusive
/// operations, otherwise those operations will fail.
///
/// In normal operation, this will return either ::T5_SUCCESS or ::T5_ERROR_TRY_AGAIN .  This
/// should be called until success or an different error is seen.
///
/// If glasses are not reserved before calling, this will return an error.
///
/// \par Threading
/// Exclusivity group 1 - Functions in this group must not be called concurrently from different
/// threads.
///
/// \param[in]  glasses - ::T5_Glasses returned by t5CreateGlasses()
///
/// \retval ::T5_SUCCESS                 Glasses ready for exclusive operations.
/// \retval ::T5_ERROR_TRY_AGAIN         Glasses not yet ready for exclusive operations.
/// \retval ::T5_ERROR_UNAVAILABLE       Glasses already in use by another client.
/// \retval ::T5_ERROR_DEVICE_LOST       Glasses have disconnected - destroy glasses with
///                                      t5DestroyGlasses(), re-obtain with
///                                      t5CreateGlasses() and try again.
/// \retval ::T5_ERROR_INVALID_STATE     Glasses weren't reserved first or client is invalid.
/// \retval ::T5_ERROR_INVALID_ARGS      Nullptr or invalid input supplied for `info`.
/// \retval ::T5_ERROR_NO_CONTEXT        `glasses` is invalid.
/// \retval ::T5_ERROR_INTERNAL          Internal error - not correctable.
/// \retval ::T5_ERROR_STRING_OVERFLOW   One or more of the provided strings is too long.
T5_EXPORT T5_Result t5EnsureGlassesReady(T5_Glasses glasses);

/// Release previously-reserved glasses.
///
/// Release glasses that were previously reserved for exclusive operations by the client.
/// After calling this, exclusive operations cannot be used with the glasses unless the
/// glasses are again reserved and readied.
///
/// \par Threading
/// Exclusivity group 1 - Functions in this group must not be called concurrently from different
/// threads.
///
/// \param[in]  glasses         - ::T5_Glasses returned by T5ApiSys::createGlasses()
///
/// \retval ::T5_SUCCESS          Glasses released.
/// \retval ::T5_ERROR_NO_CONTEXT `glasses` is invalid.
T5_EXPORT T5_Result t5ReleaseGlasses(T5_Glasses glasses);

/// \brief Get the exclusivity/connection status of the glasses
/// \ingroup glassesStateFns
///
/// \par Threading
/// Exclusivity group 1 - Functions in this group must not be called concurrently from different
/// threads.
///
/// \param[in]  glasses         - ::T5_Glasses returned by t5CreateGlasses()
/// \param[out] connectionState - ::T5_ConnectionState representing connection state
///
/// \retval ::T5_SUCCESS          Connection state was returned successfully
/// \retval ::T5_ERROR_NO_CONTEXT `glasses` is invalid.
/// \retval ::T5_ERROR_INTERNAL   Internal error - not correctable.
T5_EXPORT T5_Result t5GetGlassesConnectionState(T5_Glasses glasses,
                                                T5_ConnectionState* connectionState);

/// \brief Get the device identifier of a glasses.
/// \ingroup glassesStateFns
///
/// \par Threading
/// Exclusivity group 1 - Functions in this group must not be called concurrently from different
/// threads.
///
/// \param[in]     glasses    - ::T5_Glasses returned by t5CreateGlasses()
/// \param[out]    buffer     - C string buffer into which the result will be stored
/// \param[in,out] bufferSize - <b>On Call</b>: Size of buffer.<br/>&nbsp;
///                             <b>On Return</b>: Size of written data. Note that this may
///                             be larger than the buffer, in which case ::T5_ERROR_OVERFLOW
///                             is returned, and this value represents the size of the buffer
///                             needed to avoid overflow.
///
/// \retval ::T5_SUCCESS               Identifier written to `ident`.
/// \retval ::T5_ERROR_INVALID_ARGS    Nullptr was supplied for `ident`.
/// \retval ::T5_ERROR_NO_CONTEXT      `glasses` is invalid.
/// \retval ::T5_ERROR_STRING_OVERFLOW Buffer is too small to contain the resutl
T5_EXPORT T5_Result t5GetGlassesIdentifier(T5_Glasses glasses, char* buffer, size_t* bufferSize);

/// \defgroup c_exclusive_functions Exclusive functions
/// \brief Functions requiring an exclusive connection
///
/// Exclusive connections are established with t5ReserveGlasses() and t5EnsureGlassesReady().

/// \brief Get the latest pose of the glasses
/// \ingroup c_exclusive_functions
///
/// \par Exclusive Connection
/// Requires an exclusive connection - established with t5ReserveGlasses() and
/// t5EnsureGlassesReady().
///
/// \par Threading
/// Exclusivity group 1 - Functions in this group must not be called concurrently from different
/// threads.
///
/// \param[in]  glasses - ::T5_Glasses returned by t5CreateGlasses().
/// \param[in]  usage   - ::T5_GlassesPoseUsage indicating the intended use for the glasses pose.
/// \param[out] pose    - ::T5_GlassesPose representing current glasses pose.
///
/// \retval ::T5_SUCCESS             Pose written to `pose`.
/// \retval ::T5_ERROR_INVALID_ARGS  Nullptr was supplied for `pose`.
/// \retval ::T5_ERROR_NO_CONTEXT    `glasses` is invalid.
/// \retval ::T5_ERROR_NOT_CONNECTED Glasses aren't exclusively connected for this client.
///                                  Use t5ReserveGlasses() and t5EnsureGlassesReady() first.
/// \retval ::T5_ERROR_TRY_AGAIN     Pose wasn't been received from glasses yet.
/// \retval ::T5_ERROR_INTERNAL      Internal error - not correctable.
T5_EXPORT T5_Result t5GetGlassesPose(T5_Glasses glasses,
                                     T5_GlassesPoseUsage usage,
                                     T5_GlassesPose* pose);

/// \brief Initialize the graphics context to enable sending rendered frames to the glasses.
/// \ingroup c_exclusive_functions
///
/// \par Graphics API
/// Clients submit frames for render via texture handles, which vary between graphics engines.
///
/// The value of graphicsContext will depend on what library your application is using.
/// Refer to \ref aboutGraphicsApi for more details.
///
/// Note that not all values are supported on all platforms
/// (E.g. D3D is only available on Windows).
///
/// \par Threading
/// Exclusivity group 3 & Graphic thread only - Functions in this group must not be called
/// concurrently from different threads. The calling thread must be the thread that provided the
/// graphics context.
///
/// \param[in]  glasses         - ::T5_Glasses returned by t5CreateGlasses().
/// \param[in]  graphicsApi     - ::T5_GraphicsApi specifying the graphics API for the glasses.
/// \param[in]  graphicsContext - Meaning depends on the graphics API in use.
///                               (Vulkan only: You can free ::T5_GraphicsContextVulkan on return.)
///
/// \retval ::T5_SUCCESS                      The graphics context was successfully initialized.
/// \retval ::T5_ERROR_INVALID_ARGS           graphicsApi was ::kT5_GraphicsApi_None.
/// \retval ::T5_ERROR_INVALID_STATE          t5InitGlassesGraphicsContext() was previously called
///                                           on this Glasses object.
T5_EXPORT T5_Result t5InitGlassesGraphicsContext(T5_Glasses glasses,
                                                 T5_GraphicsApi graphicsApi,
                                                 void* graphicsContext);

/// \brief Configure the camera stream
///
/// \par Threading
/// Exclusivity group 1 - Functions in this group must not be called concurrently from different
/// threads.
///
/// \param[in]  glasses - ::T5_Glasses returned by t5CreateGlasses()
/// \param[in]  config  - ::T5_CameraStreamConfig filled by client to detail configuration
///
/// \retval ::T5_SUCCESS                Stream configured ok.
/// \retval ::T5_ERROR_IO_FAILURE       Failed to communicate with the service.
/// \retval ::T5_ERROR_NO_SERVICE       Service is unavailable.
/// \retval ::T5_ERROR_TARGET_NOT_FOUND Device not found.
/// \retval ::T5_ERROR_NO_CONTEXT       `glasses` is invalid.
///
/// The following are internal errors that should be discarded and/or logged:
/// \retval ::T5_ERROR_INTERNAL         Internal (Not correctable): Generic error.
/// \retval ::T5_ERROR_INVALID_STATE    Internal (Not correctable): Invalid state for request.
T5_EXPORT T5_Result t5ConfigureCameraStreamForGlasses(T5_Glasses glasses,
                                                      T5_CameraStreamConfig config);

/// Get a new filled image buffer from the camera stream. Will always return the oldest filled
/// buffer. This function does not block on call, and will return a TryAgain error if there are no
/// available filled buffers. To ensure images don't become stale, continual polling of
/// t5GetFilledCamImageBuffer() and resubmission of empty buffers via t5SubmitEmptyCamImageBuffer()
/// is expected.
//
/// \par Exclusive Connection
/// Requires an exclusive connection - established with makeExclusive().
///
/// \par Threading
/// Exclusivity group 1 - Functions in this group must not be called simultaneously
/// from different threads.
///
/// \param[in]  glasses - ::T5_Glasses returned by T5ApiSys::createGlasses().
/// \param[out] image   - ::T5_CamImage representing a camera image wrapper that will contain a
/// filled buffer upon successful return.
///
/// \retval ::T5_SUCCESS             Image written to `image`.
/// \retval ::T5_ERROR_TRY_AGAIN     No available image.
/// \retval ::T5_ERROR_INVALID_ARGS  Nullptr was supplied for `image`.
/// \retval ::T5_ERROR_NO_CONTEXT    `glasses` is invalid.
/// \retval ::T5_ERROR_NOT_CONNECTED Glasses aren't exclusively connected for this client.
///                                  Use makeExclusive() first.
/// \retval ::T5_INVALID_BUFFER      The buffer does not have the requisite size for a camera image
/// \retval ::T5_ERROR_INTERNAL      Internal error - not correctable.
T5_EXPORT T5_Result t5GetFilledCamImageBuffer(T5_Glasses glasses, T5_CamImage* image);

/// Submit an empty image buffer to be filled by the camera frame stream
//
/// \par Exclusive Connection
/// Requires an exclusive connection - established with makeExclusive().
///
/// \par Threading
/// Exclusivity group 1 - Functions in this group must not be called simultaneously
/// from different threads.
///
/// The memory in the image buffer Must remain valid until the corresponding wrapper is returned by
/// t5GetFilledCamImageBuffer or the buffer is canceled by t5CancelCamImageBuffer
///
/// Note, only the image buffer inside of T5_CamImage is required to be kept valid. The T5_CamImage
/// wrapper is not.
///
/// Incoming image is expected to have 0 width, height, and stride, and a buffer size larger than
/// the minimum image size.
///
/// \param[in]  glasses - ::T5_Glasses returned by T5ApiSys::createGlasses().
/// \param[in]  image   - ::T5_CamImage representing current camera image buffer to be filled.
///
/// \retval ::T5_SUCCESS             Buffer submitted to service.
/// \retval ::T5_ERROR_INVALID_ARGS  Nullptr was supplied for `image`.
/// \retval ::T5_ERROR_NO_CONTEXT    `glasses` is invalid.
/// \retval ::T5_ERROR_NOT_CONNECTED Glasses aren't exclusively connected for this client.
///                                  Use makeExclusive() first.
/// \retval ::T5_INVALID_BUFFER      The buffer does not have the requisite size for a camera image
/// \retval ::T5_ERROR_INTERNAL      Internal error - not correctable.
T5_EXPORT T5_Result t5SubmitEmptyCamImageBuffer(T5_Glasses glasses, T5_CamImage* image);

/// Clear out the remaining buffers and return all buffers as a vector of camera images
//
/// \par Exclusive Connection
/// Requires an exclusive connection - established with makeExclusive().
///
/// \par Threading
/// Exclusivity group 1 - Functions in this group must not be called simultaneously
/// from different threads.
///
/// \param[in]  glasses - ::T5_Glasses returned by T5ApiSys::createGlasses().
/// \param[in]  buffer  - A pointer to an image buffer that should be cancelled and no longer used
/// by the service
///
/// \retval ::T5_SUCCESS             Buffer is no longer in use and is available for freeing
/// \retval ::T5_ERROR_NO_CONTEXT    `glasses` is invalid.
/// \retval ::T5_ERROR_NOT_CONNECTED Glasses aren't exclusively connected for this client.
///                                  Use makeExclusive() first.
/// \retval ::T5_ERROR_INTERNAL      Internal error - not correctable.
T5_EXPORT T5_Result t5CancelCamImageBuffer(T5_Glasses glasses, uint8_t* buffer);

/// \brief Send a frame to display on the glasses
/// \ingroup c_exclusive_functions
///
/// Both left and right stereoscopic images are presented together via this call. The textures
/// referred to by the `leftTexHandle` and `rightTexHandle` fields of \a info will be used within
/// this call to enqueue and submit graphics operations that will read the texture data. Once this
/// call returns, no additional operations referencing those textures will be performed. However,
/// depending on the graphics API being used, the application may need to perform additional
/// synchronization prior to freeing the texture resources in order to ensure that the queued
/// graphics operations have completed.
///
/// \par Exclusive Connection
/// Requires an exclusive connection - established with t5ReserveGlasses() and
/// t5EnsureGlassesReady().
///
/// \par Graphics Context
/// Requires a graphics context - initialized with t5InitGraphicsContext()
//
/// \par Threading
/// Exclusivity group 3 & Graphic thread only - Functions in this group must not be called
/// concurrently from different threads. The calling thread must be the thread that provided the
/// graphics context.
///
/// \see \ref aboutGraphicsApi
///
/// \param[in]  glasses - ::T5_Glasses returned by t5CreateGlasses().
/// \param[in]  info    - ::T5_FrameInfo detailing the frame to display.
///
/// \retval ::T5_SUCCESS                      The frame was successfully queued to be sent.
/// \retval ::T5_ERROR_INVALID_ARGS           Nullptr was supplied for `info`.
/// \retval ::T5_ERROR_INVALID_STATE          graphicsApi was ::kT5_GraphicsApi_None when
///                                           t5CreateGlasses() was called.
/// \retval ::T5_ERROR_NOT_CONNECTED          Glasses aren't exclusively connected for this
///                                           client. Use t5ReserveGlasses() and
///                                           t5EnsureGlassesReady() first.
/// \retval ::T5_ERROR_INVALID_GFX_CONTEXT    Graphics context is invalid. Check that
///                                           graphicsContext was correct when
///                                           t5InitGraphicsContext() was called.
/// \retval ::T5_ERROR_GFX_CONTEXT_INIT_FAIL  Failed to initialize graphics context.
///                                           Exact meaning depends on current graphics API.
/// \retval ::T5_ERROR_NO_CONTEXT             `glasses` is invalid.
/// \retval ::T5_ERROR_INTERNAL               Internal error - not correctable.
T5_EXPORT T5_Result t5SendFrameToGlasses(T5_Glasses glasses, const T5_FrameInfo* info);

/// \brief Validate a provided T5_FrameInfo
///
/// Determines if the provided T5_FrameInfo is likely valid.
/// Note that this function <i>does not</i> validate the texture handles beyond checking that they
/// are not null - the validity of the texture handles is graphics API dependent, and currently
/// beyond the scope of this function.
///
/// If there are errors detected in the ::T5_FrameInfo, ::T5_ERROR_DECODE_ERROR is returned and
/// a human readable error message will be written to the provided detail buffer explaining the
/// errors (if there are multiple errors, each will be seperated by a newline).
///
/// In the event of a buffer overflow writing to the detail buffer, ::T5_ERROR_OVERFLOW is
/// returned and the detail message is truncated - Note that in all cases, if detailSize is
/// greater than 0, a null terminator is written, however if detailSize <i>is</i> 0 nothing
/// is written to detail and callers should ensure that they do not attempt to use the
/// (potentially invalid) string.
///
/// Note: While this function is lightweight, it is not recommended to call it for every frame
/// since the validated parameters are unlikely to change between frames. It is also <i>not</i>
/// necessary to call this function at all - it is a convenience function to sanity check your
/// inputs.
///
/// \par Threading
/// Exclusivity group 3 & Graphic thread only - Functions in this group must not be called
/// concurrently from different threads. The calling thread must be the thread that provided the
/// graphics context.
///
/// \param[in]  info          - ::T5_ParamGlasses to get value for.
/// \param[out] detail        - A buffer to receive a null terminated C string detailing the
///                             error with the T5_FrameInfo. In the case of ::T5_SUCCESS, an
///                             empty string will be written.
/// \param[in,out] detailSize - <b>On Call</b>: Size of the buffer pointed to by `detail`.
///                             <br/>&nbsp;
///                             <b>On Return</b>: Size of the message returned in detail including
///                             the string null terminator. Note that this may be larger than the
///                             buffer, in which case ::T5_ERROR_OVERFLOW is returned, and this
///                             value represents the size of the buffer needed to avoid overflow.
///
/// \retval ::T5_SUCCESS            The supplied frame info is likely valid.
/// \retval ::T5_ERROR_NO_CONTEXT   `glasses` is invalid.
/// \retval ::T5_ERROR_DECODE_ERROR There were errors with the supplied frame info.
/// \retval ::T5_ERROR_INVALID_ARGS Null was supplied for info, detail or detailSize.
/// \retval ::T5_ERROR_OVERFLOW     The detail message is larger than the supplied buffer.
T5_EXPORT T5_Result t5ValidateFrameInfo(T5_Glasses glasses,
                                        const T5_FrameInfo* info,
                                        char* detail,
                                        size_t* detailSize);

/// \defgroup glasses_getParam Glasses parameters
/// \brief Functions for getting glasses parameters

/// \brief Get a glasses integer parameter
/// \ingroup glasses_getParam
///
/// See ::T5_ParamGlasses for a list of possible parameters to retrieve.
///
/// \par Threading
/// Exclusivity group 1 - Functions in this group must not be called concurrently from different
/// threads.
///
/// \param[in]  glasses - ::T5_Glasses returned by t5CreateGlasses().
/// \param[in]  wand    - ::T5_WandHandle to get value for. Use 0 for non-wand queries.
/// \param[in]  param   - ::T5_ParamGlasses to get value for.
/// \param[out] value   - Pointer to a int64_t to be set to the current value.
///
/// \retval ::T5_SUCCESS                  Got the parameter
/// \retval ::T5_ERROR_INVALID_ARGS       `param` was not a valid enumerant
///                                       <span class='altMeaning'>or</span>
///                                       NULL was supplied for `value`
/// \retval ::T5_TIMEOUT                  Timeout waiting to read value.
/// \retval ::T5_ERROR_IO_FAILURE         Failed to communicate with the service.
/// \retval ::T5_ERROR_NO_SERVICE         Service is unavailable.
/// \retval ::T5_ERROR_NO_CONTEXT         `context` is invalid.
/// \retval ::T5_ERROR_SETTING_WRONG_TYPE The requested parameter is not an integer value.
///
/// The following are internal errors that should be discarded and/or logged:
/// \retval ::T5_ERROR_SETTING_UNKNOWN    Internal (Not correctable): Setting is unknown.
/// \retval ::T5_ERROR_INTERNAL           Internal (Not correctable): Generic error.
/// \retval ::T5_ERROR_MISC_REMOTE        Internal (Not correctable): Generic service error.
/// \retval ::T5_ERROR_OVERFLOW           Internal (Not correctable): Buffer overflow.
T5_EXPORT T5_Result t5GetGlassesIntegerParam(T5_Glasses glasses,
                                             T5_WandHandle wand,
                                             T5_ParamGlasses param,
                                             int64_t* value);

/// \brief Get a glasses floating point parameter
/// \ingroup glasses_getParam
///
/// See ::T5_ParamGlasses for a list of possible parameters to retrieve.
///
/// \par Threading
/// Exclusivity group 1 - Functions in this group must not be called concurrently from different
/// threads.
///
/// \param[in]  glasses - ::T5_Glasses returned by t5CreateGlasses().
/// \param[in]  wand    - ::T5_WandHandle to get value for. Use 0 for non-wand queries.
/// \param[in]  param   - ::T5_ParamGlasses to get value for.
/// \param[out] value   - Pointer to a double to be set to the current value.
///
/// \retval ::T5_SUCCESS                  Got the parameter
/// \retval ::T5_ERROR_INVALID_ARGS       `param` was not a valid enumerant
///                                       <span class='altMeaning'>or</span>
///                                       NULL was supplied for `value`
/// \retval ::T5_TIMEOUT                  Timeout waiting to read value.
/// \retval ::T5_ERROR_IO_FAILURE         Failed to communicate with the service.
/// \retval ::T5_ERROR_NO_SERVICE         Service is unavailable.
/// \retval ::T5_ERROR_NO_CONTEXT         `context` is invalid.
/// \retval ::T5_ERROR_SETTING_WRONG_TYPE The requested parameter is not a floating point value.
///
/// The following are internal errors that should be discarded and/or logged:
/// \retval ::T5_ERROR_SETTING_UNKNOWN    Internal (Not correctable): Setting is unknown.
/// \retval ::T5_ERROR_INTERNAL           Internal (Not correctable): Generic error.
/// \retval ::T5_ERROR_MISC_REMOTE        Internal (Not correctable): Generic service error.
/// \retval ::T5_ERROR_OVERFLOW           Internal (Not correctable): Buffer overflow.
T5_EXPORT T5_Result t5GetGlassesFloatParam(T5_Glasses glasses,
                                           T5_WandHandle wand,
                                           T5_ParamGlasses param,
                                           double* value);

/// \brief Get a glasses UTF-8 encoded string parameter
/// \ingroup glasses_getParam
///
/// See ::T5_ParamGlasses for a list of possible parameters to retrieve.
///
/// \par Threading
/// Exclusivity group 1 - Functions in this group must not be called concurrently from different
/// threads.
///
/// \param[in]  glasses       - ::T5_Glasses returned by t5CreateGlasses().
/// \param[in]  wand          - ::T5_WandHandle to get value for. Use 0 for non-wand queries.
/// \param[in]  param         - ::T5_ParamGlasses to get value for.
/// \param[out] buffer        - Pointer to a buffer into which the current value is to be
///                             written as a null-terminated string of UTF-8 code points.
/// \param[in,out] bufferSize - <b>On Call</b>: Size of the buffer pointed to by `buffer`.
///                             <br/>&nbsp;
///                             <b>On Return</b>: Size of the parameter value. Note that this
///                             may be larger than the buffer, in which case ::T5_ERROR_OVERFLOW
///                             is returned, and this value represents the size of the buffer
///                             needed to avoid overflow.
///
/// \retval ::T5_SUCCESS                  Got the parameter
/// \retval ::T5_ERROR_INVALID_ARGS       `param` was not a valid enumerant
///                                       <span class='altMeaning'>or</span>
///                                       NULL was supplied for `value` and `bufferSize` was not
///                                       0
///                                       <span class='altMeaning'>or</span>
///                                       NULL was supplied for `bufferSize`
/// \retval ::T5_TIMEOUT                  Timeout waiting to read value.
/// \retval ::T5_ERROR_IO_FAILURE         Failed to communicate with the service.
/// \retval ::T5_ERROR_NO_SERVICE         Service is unavailable.
/// \retval ::T5_ERROR_NO_CONTEXT         `context` is invalid.
/// \retval ::T5_ERROR_SETTING_WRONG_TYPE The requested parameter is not a UTF-8 string value.
///
/// The following are internal errors that should be discarded and/or logged:
/// \retval ::T5_ERROR_SETTING_UNKNOWN    Internal (Not correctable): Setting is unknown.
/// \retval ::T5_ERROR_INTERNAL           Internal (Not correctable): Generic error.
/// \retval ::T5_ERROR_MISC_REMOTE        Internal (Not correctable): Generic service error.
/// \retval ::T5_ERROR_OVERFLOW           Internal (Not correctable): Buffer overflow.
T5_EXPORT T5_Result t5GetGlassesUtf8Param(T5_Glasses glasses,
                                          T5_WandHandle wand,
                                          T5_ParamGlasses param,
                                          char* buffer,
                                          size_t* bufferSize);

/// \brief Get a glasses-specific list of changed parameters
/// \ingroup glasses_getParam
///
/// This function doesn't return the values of the changed parameters, but an
/// unordered list of the parameters that have changed since this function was
/// last called. Note that as a result, the first call to this function will
/// always result in a count of 0.
///
/// \par Threading
/// Exclusivity group 1 - Functions in this group must not be called concurrently from different
/// threads.
///
/// \param[in]     glasses - ::T5_Glasses returned by t5CreateGlasses().
/// \param[out]    buffer  - ::T5_ParamGlasses buffer to receive list of parameters.
/// \param[in,out] count   - <b>On Call</b>: Size of buffer in elements.<br/>&nbsp;
///                          <b>On Return</b>: Number of changed parameters in butter.
///
/// \retval ::T5_SUCCESS            Changed parameter list written to buffer.
/// \retval ::T5_ERROR_INVALID_ARGS Nullptr was supplied for `buffer`.
///                                  <span class='altMeaning'>or</span>
///                                 Nullptr was supplied for `count`.
/// \retval ::T5_ERROR_OVERFLOW     Buffer too small to contain parameter list.
/// \retval ::T5_ERROR_NO_CONTEXT   `glasses` is invalid.
T5_EXPORT T5_Result t5GetChangedGlassesParams(T5_Glasses glasses,
                                              T5_ParamGlasses* buffer,
                                              uint16_t* count);

/// \brief Get projection properties for glasses
///
/// \par Threading
/// Exclusivity group 1 - Functions in this group must not be called concurrently from different
/// threads.
///
/// \par Parameters
/// Each graphics API requires different properties when creating a projection matrix.
/// They each have their defaults, for which the properties are shown below. If you are not using
/// the defaults for your graphics API, ensure that you adjust the parameters used here.
///
///           ┌───────┬──────┬──────┬──────────────────────────────────┐
///           │ L/RHS │ Near │ Far  │ Native Equivalent Function       │
/// ┌─────────┼───────┼──────┼──────┼──────────────────────────────────│
/// │ OpenGL  │  RHS→ │  -1  │  +1  │ glFrustum                        │
/// │ DirectX │ ←LHS  │   0  │  +1  │ D3DXMatrixPerspectiveOffCenterLH │
/// │ Vulkan  │  RHS→ │   0  │  +1  │                                  │
/// │ Metal   │  RHS→ │  -1  │  +1  │                                  │
/// └─────────┴───────┴──────┴──────┴──────────────────────────────────┘
/// Note: When referencing D3DXMatrixPerspectiveOffCenterLH, DirectX uses Row-major matrix
///
/// \param[in]  glasses        - ::T5_Glasses returned by t5CreateGlasses().
/// \param[in]  handedness     - ::T5_CartesianCoordinateHandedness specifying handedness.
/// \param[in]  depthRange     - ::T5_DepthRange to use for matrix creation.
/// \param[in]  matrixOrder    - ::T5_MatrixOrder representing row or column major ordering.
/// \param[in]  nearPlane      - The near clipping plane in view space
/// \param[in]  farPlane       - The far clipping plane in view space
/// \param[in]  worldScale     - Conversion factor between world space and real-world units.
/// \param[out] projectionInfo - ::T5_ProjectionInfo to write values to.
///
/// \retval ::T5_SUCCESS                Projection info written to projectionInfo.
/// \retval ::T5_ERROR_INVALID_ARGS     Nullptr was supplied for `projectionInfo`.
///                                      <span class='altMeaning'>or</span>
///                                     Nullptr was supplied for `projectionInfo.matrix`.
///                                      <span class='altMeaning'>or</span>
///                                     `handedness` is invalid.
///                                      <span class='altMeaning'>or</span>
///                                     `depthRange` is invalid.
///                                      <span class='altMeaning'>or</span>
///                                     `matrixOrder` is invalid.
/// \retval ::T5_ERROR_INVALID_GEOMETRY nearPlane ≤ 0
///                                      <span class='altMeaning'>or</span>
///                                     farPlane ≤ 0
///                                      <span class='altMeaning'>or</span>
///                                     farPlane ≤ nearPlane
///                                      <span class='altMeaning'>or</span>
///                                     worldScale ≤ 0
/// \retval ::T5_ERROR_NO_CONTEXT       `glasses` is invalid.
/// \retval ::T5_ERROR_IO_FAILURE       Failed to communicate with the service.
/// \retval ::T5_ERROR_NO_SERVICE       Service is unavailable.
/// \retval ::T5_ERROR_TARGET_NOT_FOUND Device not found.
T5_EXPORT T5_Result t5GetProjection(T5_Glasses glasses,
                                    T5_CartesianCoordinateHandedness handedness,
                                    T5_DepthRange depthRange,
                                    T5_MatrixOrder matrixOrder,
                                    double nearPlane,
                                    double farPlane,
                                    double worldScale,
                                    T5_ProjectionInfo* projectionInfo);

/// \defgroup wand_functions Wand related
/// \brief Functions related to wands

/// \brief List available wands connected to this glasses
/// \ingroup wand_functions
///
/// The results are written to the buffer pointed to by 'buffer', up to the
/// number of elements specified in 'bufferCapacity'. If the number of wands
/// connected would overflow the buffer, ::T5_ERROR_OVERFLOW is returned.
///
/// The number of connected wands (and the number of elements that have been, or
/// would be, written the the buffer), is returned in \a count.
///
/// \par Threading
/// Exclusivity group 1 - Functions in this group must not be called concurrently from different
/// threads.
///
/// \param[in]     glasses - ::T5_Glasses returned by t5CreateGlasses().
/// \param[out]    buffer  - ::T5_WandHandle buffer to receive the list of wand handles.
/// \param[in,out] count   - <b>On Call</b>: Size of buffer in elements.<br/>&nbsp;
///                          <b>On Return</b>: Number of elements present.
///
/// \retval ::T5_SUCCESS                List successfully written to buffer.
/// \retval ::T5_ERROR_INVALID_ARGS     Nullptr was supplied for `buffer`.
///                                       <span class='altMeaning'>or</span>
///                                     Nullptr was supplied for `count`.
/// \retval ::T5_ERROR_OVERFLOW         Provided buffer is too small to contain wand list.
/// \retval ::T5_ERROR_NO_CONTEXT       `glasses` is invalid.
/// \retval ::T5_ERROR_IO_FAILURE       Failed to communicate with the service.
/// \retval ::T5_ERROR_NO_SERVICE       Service is unavailable.
/// \retval ::T5_ERROR_TARGET_NOT_FOUND Device not found.
///
/// The following are internal errors that should be discarded and/or logged:
/// \retval ::T5_ERROR_INTERNAL         Internal (Not correctable): Generic error.
/// \retval ::T5_ERROR_INVALID_STATE    Internal (Not correctable): Invalid state for request.
T5_EXPORT T5_Result t5ListWandsForGlasses(T5_Glasses glasses,
                                          T5_WandHandle* buffer,
                                          uint8_t* count);

/// \brief Send a haptic data buffer to a wand
/// \ingroup wand_functions
///
/// \par Threading
/// Exclusivity group 1 - Functions in this group must not be called concurrently from different
/// threads.
///
/// \param[in]     glasses - ::T5_Glasses returned by t5CreateGlasses().
/// \param[in]     wand - ::T5_WandHandle to get value for.
/// \param[in]     amplitude - The amplitude of the impulse, between 0.0 and 1.0.
/// \param[in]     duration  - The duration, in ms, of the
/// impulse. Duration must be less than or equal to 320ms.
///
/// \retval ::T5_SUCCESS                List successfully written to buffer.
/// \retval ::T5_ERROR_INVALID_ARGS     Value outside of [0.0,1.0] was supplied for amplitude.
///                                           <span class='altMeaning'>or</span>
///                                     Value outside of [0,320] was supplied for duration.
/// \retval ::T5_ERROR_NO_CONTEXT       `glasses` is invalid.
/// \retval ::T5_ERROR_IO_FAILURE       Failed to communicate with the service.
/// \retval ::T5_ERROR_NO_SERVICE       Service is unavailable.
/// \retval ::T5_ERROR_TARGET_NOT_FOUND Device not found.
///
/// The following are internal errors that should be discarded and/or logged:
/// \retval ::T5_ERROR_INTERNAL         Internal (Not correctable): Generic error.
/// \retval ::T5_ERROR_INVALID_STATE    Internal (Not correctable): Invalid state for request.
T5_EXPORT T5_Result t5SendImpulse(T5_Glasses glasses,
                                  T5_WandHandle wand,
                                  float amplitude,
                                  uint16_t duration);

/// \brief Configure the wand event stream
/// \ingroup wand_functions
///
/// \par Threading
/// Exclusivity group 1 - Functions in this group must not be called concurrently from different
/// threads.
///
/// \param[in]  glasses - ::T5_Glasses returned by t5CreateGlasses()
/// \param[in]  config  - ::T5_WandStreamConfig filled by client to detail configuration
///
/// \retval ::T5_SUCCESS                Stream configured ok.
/// \retval ::T5_ERROR_IO_FAILURE       Failed to communicate with the service.
/// \retval ::T5_ERROR_NO_SERVICE       Service is unavailable.
/// \retval ::T5_ERROR_TARGET_NOT_FOUND Device not found.
/// \retval ::T5_ERROR_NO_CONTEXT       `glasses` is invalid.
///
/// The following are internal errors that should be discarded and/or logged:
/// \retval ::T5_ERROR_INTERNAL         Internal (Not correctable): Generic error.
/// \retval ::T5_ERROR_INVALID_STATE    Internal (Not correctable): Invalid state for request.
T5_EXPORT T5_Result t5ConfigureWandStreamForGlasses(T5_Glasses glasses,
                                                    const T5_WandStreamConfig* config);

/// \brief Read from the wands event stream
/// \ingroup wand_functions
///
/// The client should repeatedly call this for as long as the wand event
/// stream is enabled. In any polling period, the client should call this
/// in a loop until it returns ::T5_TIMEOUT to ensure that the queue
/// is drained.
///
/// The server will continuously write events to the stream, and will not block
/// for clients that have fallen behind reading the stream. Wand events that
/// overflow the buffer are discarded. In the event of falling behind the client
/// should do any static query that it needs to do (such as enumerating
/// connected wands), and attempt to read the stream faster.
///
/// \par Threading
/// Exclusivity group 2 - Functions in this group must not be called concurrently from different
/// threads.
///
/// \param[in]  glasses   - ::T5_Glasses returned by t5CreateGlasses().
/// \param[out] event     - If an event is available, it will be written here.
/// \param[in]  timeoutMs - Timeout in ms to wait before returning without read.
///
/// \retval ::T5_SUCCESS           Wand stream event written to `glasses`
/// \retval ::T5_TIMEOUT           Timeout waiting for wand stream event
/// \retval ::T5_ERROR_NO_CONTEXT  `glasses` is invalid.
/// \retval ::T5_ERROR_UNAVAILABLE Wand stream isn't configured as enabled.
///                                Use t5ConfigureWandStream() to enable.
/// \retval ::T5_ERROR_IO_FAILURE  Failed to communicate with the service.
///
/// The following are internal errors that should be discarded and/or logged:
/// \retval ::T5_ERROR_DECODE_ERROR  Internal (Not correctable): Decode error.
/// \retval ::T5_ERROR_INTERNAL      Internal (Not correctable): Generic error.
/// \retval ::T5_ERROR_INVALID_STATE Internal (Not correctable): Invalid state for request.
T5_EXPORT T5_Result t5ReadWandStreamForGlasses(T5_Glasses glasses,
                                               T5_WandStreamEvent* event,
                                               uint32_t timeoutMs);

/// \}
// C_Gls

/// \{
// C_Glasses_Interface

#ifdef __cplusplus
}
#endif

#undef T5_EXPORT

/// \}
