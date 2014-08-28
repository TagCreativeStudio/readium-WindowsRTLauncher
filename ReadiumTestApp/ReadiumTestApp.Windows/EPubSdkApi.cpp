#include "pch.h"
#include "EPubSdkApi.h"

using namespace ReadiumApp;

ePub3::PackagePtr pkg;

EPubSdkApi::EPubSdkApi()
{
}

void EPubSdkApi::readPackages()
{

}

void EPubSdkApi::openFile(Platform::String^ path)
{
	std::wstring wpath(path->Data());
	openFile(wpath);
}

void EPubSdkApi::openFile(std::wstring &path)
{
	ePub3::ContainerPtr cont = ePub3::Container::OpenContainer(path);
	if (cont)
	{
		Log::Debug("[API] Document has container");
		pkg = cont->DefaultPackage();

		if (pkg == 0)
		{
			auto pkgs = cont->Packages();
			if (pkgs.size() <= 0)
			{
				Log::Debug("[API] No packages in document!");
				return;
			}
			pkg = pkgs[0];
		}
	}
}