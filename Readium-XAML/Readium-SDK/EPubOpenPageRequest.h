#ifndef EPUBOPENPAGEREQUEST_H
#define EPUBOPENPAGEREQUEST_H

namespace Readium_SDK
{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class EPubOpenPageRequest sealed
	{
	public:
		EPubOpenPageRequest(Platform::String^ idref, int spineItemPageIndex, 
			Platform::String^ elementCfi, Platform::String^ contentRefUrl,
			Platform::String^ sourceFileHref, Platform::String^ elementId) :
			
			idref(idref), spineItemPageIndex(spineItemPageIndex),
			elementCfi(elementCfi), contentRefUrl(contentRefUrl),
			sourceFileHref(sourceFileHref), elementId(elementId)
		{}

		static EPubOpenPageRequest^ fromIdref(Platform::String^ idref)
		{
			return ref new EPubOpenPageRequest(idref, 0, "", "", "", "");
		}

		static EPubOpenPageRequest^ fromIdrefAndIndex(Platform::String^ idref, int spineItemPageIndex)
		{
			return ref new EPubOpenPageRequest(idref, spineItemPageIndex, "", "", "", "");
		}

		static EPubOpenPageRequest^ fromContentUrl(Platform::String^ contentRefUrl, Platform::String^ sourceFileHref)
		{
			return ref new EPubOpenPageRequest("", 0, "", contentRefUrl, sourceFileHref, "");
		}

		static EPubOpenPageRequest^ fromElementId(Platform::String^ idref, Platform::String^ elementId)
		{
			return ref new EPubOpenPageRequest(idref, 0, "", "", "", elementId);
		}

		Windows::Data::Json::JsonObject^ toJson()
		{
			using namespace Windows::Data::Json;
			JsonObject^ json = ref new JsonObject();

			json->Insert("idref", JsonValue::CreateStringValue(idref));
			json->Insert("spineItemPageIndex", JsonValue::CreateNumberValue(spineItemPageIndex));
			json->Insert("elementCfi", JsonValue::CreateStringValue(elementCfi));
			json->Insert("contentRefUrl", JsonValue::CreateStringValue(contentRefUrl));
			json->Insert("sourceFileHref", JsonValue::CreateStringValue(sourceFileHref));
			json->Insert("elementId", JsonValue::CreateStringValue(elementId));

			return json;
		}

	private:

		Platform::String^ idref;
		int spineItemPageIndex;
		Platform::String^ elementCfi;
		Platform::String^ contentRefUrl;
		Platform::String^ sourceFileHref;
		Platform::String^ elementId;
	};
}

#endif // EPUBOPENPAGEREQUEST_H