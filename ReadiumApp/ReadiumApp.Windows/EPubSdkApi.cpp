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
	Readium::Container::OpenContainer(file);
	/*std::wstring wpath(path->Data());
	openFile(wpath);*/
}

void EPubSdkApi::openFile(Platform::String^ path)
{
	Log::Debug(path);
	Readium::Container::OpenContainer(path);
}
