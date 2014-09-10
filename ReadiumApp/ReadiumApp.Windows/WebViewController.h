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

	private:

		Windows::UI::Xaml::Controls::WebView^ webView;
	};
}

#endif // WEBVIEWCONTROLLER_H