#ifndef STRINGHELPER_H
#define STRINGHELPER_H

#include <string>
#include <sstream>

namespace ReadiumApp
{
	public ref class StringHelper sealed
	{
	public:

		static std::wstring PlatformToStd(const Platform::String^ str)
		{
			std::wstring ws(str->Data);
			return ws;
		}

		static Platform::String^ StdToPlatform(const std::wstring& str)
		{
			return ref new Platform::String(str.c_str());
		}

		static void Split(const Platform::String^ s, char delim)
		{
			Platform::Collections::Vector<Platform::String^>^ vec = ref new Platform::Coll
		}

	private:

		
	};
}

#endif // STRINGHELPER_H