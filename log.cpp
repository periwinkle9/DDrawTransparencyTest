#include "log.h"
#include "framework.h"
#include <string>
#include <sstream>

void reportFailure(const char* funcName, int lineNum, int returnVal)
{
	std::ostringstream oss;
	oss << funcName << " (line " << lineNum << ") failed: return value " << returnVal;
	std::string message = oss.str();

	MessageBoxA(NULL, message.c_str(), "Uh-oh!", MB_OK);
}
