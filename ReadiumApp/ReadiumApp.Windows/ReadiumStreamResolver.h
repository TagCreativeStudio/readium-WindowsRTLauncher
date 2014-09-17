#ifndef READIUMSTREAMRESOLVER_H
#define READIUMSTREAMRESOLVER_H

#include "pch.h"

namespace ReadiumApp
{
	public ref class ReadiumStreamResolver sealed : public Windows::Web::IUriToStreamResolver
	{
	public:

		ReadiumStreamResolver();
		ReadiumStreamResolver(Readium::Package^ package);
		void SetPackage(Readium::Package^ package);
		virtual Windows::Foundation::IAsyncOperation<Windows::Storage::Streams::IInputStream^>^ UriToStreamAsync(Windows::Foundation::Uri^ uri);
		
	private:

		Windows::Foundation::IAsyncOperation<bool>^ FileExists(Windows::Foundation::Uri^ uri);
		Windows::Foundation::IAsyncOperation<Windows::Storage::Streams::IInputStream^>^ GetFileStreamFromApplicationUriAsync(Windows::Foundation::Uri^ uri);
		///Windows::Foundation::IAsyncOperation<Windows::Storage::Streams::IInputStream^>^ GetFileStreamFromApplicationUriAsync(Windows::Foundation::Uri^ uri);

		Readium::Package^ package;
		
	};
}

#endif // READIUMSTREAMRESOLVER_H