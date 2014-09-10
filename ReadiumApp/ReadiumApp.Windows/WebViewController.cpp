#include "pch.h"
#include "WebViewController.h"

#include "EPubSdkApi.h"

using namespace ReadiumApp;
using namespace Readium;

using namespace Windows::Data::Json;
using namespace Windows::UI::Xaml::Controls;

WebViewController::WebViewController(Windows::UI::Xaml::Controls::WebView^ view)
{
	webView = view;
}

void WebViewController::openPackage(Readium::Package^ package)
{
	JsonObject^ json = EPubSdkApi::packageToJson(package);
	Platform::String^ jsonString = json->Stringify();

	Platform::String^ tmp[] = { jsonString };
	Platform::Array<Platform::String^>^ args = { jsonString }; // ref new Platform::Array<Platform::String^> ^ (tmp, 1);
	webView->InvokeScript("ReadiumSDK.reader.openBook", args);
}