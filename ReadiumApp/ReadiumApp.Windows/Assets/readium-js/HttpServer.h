#ifndef HTTPSERVER_H
#define HTTPSERVER_H

namespace ReadiumApp
{
	public ref class HttpServer sealed
	{
	public:
		HttpServer(int port) :
			port(port)
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
				// Delegate!
				listener->BindServiceNameAsync(port.ToString());
			}
		}

		void Stop()
		{
			if (IsActive)
			{
				listener->Close();
				listener = nullptr;
				isActive = false;
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
			HandleRequest(args->Socket);
		}

		void HandleRequest(StreamSock^ sock)
		{
			// Init IO
			Reader^ reader = ref new Reader(sock->InputStream);
			Writer^ writer = ref new Writer(sock->OutputStream);
			writer->UnicodeEncoding = Windows::Storage::Streams::UnicodeEncoding::Utf8;

			// Read the HTTP request
			Platform::String^ req = StreamReadline(reader);
			
		}

		Platform::String^ StreamReadline(Reader^ reader)
		{
			//using namespace concurrency;

			
			Platform::String^ data = "";
			while (true)
			{
				/*create_task(reader->LoadAsync(1))
					.then([data, reader]()
				{
					char nextChar = reader->ReadByte();
					if (nextChar == "\n") { break; }
					if (nextChar != "\r")
					{
						data += nextChar;
					}
				});*/
			}

			return data;
		}

		

		int port;
		bool isActive;
		SockList^ listener;
		

	};
}

#endif // HTTPSERVER_H