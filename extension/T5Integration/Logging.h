#pragma once
#include <memory>
#include <sstream>
#include <TiltFiveNative.h>

namespace T5Integration {
	
class Logger {
public:
	using Ptr = std::shared_ptr<Logger>;

	virtual void log_error(const char* message, const char* func_name, const char* file_name, int line_num) = 0;
	virtual void log_warning(const char* message, const char* func_name, const char* file_name, int line_num) = 0;
	virtual void log_string(const char* message) = 0;
};

class DefaultLogger : public Logger {
public:
	void log_error(const char* message, const char* func_name, const char* file_name, int line_num) override;
	void log_warning(const char* message, const char* func_name, const char* file_name, int line_num) override;
	void log_string(const char* message) override;
};

void log_string(const char* message);
void log_error(const char* message, const char* p_function, const char* p_file, int p_line);
void log_warning(const char* message, const char* p_function, const char* p_file, int p_line);
void log_tilt_five_error(T5_Result result_code, const char* p_function, const char* p_file, int p_line);
void log_tilt_five_warning(T5_Result result_code, const char* p_function, const char* p_file, int p_line);
void log_toggle(bool current, bool& state, const char* msg1, const char* msg2);

inline void log_message_stream(std::stringstream& stream) {}

template<typename T, typename... Types>
void log_message_stream(std::stringstream& stream, T var1, Types... var2) {
	stream << var1;

	log_message_stream(stream, var2...);
}

template<typename T, typename... Types>
void log_message(T var1, Types... var2) {
	std::stringstream stream;
	stream << var1;
	log_message_stream(stream, var2...);
	log_string(stream.str().c_str());
}

} //namespace T5Integration


#define LOG_CHECK_POINT T5Integration::log_message("===> ", __func__, " : ", __LINE__);

#ifndef LOG_CHECK_POINT_ONCE
#define LOG_CHECK_POINT_ONCE { static bool once ## __LINE__ = false; if(! once ## __LINE__) {LOG_CHECK_POINT once ## __LINE__ = true; }}
#endif


#ifndef LOG_TOGGLE
#define LOG_TOGGLE(INIT, TEST, MSG1, MSG2) { static bool toggle ## __LINE__ = (INIT);  T5Integration::log_toggle((TEST), toggle ## __LINE__, MSG1, MSG2); }
#endif

#ifndef LOG_MESSAGE
#define LOG_MESSAGE(...) T5Integration::log_message(__func__, ":", __LINE__, " ", __VA_ARGS__)
#endif

#ifndef LOG_ERROR
#define LOG_ERROR(message) T5Integration::log_error((message), __func__, __FILE__, __LINE__)
#endif

#ifndef LOG_WARNING
#define LOG_WARNING(message) T5Integration::log_warning((message), __func__, __FILE__, __LINE__)
#endif

#ifndef LOG_T5_ERROR
#define LOG_T5_ERROR(result_code) T5Integration::log_tilt_five_error(result_code, __func__, __FILE__, __LINE__)
#endif

#ifndef LOG_ERROR_ONCE
#define LOG_ERROR_ONCE(MSG) { static bool once ## __LINE__ = false; if(! once ## __LINE__) {LOG_ERROR(MSG); once ## __LINE__ = true; }}
#endif

