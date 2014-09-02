#ifndef EPUB_SDK_API_H
#define EPUB_SDK_API_H

// System Includes
#include <memory>
#include <string>

// Project Includes
#include "Log.h"

// Library Includes
#include "container.h"


namespace ReadiumApp
{
	public ref class EPubSdkApi sealed
	{
	public:
		EPubSdkApi();

		void readPackages();

		void openFile(Windows::Storage::IStorageFile^ file);

	private:

		Readium::Container^ container;
		Windows::Foundation::Collections::IVectorView<Readium::Package^>^ packages;
		Readium::Package^ currentPackage;

	};
}

#endif