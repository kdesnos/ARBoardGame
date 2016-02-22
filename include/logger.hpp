/*!
* Macro used to format all entry to the log as follows: <br>
* "[<Time>] message (in <Function>)"
*/
#define LOG(LOGGER,MSG) {						\
	time_t rawtime;								\
	struct tm timeinfo ;						\
	time(&rawtime);								\
	char buffer[9];								\
												\
	localtime_s(&timeinfo, &rawtime);			\
	strftime(buffer, 9, "%T", &timeinfo);		\
	LOGGER << "[" << buffer << "] ";			\
	LOGGER << MSG ;								\
	LOGGER << " (in " <<  __FUNCTION__ <<")";	\
	LOGGER <<  endl;							\
}
