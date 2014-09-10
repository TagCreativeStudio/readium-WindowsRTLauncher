#ifndef WEBVIEWCONTROLLER_H
#define WEBVIEWCONTROLLER_H

namespace ReadiumApp
{
	public ref class WebViewController sealed
	{
	public:
		WebViewController(Windows::UI::Xaml::Controls::WebView^ view);

		void openPackage(Readium::Package^ package);

	private:

		Windows::UI::Xaml::Controls::WebView^ webView;
	};
}

#endif // WEBVIEWCONTROLLER_H