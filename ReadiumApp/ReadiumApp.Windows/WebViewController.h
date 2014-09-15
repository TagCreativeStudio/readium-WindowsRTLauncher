#ifndef WEBVIEWCONTROLLER_H
#define WEBVIEWCONTROLLER_H

#include "EPubOpenPageRequest.h"
#include "EPubViewerSettings.h"

namespace ReadiumApp
{
	public ref class WebViewController sealed
	{
	public:
		WebViewController(Windows::UI::Xaml::Controls::WebView^ view);

		void openPackage(Readium::Package^ package);
		void openBook(Readium::Package^ bookPackage);// , EPubViewerSettings^ viewerSettings, EPubOpenPageRequest^ req);

		void onScriptNotify(Platform::Object^ sender, Windows::UI::Xaml::Controls::NotifyEventArgs^ e);
		void notify(Windows::Data::Json::JsonObject^ json);
		void notify(Platform::String^ data);

		void openPageLeft();
		void openPageRight();

	private:

		delegate void JsHandler(Windows::Data::Json::JsonObject^ json);

		typedef Platform::Collections::Map < Platform::String^, JsHandler^ > FunctionMap;
		FunctionMap^ onCallback;

		Windows::UI::Xaml::Controls::WebView^ webView;
	};
}

#endif // WEBVIEWCONTROLLER_H