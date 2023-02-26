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
/// \brief C common types for the Tilt Five™ API

#ifndef __cplusplus
#include <stdbool.h>
#include <stdint.h>
#else
#include <cstdint>
#endif

//////////////////////////////////////////////////////////
////                  Common Types                    ////
//////////////////////////////////////////////////////////

/// \defgroup C_Common_Types Tilt Five™ Common Types
/// Common types for use with C and C++ interface
/// \{

/// \brief The maximum number of characters allowed for string values
#define T5_MAX_STRING_PARAM_LEN (260)

/// \brief The minimum width required for camera image buffers
#define T5_MIN_CAM_IMAGE_BUFFER_WIDTH (768)

/// \brief The minimum height required for camera image buffers
#define T5_MIN_CAM_IMAGE_BUFFER_HEIGHT (600)

/// \brief 2D vector
typedef struct {
    float x;
    float y;
} T5_Vec2;

/// \brief 3D vector
typedef struct {
    float x;
    float y;
    float z;
} T5_Vec3;

/// \brief Quaternion
typedef struct {
    float w;
    float x;
    float y;
    float z;
} T5_Quat;

/// \brief Opaque handle used with system-wide functions
///
/// Obtained from t5GetContext().<br/>
/// Release with t5ReleaseContext().
typedef struct T5_ContextImpl* T5_Context;

/// \brief Opaque handle used with glasses
///
/// Obtained from ::t5CreateGlasses().<br/>
/// Released with ::t5DestroyGlasses().
typedef struct T5_GlassesImpl* T5_Glasses;

/// \brief Opaque handle used with wands
///
/// Obtained from t5ListWandsForGlasses().<br/>
/// Release not currently required.
typedef uint8_t T5_WandHandle;

/// \brief Graphics API types
typedef enum {
    /// \brief No graphics API (for clients that don't send frames)
    kT5_GraphicsApi_None = 1,

    /// \brief OpenGL
    kT5_GraphicsApi_GL = 2,

    /// \brief Direct3D 11 (Windows Only)
    kT5_GraphicsApi_D3D11 = 3,
} T5_GraphicsApi;

/// \brief Possible gameboard types
typedef enum {
    /// \brief No gameboard
    kT5_GameboardType_None = 1,

    /// \brief An LE gameboard
    kT5_GameboardType_LE = 2,

    /// \brief An XE gameboard, flap laid flat
    kT5_GameboardType_XE = 3,

    /// \brief An XE gameboard, flap raised at an angle on the kickstand
    kT5_GameboardType_XE_Raised = 4,
} T5_GameboardType;

/// \brief Physical dimensions of a gameboard.
typedef struct {
    /// \brief The distance in meters from the gameboard origin to the edge of the viewable area in
    /// the positive X direction.
    float viewableExtentPositiveX;

    /// \brief The distance in meters from the gameboard origin to the edge of the viewable area in
    /// the negative X direction.
    float viewableExtentNegativeX;

    /// \brief The distance in meters from the gameboard origin to the edge of the viewable area in
    /// the positive Y direction.
    float viewableExtentPositiveY;

    /// \brief The distance in meters from the gameboard origin to the edge of the viewable area in
    /// the negative Y direction.
    float viewableExtentNegativeY;

    /// \brief The distance in meters above the gameboard origin that the viewable area extends in
    /// the positive Z direction.
    float viewableExtentPositiveZ;
} T5_GameboardSize;

/// \brief Client provided information for use with t5CreateGlasses()
typedef struct {
    /// \brief The application ID.
    const char* applicationId;

    /// \brief The application version.
    const char* applicationVersion;

    /// \brief The SDK type.
    ///
    /// Should be set to 0x00 unless otherwise instructed by T5 staff.
    uint8_t sdkType;

    /// \brief RESERVED: Must be set to 0
    uint64_t reserved;
} T5_ClientInfo;

/// \brief Glasses connection state
typedef enum {
    /// \brief Glasses are connected for exclusive use
    kT5_ConnectionState_ExclusiveConnection = 1,

    /// \brief Glasses are reserved for exclusive use
    kT5_ConnectionState_ExclusiveReservation = 2,

    /// \brief Glasses have not been exclusively connected or reserved
    kT5_ConnectionState_NotExclusivelyConnected = 3,

    /// \brief Glasses were previously exclusively connected, but the device has disconnected
    kT5_ConnectionState_Disconnected = 4,
} T5_ConnectionState;

/// \brief Glasses pose usage indicator
typedef enum {
    /// \brief The pose will be used to render images to be presented on the glasses.
    ///
    /// Querying a glasses pose for this usage will return a pose prediction intended to account for
    /// the render and presentation latency. The predicted pose is prone to include errors, and the
    /// rendered images may appear very jittery if they are displayed on a device other than the
    /// glasses. When displayed via the glasses, the on-glasses image stabilization compensates for
    /// this prediction error, so the image should not appear jittery.
    kT5_GlassesPoseUsage_GlassesPresentation = 1,

    /// \brief The pose will be used to render images to be presented on a device other than the
    /// glasses, such at the host system's primary display.
    ///
    /// Querying a glasses pose for this usage will return a pose with less noise than that intended
    /// for presentation via the glasses.
    kT5_GlassesPoseUsage_SpectatorPresentation = 2,
} T5_GlassesPoseUsage;

/// \brief Glasses pose information to be retrieved with t5GetGlassesPose()
///
/// The pose describes the relationship between two reference frames: one defined in terms of the
/// glasses, and the other in terms of the gameboard. Both reference frames are right-handed. The
/// glasses reference frame, abbreviated as GLS, has its origin at the midpoint between the
/// effective optical position of the projectors. It is oriented such that +X points to the right,
/// +Y points up, and +Z points backward for someone wearing the glasses. The gameboard reference
/// frame, abbreviated GBD, is oriented such that +X points to the right, +Y points forward, and +Z
/// points up from the perspective of a person facing the gameboard on a table from the side of the
/// gameboard with the T5 logo. The origin of the gameboard reference frame is located at the point
/// equidistant from the three gameboard sides nearest to the T5 logo (i.e. the side on which the
/// logo appears and the two adjacent sides). This places the gameboard origin in the center of the
/// square LE gameboard, and off-center along the longer dimension of the rectangular XE gameboard.
typedef struct {
    /// \brief The timestamp of the pose.
    uint64_t timestampNanos;

    /// \brief The position of the origin of the GLS (glasses) frame relative to the GBD (gameboard)
    /// frame.
    T5_Vec3 posGLS_GBD;

    /// \brief The rotation that transforms points in the GBD (gameboard) frame orientation to the
    /// GLS (glasses) frame orientation.
    T5_Quat rotToGLS_GBD;

    /// \brief The type of gameboard visible for this pose
    T5_GameboardType gameboardType;
} T5_GlassesPose;

/// \brief Camera stream configuration
typedef struct {
    /// \brief The index of the camera to be modified.
    uint8_t cameraIndex;

    /// \brief Enable or disable the camera stream. True = enabled
    bool enabled;
} T5_CameraStreamConfig;

/// Render information to be used with t5SendFrameToGlasses()
typedef struct {
    /// \brief Texture handle for the left image.
    ///
    /// The meaning of the handle will depend on the current graphics API.
    ///
    /// \see \ref aboutGraphicsApi for further details.
    void* leftTexHandle;

    /// \brief Texture handle for the right image.
    ///
    /// The meaning of the handle will depend on the current graphics API.
    ///
    /// \see \ref aboutGraphicsApi for further details.
    void* rightTexHandle;

    /// \brief Width of the textures pointed to by leftTexHandle and rightTexHandle.
    uint16_t texWidth_PIX;

    /// \brief Height of the textures pointed to by leftTexHandle and rightTexHandle.
    uint16_t texHeight_PIX;

    /// \brief True if the texture is srgb. This is only relevant for the OpenGL graphics API.
    bool isSrgb;

    /// \brief True if the image is 'upside down'.
    bool isUpsideDown;

    /// \brief The image rectangle in the normalized (z=1) image space of the virtual cameras.
    struct {
        float startX_VCI;
        float startY_VCI;
        float width_VCI;
        float height_VCI;
    } vci;

    /// \brief The rotation from GBD to VC, the virtual camera reference frame for the left eye.
    T5_Quat rotToLVC_GBD;

    /// \brief The position of VC, the virtual camera reference frame, relative to GBD for the left
    /// eye.
    T5_Vec3 posLVC_GBD;

    /// \brief The rotation from GBD to VC, the virtual camera reference frame for the right eye
    T5_Quat rotToRVC_GBD;

    /// \brief The position of VC, the virtual camera reference frame, relative to GBD for the right
    /// eye.
    T5_Vec3 posRVC_GBD;
} T5_FrameInfo;

/// \brief Camera Frame information to be retrieved with t5GetFilledCamImageBuffer()
typedef struct {
    /// \brief The width of the image in the image buffer. Empty buffers should set these parameters
    /// to 0.
    uint16_t imageWidth;

    /// \brief The height of the image in the image buffer. Empty buffers should set these
    /// parameters to 0.
    uint16_t imageHeight;

    /// \brief The stride of the image in the image buffer. Empty buffers should set these
    /// parameters to 0.
    uint16_t imageStride;

    /// \brief The index of the desired camera. 0 for tangible tracking camera, 1 for head tracking
    /// camera.
    uint8_t cameraIndex;

    /// \brief The illumination mode for incoming frames. 0 for unknown frame. 1 for Light frames. 2
    /// for dark frame.
    uint8_t illuminationMode;

    /// \brief The total size of the provided image buffer. Must be at least
    /// T5_MIN_CAM_IMAGE_BUFFER_WIDTH * T5_MIN_CAM_IMAGE_BUFFER_HEIGHT.
    uint32_t bufferSize;

    /// \brief The image buffer being filled by the Tilt Five service.
    uint8_t* pixelData;

    /// \brief The position of the camera relative to the GBD.
    T5_Vec3 posCAM_GBD;

    /// \brief The rotation of the camera relative to the GBD.
    T5_Quat rotToCAM_GBD;
} T5_CamImage;

/// \brief Wand stream configuration
typedef struct {
    /// \brief Enable or disable the entire stream. True = enabled
    bool enabled;

} T5_WandStreamConfig;

/// \brief Wand stream event type
typedef enum {
    /// \brief Wand connected
    kT5_WandStreamEventType_Connect = 1,

    /// \brief Wand disconnected
    kT5_WandStreamEventType_Disconnect = 2,

    /// \brief Stream has desynchronized
    kT5_WandStreamEventType_Desync = 3,

    /// \brief Wand report (Pose, Buttons, Trigger, Stick, Battery)
    kT5_WandStreamEventType_Report = 4,
} T5_WandStreamEventType;

/// \brief Wand hand
typedef enum {
    /// \brief Hand unknown
    kT5_Hand_Unknown = 0,

    /// \brief Left hand
    kT5_Hand_Left = 1,

    /// \brief Right hand
    kT5_Hand_Right = 2,
} T5_Hand;

/// \brief Contains wand related information (Pose, Buttons, Trigger, Stick, Battery)
typedef struct {
    /// \brief The timestamp of the wand event in nanoseconds
    uint64_t timestampNanos;

    /// \brief Validity of analog parameters. True = valid
    bool analogValid;

    /// \brief Validity of battery parameters. True = valid
    bool batteryValid;

    /// \brief Validity of button parameters. True = valid
    bool buttonsValid;

    /// \brief Validity of pose parameters. True = valid
    bool poseValid;

    /// \brief Trigger - Analog, Range [0.0 - 1.0], 1.0 = Fully depressed
    float trigger;

    /// \brief Stick (X/Y) - Analog, Range [-1.0 - 1.0], 0 = Centered, 1.0 = Top/Right
    T5_Vec2 stick;

    // TODO(khunt) : Determine units
    /// Battery
    uint8_t battery;

    /// \brief Buttons state. True = Pressed
    struct {
        bool t5;
        bool one;
        bool two;
        bool three;
        bool a;
        bool b;
        bool x;
        bool y;
    } buttons;

    /// \brief WND/GBD rotation unit quaternion
    //
    /// The rotation unit quaternion that takes points from the GBD (gameboard)
    /// reference frame to the WND (wand) reference frame orientation.
    T5_Quat rotToWND_GBD;

    /// \brief Position (Aim Point) - Vector3f
    T5_Vec3 posAim_GBD;

    /// \brief Position (Fingertips) - Vector3f
    T5_Vec3 posFingertips_GBD;

    /// \brief Position (Grip) - Vector3f
    T5_Vec3 posGrip_GBD;

    /// \brief Wand hand
    T5_Hand hand;
} T5_WandReport;

/// \brief Represents an event from the wand stream
typedef struct {
    /// \brief Opaque identifier for the wand
    T5_WandHandle wandId;

    /// \brief Type of event
    T5_WandStreamEventType type;

    /// \brief The timestamp of the wand event in nanoseconds
    uint64_t timestampNanos;

    /// \brief Report (Valid if type = ::kT5_WandStreamEventType_Report)
    T5_WandReport report;
} T5_WandStreamEvent;

/// \brief Possible parameters that can be retrieved for glasses
typedef enum {
    /// \brief <a href="https://en.wikipedia.org/wiki/Pupillary_distance">Interpupillary
    /// distance</a> - Float, millimeters
    kT5_ParamGlasses_Float_IPD = 1,

    /// User-facing name of the glasses - UTF8
    kT5_ParamGlasses_UTF8_FriendlyName = 6,
} T5_ParamGlasses;

/// \brief Possible parameters that can be retrieved with \ref sys_getParam.
typedef enum {
    /// \brief Version of the service software - UTF8
    kT5_ParamSys_UTF8_Service_Version = 1,

    /// \brief Non-zero if the control panel requires user interaction
    /// (E.g. Important firmware update) - Integer, boolean
    kT5_ParamSys_Integer_CPL_AttRequired = 2,
} T5_ParamSys;

/// \}
