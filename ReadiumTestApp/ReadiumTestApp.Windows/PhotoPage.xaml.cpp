//
// PhotoPage.xaml.cpp
// Implementation of the PhotoPage class
//

#include "pch.h"
#include "PhotoPage.xaml.h"

using namespace ReadiumTestApp;

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

using namespace concurrency;

// The Basic Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234237

PhotoPage::PhotoPage()
{
	InitializeComponent();
	SetValue(_defaultViewModelProperty, ref new Map<String^,Object^>(std::less<String^>()));
	auto navigationHelper = ref new Common::NavigationHelper(this);
	SetValue(_navigationHelperProperty, navigationHelper);
	navigationHelper->LoadState += ref new Common::LoadStateEventHandler(this, &PhotoPage::LoadState);
	navigationHelper->SaveState += ref new Common::SaveStateEventHandler(this, &PhotoPage::SaveState);
}

DependencyProperty^ PhotoPage::_defaultViewModelProperty =
	DependencyProperty::Register("DefaultViewModel",
		TypeName(IObservableMap<String^,Object^>::typeid), TypeName(PhotoPage::typeid), nullptr);

/// <summary>
/// used as a trivial view model.
/// </summary>
IObservableMap<String^, Object^>^ PhotoPage::DefaultViewModel::get()
{
	return safe_cast<IObservableMap<String^, Object^>^>(GetValue(_defaultViewModelProperty));
}

DependencyProperty^ PhotoPage::_navigationHelperProperty =
	DependencyProperty::Register("NavigationHelper",
		TypeName(Common::NavigationHelper::typeid), TypeName(PhotoPage::typeid), nullptr);

/// <summary>
/// Gets an implementation of <see cref="NavigationHelper"/> designed to be
/// used as a trivial view model.
/// </summary>
Common::NavigationHelper^ PhotoPage::NavigationHelper::get()
{
	return safe_cast<Common::NavigationHelper^>(GetValue(_navigationHelperProperty));
}

#pragma region Navigation support

/// The methods provided in this section are simply used to allow
/// NavigationHelper to respond to the page's navigation methods.
/// 
/// Page specific logic should be placed in event handlers for the  
/// <see cref="NavigationHelper::LoadState"/>
/// and <see cref="NavigationHelper::SaveState"/>.
/// The navigation parameter is available in the LoadState method 
/// in addition to page state preserved during an earlier session.

void PhotoPage::OnNavigatedTo(NavigationEventArgs^ e)
{
	NavigationHelper->OnNavigatedTo(e);
}

void PhotoPage::OnNavigatedFrom(NavigationEventArgs^ e)
{
	NavigationHelper->OnNavigatedFrom(e);
}

#pragma endregion

/// <summary>
/// Populates the page with content passed during navigation. Any saved state is also
/// provided when recreating a page from a prior session.
/// </summary>
/// <param name="sender">
/// The source of the event; typically <see cref="NavigationHelper"/>
/// </param>
/// <param name="e">Event data that provides both the navigation parameter passed to
/// <see cref="Frame.Navigate(Type, Object)"/> when this page was initially requested and
/// a dictionary of state preserved by this page during an earlier
/// session. The state will be null the first time a page is visited.</param>
void PhotoPage::LoadState(Object^ sender, Common::LoadStateEventArgs^ e)
{
	(void) sender;	// Unused parameter
	(void) e;	// Unused parameter

	if (e->PageState != nullptr && e->PageState->HasKey("mruToken"))
	{
		Object^ value = e->PageState->Lookup("mruToken");
		if (value != nullptr)
		{
			mruToken = value->ToString();

			create_task(Windows::Storage::AccessCache::StorageApplicationPermissions::MostRecentlyUsedList->GetFileAsync(mruToken))
				.then([this](Windows::Storage::StorageFile^ file)
			{
				if (file != nullptr)
				{
					// Open a stream for the selected file->
					//  Windows.Storage->Streams.IRandomAccessStream fileStream =
					create_task(file->OpenAsync(Windows::Storage::FileAccessMode::Read))
						.then([this, file](Windows::Storage::Streams::IRandomAccessStream^ fileStream)
					{
						// Set the image source to a bitmap.
						auto bitmapImage =
							ref new Windows::UI::Xaml::Media::Imaging::BitmapImage();

						bitmapImage->SetSource(fileStream);
						displayImage->Source = bitmapImage;

						// Set the data context for the page
						this->DataContext = file;
					});
				}
			});
		}
	}
}

/// <summary>
/// Preserves state associated with this page in case the application is suspended or the
/// page is discarded from the navigation cache.  Values must conform to the serialization
/// requirements of <see cref="SuspensionManager::SessionState"/>.
/// </summary>
/// <param name="sender">The source of the event; typically <see cref="NavigationHelper"/></param>
/// <param name="e">Event data that provides an empty dictionary to be populated with
/// serializable state.</param>
void PhotoPage::SaveState(Object^ sender, Common::SaveStateEventArgs^ e){
	(void)sender;	// Unused parameter
	(void) e; // Unused parameter
	
	if (mruToken != nullptr && !mruToken->IsEmpty())
	{
		e->PageState->Insert("mruToken", mruToken);
	}
}


void ReadiumTestApp::PhotoPage::GetPhotoButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	auto openPicker = ref new Windows::Storage::Pickers::FileOpenPicker();
	openPicker->SuggestedStartLocation = Windows::Storage::Pickers::PickerLocationId::PicturesLibrary;
	openPicker->ViewMode = Windows::Storage::Pickers::PickerViewMode::Thumbnail;

	openPicker->FileTypeFilter->Clear();
	openPicker->FileTypeFilter->Append(".bmp");
	openPicker->FileTypeFilter->Append(".png");
	openPicker->FileTypeFilter->Append(".jpeg");
	openPicker->FileTypeFilter->Append(".jpg");

	create_task(openPicker->PickSingleFileAsync())
	.then([this](Windows::Storage::StorageFile^ file)
	{
		if (file != nullptr)
		{
			create_task([file]()
			{
				return file->OpenAsync(Windows::Storage::FileAccessMode::Read);
			})
			.then([this, file](Windows::Storage::Streams::IRandomAccessStream^ fileStream)
			{
				Windows::UI::Xaml::Media::Imaging::BitmapImage^ bmp = ref new Windows::UI::Xaml::Media::Imaging::BitmapImage();
				bmp->SetSource(fileStream);
				displayImage->Source = bmp;
				this->DataContext = file;
			})
			.then([this, file]()
			{
				mruToken = Windows::Storage::AccessCache::StorageApplicationPermissions::MostRecentlyUsedList->Add(file);
			});
		}
	});
		/*if (file == nullptr)
		{
			cancel_current_task();
		}

		this->DataContext = file;
		mruToken = Windows::Storage::AccessCache::StorageApplicationPermissions::MostRecentlyUsedList->Add(file);

		return file->OpenAsync(Windows::Storage::FileAccessMode::Read);
	})
	.then([this](Windows::Storage::Streams::IRandomAccessStream^ fileStream)
	{
		Windows::UI::Xaml::Media::Imaging::BitmapImage^ bmp = ref new Windows::UI::Xaml::Media::Imaging::BitmapImage();
		bmp->SetSource(fileStream);

		displayImage->Source = bmp;
	});*/
}
