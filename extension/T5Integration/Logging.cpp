#include <iostream>
#include <ObjectRegistry.h>
#include <Logging.h>


namespace T5Integration {

	void DefaultLogger::log_error(const char* message, const char* func_name, const char* file_name, int line_num) {
		std::cerr << file_name << "(" << line_num << ") [" << func_name << "] Error: " << message << std::endl;
	}

	void DefaultLogger::log_warning(const char* message, const char* func_name, const char* file_name, int line_num) {
		std::cerr << file_name << "(" << line_num << ") [" << func_name << "] Warning: " << message << std::endl;
	}

	void DefaultLogger::log_string(const char* message) 
	{
		std::cout << message << std::endl;
	}

	void log_string(const char* message) {
		ObjectRegistry::logger()->log_string(message);
	}

	
	void log_error(const char* message, const char* p_function, const char* p_file, int p_line){
		ObjectRegistry::logger()->log_error(message, p_function, p_file, p_line);
	}

	void log_warning(const char* message, const char* p_function, const char* p_file, int p_line) {
		ObjectRegistry::logger()->log_warning(message, p_function, p_file, p_line);
	}

	void log_tilt_five_error(T5_Result result_code, const char *p_function, const char *p_file, int p_line) {
		ObjectRegistry::logger()->log_error(t5GetResultMessage(result_code), p_function, p_file, p_line);
	}

	void log_tilt_five_warning(T5_Result result_code, const char *p_function, const char *p_file, int p_line) {
		ObjectRegistry::logger()->log_warning(t5GetResultMessage(result_code), p_function, p_file, p_line);
	}

	void log_toggle(bool newValue, bool& prevVal, const char* msg1, const char* msg2) 
	{
		if(newValue != prevVal) 
		{
			prevVal = newValue;
			if(newValue)
				log_message(msg1);
			else
				log_message(msg2);

		}
	}

}