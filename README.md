# Readium Test App for Windows 8.1

This is a Windows Store test app that exists to determine if/how we can get Readium working on Windows 8.1. __NOTE:__ there is no chance this will work on Windows 8.__0__.

## Table of Contents

- [Dependencies](#dependencies)
- [Linking to the Readium SDK](#linking-to-the-readium-sdk)
- [Passing Cert](#passing-cert)
- [Copying Readium Web Content](#copying-readium-web-content)
- [Displaying an ePub in a WebView](#displaying-an-epub-in-a-webview)

## Dependencies

- [Readium SDK](https://github.com/TagCreativeStudio/readium-sdk), use the `win8` branch.
- Visual Studio 2013.
- Windows 8.1 SDK.
- Patience.

## Linking to the Readium SDK

There are to ways to link to the Readium SDK: a direct reference to the `.winmd` file, or using the system-installed version of the SDK (after running the VSIX installer).
The `.winmd` files (basically you can think of these as DLLs for WinRT) for both Release and Debug builds are in this repo in the `ReadiumApp\ReadiumApp.Windows\Readium` folder. 

### Linking directly to .winmd file

As we're linking the `.winmd` files rather than including the Readium SDK project in the app solution you _must_ change the `.winmd` reference to the proper build type (i.e., if you're making a Release build you must link to the Release version of the `.winmd`).

Change which `.winmd` file is being referenced by right-clicking the __ReadiumApp.Window__ project in the Solution Explorer then selecting __Properties...__. In the left-hand pane select __References__ under __Common Properties__, then click the __Add New Reference...__ button to link to the correct `.winmd` file as described earlier.

### Linking to VSIX SDK

Go to __Add New Reference...__ as above, but select` Windows 8.1 > Extensions > Readium SDK` instead of selecting the `.winmd` file directly.

## Passing Cert

All Windows Store apps must pass all the tests in the Windows App Cert Kit v3.4 (this should be installed along either the Windows SDK or Visual Studio).

Only the Release build of the app will pass cert as Debug builds are not allowed on the Store.

## Copying Readium Web Content

In the `Assets` folder of the Windows project there is a sub-folder called `readium-js` that is required to load web content. Currently this folder is not part of the project (and I'm unable to add it for some reason or another) and must be manually copied to the output folder which is in the root of the repo and called `Debug` or `Release` depending on how you're building.

## Displaying an ePub in a WebView

To display an ePub we use WebView's [NavigateToLocalStreamUri](http://msdn.microsoft.com/en-us/library/windows/apps/windows.ui.xaml.controls.webview.navigatetolocalstreamuri.aspx). This allows us to provide a stream resolver which effectively acts as an HTTP server (HTTP servers are what all the other launcher apps use but there is no off-the-shelf HTTP server for WinRT). Microsoft's explanation and documentation of the functionality is somewhat..lacking...but [this helpful article](http://blogs.msdn.com/b/wsdevsol/archive/2014/06/20/a-primer-on-webview-navigatetolocalstreamuri.aspx) does a good job of explaing how `NavigateToLocalStreamUri` works with examples.