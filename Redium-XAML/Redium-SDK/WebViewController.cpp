#include "pch.h"
#include "WebViewController.h"

#include "EPubSdkApi.h"

using namespace Redium_SDK;
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

	JsHandler^ onLoadSpineItem = ref new JsHandler([this](Windows::Data::Json::JsonObject^ json)
	{
		Log::Debug("[WebViewController] onLoadSpineItem");
		
		using namespace Windows::Storage;
		using namespace Windows::Data::Json;

		auto folder = Windows::ApplicationModel::Package::Current->InstalledLocation;
		Log::Debug("[WebViewController] folder: " + folder->Path->ToString());
		auto reader = std::make_shared<Streams::DataReader^>(nullptr);
		
		create_task(folder->GetFileAsync("ch01.xhtml"))
			.then([] (StorageFile^ file)
		{
			Log::Debug("[WebViewController] file: " + file->Path->ToString());
			return file->OpenReadAsync();
		})
		.then([reader](Streams::IRandomAccessStreamWithContentType^ istream)
		{
			*reader = ref new Streams::DataReader(istream->GetInputStreamAt(0));
			return (*reader)->LoadAsync(istream->Size);
		}).then([this, reader](UINT bytesRead)
		{
			Platform::String^ contents = (*reader)->ReadString(bytesRead);

			JsonObject^ jsonData = ref new JsonObject();
			jsonData->Insert("contents", JsonValue::CreateStringValue(contents));
			this->notify(jsonData);
			//this->notify(contents);
		});
	});
	onCallback->Insert("onLoadSpineItem", onLoadSpineItem);
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
	Readium::SpineItem^ spineItem = bookPackage->SpineItemAt(2);
	Platform::String^ firstPageHref = spineItem->ManifestItemRef->BaseHref;

	EPubOpenPageRequest^ req = EPubOpenPageRequest::fromContentUrl(firstPageHref, firstPageHref);
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

void WebViewController::notify(Windows::Data::Json::JsonObject^ json)
{
	Log::Debug("[WebViewController] notify (json)");
	auto args = ref new Platform::Collections::Vector<Platform::String^>();
	args->Append(json->Stringify());
	webView->InvokeScriptAsync("ReadiumLoadSpineItem", args);
}

void WebViewController::notify(Platform::String^ data)
{
	Log::Debug("[WebViewController] notify (string)");
	auto args = ref new Platform::Collections::Vector<Platform::String^>();
	args->Append(data);
	webView->InvokeScriptAsync("ReadiumLoadSpineItem", args);
}

void WebViewController::openPageLeft()
{
	webView->InvokeScriptAsync("ReadiumOpenPageLeft", nullptr);
}

void WebViewController::openPageRight()
{
	webView->InvokeScriptAsync("ReadiumOpenPageRight", nullptr);
}
