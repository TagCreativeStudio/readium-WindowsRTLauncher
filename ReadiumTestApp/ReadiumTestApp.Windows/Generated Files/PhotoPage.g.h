﻿

#pragma once
//------------------------------------------------------------------------------
//     This code was generated by a tool.
//
//     Changes to this file may cause incorrect behavior and will be lost if
//     the code is regenerated.
//------------------------------------------------------------------------------

namespace Windows {
    namespace UI {
        namespace Xaml {
            namespace Controls {
                ref class Grid;
                ref class StackPanel;
                ref class TextBlock;
                ref class Image;
                ref class Button;
            }
        }
    }
}

namespace ReadiumApp
{
    partial ref class PhotoPage : public ::Windows::UI::Xaml::Controls::Page, 
        public ::Windows::UI::Xaml::Markup::IComponentConnector
    {
    public:
        void InitializeComponent();
        virtual void Connect(int connectionId, ::Platform::Object^ target);
    
    private:
        bool _contentLoaded;
    
        private: ::Windows::UI::Xaml::Controls::Grid^ contentRoot;
        private: ::Windows::UI::Xaml::Controls::StackPanel^ imagePanel;
        private: ::Windows::UI::Xaml::Controls::TextBlock^ tbName;
        private: ::Windows::UI::Xaml::Controls::TextBlock^ tbPath;
        private: ::Windows::UI::Xaml::Controls::Image^ displayImage;
        private: ::Windows::UI::Xaml::Controls::Button^ backButton;
        private: ::Windows::UI::Xaml::Controls::TextBlock^ pageTitle;
    };
}

