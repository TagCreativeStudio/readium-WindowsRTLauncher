#ifndef STRINGHELPER_H
#define STRINGHELPER_H

#include "pch.h"

#include <string>
#include <sstream>

#include "HttpServer.h"

namespace ReadiumApp
{

	class StringHelper sealed
	{
	private:

		typedef Platform::Collections::Vector<Platform::String^> StringVector;

	public:
		
		static std::wstring PlatformToStd(Platform::String^ str)
		{
			std::wstring ws(str->Data());
			return ws;
		}

		static Platform::String^ StdToPlatform(std::wstring& str)
		{
			return ref new Platform::String(str.c_str());
		}

		static StringVector^ Split(Platform::String^ str, wchar_t delim)
		{
			StringVector^ vec = ref new StringVector();

			Split(str, delim, vec);

			return vec;
		}

	private:

		static StringVector^ Split(Platform::String^ str, wchar_t delim, StringVector^ &elems)
		{
			std::wstringstream wss(PlatformToStd(str));
			std::wstring item;
			while (std::getline(wss, item, delim))
			{
				elems->Append(StdToPlatform(item));
			}
			return elems;
		}

	};
}

#endif // STRINGHELPER_H