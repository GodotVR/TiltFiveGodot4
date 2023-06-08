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
/// \brief C++ Templated common return type for the Tilt Five™ API

#include <iostream>
#include <string>

namespace tiltfive {

/// Throw when attempting to access a bad result
class BadResultAccess : public std::logic_error {
public:
    BadResultAccess() : logic_error("bad result access") {}
    explicit BadResultAccess(const char* what) : logic_error(what) {}
    ~BadResultAccess() noexcept override = default;
};

/// Templated return type with support for error conditions
template <typename T>
class [[nodiscard]] Result {
public:
    using Value = T;

    Result(Value&& value) noexcept : mValue(std::move(value)), mErrFlags(kErrFlagsNone) {}

    Result(const Value& value) : mValue(value), mErrFlags(kErrFlagsNone) {}

    Result(std::error_code err) noexcept : mErr(err), mErrFlags(kErrFlagHaveErr) {}

    Result(Result&& other) : mErrFlags(other.mErrFlags) {
        if (mErrFlags == kErrFlagsNone) {
            new (&mValue) Value(std::move(other.mValue));
        } else {
            new (&mErr) std::error_code(std::move(other.mErr));
        }
    }

    Result(const Result& other) noexcept : mErrFlags(other.mErrFlags) {
        if (mErrFlags == kErrFlagsNone) {
            new (&mValue) Value(other.mValue);
        } else {
            new (&mErr) std::error_code(other.mErr);
        }
    }

    template <
        typename ErrorCodeEnum,
        typename = typename std::enable_if<std::is_error_code_enum<ErrorCodeEnum>::value>::type>
    Result(ErrorCodeEnum err) noexcept : mErr(err), mErrFlags(kErrFlagHaveErr) {}

    Result(std::errc err) noexcept : mErr(std::make_error_code(err)), mErrFlags(kErrFlagHaveErr) {}

    ~Result() {
        if (mErrFlags == kErrFlagsNone) {
            mValue.~Value();
        } else {
            using std::error_code;
            mErr.~error_code();
        }
    }

    auto operator=(Result&& other) noexcept -> Result& {
        if (mErrFlags == kErrFlagsNone) {
            if (other.mErrFlags == kErrFlagsNone) {
                mValue = std::move(other.mValue);
            } else {
                mValue.~Value();
                new (&mErr) std::error_code(std::move(other.mErr));
            }
        } else {
            if (other.mErrFlags == kErrFlagsNone) {
                using std::error_code;
                mErr.~error_code();
                new (&mValue) Value(std::move(other.mValue));
            } else {
                mErr = std::move(other.mErr);
            }
        }
        mErrFlags = other.mErrFlags;
        return *this;
    }

    auto operator=(const Result& other) -> Result& {
        if (mErrFlags == kErrFlagsNone) {
            if (other.mErrFlags == kErrFlagsNone) {
                mValue = other.mValue;
            } else {
                mValue.~Value();
                new (&mErr) std::error_code(other.mErr);
            }
        } else {
            if (other.mErrFlags == kErrFlagsNone) {
                using std::error_code;
                mErr.~error_code();
                new (&mValue) Value(other.mValue);
            } else {
                mErr = other.mErr;
            }
        }
        mErrFlags = other.mErrFlags;
        return *this;
    }

    explicit operator bool() const noexcept {
        return mErrFlags == kErrFlagsNone;
    }

    auto operator*() -> Value& {
        if (mErrFlags != kErrFlagsNone) {
            throwBadResultAccess();
        }
        return mValue;
    }

    auto operator*() const -> const Value& {
        if (mErrFlags != kErrFlagsNone) {
            throwBadResultAccess();
        }
        return mValue;
    }

    auto operator->() -> Value* {
        if (mErrFlags != kErrFlagsNone) {
            throwBadResultAccess();
        }
        return &mValue;
    }

    auto operator->() const -> const Value* {
        if (mErrFlags != kErrFlagsNone) {
            throwBadResultAccess();
        }
        return &mValue;
    }

    [[nodiscard]] auto error() const noexcept -> std::error_code {
        if (mErrFlags != kErrFlagsNone) {
            return mErr;
        }
        return {};
    }

    [[nodiscard]] auto logged() const noexcept -> bool {
        return (mErrFlags & kErrFlagLogged) != 0;
    }

    [[nodiscard]] auto skipped() const noexcept -> bool {
        return (mErrFlags & kErrFlagSkipped) != 0;
    }

private:
    [[noreturn]] void throwBadResultAccess() const {
#if (__has_feature__cxx_exceptions)
        throw BadResultAccess{};
#else
        std::terminate();
#endif
    }

    union {
        Value mValue;
        std::error_code mErr;
    };

    static constexpr uint8_t kErrFlagsNone   = 0x00;
    static constexpr uint8_t kErrFlagHaveErr = 0x01;  // Do we have an error?
    static constexpr uint8_t kErrFlagLogged  = 0x02;  // Is mErr already logged?
    static constexpr uint8_t kErrFlagSkipped = 0x04;  // Was logging skipped?

    uint8_t mErrFlags;
};

/// \private
struct success_t {
    enum class Construct { kToken };

    explicit constexpr success_t(Construct) {}
};

/// Specialization of tiltfive::Result for functions with 'no return'
template <>
class [[nodiscard]] Result<void> {
public:
    using Value = void;

    Result() noexcept : mDummy(), mErrFlags(kErrFlagsNone) {}

    Result(std::error_code err) noexcept : mErr(err), mErrFlags(kErrFlagHaveErr) {}

    Result(Result&& other) : mErrFlags(other.mErrFlags) {
        if (mErrFlags != kErrFlagsNone) {
            new (&mErr) std::error_code(other.mErr);
        }
    }

    Result(const Result& other) noexcept : mErrFlags(other.mErrFlags) {
        if (mErrFlags != kErrFlagsNone) {
            new (&mErr) std::error_code(other.mErr);
        }
    }

    template <
        typename ErrorCodeEnum,
        typename = typename std::enable_if<std::is_error_code_enum<ErrorCodeEnum>::value>::type>
    Result(ErrorCodeEnum err) noexcept : mErr(err), mErrFlags(kErrFlagHaveErr) {}

    Result(std::errc err) noexcept : mErr(std::make_error_code(err)), mErrFlags(kErrFlagHaveErr) {}

    Result(success_t) noexcept : mDummy(), mErrFlags(kErrFlagsNone) {}

    ~Result() {
        if (mErrFlags != kErrFlagsNone) {
            using std::error_code;
            mErr.~error_code();
        }
    }

    auto operator=(Result&& other) -> Result& {
        if (mErrFlags == kErrFlagsNone) {
            if (other.mErrFlags != kErrFlagsNone) {
                new (&mErr) std::error_code(other.mErr);
            }
        } else {
            if (other.mErrFlags == kErrFlagsNone) {
                using std::error_code;
                mErr.~error_code();
            } else {
                mErr = other.mErr;
            }
        }
        mErrFlags = other.mErrFlags;
        return *this;
    }

    auto operator=(const Result& other) -> Result& {
        if (mErrFlags == kErrFlagsNone) {
            if (other.mErrFlags != kErrFlagsNone) {
                new (&mErr) std::error_code(other.mErr);
            }
        } else {
            if (other.mErrFlags == kErrFlagsNone) {
                using std::error_code;
                mErr.~error_code();
            } else {
                mErr = other.mErr;
            }
        }
        mErrFlags = other.mErrFlags;
        return *this;
    }

    auto operator=(success_t) -> Result& {
        if (mErrFlags != kErrFlagsNone) {
            using std::error_code;
            mErr.~error_code();
        }
        mErrFlags = kErrFlagsNone;
        return *this;
    }

    explicit operator bool() const noexcept {
        return mErrFlags == kErrFlagsNone;
    }

    auto operator==(success_t) const noexcept -> bool {
        return mErrFlags == kErrFlagsNone;
    }

    [[nodiscard]] auto error() const noexcept -> std::error_code {
        if (mErrFlags != kErrFlagsNone) {
            return mErr;
        }
        return {};
    }

    [[nodiscard]] auto logged() const noexcept -> bool {
        return (mErrFlags & kErrFlagLogged) != 0;
    }

    [[nodiscard]] auto skipped() const noexcept -> bool {
        return (mErrFlags & kErrFlagSkipped) != 0;
    }

private:
    [[noreturn]] static void throwBadResultAccess() {
#if (__has_feature__cxx_exceptions)
        throw BadResultAccess{};
#else
        std::terminate();
#endif
    }

    union {
        uint8_t mDummy[sizeof(std::error_code)]{};
        std::error_code mErr;
    };

    static constexpr uint8_t kErrFlagsNone   = 0x00;
    static constexpr uint8_t kErrFlagHaveErr = 0x01;  // Do we have an error?
    static constexpr uint8_t kErrFlagLogged  = 0x02;  // Is mErr already logged?
    static constexpr uint8_t kErrFlagSkipped = 0x04;  // Was logging skipped?

    uint8_t mErrFlags;
};

/// Indicates 'success' for a Result<void> function
static constexpr success_t kSuccess{success_t::Construct::kToken};

// Support struct to determine if a type supports std::ostream& operator<<
template <typename T, typename Enable = std::ostream&>
struct supports_ostream : std::false_type {};
template <typename T>
struct supports_ostream<T, decltype(std::declval<std::ostream&>() << std::declval<T>())>
    : std::true_type {};

template <typename T>
void stringifyForStream(T& t,
                        std::ostream& os,
                        typename std::enable_if<supports_ostream<T>::value, T>::type* = 0) {
    os << t;
}

template <typename T>
void stringifyForStream(T& t,
                        std::ostream& os,
                        typename std::enable_if<!supports_ostream<T>::value, T>::type* = 0) {
    os << "[" << typeid(t).name() << "]";
}

}  // namespace tiltfive

/// Support for writing tiltfive::Result<> to an std::ostream
template <typename T>
std::ostream& operator<<(std::ostream& os, const tiltfive::Result<T>& instance) {
    if (!instance) {
        os << instance.error().message();
    } else {
        tiltfive::stringifyForStream(*instance, os);
    }
    return os;
}

///// Specialization for writing tiltfive::Result<void> to an std::ostream
template <>
inline std::ostream& operator<<(std::ostream& os, const tiltfive::Result<void>& instance) {
    if (!instance) {
        os << instance.error().message();
    } else {
        os << "Success";
    }
    return os;
}
