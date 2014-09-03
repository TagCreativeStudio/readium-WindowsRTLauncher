#ifndef LOG_H
#define LOG_H

#include <string>
#include <sstream>

namespace ReadiumApp
{
	public ref class Log sealed
	{
	public:

		/// <summary>
		/// Write a Platform::String to debug log.
		/// </summary>
		static void Debug(Platform::String^ msg)
		{
#ifdef _DEBUG
			std::wstring wmsg(msg->Data());
			std::wstringstream ss;
			ss << wmsg << std::endl;
			OutputDebugString(ss.str().c_str());
#endif
		}

		static void PrintLastErrorMessage()
		{
#ifdef _DEBUG
			std::wstringstream ss;
			TCHAR buff[256];
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, buff, sizeof(buff), NULL);
			ss << buff << std::endl;

			OutputDebugString(ss.str().c_str());
#endif
		}
	};
}

#endif