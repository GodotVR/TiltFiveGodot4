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
/// \brief Main include for the Tilt Five™ C++ API Binder

#include "TiltFiveNative.h"
#include "errors.hpp"
#include "result.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <utility>
#include <vector>

/// \defgroup CPP_Interface Tilt Five™ Native Interface (C++)
/// C++ binder for the Tilt Five NDK
/// \{

namespace tiltfive {

/// \defgroup CPP_Interface Tilt Five™ Native Interface (C++)
/// C++ binder for the Tilt Five NDK
/// \{

//////////////////////////////////////////////////////////////////////////////
////                                 Client                               ////
//////////////////////////////////////////////////////////////////////////////

class Client;
class Glasses;
class Wand;
class WandStreamHelper;
class GlassesConnectionHelper;
class ParamChangeHelper;
class ParamChangeListener;

/// \cond DO_NOT_DOCUMENT
/// Internal utility functions - Do not call directly
inline auto obtainWand(T5_WandHandle handle, std::shared_ptr<WandStreamHelper> wandStreamHelper)
    -> std::shared_ptr<Wand>;
inline auto obtainWandStreamHelper(
    std::shared_ptr<Glasses> glasses,
    std::chrono::milliseconds pollTimeout = std::chrono::milliseconds(100))
    -> std::shared_ptr<WandStreamHelper>;
inline auto obtainGlassesConnectionHelper(std::shared_ptr<Glasses> glasses,
                                          const std::string& displayName,
                                          std::chrono::milliseconds connectionPollInterval)
    -> std::unique_ptr<GlassesConnectionHelper>;
inline auto obtainParamChangeHelper(std::shared_ptr<Client> client,
                                    std::weak_ptr<ParamChangeListener> listener,
                                    std::chrono::milliseconds pollInterval)
    -> std::unique_ptr<ParamChangeHelper>;
/// \endcond

/// \brief Client for communicating with the Tilt Five™ API
class Client : public std::enable_shared_from_this<Client> {
private:
    static constexpr bool kDebug = true;

    Client(std::string applicationId, std::string applicationVersion, const uint8_t sdkType)
        : mApplicationId(std::move(applicationId))
        , mApplicationVersion(std::move(applicationVersion)) {

        mClientInfo.applicationId      = mApplicationId.c_str();
        mClientInfo.applicationVersion = mApplicationVersion.c_str();
        mClientInfo.sdkType            = sdkType;
    }

    const std::string mApplicationId;
    const std::string mApplicationVersion;

    T5_Context mContext{};
    T5_ClientInfo mClientInfo{};

    friend Glasses;
    friend ParamChangeHelper;

    friend std::ostream& operator<<(std::ostream& os, std::shared_ptr<Client> const& instance) {
        os << *instance;
        return os;
    }

    friend std::ostream& operator<<(std::ostream& os, Client const& instance) {
        os << "<Client:" << instance.mApplicationId << ">";
        return os;
    }

    friend auto obtainClient(const std::string& applicationId,
                             const std::string& applicationVersion,
                             void* platformContext,
                             const uint8_t sdkType) -> Result<std::shared_ptr<Client>>;

    static auto create(const std::string& applicationId,
                       const std::string& applicationVersion,
                       void* platformContext,
                       const uint8_t sdkType) -> Result<std::shared_ptr<Client>> {

        // Validate inputs
        if (applicationId.length() > T5_MAX_STRING_PARAM_LEN) {
            return Error::kStringOverflow;
        }

        if (applicationVersion.length() > T5_MAX_STRING_PARAM_LEN) {
            return Error::kStringOverflow;
        }

        // Create client
        auto client =
            std::shared_ptr<Client>(new Client(applicationId, applicationVersion, sdkType));

        // Start up the service connection
        auto err = t5CreateContext(&client->mContext, &client->mClientInfo, platformContext);
        if (err) {
            return static_cast<Error>(err);
        }

        return client;
    };

public:
    /// \cond DO_NOT_DOCUMENT
    virtual ~Client() {
        // Release context and library
        t5DestroyContext(&mContext);
        mContext = nullptr;
    }
    /// \endcond

    //////////////////////////////////////////////////////////
    ////                 System Functions                 ////
    //////////////////////////////////////////////////////////

    /// \brief Enumerate glasses
    ///
    /// Glasses may not be ready to connect if they are in the process of booting (or rebooting).
    ///
    /// \return Result containing either a vector of glasses identifier strings or an error.
    auto listGlasses() -> Result<std::vector<std::string>> {
        std::vector<char> buffer;
        buffer.resize(64);

        size_t bufferSize;

        // If the buffer passed to listGlasses() is too small, it'll return with
        // an overflow condition, in which case, increase the size of the buffer
        // and try again.
        for (;;) {
            bufferSize    = buffer.size();
            T5_Result err = t5ListGlasses(mContext, buffer.data(), &bufferSize);
            if (!err) {
                break;
            } else if (err == T5_ERROR_OVERFLOW) {
                if (bufferSize > 1024) {
                    return Error::kOverflow;
                }

                buffer.resize(bufferSize);
            } else {
                return static_cast<Error>(err);
            }
        }

        // Peel off string until we encounter a naked null (empty string)
        std::vector<std::string> glassesList;
        auto buffPtr = buffer.data();
        for (;;) {
            std::string id = buffPtr;
            if (id.empty()) {
                break;
            }

            buffPtr += id.length() + 1;
            glassesList.push_back(id);
        }
        return glassesList;
    }

    /// \brief Get the version of the Tilt Five™ service
    ///
    /// \return String representing the version of the Tilt Five™ service.
    /// Format is <a href="https://semver.org/spec/v2.0.0.html">Semantic Versioning</a>.
    auto getServiceVersion() -> Result<std::string> {
        std::unique_ptr<char[]> value(new char[T5_MAX_STRING_PARAM_LEN]);
        size_t size = T5_MAX_STRING_PARAM_LEN;
        T5_Result err =
            t5GetSystemUtf8Param(mContext, kT5_ParamSys_UTF8_Service_Version, value.get(), &size);
        if (!err) {
            return std::string(value.get(), size);
        } else {
            return static_cast<Error>(err);
        }
    }

    /// \brief Get a system-wide list of changed parameters
    ///
    /// This function doesn't return the values of the changed parameters, but vector of the
    /// parameters that have changed since this function was last called. Note that as a result,
    /// the first call to this function will always result in a count of 0.
    ///
    /// \return A std::vector of ::T5_ParamSys containing the changed parameters
    auto getChangedParams() -> Result<std::vector<T5_ParamSys>> {
        uint16_t changedParamsCount = 32;
        std::vector<T5_ParamSys> changedParamsBuffer(changedParamsCount);

        changedParamsBuffer.resize(changedParamsCount);
        T5_Result err =
            t5GetChangedSystemParams(mContext, changedParamsBuffer.data(), &changedParamsCount);

        for (;;) {
            if (!err) {
                changedParamsBuffer.resize(changedParamsCount);
                return changedParamsBuffer;

            } else if (err == T5_ERROR_OVERFLOW) {
                changedParamsCount = changedParamsBuffer.size() * 2;
                continue;

            } else {
                return static_cast<Error>(err);
            }
        }
    }

    /// \brief Check if the Tilt Five™ UI wants the users attention
    ///
    /// From time to time, the Tilt Five™ UI may want the users attention, for example if there is
    /// an urgent wand firmware upgrade, or a connectivity problem that requires troubleshooting.
    ///
    /// Applications may query this to determine if this is the case. This results in a better user
    /// experience - for example if a user is attempting to run your application with the
    /// Tilt Five™ hardware, and it's not working, it may be due to an issue that can be resolved
    /// in the control panel.
    ///
    /// \return `true` if the Tilt Five™ UI wants the users attention, `false` otherwise.
    auto isTiltFiveUiRequestingAttention() -> Result<bool> {
        int64_t value = 0;

        T5_Result err =
            t5GetSystemIntegerParam(mContext, kT5_ParamSys_Integer_CPL_AttRequired, &value);
        if (!err) {
            return value != 0;
        } else if (err == T5_ERROR_SETTING_UNKNOWN) {
            return false;
        } else {
            return static_cast<Error>(err);
        }
    }

    /// \brief Obtain the dimensions of the gameboard
    ///
    /// \param[in]  type - ::T5_GameboardType of the gameboard to get dimensions for.
    /// \return ::T5_GameboardType containing the requested dimensions.
    auto getGameboardSize(T5_GameboardType type) -> Result<T5_GameboardSize> {
        T5_GameboardSize size;

        T5_Result err = t5GetGameboardSize(mContext, type, &size);
        if (!err) {
            return size;
        } else {
            return static_cast<Error>(err);
        }
    }

    /// \brief Create a ParamChangeHelper
    ///
    /// \ref UsingParamChangeHelper
    ///
    /// \param[in]  listener     - A std::weak_ptr to a ParamChangeListener to receive callbacks
    /// \param[in]  pollInterval - Polling interval for changes
    /// \return A std::unique_ptr to a ParamChangeHelper
    [[nodiscard]] auto createParamChangedHelper(
        std::weak_ptr<ParamChangeListener> listener,
        std::chrono::milliseconds pollInterval = std::chrono::milliseconds(100))
        -> std::unique_ptr<ParamChangeHelper> {

        return obtainParamChangeHelper(shared_from_this(), std::move(listener), pollInterval);
    }
};

/// Represents the exclusivity connection state of glasses
enum class ConnectionState : int {
    /// \brief Glasses have not been connected for exclusive use.
    ///        They may still be connected to the host.
    kNotExclusivelyConnected,

    /// \brief Glasses are reserved for exclusive use by this client.
    kReserved,

    /// \brief Glasses are connected for exclusive use by this client.
    kConnected,

    /// \brief Glasses were previously connected for exclusive use, but have now disconnected.
    kDisconnected,
};

/// \brief Represents an instance of Tilt Five™ glasses
class Glasses : public std::enable_shared_from_this<Glasses> {
    friend Client;
    friend ParamChangeHelper;

    const std::string mIdentifier;
    const std::shared_ptr<Client> mClient;
    std::weak_ptr<WandStreamHelper> mWandStreamHelper{};
    T5_Glasses mGlasses{};

    friend std::ostream& operator<<(std::ostream& os, std::shared_ptr<Glasses> const& instance) {
        os << *instance;
        return os;
    }

    friend std::ostream& operator<<(std::ostream& os, Glasses const& instance) {
        os << "<Glasses:" << instance.mIdentifier << ">";
        return os;
    }

    friend auto obtainGlasses(const std::string& identifier, const std::shared_ptr<Client>& client)
        -> Result<std::shared_ptr<Glasses>>;

    explicit Glasses(std::string identifier, std::shared_ptr<Client> client)
        : mIdentifier(std::move(identifier)), mClient(std::move(client)) {}

    static auto create(std::string identifier, std::shared_ptr<Client> client)
        -> Result<std::shared_ptr<Glasses>> {

        if (!client) {
            return tiltfive::Error::kInvalidArgument;
        }

        T5_Glasses handle;
        T5_Result err = t5CreateGlasses(client->mContext, identifier.c_str(), &handle);

        if (err) {
            return static_cast<Error>(err);
        }

        std::shared_ptr<Glasses> glasses{new Glasses(identifier, client)};

        glasses->mGlasses = handle;

        return glasses;
    }

public:
    /// \brief Obtain a hardware (not user facing) identifier for the glasses
    ///
    /// Guaranteed to be stable for the duration of the glasses connection.
    ///
    /// Not guaranteed to be stable between sessions. Commonly it will be, however if a user
    /// de-registers their glasses and re-registers them the identifier may change. IE - It's a
    /// local semi-stable software identifier, not a persistent hardware identifier.
    ///
    /// \return Non user-facing identifier for the glasses.
    [[nodiscard]] auto getIdentifier() const -> std::string {
        return mIdentifier;
    }

    /// \brief Get the current connection state of the glasses
    ///
    /// \return ConnectionState representing the current connection state.
    auto getConnectionState() -> Result<ConnectionState> {
        T5_ConnectionState connectionState;
        T5_Result err = t5GetGlassesConnectionState(mGlasses, &connectionState);
        if (err != T5_SUCCESS) {
            return static_cast<Error>(err);
        }

        switch (connectionState) {
            case kT5_ConnectionState_NotExclusivelyConnected:
                return ConnectionState::kNotExclusivelyConnected;

            case kT5_ConnectionState_ExclusiveReservation:
                return ConnectionState::kReserved;

            case kT5_ConnectionState_ExclusiveConnection:
                return ConnectionState::kConnected;

            case kT5_ConnectionState_Disconnected:
                return ConnectionState::kDisconnected;

            default:
                return Error::kInternalError;
        }
    }

    /// \brief Get a system-wide list of changed parameters
    ///
    /// This function doesn't return the values of the changed parameters, but vector of the
    /// parameters that have changed since this function was last called. Note that as a result,
    /// the first call to this function will always result in a count of 0.
    ///
    /// \return A std::vector of T5_ParamGlasses containing the changed parameters
    auto getChangedParams() -> Result<std::vector<T5_ParamGlasses>> {
        uint16_t changedParamsCount = 32;
        std::vector<T5_ParamGlasses> changedParamsBuffer(changedParamsCount);

        changedParamsBuffer.resize(changedParamsCount);
        T5_Result err =
            t5GetChangedGlassesParams(mGlasses, changedParamsBuffer.data(), &changedParamsCount);

        for (;;) {
            if (!err) {
                changedParamsBuffer.resize(changedParamsCount);
                return changedParamsBuffer;

            } else if (err == T5_ERROR_OVERFLOW) {
                changedParamsCount = changedParamsBuffer.size() * 2;
                continue;

            } else {
                return static_cast<Error>(err);
            }
        }
    }

    /// \brief Get the current IPD setting for this glasses
    ///
    /// The value of the <a href="https://en.wikipedia.org/wiki/Pupillary_distance">IPD</a> is user
    /// specified in the Tilt Five™ UI. Clients should query this parameter and adjust their
    /// rendering appropriately to provide a comfortable user experience.
    ///
    /// \return Current IPD in meters.
    auto getIpd() -> Result<double> {
        double value  = 0;
        T5_Result err = t5GetGlassesFloatParam(mGlasses, 0, kT5_ParamGlasses_Float_IPD, &value);
        if (!err) {
            return value;
        } else {
            return static_cast<Error>(err);
        }
    }

    /// \brief Get the user-facing name of the glasses
    ///
    /// The value of the friendly name is user specified in the Tilt Five™ UI.
    ///
    /// This can be changed by the user during a session, and as such must not be used as any kind
    /// of key. Use getIdentifier() instead.
    ///
    /// \return UTF8 encoded friendly name.
    auto getFriendlyName() -> Result<std::string> {
        std::unique_ptr<char[]> value(new char[T5_MAX_STRING_PARAM_LEN]);
        size_t size   = T5_MAX_STRING_PARAM_LEN;
        T5_Result err = t5GetGlassesUtf8Param(
            mGlasses, 0, kT5_ParamGlasses_UTF8_FriendlyName, value.get(), &size);
        if (!err) {
            return std::string(value.get());
        } else {
            return static_cast<Error>(err);
        }
    }

    /// \brief Reserve glasses for exclusive operations by the client.
    ///
    /// Although several operations can be performed without acquiring an exclusive lock on glasses,
    /// there are a few for which an exclusive lock is required. Primarily, the ability to get poses
    /// (getLatestGlassesPose()) and send frames (sendFrame()). To reserve glasses for exclusive
    /// use, use this function.
    ///
    /// Clients may request glasses that aren't fully available yet (e.g. a device that isn't fully
    /// booted, or needs to be rebooted to be compatible with the client). That is why there's a
    /// two-step approach here requiring a request for exclusive access first.  To finish preparing
    /// for exclusive operations, use ensureReady().
    ///
    /// Repeated calls to this function should be made until a exclusive reservation is successful
    /// or an error occurs.
    ///
    /// \attention Ensure that the lifetime of the graphics context remains valid for the duration
    /// of the glasses connection.
    ///
    /// \param[in] displayName - string to display for this program in control panel (localized),
    ///                          e.g. "Awesome Game (Player 1)"
    auto reserve(const std::string& displayName) -> Result<void> {
        T5_Result err = t5ReserveGlasses(mGlasses, displayName.c_str());
        if (!err) {
            return kSuccess;
        } else {
            return static_cast<Error>(err);
        }
    }

    /// \brief Ensure that reserved glasses are ready for exclusive operations.
    ///
    /// Ensure that reserved glasses are ready for exclusive operations, such as the ability to get
    /// poses (getLatestGlassesPose()) and send frames (sendFrame()).  To reserve glasses for
    /// exclusive use, see reserve(). This *must* be checked for success prior to exclusive
    /// operations, otherwise those operations will fail.
    ///
    /// In normal operation, this will return successfully or contain the error
    /// tiltfive::Error::kTryAgain.  This should be called until success or an different error is
    /// seen.
    ///
    /// If glasses are not reserved before calling, this will return an error.
    ///
    /// \return Result indicating success or error.
    auto ensureReady() -> Result<void> {
        T5_Result err = t5EnsureGlassesReady(mGlasses);
        if (!err) {
            return kSuccess;
        } else {
            return static_cast<Error>(err);
        }
    }

    /// \brief Release previously-reserved glasses
    ///
    /// Release glasses that were previously reserved for exclusive operations by the client.
    /// After calling this, exclusive operations cannot be used with the glasses unless the
    /// glasses are again reserved and readied.
    ///
    /// \return Result indicating success or error.
    auto release() -> Result<void> {
        T5_Result err = t5ReleaseGlasses(mGlasses);
        if (!err) {
            return kSuccess;
        } else {
            return static_cast<Error>(err);
        }
    }

    /// \brief Get the latest pose for this glasses
    ///
    /// \param[in] usage ::T5_GlassesPoseUsage indicating the intended use for the glasses pose.
    ///
    /// \return ::T5_GlassesPose representing the most recent pose.
    auto getLatestGlassesPose(T5_GlassesPoseUsage usage) -> Result<T5_GlassesPose> {
        T5_GlassesPose pose;
        T5_Result err = t5GetGlassesPose(mGlasses, usage, &pose);

        if (!err) {
            return pose;
        } else {
            return static_cast<Error>(err);
        }
    }

    /// \brief Initialize the glasses for graphics operations.
    ///
    /// \param[in] graphicsApi     - ::T5_GraphicsApi specifying the graphics API for the glasses.
    /// \param[in] graphicsContext - Meaning depends on the graphics API in use.
    auto initGraphicsContext(T5_GraphicsApi graphicsApi, void* graphicsContext) -> Result<void> {
        T5_Result err = t5InitGlassesGraphicsContext(mGlasses, graphicsApi, graphicsContext);
        if (!err) {
            return kSuccess;
        }
        return static_cast<Error>(err);
    }

    /// \brief Configure the wand event stream
    ///
    /// \param[in]  config  - ::T5_WandStreamConfig filled by client to detail configuration
    auto configureCameraStream(T5_CameraStreamConfig config) -> Result<void> {
        T5_Result err = t5ConfigureCameraStreamForGlasses(mGlasses, config);
        if (!err) {
            return kSuccess;
        } else {
            return static_cast<Error>(err);
        }
    }

    /// Get the latest camera image for this glasses
    //
    /// \return ::T5_CamImage representing the most recent tt image.
    auto getFilledCamImageBuffer() -> Result<T5_CamImage> {
        T5_CamImage img;
        T5_Result err = t5GetFilledCamImageBuffer(mGlasses, &img);
        if (!err) {
            return std::move(img);
        } else {
            return static_cast<Error>(err);
        }
    }

    /// Submit a buffer to the camera image stream to hold Camera Frame data.
    //
    /// \param[in] imgBuffer - ::T5_CamImage representing the buffer to be filled.
    auto submitEmptyCamImageBuffer(T5_CamImage* imgBuffer) -> Result<void> {
        T5_Result err = t5SubmitEmptyCamImageBuffer(mGlasses, imgBuffer);
        if (!err) {
            return kSuccess;
        } else {
            return static_cast<Error>(err);
        }
    }

    /// Cancel an image buffer in use by the service for freeing.
    //
    /// \param[in] buffer - A pointer to the buffer to be canceled and released from use by the
    /// service.
    auto cancelCamImageBuffer(uint8_t* buffer) -> Result<void> {
        T5_Result err = t5CancelCamImageBuffer(mGlasses, buffer);
        if (!err) {
            return kSuccess;
        } else {
            return static_cast<Error>(err);
        }
    }

    /// \brief Send a frame to display on the glasses
    ///
    /// \param[in] frameInfo - ::T5_FrameInfo detailing the frame to display.
    auto sendFrame(const T5_FrameInfo* const frameInfo) -> Result<void> {
        T5_Result err = t5SendFrameToGlasses(mGlasses, frameInfo);
        if (!err) {
            return kSuccess;
        } else {
            return static_cast<Error>(err);
        }
    }

    /// \brief Obtain a list of connected wands
    ///
    /// \return std::vector <::T5_WandHandle> representing the available wands.
    auto listWands() -> Result<std::vector<T5_WandHandle>> {
        uint8_t wandCount = 4;
        std::vector<T5_WandHandle> wandBuffer(wandCount);

        for (;;) {
            wandBuffer.resize(wandCount);
            T5_Result err = t5ListWandsForGlasses(mGlasses, wandBuffer.data(), &wandCount);

            if (!err) {
                std::vector<T5_WandHandle> wands;
                wands.reserve(wandCount);

                for (auto i = 0; i < wandCount; i++) {
                    wands.push_back(wandBuffer[i]);
                }

                return wands;

            } else if (err == T5_ERROR_OVERFLOW) {
                wandCount = wandBuffer.size() * 2;
                continue;

            } else {
                return static_cast<Error>(err);
            }
        }
    }

    /// \brief Configure the wand event stream
    ///
    /// \param[in]  config  - ::T5_WandStreamConfig filled by client to detail configuration
    auto configureWandStream(const T5_WandStreamConfig* const config) -> Result<void> {
        T5_Result err = t5ConfigureWandStreamForGlasses(mGlasses, config);
        if (!err) {
            return kSuccess;
        } else {
            return static_cast<Error>(err);
        }
    }

    /// \brief Read from the wands event stream
    ///
    /// The client should repeatedly call this for as long as the wand event
    /// stream is enabled. In any polling period, the client should call this
    /// in a loop until it returns Error::kTimeout to ensure that the queue
    /// is drained.
    ///
    /// The server will continuously write events to the stream, and will not block
    /// for clients that have fallen behind reading the stream. Wand events that
    /// overflow the buffer are discarded. In the event of falling behind the client
    /// should do any static query that it needs to do (such as enumerating
    /// connected wands), and attempt to read the stream faster.
    ///
    /// \param[in]  timeout - Timeout in ms to wait before returning without read.
    auto readWandStream(std::chrono::milliseconds timeout = std::chrono::milliseconds(100))
        -> Result<T5_WandStreamEvent> {
        T5_WandStreamEvent event;

        T5_Result err = t5ReadWandStreamForGlasses(mGlasses, &event, timeout.count());
        if (!err) {
            return event;
        } else {
            return static_cast<Error>(err);
        }
    }

    /// \brief Get a WandStreamHelper
    ///
    /// \ref UsingWandStreamHelper
    /// \return A std::shared_ptr to a WandStreamHelper
    auto getWandStreamHelper() -> std::shared_ptr<WandStreamHelper> {
        auto wandStreamHelper = mWandStreamHelper.lock();
        if (!wandStreamHelper) {
            // needs initialization
            wandStreamHelper  = obtainWandStreamHelper(shared_from_this());
            mWandStreamHelper = wandStreamHelper;
        }
        return wandStreamHelper;
    }

    /// \brief Create a GlassesConnectionHelper
    ///
    /// \ref UsingGlassesConnectionHelper
    ///
    /// \param[in]  displayName            - The user facing display name for this instance.
    /// \param[in]  connectionPollInterval - Period between attempts to connect.
    /// \return A std::unique_ptr to a GlassesConnectionHelper
    auto createConnectionHelper(
        const std::string& displayName,
        std::chrono::milliseconds connectionPollInterval = std::chrono::milliseconds(100))
        -> std::unique_ptr<GlassesConnectionHelper> {

        return obtainGlassesConnectionHelper(
            shared_from_this(), displayName, connectionPollInterval);
    }

    /// \cond DO_NOT_DOCUMENT
    virtual ~Glasses() {
        // Disconnect the glasses if they're connected
        auto connectionState = getConnectionState();
        if (!connectionState) {
            return;
        }

        if (mGlasses) {
            t5DestroyGlasses(&mGlasses);
            mGlasses = nullptr;
        }
    }
    /// \endcond
};

/// \brief Utility class to automate the Glasses exclusive connection process
///
/// See \ref UsingGlassesConnectionHelper for usage.
class GlassesConnectionHelper {
private:
    const std::shared_ptr<Glasses> mGlasses;
    const std::string mDisplayName;
    const std::chrono::milliseconds mConnectionPollInterval;
    const std::chrono::milliseconds mConnectedPollInterval = mConnectionPollInterval * 10;

    std::atomic<bool> mRunning{true};
    std::thread mThread;

    std::mutex mLastAsyncErrorMtx;
    std::atomic<std::error_code> mLastAsyncError{};

    void setLastAsyncError(std::error_code err) {
        std::lock_guard<std::mutex> lock(mLastAsyncErrorMtx);
        mLastAsyncError = err;
    }

    void threadMain() {
        while (mRunning) {
            auto connectionState = mGlasses->getConnectionState();
            if (!connectionState) {
                setLastAsyncError(connectionState.error());
                std::this_thread::sleep_for(mConnectionPollInterval);
                continue;
            }

            switch (*connectionState) {
                case ConnectionState::kNotExclusivelyConnected: {
                    // Attempt to connect
                    auto result = mGlasses->reserve(mDisplayName);
                    if (!result) {
                        setLastAsyncError(result.error());
                    }
                    // No action on success - the next call to getConnectionState() will
                    // detect the change

                    break;
                }

                case ConnectionState::kReserved:
                case ConnectionState::kDisconnected: {
                    auto result = mGlasses->ensureReady();
                    if (!result) {
                        setLastAsyncError(result.error());
                    }
                    // No action on success - the next call to getConnectionState() will
                    // detect the change

                    break;
                }

                case ConnectionState::kConnected:
                    // If we're connected, increase polling interval to reduce excessive
                    // connections state queries (at the expense of slowing detection of
                    // disconnected devices).
                    std::this_thread::sleep_for(mConnectedPollInterval);
                    break;
            }

            std::this_thread::sleep_for(mConnectionPollInterval);
        }
    }

    friend auto obtainGlassesConnectionHelper(std::shared_ptr<Glasses> glasses,
                                              const std::string& displayName,
                                              std::chrono::milliseconds connectionPollInterval)
        -> std::unique_ptr<GlassesConnectionHelper>;

    explicit GlassesConnectionHelper(std::shared_ptr<Glasses> glasses,
                                     std::string displayName,
                                     std::chrono::milliseconds connectionPollInterval)
        : mGlasses(std::move(glasses))
        , mDisplayName{std::move(displayName)}
        , mConnectionPollInterval(connectionPollInterval) {

        mThread = std::thread(&GlassesConnectionHelper::threadMain, this);
    }

public:
    /// \brief Obtain a reference to the wrapped tiltfive::Glasses object
    [[nodiscard]] auto glasses() -> Glasses& {
        return *mGlasses;
    }

    /// \brief Block until a connection is established
    auto awaitConnection() -> Result<void> {
        auto connectionState = mGlasses->getConnectionState();
        if (!connectionState) {
            return connectionState.error();
        }

        while (*connectionState != ConnectionState::kConnected) {
            std::this_thread::sleep_for(mConnectionPollInterval);

            connectionState = mGlasses->getConnectionState();
            if (!connectionState) {
                return connectionState.error();
            }
        }

        return kSuccess;
    }

    /// \brief Block until a connection is established or timed out
    ///
    /// \param[in]  timeout - Time to wait for connection before timeout
    auto awaitConnection(const std::chrono::milliseconds timeout) -> Result<void> {
        auto start = std::chrono::steady_clock::now();

        auto connectionState = mGlasses->getConnectionState();
        if (!connectionState) {
            return connectionState.error();
        }

        while (*connectionState != ConnectionState::kConnected) {
            if ((std::chrono::steady_clock::now() - start) > timeout) {
                return Error::kTimeout;
            }

            std::this_thread::sleep_for(mConnectionPollInterval);

            connectionState = mGlasses->getConnectionState();
            if (!connectionState) {
                return connectionState.error();
            }
        }

        return kSuccess;
    }

    /// \brief Obtain and consume the last asynchronous error
    ///
    /// The connection process may produce errors asynchronously which can be detected by calling
    /// this.
    ///
    /// \return The last known error or a default std::error_code if no error was present
    auto consumeLastAsyncError() -> std::error_code {
        std::lock_guard<std::mutex> lock(mLastAsyncErrorMtx);
        return mLastAsyncError.exchange({});
    }

    /// \cond DO_NOT_DOCUMENT
    virtual ~GlassesConnectionHelper() {
        mRunning = false;
        if (mThread.joinable()) {
            mThread.join();
        }
    }
    /// \endcond
};

/// \brief Utility class to manage the wand stream
///
/// De-multiplexes the wand stream into abstract tiltfive::Wand
/// objects that can be independently queried for their last
/// known state.
///
/// See \ref UsingWandStreamHelper for usage.
class WandStreamHelper : public std::enable_shared_from_this<WandStreamHelper> {
private:
    friend Wand;

    const std::shared_ptr<Glasses> mGlasses;
    const std::chrono::milliseconds mPollTimeout;

    std::atomic<bool> mWandListDirty{true};
    std::mutex mWandListMtx;  // guards access to mWandList
    std::vector<T5_WandHandle> mWandList;

    std::atomic<bool> mRunning{true};
    std::thread mThread;

    std::mutex mLastWandReportsMtx;  // guards access to mLastWandReports
    std::map<T5_WandHandle, T5_WandReport> mLastWandReports;

    std::mutex mLastAsyncErrorMtx;
    std::atomic<std::error_code> mLastAsyncError{};

    void setLastAsyncError(std::error_code err) {
        std::lock_guard<std::mutex> lock(mLastAsyncErrorMtx);
        mLastAsyncError = err;
    }

    auto drainStream(const std::shared_ptr<Glasses>& glasses) -> Result<void> {
        while (mRunning) {
            auto result = glasses->readWandStream(mPollTimeout);
            if (!result) {
                return result.error();
            }

            std::lock_guard<std::mutex> lock{mLastWandReportsMtx};

            // Process the event
            switch (result->type) {
                case kT5_WandStreamEventType_Connect:
                    mLastWandReports[result->wandId] = {};
                    mWandListDirty                   = true;
                    break;

                case kT5_WandStreamEventType_Disconnect:
                    mLastWandReports.erase(result->wandId);
                    mWandListDirty = true;
                    break;

                case kT5_WandStreamEventType_Desync:
                    mWandListDirty = true;
                    break;

                case kT5_WandStreamEventType_Report:
                    mLastWandReports[result->wandId] = result->report;
                    break;
            }
        }

        return Error::kUnavailable;
    }

    // Update the reports map based on the latest wand list.
    // Ensure empty reports are populated for newly-connected wands.
    // Remove reports for wands that are no longer connected.
    //
    // PRECONDITIONS: Wand list mutex must be held.
    auto refreshReports() -> void {
        std::lock_guard<std::mutex> lock{mLastWandReportsMtx};

        // Obtain a set of the wand handles held in mLastWandReports
        std::set<T5_WandHandle> lastWandReportKeys;
        std::transform(mLastWandReports.cbegin(),
                       mLastWandReports.cend(),
                       std::inserter(lastWandReportKeys, lastWandReportKeys.end()),
                       [](std::pair<T5_WandHandle, T5_WandReport> pair) { return pair.first; });

        // Remove from the list all connected wands and add empty reports for new wands.
        for (const auto& connectedWand : mWandList) {
            lastWandReportKeys.erase(connectedWand);
            mLastWandReports.insert({connectedWand, T5_WandReport{}});
        }

        // The remainder of the list is wand reports for disconnected wands - remove them
        for (auto defunctKey : lastWandReportKeys) {
            mLastWandReports.erase(defunctKey);
        }
    }

    void threadMain() {
        T5_WandStreamConfig streamConfig{true};
        bool configured = false;

        while (mRunning) {
            // Configure the stream if we haven't already
            if (!configured) {
                auto configureRequest = mGlasses->configureWandStream(&streamConfig);
                if (!configureRequest) {
                    setLastAsyncError(configureRequest.error());
                    std::this_thread::sleep_for(std::chrono::milliseconds(20));
                    continue;
                }
                configured = true;
            }

            // Drain the stream
            auto result = drainStream(mGlasses);
            if ((result.error() != tiltfive::Error::kTimeout) &&
                (result.error() != tiltfive::Error::kUnavailable)) {

                // For errors other than timeout, record it, small delay and loop
                setLastAsyncError(result.error());
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
            }
        }

        // Disable the stream
        streamConfig.enabled  = false;
        auto configureRequest = mGlasses->configureWandStream(&streamConfig);
        if (!configureRequest) {
            setLastAsyncError(configureRequest.error());
        }

        // Flag as no longer running if we've exited due to error
        mRunning = false;
    }

    friend inline auto obtainWandStreamHelper(std::shared_ptr<Glasses> glasses,
                                              std::chrono::milliseconds pollTimeout)
        -> std::shared_ptr<WandStreamHelper>;

    explicit WandStreamHelper(
        std::shared_ptr<Glasses> glasses,
        std::chrono::milliseconds pollTimeout = std::chrono::milliseconds(100))
        : mGlasses(std::move(glasses)), mPollTimeout(pollTimeout) {

        mThread = std::thread(&WandStreamHelper::threadMain, this);
    }

    auto getLatestReport(const T5_WandHandle& handle) -> Result<T5_WandReport> {
        std::lock_guard<std::mutex> lock{mLastWandReportsMtx};

        auto report = mLastWandReports.find(handle);
        if (report == mLastWandReports.end()) {
            return tiltfive::Error::kTargetNotFound;
        }

        return report->second;
    };

public:
    /// \brief Obtain and consume the last asynchronous error
    ///
    /// The connection process may produce errors asynchronously which can
    /// be detected by calling this.
    ///
    /// \return The last known error or a default std::error_code if no error
    /// was present
    auto consumeLastAsyncError() -> std::error_code {
        std::lock_guard<std::mutex> lock(mLastAsyncErrorMtx);
        return mLastAsyncError.exchange({});
    }

    /// \brief Obtain a list of tiltfive::Wand
    ///
    /// The tiltfive::WandStreamHelper maintains a stream reader thread
    /// while there are any outstanding references to the returned
    /// tiltfive::Wand objects, automatically terminating the stream
    /// when they go out of scope.
    auto listWands() -> Result<std::vector<std::shared_ptr<Wand>>> {
        std::lock_guard<std::mutex> lock{mWandListMtx};

        // Update the wand list if it's been invalidated
        if (mWandListDirty.exchange(false)) {
            auto result = mGlasses->listWands();
            if (!result) {
                mWandListDirty = true;
                return result.error();
            }

            std::vector<T5_WandHandle> wandHandles;
            for (auto wandHandle : *result) {
                wandHandles.push_back(wandHandle);
            }
            mWandList = wandHandles;

            refreshReports();
        }

        // Realize wand list
        std::vector<std::shared_ptr<Wand>> wands;
        for (auto wandHandle : mWandList) {
            wands.push_back(obtainWand(wandHandle, shared_from_this()));
        }

        return wands;
    };

    /// \cond DO_NOT_DOCUMENT
    virtual ~WandStreamHelper() {
        mRunning = false;
        if (mThread.joinable()) {
            mThread.join();
        }
    }
    /// \endcond
};

/// \brief Virtual base class for use with tiltfive::ParamChangeHelper
class ParamChangeListener {
public:
    /// \brief Called by a tiltfive::ParamChangeHelper when system-wide (::T5_ParamSys) params
    /// have changed
    virtual auto onSysParamChanged(const std::vector<T5_ParamSys>& changed) -> void = 0;

    /// \brief Called by a tiltfive::ParamChangeHelper when glasses specific (::T5_ParamGlasses)
    /// params have changed
    virtual auto onGlassesParamChanged(const std::shared_ptr<Glasses>& glasses,
                                       const std::vector<T5_ParamGlasses>& changed) -> void = 0;

    /// \cond DO_NOT_DOCUMENT
    virtual ~ParamChangeListener() = default;
    /// \endcond
};

/// \brief Utility class to track changes to parameters
/// \ref UsingParamChangeHelper
class ParamChangeHelper {
private:
    const std::shared_ptr<Client> mClient;
    const std::weak_ptr<ParamChangeListener> mChangeListener;

    static constexpr size_t kDefaultSettingBufferSize = 16;

    std::mutex mRegisteredGlassesMtx;
    std::set<std::shared_ptr<Glasses>> mRegisteredGlasses;

    std::vector<T5_ParamSys> mChangedSysParams;
    std::vector<T5_ParamGlasses> mChangedGlassesParams;

    std::chrono::milliseconds mPollInterval;

    std::thread mThread;
    std::atomic<bool> mRunning{true};

    std::mutex mLastAsyncErrorMtx;
    std::atomic<std::error_code> mLastAsyncError{};

    void setLastAsyncError(std::error_code err) {
        std::lock_guard<std::mutex> lock(mLastAsyncErrorMtx);
        mLastAsyncError = err;
    }

    friend auto obtainParamChangeHelper(std::shared_ptr<Client> client,
                                        std::weak_ptr<ParamChangeListener> listener,
                                        std::chrono::milliseconds pollInterval)
        -> std::unique_ptr<ParamChangeHelper>;

    ParamChangeHelper(std::shared_ptr<Client> client,
                      std::weak_ptr<ParamChangeListener> listener,
                      std::chrono::milliseconds pollInterval)
        : mClient(std::move(client))
        , mChangeListener(std::move(listener))
        , mPollInterval(pollInterval) {

        mThread = std::thread(&ParamChangeHelper::threadMain, this);
    }

    auto checkGlassesParams(const std::shared_ptr<ParamChangeListener>& listener) -> void {
        std::lock_guard<std::mutex> lock(mRegisteredGlassesMtx);
        for (const auto& glasses : mRegisteredGlasses) {
            checkGlassesParams(glasses, listener);
        }
    }

    auto checkGlassesParams(const std::shared_ptr<Glasses>& glasses,
                            const std::shared_ptr<ParamChangeListener>& listener) -> void {
        uint16_t changeCount;

        mChangedGlassesParams.resize(kDefaultSettingBufferSize);
        for (;;) {
            changeCount   = mChangedGlassesParams.size();
            T5_Result err = t5GetChangedGlassesParams(
                glasses->mGlasses, mChangedGlassesParams.data(), &changeCount);

            if (!err) {
                if (changeCount > 0) {
                    mChangedGlassesParams.resize(changeCount);
                    listener->onGlassesParamChanged(glasses, mChangedGlassesParams);
                }
                break;
            }

            // Error - increase buffer if we overflowed, or record the error and exit
            if (err == T5_ERROR_OVERFLOW) {
                mChangedSysParams.resize(mChangedSysParams.size() * 2);
                continue;
            }

            setLastAsyncError(static_cast<Error>(err));
            break;
        }
    }

    auto checkSysParams(const std::shared_ptr<ParamChangeListener>& listener) -> void {
        uint16_t changeCount;

        mChangedSysParams.resize(kDefaultSettingBufferSize);
        for (;;) {
            changeCount = mChangedSysParams.size();
            T5_Result err =
                t5GetChangedSystemParams(mClient->mContext, mChangedSysParams.data(), &changeCount);

            if (!err) {
                if (changeCount > 0) {
                    mChangedSysParams.resize(changeCount);
                    listener->onSysParamChanged(mChangedSysParams);
                }
                break;
            }

            // Error - increase buffer if we overflowed, or record the error and exit
            if (err == T5_ERROR_OVERFLOW) {
                mChangedSysParams.resize(mChangedSysParams.size() * 2);
                continue;
            }

            setLastAsyncError(static_cast<Error>(err));
            break;
        }
    }

    auto threadMain() -> void {
        while (mRunning) {
            // Listener weak_ptr -> shared_ptr or exit
            {
                auto listener = mChangeListener.lock();
                if (!listener) {
                    break;
                }

                checkGlassesParams(listener);

                checkSysParams(listener);
            }

            std::this_thread::sleep_for(mPollInterval);
        }
    }

public:
    /// \cond DO_NOT_DOCUMENT
    virtual ~ParamChangeHelper() {
        mRunning = false;
        if (mThread.joinable()) {
            mThread.join();
        }
    }
    /// \endcond

    /// \brief Obtain and consume the last asynchronous error
    ///
    /// The connection process may produce errors asynchronously which can
    /// be detected by calling this.
    ///
    /// \return The last known error or a default std::error_code if no error
    /// was present
    auto consumeLastAsyncError() -> std::error_code {
        std::lock_guard<std::mutex> lock(mLastAsyncErrorMtx);
        return mLastAsyncError.exchange({});
    }

    /// \brief Register glasses for parameter change tracking
    auto registerGlasses(const std::shared_ptr<Glasses>& glasses) -> void {
        std::lock_guard<std::mutex> lock(mRegisteredGlassesMtx);
        mRegisteredGlasses.insert(glasses);
    }

    /// \brief De-register glasses for parameter change tracking
    auto deregisterGlasses(const std::shared_ptr<Glasses>& glasses) -> void {
        std::lock_guard<std::mutex> lock(mRegisteredGlassesMtx);
        mRegisteredGlasses.erase(glasses);
    }
};

/// \brief Represents an abstract instance of a Tilt Five™ wand
/// Used with tiltfive::WandStreamHelper
class Wand {
private:
    T5_WandHandle mHandle;
    std::shared_ptr<WandStreamHelper> mWandStreamHelper;

    friend WandStreamHelper;

    friend std::ostream& operator<<(std::ostream& os, std::shared_ptr<Wand> const& instance) {
        os << *instance;
        return os;
    }

    friend std::ostream& operator<<(std::ostream& os, Wand const& instance) {
        os << "<Wand:" << +instance.mHandle << ">";
        return os;
    }

    friend auto obtainWand(T5_WandHandle handle, std::shared_ptr<WandStreamHelper> wandStreamHelper)
        -> std::shared_ptr<Wand>;

    Wand(T5_WandHandle handle, std::shared_ptr<WandStreamHelper> wandStreamHelper)
        : mHandle(handle), mWandStreamHelper(std::move(wandStreamHelper)) {}

public:
    /// \brief Get the latest wand report for this wand
    auto getLatestReport() const -> Result<T5_WandReport> {
        return mWandStreamHelper->getLatestReport(mHandle);
    }

    /// \brief Get the wand handle
    [[nodiscard]] T5_WandHandle handle() const {
        return mHandle;
    }
};

/// \brief Obtain an instance of the Tilt Five™ API client
///
/// \param[in] applicationId      - Application ID. Refer to T5 docs for format.
/// \param[in] applicationVersion - Application version. Refer to T5 docs for format.
/// \param[in] platformContext    - Platform specific context. Refer to T5 docs for format.
/// \param[in] sdkType            - Internal type. Leave at default value unless otherwise
///                                 instructed by T5 staff.
///
/// \return Instance of the Tilt Five™ API client or error.
inline auto obtainClient(const std::string& applicationId,
                         const std::string& applicationVersion,
                         void* platformContext,
                         const uint8_t sdkType = 0) -> Result<std::shared_ptr<Client>> {

    return Client::create(applicationId, applicationVersion, platformContext, sdkType);
}

/// \brief Obtain an instance of the Tilt Five™ Glasses
///
/// \param[in] identifier         - Unique identifier for the glasses from listGlasses()
/// \param[in] client             - Instance of the Tilt Five™ API client
///
/// \return Instance of the Tilt Five™ API Glasses or error.
inline auto obtainGlasses(const std::string& identifier, const std::shared_ptr<Client>& client)
    -> Result<std::shared_ptr<Glasses>> {
    return Glasses::create(identifier, client);
}

/// Internal utility function - Do not call directly
inline auto obtainWandStreamHelper(std::shared_ptr<Glasses> glasses,
                                   std::chrono::milliseconds pollTimeout)
    -> std::shared_ptr<WandStreamHelper> {
    return std::shared_ptr<WandStreamHelper>(new WandStreamHelper(std::move(glasses), pollTimeout));
}

/// Internal utility function - Do not call directly
inline auto obtainWand(T5_WandHandle handle, std::shared_ptr<WandStreamHelper> wandStreamHelper)
    -> std::shared_ptr<Wand> {
    return std::shared_ptr<Wand>(new Wand(handle, std::move(wandStreamHelper)));
}

/// Internal utility function - Do not call directly
inline auto obtainGlassesConnectionHelper(std::shared_ptr<Glasses> glasses,
                                          const std::string& displayName,
                                          std::chrono::milliseconds connectionPollInterval)
    -> std::unique_ptr<GlassesConnectionHelper> {

    return std::unique_ptr<GlassesConnectionHelper>(
        new GlassesConnectionHelper(std::move(glasses), displayName, connectionPollInterval));
}

/// Internal utility function - Do not call directly
inline auto obtainParamChangeHelper(std::shared_ptr<Client> client,
                                    std::weak_ptr<ParamChangeListener> listener,
                                    std::chrono::milliseconds pollInterval)
    -> std::unique_ptr<ParamChangeHelper> {

    return std::unique_ptr<ParamChangeHelper>(
        new ParamChangeHelper(std::move(client), std::move(listener), pollInterval));
}
/// \endcond

/// \}

}  // namespace tiltfive

/// \defgroup ostreamFormatters std::ostream formatters

/// \brief Support for writing ::T5_WandReport to an std::ostream
/// \ingroup ostreamFormatters
inline std::ostream& operator<<(std::ostream& os, const T5_WandReport& instance) {
    // Print the validity flags
    os << "[" << (instance.analogValid ? "A" : "_") << (instance.buttonsValid ? "B" : "_")
       << (instance.poseValid ? "P" : "_") << "]";

    if (instance.analogValid) {
        os << "[A: " << std::right << std::fixed << std::setw(10) << instance.stick.x << "x"
           << std::right << std::fixed << std::setw(10) << instance.stick.y << " | " << std::right
           << std::fixed << std::setw(10) << instance.trigger << "]";
    } else {
        os << "[A: Invalid]";
    }

    if (instance.buttonsValid) {
        os << "[B: " << (instance.buttons.t5 ? "T" : "_") << (instance.buttons.one ? "1" : "_")
           << (instance.buttons.two ? "2" : "_") << (instance.buttons.three ? "3" : "_")
           << (instance.buttons.a ? "A" : "_") << (instance.buttons.b ? "B" : "_")
           << (instance.buttons.x ? "X" : "_") << (instance.buttons.y ? "Y" : "_") << "]";
    } else {
        os << "[B: Invalid]";
    }

    if (instance.poseValid) {
        os << "[P: (" << std::right << std::fixed << std::setw(10) << instance.posGrip_GBD.x << ","
           << std::right << std::fixed << std::setw(10) << instance.posGrip_GBD.y << ","
           << std::right << std::fixed << std::setw(10) << instance.posGrip_GBD.z << ") ("
           << std::right << std::fixed << std::setw(10) << instance.rotToWND_GBD.w << ","
           << std::right << std::fixed << std::setw(10) << instance.rotToWND_GBD.x << ","
           << std::right << std::fixed << std::setw(10) << instance.rotToWND_GBD.y << ","
           << std::right << std::fixed << std::setw(10) << instance.rotToWND_GBD.z << ")"
           << "]";
    }

    return os;
}

/// \brief Support for writing ::T5_GlassesPose to an std::ostream
/// \ingroup ostreamFormatters
inline std::ostream& operator<<(std::ostream& os, const T5_GlassesPose& instance) {
    std::string gameboardType;
    switch (instance.gameboardType) {
        case kT5_GameboardType_None:
            gameboardType = "None";
            break;
        case kT5_GameboardType_LE:
            gameboardType = "LE";
            break;
        case kT5_GameboardType_XE:
            gameboardType = "XE";
            break;
        case kT5_GameboardType_XE_Raised:
            gameboardType = "XE (Raised)";
            break;
        default:
            // Shouldn't happen unless there's some bad casting going on elsewhere.
            gameboardType = std::string("[Invalid T5_GameboardType : ") +
                            std::to_string(static_cast<int>(instance.gameboardType)) +
                            std::string("]");
            break;
    }

    os << "[" << instance.timestampNanos << "| " << gameboardType << " (" << std::right
       << std::fixed << std::setw(10) << instance.posGLS_GBD.x << "," << std::right << std::fixed
       << std::setw(10) << instance.posGLS_GBD.y << "," << std::right << std::fixed << std::setw(10)
       << instance.posGLS_GBD.z << ") (" << std::right << std::fixed << std::setw(10)
       << instance.rotToGLS_GBD.w << "," << std::right << std::fixed << std::setw(10)
       << instance.rotToGLS_GBD.x << "," << std::right << std::fixed << std::setw(10)
       << instance.rotToGLS_GBD.y << "," << std::right << std::fixed << std::setw(10)
       << instance.rotToGLS_GBD.z << ")"
       << "]";

    return os;
}

/// \brief Support for writing ::T5_ParamSys to an std::ostream
/// \ingroup ostreamFormatters
inline std::ostream& operator<<(std::ostream& os, const T5_ParamSys& instance) {
    switch (instance) {
        case kT5_ParamSys_UTF8_Service_Version:
            os << "Service Version";
            break;

        case kT5_ParamSys_Integer_CPL_AttRequired:
            os << "UI Attention Required";
            break;

        default:
            // Shouldn't happen unless there's some bad casting going on elsewhere.
            os << "[Invalid T5_ParamSys : " << static_cast<int>(instance) << "]";
            break;
    }

    return os;
}

/// \brief Support for writing ::T5_ParamGlasses to an std::ostream
/// \ingroup ostreamFormatters
inline std::ostream& operator<<(std::ostream& os, const T5_ParamGlasses& instance) {
    switch (instance) {
        case kT5_ParamGlasses_Float_IPD:
            os << "IPD";
            break;

        case kT5_ParamGlasses_UTF8_FriendlyName:
            os << "Friendly Name";
            break;

        default:
            // Shouldn't happen unless there's some bad casting going on elsewhere.
            os << "[Invalid T5_ParamGlasses : " << static_cast<int>(instance) << "]";
            break;
    }

    return os;
}

/// \}
