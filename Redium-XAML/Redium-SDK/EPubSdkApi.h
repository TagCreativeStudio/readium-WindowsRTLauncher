#ifndef EPUB_SDK_API_H
#define EPUB_SDK_API_H

// System Includes
#include <memory>
#include <string>

// Project Includes
#include "EPubOpenPageRequest.h"
#include "EPubViewerSettings.h"
#include "Log.h"

namespace Redium_SDK
{
	public ref class EPubSdkApi sealed
	{
	public:
		EPubSdkApi();

		void readPackages();

		Readium::Package^ openFile(Windows::Storage::IStorageFile^ file);
		static Windows::Data::Json::JsonObject^ openBook(Readium::Package^ bookPackage, EPubViewerSettings^ viewerSettings, EPubOpenPageRequest^ req);
		static Windows::Data::Json::JsonObject^ packageToJson(Readium::Package^ package);
		static Windows::Data::Json::JsonArray^ SMILSequenceToJson(Readium::ISMILSequence^ sequence);
		static Windows::Data::Json::IJsonValue^ SMILTimeContainerToJson(Readium::ISMILTimeContainer^ container);
		static Windows::Data::Json::IJsonValue^ SMILParallelToJson(Readium::ISMILParallel^ par);
		static Windows::Data::Json::JsonObject^ SMILParTextToJson(Readium::ISMILText^ text);
		static Windows::Data::Json::JsonObject^ SMILParAudioToJson(Readium::ISMILAudio^ audio);

	private:

		Readium::Container^ container;
		Windows::Foundation::Collections::IVectorView<Readium::Package^>^ packages;
		Readium::Package^ currentPackage;

	};
}

#endif