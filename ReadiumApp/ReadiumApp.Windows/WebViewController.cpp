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
	onCallback = ref new FunctionMap();

	// Callback for Reader initialization
	JsHandler^ onReaderInitialized = ref new JsHandler([](Windows::Data::Json::JsonObject^ json)
	{
		Log::Debug("[WebViewController] onReaderInitialized");
	});
	onCallback->Insert("onReaderInitialized", onReaderInitialized);
}

void WebViewController::openPackage(Readium::Package^ package)
{
	JsonObject^ json = EPubSdkApi::packageToJson(package);
	Platform::String^ jsonString = json->Stringify();

	auto args = ref new Platform::Collections::Vector<Platform::String^>();
	args->Append(jsonString);

	if (webView != nullptr)
	{
		try
		{
			create_task(webView->InvokeScriptAsync("ReadiumOpenBook", args));
		}
		catch (Platform::Exception^ e)
		{
			Log::Debug("[WebViewController] " + e->HResult.ToString() + ", " + e->Message);
		}
	}
}

void WebViewController::openBook(Readium::Package^ bookPackage)
{
	/*EPubOpenPageRequest^ req = EPubOpenPageRequest::fromContentUrl("", "chapter_001.xhtml");*/
	EPubOpenPageRequest^ req = EPubOpenPageRequest::fromContentUrl("pr01.xhtml", "pr01.xhtml");
	EPubViewerSettings^ view = ref new EPubViewerSettings(true, 100, 20);
	JsonObject^ json = EPubSdkApi::openBook(bookPackage, view, req);

	Platform::String^ jsonString = json->Stringify();

	auto args = ref new Platform::Collections::Vector<Platform::String^>();
	args->Append(jsonString);

	if (webView != nullptr)
	{
		try
		{
			create_task(webView->InvokeScriptAsync("ReadiumOpenBook", args));
		}
		catch (Platform::Exception^ e)
		{
			Log::Debug("[WebViewController] " + e->HResult.ToString() + ", " + e->Message);
		}
	}
}

void WebViewController::onScriptNotify(Platform::Object^ sender, Windows::UI::Xaml::Controls::NotifyEventArgs^ e)
{
	Log::Debug("[WebViewController] onScriptNotify");
	Platform::String^ val = e->Value;
	Log::Debug(val);
	JsonValue^ jsonVal;
	if (!JsonValue::TryParse(val, &jsonVal))
	{
		Log::Debug("[WebViewController] Could not parse string '" + val + "'.");
		return;
	}

	JsonObject^ json = jsonVal->GetObject();

	Platform::String^ functionToCall = json->GetNamedString("function");
	if (onCallback->HasKey(functionToCall))
	{
		onCallback->Lookup(functionToCall)->Invoke(json);
	}
}