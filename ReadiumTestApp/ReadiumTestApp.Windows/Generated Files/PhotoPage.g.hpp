﻿

//------------------------------------------------------------------------------
//     This code was generated by a tool.
//
//     Changes to this file may cause incorrect behavior and will be lost if
//     the code is regenerated.
//------------------------------------------------------------------------------
#include "pch.h"
#include "PhotoPage.xaml.h"




void ::ReadiumTestApp::PhotoPage::InitializeComponent()
{
    if (_contentLoaded)
        return;

    _contentLoaded = true;

    // Call LoadComponent on ms-appx:///PhotoPage.xaml
    ::Windows::UI::Xaml::Application::LoadComponent(this, ref new ::Windows::Foundation::Uri(L"ms-appx:///PhotoPage.xaml"), ::Windows::UI::Xaml::Controls::Primitives::ComponentResourceLocation::Application);

    // Get the Grid named 'contentRoot'
    contentRoot = safe_cast<::Windows::UI::Xaml::Controls::Grid^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"contentRoot"));
    // Get the StackPanel named 'imagePanel'
    imagePanel = safe_cast<::Windows::UI::Xaml::Controls::StackPanel^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"imagePanel"));
    // Get the TextBlock named 'tbName'
    tbName = safe_cast<::Windows::UI::Xaml::Controls::TextBlock^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"tbName"));
    // Get the TextBlock named 'tbPath'
    tbPath = safe_cast<::Windows::UI::Xaml::Controls::TextBlock^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"tbPath"));
    // Get the Image named 'displayImage'
    displayImage = safe_cast<::Windows::UI::Xaml::Controls::Image^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"displayImage"));
    // Get the Button named 'backButton'
    backButton = safe_cast<::Windows::UI::Xaml::Controls::Button^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"backButton"));
    // Get the TextBlock named 'pageTitle'
    pageTitle = safe_cast<::Windows::UI::Xaml::Controls::TextBlock^>(static_cast<Windows::UI::Xaml::IFrameworkElement^>(this)->FindName(L"pageTitle"));
}

void ::ReadiumTestApp::PhotoPage::Connect(int connectionId, Platform::Object^ target)
{
    switch (connectionId)
    {
    case 1:
        (safe_cast<::Windows::UI::Xaml::Controls::Primitives::ButtonBase^>(target))->Click +=
            ref new ::Windows::UI::Xaml::RoutedEventHandler(this, (void (::ReadiumTestApp::PhotoPage::*)(Platform::Object^, Windows::UI::Xaml::RoutedEventArgs^))&PhotoPage::GetPhotoButton_Click);
        break;
    }
    (void)connectionId; // Unused parameter
    (void)target; // Unused parameter
    _contentLoaded = true;
}

