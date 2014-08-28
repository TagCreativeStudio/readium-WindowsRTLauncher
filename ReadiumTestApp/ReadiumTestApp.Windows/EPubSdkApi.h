#ifndef EPUB_SDK_API_H
#define EPUB_SDK_API_H

// System Includes
#include <memory>
#include <string>

// Project Includes
#include "Log.h"

// Library Includes
#include "container.h"


namespace ReadiumTestApp
{
	public ref class EPubSdkApi sealed
	{
	public:
		EPubSdkApi();

		void readPackages();

		void openFile(Platform::String^ path);

	private:
		void openFile(std::wstring &path);

		

	};
}

#endif