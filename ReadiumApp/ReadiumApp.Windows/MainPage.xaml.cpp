﻿//
// MainPage.xaml.cpp
// Implementation of the MainPage class
//

#include "pch.h"
#include "MainPage.xaml.h"
#include "PhotoPage.xaml.h"

#include "Log.h"

using namespace ReadiumApp;

using namespace Platform;
using namespace Platform::Collections;
//using namespace Windows::Foundation; // see: http://stackoverflow.com/a/16800980/608884, basically EventRegistrationToken causes the compiler to barf up a ton of errors.
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

MainPage::MainPage()
{
	InitializeComponent();
	SetValue(_defaultViewModelProperty, ref new Map<String^,Object^>(std::less<String^>()));
	auto navigationHelper = ref new Common::NavigationHelper(this);
	SetValue(_navigationHelperProperty, navigationHelper);
	navigationHelper->LoadState += ref new Common::LoadStateEventHandler(this, &MainPage::LoadState);
	navigationHelper->SaveState += ref new Common::SaveStateEventHandler(this, &MainPage::SaveState);
}

DependencyProperty^ MainPage::_defaultViewModelProperty =
	DependencyProperty::Register("DefaultViewModel",
		TypeName(IObservableMap<String^,Object^>::typeid), TypeName(MainPage::typeid), nullptr);

/// <summary>
/// used as a trivial view model.
/// </summary>
IObservableMap<String^, Object^>^ MainPage::DefaultViewModel::get()
{
	return safe_cast<IObservableMap<String^, Object^>^>(GetValue(_defaultViewModelProperty));
}

DependencyProperty^ MainPage::_navigationHelperProperty =
	DependencyProperty::Register("NavigationHelper",
		TypeName(Common::NavigationHelper::typeid), TypeName(MainPage::typeid), nullptr);

/// <summary>
/// Gets an implementation of <see cref="NavigationHelper"/> designed to be
/// used as a trivial view model.
/// </summary>
Common::NavigationHelper^ MainPage::NavigationHelper::get()
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

void MainPage::OnNavigatedTo(NavigationEventArgs^ e)
{
	NavigationHelper->OnNavigatedTo(e);
}

void MainPage::OnNavigatedFrom(NavigationEventArgs^ e)
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
void MainPage::LoadState(Object^ sender, Common::LoadStateEventArgs^ e)
{
	(void) sender;	// Unused parameter
	(void) e;

	httpServer = ref new HttpServer(8080);
	httpServer->Start(); 
	Platform::String^ ip = httpServer->FindIpAddress();
	if (ip == nullptr)
	{
		Log::Debug("[MainPage] IP address is null!");
	}
	else
	{
		Log::Debug("[MainPage] Great success! Hosting at: " + ip);
	}

	init.InitializeSdk();
	controller = ref new WebViewController(reader);
	Windows::Foundation::Uri^ url = ref new Windows::Foundation::Uri("ms-appx-web:///Assets/readium-js/reader.html");
	//Windows::Foundation::Uri^ url = ref new Windows::Foundation::Uri("ms-appx-web:///Assets/readium-js/test.html");
	//url = ref new Windows::Foundation::Uri("http://" + ip + ":8080/reader.html");
	reader->Navigate(url);
}

/// <summary>
/// Preserves state associated with this page in case the application is suspended or the
/// page is discarded from the navigation cache.  Values must conform to the serialization
/// requirements of <see cref="SuspensionManager::SessionState"/>.
/// </summary>
/// <param name="sender">The source of the event; typically <see cref="NavigationHelper"/></param>
/// <param name="e">Event data that provides an empty dictionary to be populated with
/// serializable state.</param>
void MainPage::SaveState(Object^ sender, Common::SaveStateEventArgs^ e){
	(void) sender;	// Unused parameter
	(void) e; // Unused parameter
}

void ReadiumApp::MainPage::SelectEPubBtn_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	auto openPicker = ref new Windows::Storage::Pickers::FileOpenPicker();
	openPicker->SuggestedStartLocation = Windows::Storage::Pickers::PickerLocationId::DocumentsLibrary;
	openPicker->ViewMode = Windows::Storage::Pickers::PickerViewMode::Thumbnail;

	openPicker->FileTypeFilter->Clear();
	openPicker->FileTypeFilter->Append(".epub");

	create_task(openPicker->PickSingleFileAsync())
		.then([this](Windows::Storage::StorageFile^ file)
	{
		if (file != nullptr)
		{
			auto folder = Windows::Storage::ApplicationData::Current->TemporaryFolder;
			auto opt = Windows::Storage::NameCollisionOption::ReplaceExisting;
			mruToken = Windows::Storage::AccessCache::StorageApplicationPermissions::MostRecentlyUsedList->Add(file);
			create_task(file->CopyAsync(folder, file->Name, opt))
				.then([this](Windows::Storage::StorageFile^ copiedFile)
			{
				Log::Debug(copiedFile->Path);
				Readium::Package^ package = api.openFile(copiedFile);
				controller->openBook(package);
			});
		}
	});
}


void ReadiumApp::MainPage::reader_NavigationCompleted(Windows::UI::Xaml::Controls::WebView^ sender, Windows::UI::Xaml::Controls::WebViewNavigationCompletedEventArgs^ args)
{
	Log::Debug("[MainPage] NAVIGATION COMPLETED!");
	Log::Debug("[MainPage] Status " + args->WebErrorStatus.ToString());
}


void ReadiumApp::MainPage::reader_ScriptNotify(Platform::Object^ sender, Windows::UI::Xaml::Controls::NotifyEventArgs^ e)
{
	if (controller != nullptr)
	{
		controller->onScriptNotify(sender, e);
	}
}


void ReadiumApp::MainPage::reader_FrameContentLoading(Windows::UI::Xaml::Controls::WebView^ sender, Windows::UI::Xaml::Controls::WebViewContentLoadingEventArgs^ args)
{
	Log::Debug("[MainPage] FRAME CONTENT LOADING!");
}


void ReadiumApp::MainPage::reader_FrameNavigationStarting(Windows::UI::Xaml::Controls::WebView^ sender, Windows::UI::Xaml::Controls::WebViewNavigationStartingEventArgs^ args)
{
	Log::Debug("[MainPage] FRAME NAVIGATION STARTING: " + args->Uri->ToString());
}


void ReadiumApp::MainPage::prev_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	Log::Debug("[MainPage] Previous page");
	if (controller)
	{
		controller->openPageLeft();
	}
}


void ReadiumApp::MainPage::next_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	Log::Debug("[MainPage] Next page");
	if (controller)
	{
		controller->openPageRight();
	}
}
