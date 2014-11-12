#include "pch.h"

#include "ReadiumStreamResolver.h"
#include "StringHelper.h"

#include "Log.h"

using namespace Readium_SDK;

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
			Uri^ appDataUriTest = ref new Uri(L"ms-appx:///Assets/readium-js/empty.html");
			return GetFileStreamFromApplicationUriAsync(appDataUriTest);
			/*
			return create_async([]()->IInputStream^
			{
				return nullptr;
			});
			*/
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
			auto wFileName = StringHelper::PlatformToStd(relativePath);
			std::replace(wFileName.begin(), wFileName.end(), '/', '-');
			String^ longFileName = StringHelper::StdToPlatform(wFileName);
			Readium::IClosableStream^ stream = this->package->ReadStreamForRelativePath(StringHelper::TrimLeadingSlash(relativePath));
			auto read_size = stream->Size;

			auto reader = ref new DataReader(stream);
			StorageFolder^ item = ApplicationData::Current->LocalFolder;

			/***
			//create our file
			auto fileCreationTask = item->CreateFileAsync(longFileName, Windows::Storage::CreationCollisionOption::ReplaceExisting);
			fileCreationTask->Completed = ref new AsyncOperationCompletedHandler<StorageFile^>
				([=](IAsyncOperation<StorageFile^>^ operation, AsyncStatus status)
			{
				StorageFile^ myNewFile = operation->GetResults();

				// open the new file
				auto openMyFileOperation = myNewFile->OpenAsync(Windows::Storage::FileAccessMode::ReadWrite);
				openMyFileOperation->Completed = ref new AsyncOperationCompletedHandler<IRandomAccessStream^>
					([=](IAsyncOperation<IRandomAccessStream^>^ operation, AsyncStatus status)
				{
					auto myRandomAccessStream = operation->GetResults();

					// read from our inputstream
					auto loadOperation = reader->LoadAsync(read_size);

					loadOperation->Completed = ref new AsyncOperationCompletedHandler<unsigned int>
						([=](IAsyncOperation<unsigned int>^ operation, AsyncStatus status)
					{
						IBuffer^ myBuffer = reader->ReadBuffer(read_size);

						create_task(FileIO::WriteBufferAsync(myNewFile, myBuffer)).then([=](task<void> task)
						{
							try{
								task.get();

								auto openCacheFileOperation = myNewFile->OpenReadAsync();
								openCacheFileOperation->Completed = ref new AsyncOperationCompletedHandler<IRandomAccessStreamWithContentType^>
									([=](IAsyncOperation<IRandomAccessStreamWithContentType^>^ operation, AsyncStatus status)
								{
									auto ras = operation->GetResults();
									return ras->GetInputStreamAt(0);
								});
							}
							catch (COMException^ ex) {
								//handle exception raised during file write
							}
						});
						

						//auto writeAction = FileIO::WriteBufferAsync(myNewFile, myBuffer);
					});
				});
			});
			***/

			task<StorageFile^> createFileTask(item->CreateFileAsync(longFileName, Windows::Storage::CreationCollisionOption::ReplaceExisting));

			createFileTask.wait();

			StorageFile^ storageFile = createFileTask.get();
			task<IRandomAccessStream^> openFileTask(storageFile->OpenAsync(Windows::Storage::FileAccessMode::ReadWrite));

			openFileTask.wait();

			IRandomAccessStream^ writeStream = openFileTask.get();
			task<unsigned int> readStreamTask(reader->LoadAsync(read_size));

			readStreamTask.wait();

			// TODO: error checking
			unsigned int readBytes = readStreamTask.get();
			Log::Debug("[ReadiumStreamResolver] read from stream: " + readBytes + " of " + read_size + " bytes ");

			IBuffer^ contentBuffer = reader->DetachBuffer();
			InMemoryRandomAccessStream^ memoryStream = ref new InMemoryRandomAccessStream();
			DataWriter^ dataWriter = ref new DataWriter(memoryStream->GetOutputStreamAt(0));
			dataWriter->WriteBuffer(contentBuffer);
			task<unsigned int> storeTask(dataWriter->StoreAsync());

			unsigned int wroteBytes = storeTask.get();
			Log::Debug("[ReadiumStreamResolver] wrote to memoryStream: " + wroteBytes + " of " + readBytes + " bytes ");

			storeTask.wait();
			return memoryStream->GetInputStreamAt(0);

			//return this->package->ReadStreamForRelativePath(StringHelper::TrimLeadingSlash(relativePath));
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

