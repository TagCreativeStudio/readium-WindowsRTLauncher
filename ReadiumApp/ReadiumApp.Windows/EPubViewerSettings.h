#ifndef VIEWERSETTINGS_H
#define VIEWERSETTINGS_H

namespace ReadiumApp
{
	public ref class EPubViewerSettings sealed
	{
	public:
		EPubViewerSettings(bool isSyntheticSpread, int fontSize, int columnGap) :
			isSyntheticSpread(isSyntheticSpread), fontSize(fontSize), columnGap(columnGap)
		{}

		Windows::Data::Json::JsonObject^ toJson()
		{
			using namespace Windows::Data::Json;
			Windows::Data::Json::JsonObject^ json = ref new Windows::Data::Json::JsonObject();
			json->Insert("isSyntheticSpread", JsonValue::CreateBooleanValue(isSyntheticSpread));
			json->Insert("fontSize", JsonValue::CreateNumberValue(fontSize));
			json->Insert("columnGap", JsonValue::CreateNumberValue(columnGap));

			return json;
		}

	private:

		bool isSyntheticSpread;
		int fontSize;
		int columnGap;
	};
}


#endif // VIEWERSETTINGS_H