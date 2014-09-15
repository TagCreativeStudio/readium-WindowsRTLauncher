#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include "pch.h"

#include "Log.h"
#include "StringHelper.h"

namespace ReadiumApp
{
	public ref class HttpServer sealed
	{
	public:
		HttpServer(int port) :
			port(port), isActive(false)
		{
		}

		property bool IsActive
		{
			bool get() { return isActive;  }
		}

		void Start()
		{
			if (!isActive)
			{
				isActive = true;
				listener = ref new SockList();
				listener->Control->QualityOfService = Windows::Networking::Sockets::SocketQualityOfService::Normal;
				listener->ConnectionReceived += ref new Windows::Foundation::TypedEventHandler<SockList^, SockArgs^>(this, &HttpServer::OnConnectionReceived);
				Log::Debug("[HttpServer] Starting...");
				listener->BindServiceNameAsync(port.ToString());
			}
		}

		void Stop()
		{
			if (IsActive)
			{
				//listener->Close();
				listener = nullptr;
				isActive = false;
			}
		}

		static Platform::String^ FindIpAddress()
		{
			using namespace Platform::Collections;
			Vector<Platform::String^>^ ipAddresses = ref new Vector<Platform::String^>();
			auto hostnames = Windows::Networking::Connectivity::NetworkInformation::GetHostNames();
			for each (auto hn in hostnames)
			{
				if (hn->IPInformation != nullptr &&
					(hn->IPInformation->NetworkAdapter->IanaInterfaceType == 71 ||
					hn->IPInformation->NetworkAdapter->IanaInterfaceType == 6))
				{
					Platform::String^ ipaddr = hn->DisplayName;
					ipAddresses->Append(ipaddr);
				}
			}

			if (ipAddresses->Size < 1)
			{
				return nullptr;
			}
			else if (ipAddresses->Size == 1)
			{
				return ipAddresses->GetAt(0);
			}
			else
			{
				return ipAddresses->GetAt(ipAddresses->Size - 1);
			}
		}

	
	private:

		typedef Windows::Storage::Streams::DataReader Reader;
		typedef Windows::Storage::Streams::DataWriter Writer;
		typedef Windows::Networking::Sockets::StreamSocket StreamSock;
		typedef Windows::Networking::Sockets::StreamSocketListener SockList;
		typedef Windows::Networking::Sockets::StreamSocketListenerConnectionReceivedEventArgs SockArgs;
		
		void OnConnectionReceived(SockList^ sender, SockArgs^ args)
		{
			Log::Debug("[HttpServer] OnConnectionReceived");
			//concurrency::create_task(HandleRequest(args->Socket));
			HandleRequestAsync2(args->Socket);
		}

		void HandleRequest(StreamSock^ sock)
		{
			using namespace Platform::Collections;

			// Init IO
			Reader^ reader = ref new Reader(sock->InputStream);
			Writer^ writer = ref new Writer(sock->OutputStream);
			writer->UnicodeEncoding = Windows::Storage::Streams::UnicodeEncoding::Utf8;

			// Read the HTTP request
			Platform::String^ req = StreamReadline(reader);
			Vector<Platform::String^>^ tokens = StringHelper::Split(req, ' ');
			if (tokens->Size != 3)
			{
				Log::Debug("[HttpServer] Invalid HTTP request line.");
				return;
			}

			Platform::String^ method = tokens->GetAt(0);
			Platform::String^ url = tokens->GetAt(1);
		}

		Platform::String^ StreamReadline(Reader^ reader)
		{
			std::wstring data(L"");

			while (true)
			{
				wchar_t nextChar = reader->ReadByte();
				if (nextChar == '\n')
				{
					return ref new Platform::String(data.c_str());
				}
				else if (nextChar != '\r')
				{
					data += nextChar;
				}
			}

			return ref new Platform::String(data.c_str());
		}

		//Windows::Foundation::IAsyncOperation<bool>^ HandleRequestAsync(StreamSock^ sock)
		void HandleRequestAsync(StreamSock^ sock)
		{
			using namespace concurrency;
			using namespace Platform::Collections;
			using namespace Windows::Foundation;

			// Init IO
			Reader^ reader = ref new Reader(sock->InputStream);
			Writer^ writer = ref new Writer(sock->OutputStream);
			writer->UnicodeEncoding = Windows::Storage::Streams::UnicodeEncoding::Utf8;

			// Read the HTTP request
			/*create_task(StreamReadlineAsync(reader))
				.then([this](Platform::String^ req)
			{
				Vector<Platform::String^>^ tokens = StringHelper::Split(req, ' ');
				if (tokens->Size != 3)
				{
					Log::Debug("[HttpServer] Invalid HTTP request line.");
					return;
				}

				Platform::String^ method = tokens->GetAt(0);
				Platform::String^ url = tokens->GetAt(1);
			});*/

			Platform::String^ req = StreamReadlineAsync(reader);
			Vector<Platform::String^>^ tokens = StringHelper::Split(req, ' ');
			if (tokens->Size != 3)
			{
				Log::Debug("[HttpServer] Invalid HTTP request line.");
				//return nullptr;
				return;
			}

			Platform::String^ method = tokens->GetAt(0);
			Platform::String^ url = tokens->GetAt(1);

			Log::Debug("[HttpServer] Method: " + method);
			Log::Debug("[HttpServer] URL: " + url);

			Platform::String^ str = ref new Platform::String();
			str =  "HTTP/1.0 200 OK\n";
			str += "Content-Type: text/html\n";
			str += "Connection: close\n";
			str += "\n";

			str += "<html><head><title>Test</title></head>";
			str += "<body>";
			str += "<h1>Hello From HttpServer!</h1>";
			str += "</body>";
			str += "</html>";

			Log::Debug("[HttpServer] " + str);

			writer->WriteString(str);
			writer->StoreAsync();
			//return writer->FlushAsync();
		}

		void HandleRequestAsync2(StreamSock^ sock)
		{
			using namespace concurrency;
			using namespace Platform::Collections;
			using namespace Windows::Foundation;

			// Init IO
			Reader^ reader = ref new Reader(sock->InputStream);
			Writer^ writer = ref new Writer(sock->OutputStream);
			writer->UnicodeEncoding = Windows::Storage::Streams::UnicodeEncoding::Utf8;

			// Read the HTTP request
			/*create_task(StreamReadlineAsync(reader))
			.then([this](Platform::String^ req)
			{
			Vector<Platform::String^>^ tokens = StringHelper::Split(req, ' ');
			if (tokens->Size != 3)
			{
			Log::Debug("[HttpServer] Invalid HTTP request line.");
			return;
			}

			Platform::String^ method = tokens->GetAt(0);
			Platform::String^ url = tokens->GetAt(1);
			});*/

			Platform::String^ req = StreamReadlineAsync(reader);
			//create_task(StreamReadlineAsync2(reader))
			//	.then([this, writer](Platform::String^ req)
			//{
			//	Log::Debug("ASDFASDFSaDF");
			//	Vector<Platform::String^>^ tokens = StringHelper::Split(req, ' ');
			//	if (tokens->Size != 3)
			//	{
			//		Log::Debug("[HttpServer] Invalid HTTP request line.");
			//		//return nullptr;
			//		return;
			//	}

			//	Platform::String^ method = tokens->GetAt(0);
			//	Platform::String^ url = tokens->GetAt(1);

			//	Log::Debug("[HttpServer] Method: " + method);
			//	Log::Debug("[HttpServer] URL: " + url);

			//	Platform::String^ str = ref new Platform::String();
			//	str = "HTTP/1.0 200 OK\n";
			//	str += "Content-Type: text/html\n";
			//	str += "Connection: close\n";
			//	str += "\n";

			//	str += "<html><head><title>Test</title></head>";
			//	str += "<body>";
			//	str += "<h1>Hello From HttpServer!</h1>";
			//	str += "</body>";
			//	str += "</html>";

			//	Log::Debug("[HttpServer] " + str);

			//	writer->WriteString(str);
			//	writer->StoreAsync();
			//	//return writer->FlushAsync();
			//});
			
		}


		/*static Windows::Foundation::IAsyncOperation<Platform::String^>^ StreamReadlineAsync(Reader^ reader)*/
		static Platform::String^ StreamReadlineAsync(Reader^ reader)
		{
			using namespace concurrency;
			std::wstring data(L"");

			while (true)
			{
				Windows::Storage::Streams::DataReaderLoadOperation^ op = reader->LoadAsync(1);
				while (op->Status != Windows::Foundation::AsyncStatus::Completed)
				{ }
				/*create_task(reader->LoadAsync(1))
					.then([&](Windows::Storage::Streams::DataReaderLoadOperation^ op)
				{*/
					wchar_t nextChar = reader->ReadByte();
					if (nextChar == '\n')
					{
						//Log::Debug("[HttpServer] done!");
						return ref new Platform::String(data.c_str());
					}
					else if (nextChar != '\r')
					{
						data += nextChar;
					}
					//Log::Debug("[HttpServer] data: " + ref new Platform::String(data.c_str()));
				//});
			}

			return ref new Platform::String(data.c_str());

			//return ref new Windows::Foundation::AsyncOperation<Platform::String^>(ref new Platform::String(data.c_str()));
			/*create_task([&data]() {
				return ref new Platform::String(data.c_str());
			});*/
		}

//		Windows::Foundation::IAsyncOperation<Platform::String^>^ StreamReadlineAsync2(Reader^ reader)
//		{
//			using namespace concurrency;
//			std::wstring data(L"");
//			bool loaded = true;
//			bool reading = true;
//			
//
//			//Windows::Storage::Streams::DataReaderLoadOperation^ op = reader->LoadAsync(1);
//			/*while (true)
//			{
//				if (!loaded || !reading)
//				{
//					continue;
//				}
//
//				loaded = false;*/
//				create_task(reader->LoadAsync(2048))
//					.then([this, reader, &data, &loaded, &reading](unsigned int size)
//				{
//					loaded = true;
//					wchar_t nextChar = reader->ReadByte();
//					while (nextChar != '\n')
//					{
//						if (nextChar != '\r')
//						{
//							data += nextChar;
//						}
//						else if (nextChar == '\n')
//						{
//							return ref new Platform::String(data.c_str());
//						}
//					}
//
//					return ref new Platform::String(data.c_str());
///*
//					if (nextChar == '\n')
//					{
//						Log::Debug("done");
//						reading = false;
//						return ref new Platform::String(data.c_str());
//					}
//					else if (nextChar != '\r')
//					{
//						data += nextChar;
//					}
//					Log::Debug(ref new Platform::String(data.c_str()));*/
//				});
//			//}
//
//			//return ref new Platform::String(data.c_str());
//
//			create_task([&data]() {
//			    return ref new Platform::String(data.c_str()); 
//			});
//		}

		int port;
		bool isActive;
		SockList^ listener;
		
	};
}

#endif // HTTPSERVER_H