#include "pch.h"

#include "ReadiumStreamResolver.h"
#include "StringHelper.h"

#include "Log.h"

using namespace Redium_SDK;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;

using namespace concurrency;

ReadiumStreamResolver::ReadiumStreamResolver()
{
}

ReadiumStreamResolver::ReadiumStreamResolver(Readium::Package^ package)
{
	this->package = package;
}

void ReadiumStreamResolver::SetPackage(Readium::Package^ package)
{
	this->package = package;
}

IAsyncOperation<IInputStream^>^ ReadiumStreamResolver::UriToStreamAsync(Windows::Foundation::Uri^ uri)
{
	if (uri == nullptr)
	{
		throw Exception::CreateException(0, "Uri should not be null");
	}

	String^ relativePath = ref new String(uri->Path->Data());
	Uri^ appDataUri = ref new Uri(L"ms-appx:///Assets" + relativePath);
	Log::Debug("[ReadiumStreamResolver] loading: " + appDataUri);

	String^ strToSplit = StringHelper::TrimLeadingSlash(relativePath);

	auto split = StringHelper::Split(strToSplit, '/');

	if (split->GetAt(0) == L"readium-js")
	{
		// Load from Assets/ folder
		if (split->Size == 1)
		{
			return create_async([]()->IInputStream^
			{
				return nullptr;
			});
		}
		else
		{
			return GetFileStreamFromApplicationUriAsync(appDataUri);
		}
	}
	else
	{
		// Load from ePub document
		return create_async([this, relativePath]()->IInputStream^
		{
			return this->package->ReadStreamForRelativePath(StringHelper::TrimLeadingSlash(relativePath));
			//return this->package->ReadStreamForItemAtPath("/EPUB/" + relativePath);
		});
	}
}

IAsyncOperation<bool>^ ReadiumStreamResolver::FileExists(Uri^ uri)
{
	return create_async([uri]()->bool
	{
		task<StorageFile^> getFileTask(StorageFile::GetFileFromApplicationUriAsync(uri));
		return getFileTask.then([](StorageFile^ file)
		{
			return file != nullptr;
		}).get();
	});
}

IAsyncOperation<IInputStream^>^ ReadiumStreamResolver::GetFileStreamFromApplicationUriAsync(Uri^ uri)
{
	return create_async([this, uri]()->IInputStream^
	{
		String^ uriStr = uri->ToString();
		task<StorageFile^> getFileTask(StorageFile::GetFileFromApplicationUriAsync(uri));

		task<IInputStream^> getInputStreamTask = getFileTask.then([](StorageFile^ storageFile)
		{
			String^ storagePath = storageFile->Path;
			return storageFile->OpenAsync(FileAccessMode::Read);
		}).then([](IRandomAccessStream^ stream)
		{
			return static_cast<IInputStream^>(stream);
		});

		return getInputStreamTask.get();
	});
}