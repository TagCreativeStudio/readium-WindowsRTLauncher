#include "pch.h"
#include "EPubSdkApi.h"

using namespace Redium_SDK;
using namespace Readium;

using namespace Windows::Data::Json;

EPubSdkApi::EPubSdkApi()
{
}

void EPubSdkApi::readPackages()
{

}

Readium::Package^ EPubSdkApi::openFile(Windows::Storage::IStorageFile^ file)
{
	container = Readium::Container::OpenContainer(file);
	if (!container)
	{
		Log::Debug("[EPubSdkApi] Unable to open container.");
		return nullptr;
	}

	packages = container->Packages();
	if (!packages)
	{
		Log::Debug("[EPubSdkApi] Could not retrieve pacakages from container.");
		return nullptr;
	}

	if (packages->Size <= 0)
	{
		Log::Debug("[EPubSdkApi] No packages in container.");
		return nullptr;
	}
	else
	{
		currentPackage = packages->GetAt(0);
	}

	return currentPackage;
}

Windows::Data::Json::JsonObject^ EPubSdkApi::openBook(Readium::Package^ bookPackage, EPubViewerSettings^ viewerSettings, EPubOpenPageRequest^ req)
{
	JsonObject^ json = ref new JsonObject();

	json->Insert("package", packageToJson(bookPackage));
	json->Insert("settings", viewerSettings->toJson());
	json->Insert("openPageRequest", req->toJson());

	return json;
}

Windows::Data::Json::JsonObject^ EPubSdkApi::packageToJson(Readium::Package^ package)
{
	JsonObject^ json = ref new JsonObject();

	json->Insert("rootUrl", JsonValue::CreateStringValue("/"));
	json->Insert("rendition_layout", JsonValue::CreateStringValue(""));
	json->Insert("rendition_flow", JsonValue::CreateStringValue(""));

	JsonObject^ spine = ref new JsonObject();
	JsonArray^ spineItems = ref new JsonArray();

	unsigned int idx = 0;
	Readium::SpineItem^ spineItem = package->SpineItemAt(idx++);
	while (spineItem != nullptr)
	{
		JsonObject^ currItem = ref new JsonObject();
		Readium::ManifestItem^ manifestItem = spineItem->ManifestItemRef;
		if (manifestItem)
		{
			currItem->Insert("href", JsonValue::CreateStringValue(manifestItem->BaseHref));
		}
		else
		{
			currItem->Insert("href", JsonValue::CreateStringValue(""));
		}

		currItem->Insert("idref", JsonValue::CreateStringValue(spineItem->Idref));

		currItem->Insert("page_spread", JsonValue::CreateStringValue(""));
		currItem->Insert("rendition_layout", JsonValue::CreateStringValue(""));
		currItem->Insert("rendition_spread", JsonValue::CreateStringValue(""));
		currItem->Insert("rendition_flow", JsonValue::CreateStringValue(""));
		currItem->Insert("media_overlay_id", JsonValue::CreateStringValue(""));
		currItem->Insert("media_type", JsonValue::CreateStringValue(manifestItem->MediaOverlayID));

		spineItems->Append(currItem);

		spineItem = package->SpineItemAt(idx++);
	}

	spine->Insert("items", spineItems);
	spine->Insert("direction", JsonValue::CreateStringValue("default"));
	json->Insert("spine", spine);

	JsonObject^ mediaOverlay = ref new JsonObject();

	{
		Readium::MediaOverlaysSMILModel^ smilModel = package->SMILModel;
		unsigned int n = smilModel->SMILDataCount;
		JsonArray^ smilModels = ref new JsonArray();
		unsigned int i = 0;

		for (i = 0; i < n; ++i)
		{
			Readium::SMILData^ currSmil = smilModel->SMILDataAt(i);
			JsonObject^ model = ref new JsonObject();

			if (currSmil->XhtmlSpineItem)
			{
				model->Insert("spineItemId", JsonValue::CreateStringValue(currSmil->XhtmlSpineItem->Idref));
			}
			else
			{
				model->Insert("spineItemId", JsonValue::CreateStringValue(""));
			}

			std::ostringstream ss;
			ss << currSmil->DurationMilliseconds_Calculated / 1000.0f;
			std::string s = ss.str();
			std::wstring ws(s.begin(), s.end());
			Platform::String^ str = ref new Platform::String(ws.c_str());
			model->Insert("duration", JsonValue::CreateStringValue(str));

			if (currSmil->SmilManifestItem)
			{
				model->Insert("id", JsonValue::CreateStringValue(currSmil->SmilManifestItem->Identifier));
				model->Insert("href", JsonValue::CreateStringValue(currSmil->SmilManifestItem->Href));
			}
			else
			{
				model->Insert("id", JsonValue::CreateStringValue(""));
				model->Insert("href", JsonValue::CreateStringValue("fake.smil"));
			}

			model->Insert("smilVersion", JsonValue::CreateStringValue("3.0"));

			Readium::ISMILSequence^ seqBody = currSmil->Body;
			//JsonArray^ arrChildren = getJson(seqBody);
			//model->Insert("children", arrChildren);

			smilModels->Append(model);
		}

		mediaOverlay->Insert("smil_models", smilModels);

		JsonArray^ skippables = ref new JsonArray();
		JsonArray^ escapables = ref new JsonArray();

		smilModel = package->SMILModel;
		if (smilModel)
		{
			unsigned int count = smilModel->SkippablesCount;
			for (unsigned int i = 0; i < count; ++i)
			{
				Platform::String^ skip = smilModel->SkippableAt(i);
				skippables->Append(JsonValue::CreateStringValue(skip));
			}

			count = smilModel->EscapablesCount;
			for (unsigned int i = 0; i < count; ++i)
			{
				Platform::String^ esc = smilModel->EscapableAt(i);
				skippables->Append(JsonValue::CreateStringValue(esc));
			}
		}

		mediaOverlay->Insert("skippables", skippables);
		mediaOverlay->Insert("escapables", escapables);
		
	}
	
	json->Insert("media_overlay", mediaOverlay);

	return json;
}
