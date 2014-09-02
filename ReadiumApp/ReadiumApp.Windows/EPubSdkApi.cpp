#include "pch.h"
#include "EPubSdkApi.h"

using namespace ReadiumApp;
using namespace Readium;

ePub3::PackagePtr pkg;

EPubSdkApi::EPubSdkApi()
{
}

void EPubSdkApi::readPackages()
{

}

void EPubSdkApi::openFile(Windows::Storage::IStorageFile^ file)
{
	Readium::Container^ container = Readium::Container::OpenContainer(file);
	if (!container)
	{
		Log::Debug("[EPubSdkApi] Unable to open container.");
		return;
	}

	Windows::Foundation::Collections::IVectorView<Readium::Package^>^ packages = container->Packages();
	if (!packages)
	{
		Log::Debug("[EPubSdkApi] Could not retrieve pacakages from container.");
		return;
	}

	if (packages->Size <= 0)
	{
		Log::Debug("[EPubSdkApi] No packages in container.");
		return;
	}
	else
	{
		Readium::Package^ currentPackage = packages->GetAt(0);
	}
}
