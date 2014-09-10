#include "pch.h"
#include "WebViewController.h"

#include "EPubSdkApi.h"

using namespace ReadiumApp;
using namespace Readium;

using namespace Windows::Data::Json;
using namespace Windows::UI::Xaml::Controls;

using namespace concurrency;

WebViewController::WebViewController(Windows::UI::Xaml::Controls::WebView^ view)
{
	webView = view;
}

void WebViewController::openPackage(Readium::Package^ package)
{
	JsonObject^ json = EPubSdkApi::packageToJson(package);
	Platform::String^ jsonString = json->Stringify();

	//Platform::String^ tmp[] = { jsonString };
	//Platform::Array<Platform::String^>^ args = { jsonString }; // ref new Platform::Array<Platform::String^> ^ (tmp, 1);

	auto args = ref new Platform::Collections::Vector<Platform::String^>();
	args->Append(jsonString);

	if (webView != nullptr)
	{
		//webView->InvokeScript("ReadiumSDK.reader.openBook", args);
		try
		{
			//create_task(webView->InvokeScriptAsync("Tester.foo", nullptr));
			create_task(webView->InvokeScriptAsync("ReadiumOpenBook", args));
		}
		catch (Platform::Exception^ e)
		{
			Log::Debug("[WebViewController] " + e->HResult.ToString() + ", " + e->Message);
		}
		
		/*create_task(webView->InvokeScriptAsync("ReadiumSDK.reader.openBook", args));*/
			/*.then([]()
		{
			Log::Debug("[WebViewController] script complete!");
		});*/
	}
}