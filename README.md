# Readium Test App for Windows 8.1

This is a Windows Store test app that exists to determine if/how we can get Readium working on Windows 8.1. __NOTE:__ there is no chance this will work on Windows 8.__0__.

## Table of Contents

- [Dependencies](#dependencies)
- [Linking to the Readium SDK](#linking-to-the-readium-sdk)
- [Passing Cert](#passing-cert)
- [Copying Readium Web Content](#copying-readium-web-content)

## Dependencies

- [Readium SDK](https://bitbucket.org/tagdevelopers/readium-sdk), use the `win8` branch.
- Visual Studio 2013.
- Windows 8.1 SDK.
- Patience.

## Linking to the Readium SDK

The `.winmd` files (basically you can think of these as DLLs for WinRT) for both Release and Debug builds are in this repo in the `ReadiumApp\ReadiumApp.Windows\Readium` folder. 

As we're linking the `.winmd` files rather than including the Readium SDK project in the app solution you _must_ change the `.winmd` reference to the proper build type (i.e., if you're making a Release build you must link to the Release version of the `.winmd`).

Change which `.winmd` file is being referenced by right-clicking the __ReadiumApp.Window__ project in the Solution Explorer then selecting __Properties...__. In the left-hand pane select __References__ under __Common Properties__, then click the __Add New Reference...__ button to link to the correct `.winmd` file as described earlier.

## Passing Cert

All Windows Store apps must pass all the tests in the Windows App Cert Kit v3.4 (this should be installed along either the Windows SDK or Visual Studio).

Only the Release build of the app will pass cert as Debug builds are not allowed on the Store.

## Copying Readium Web Content

In the `Assets` folder of the Windows project there is a sub-folder called `readium-js` that is required to load web content. Currently this folder is not part of the project (and I'm unable to add it for some reason or another) and must be manually copied to the output folder which is in the root of the repo and called `Debug` or `Release` depending on how you're building.