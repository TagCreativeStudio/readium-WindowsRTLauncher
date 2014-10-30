using Readium;
using Redium_SDK;
using Redium_XAML.Common;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Storage;
using Windows.Storage.AccessCache;
using Windows.Storage.Pickers;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

// The Basic Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234237

namespace Redium_XAML
{
    /// <summary>
    /// A basic page that provides characteristics common to most applications.
    /// </summary>
    public sealed partial class MainPage : Page
    {

        private NavigationHelper navigationHelper;
        private ObservableDictionary defaultViewModel = new ObservableDictionary();

        /// <summary>
        /// This can be changed to a strongly typed view model.
        /// </summary>
        public ObservableDictionary DefaultViewModel
        {
            get { return this.defaultViewModel; }
        }

        /// <summary>
        /// NavigationHelper is used on each page to aid in navigation and 
        /// process lifetime management
        /// </summary>
        public NavigationHelper NavigationHelper
        {
            get { return this.navigationHelper; }
        }


        public MainPage()
        {
            this.InitializeComponent();
            this.navigationHelper = new NavigationHelper(this);
            this.navigationHelper.LoadState += navigationHelper_LoadState;
            this.navigationHelper.SaveState += navigationHelper_SaveState;
        }

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
        private void navigationHelper_LoadState(object sender, LoadStateEventArgs e)
        {
            init = new Initialization();
            init.InitializeSdk();
            controller = new WebViewController(reader);
            Uri url = new Uri("ms-appx-web:///Assets/readium-js/reader.html");
            resolver = new WebViewStreamResolver();
            url = reader.BuildLocalStreamUri("Redium", "/readium-js/reader.html");
            reader.NavigateToLocalStreamUri(url, resolver);
        }

        /// <summary>
        /// Preserves state associated with this page in case the application is suspended or the
        /// page is discarded from the navigation cache.  Values must conform to the serialization
        /// requirements of <see cref="SuspensionManager.SessionState"/>.
        /// </summary>
        /// <param name="sender">The source of the event; typically <see cref="NavigationHelper"/></param>
        /// <param name="e">Event data that provides an empty dictionary to be populated with
        /// serializable state.</param>
        private void navigationHelper_SaveState(object sender, SaveStateEventArgs e)
        {
        }

        #region NavigationHelper registration

        /// The methods provided in this section are simply used to allow
        /// NavigationHelper to respond to the page's navigation methods.
        /// 
        /// Page specific logic should be placed in event handlers for the  
        /// <see cref="GridCS.Common.NavigationHelper.LoadState"/>
        /// and <see cref="GridCS.Common.NavigationHelper.SaveState"/>.
        /// The navigation parameter is available in the LoadState method 
        /// in addition to page state preserved during an earlier session.

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            navigationHelper.OnNavigatedTo(e);
        }

        protected override void OnNavigatedFrom(NavigationEventArgs e)
        {
            navigationHelper.OnNavigatedFrom(e);
        }

        #endregion

        private string mruToken;
        private EPubSdkApi api = new EPubSdkApi();
        private WebViewStreamResolver resolver;
        private WebViewController controller;
        private Initialization init;
        
        private async void selectEpubBtn_Click(object sender, RoutedEventArgs e)
        {
            var openPicker = new FileOpenPicker();
            openPicker.SuggestedStartLocation = PickerLocationId.DocumentsLibrary;
            openPicker.ViewMode = PickerViewMode.List;

            openPicker.FileTypeFilter.Clear();
            openPicker.FileTypeFilter.Add(".epub");

            var file = await openPicker.PickSingleFileAsync();

            if (file != null)
            {
                var folder = ApplicationData.Current.TemporaryFolder;
                var opt = NameCollisionOption.ReplaceExisting;

                mruToken = StorageApplicationPermissions.MostRecentlyUsedList.Add(file);

                var copiedFile = await file.CopyAsync(folder, file.Name, opt);

                var package = api.openFile(copiedFile);
                resolver.package = package;
                controller.openBook(package);

                Debug.WriteLine("[MainPage] File selected and coped to: " + copiedFile.Path);
            }
        }

        private void prev_Click(object sender, RoutedEventArgs e)
        {
            Debug.WriteLine("[MainPage] Previous page");
	        if (controller != null)
	        {
		        controller.openPageLeft();
	        }
        }

        private void next_Click(object sender, RoutedEventArgs e)
        {
            Debug.WriteLine("[MainPage] Next page");
	        if (controller != null)
	        {
		        controller.openPageRight();
	        }
        }

        private void reader_NavigationCompleted(WebView sender, WebViewNavigationCompletedEventArgs args)
        {
            Debug.WriteLine("[MainPage] NAVIGATION COMPLETED!");
	        Debug.WriteLine("[MainPage] Status " + args.WebErrorStatus.ToString());
        }

        private void reader_ScriptNotify(object sender, NotifyEventArgs e)
        {
            if (controller != null)
            {
                controller.onScriptNotify(sender, e);
            }
        }

        private void reader_FrameContentLoading(WebView sender, WebViewContentLoadingEventArgs args)
        {
            Debug.WriteLine("[MainPage] FRAME CONTENT LOADING!");
        }

        private void reader_FrameNavigationStarting(WebView sender, WebViewNavigationStartingEventArgs args)
        {
            Debug.WriteLine("[MainPage] FRAME NAVIGATION STARTING: " + args.Uri.ToString());
        }
    }
}
