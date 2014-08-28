//
//  library.cpp
//  ePub3
//
//  Created by Jim Dovey on 2012-12-14.
//  Copyright (c) 2014 Readium Foundation and/or its licensees. All rights reserved.
//  
//  This program is distributed in the hope that it will be useful, but WITHOUT ANY 
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
//  
//  Licensed under Gnu Affero General Public License Version 3 (provided, notwithstanding this notice, 
//  Readium Foundation reserves the right to license this material under a different separate license, 
//  and if you have done so, the terms of that separate license control and the following references 
//  to GPL do not apply).
//  
//  This program is free software: you can redistribute it and/or modify it under the terms of the GNU 
//  Affero General Public License as published by the Free Software Foundation, either version 3 of 
//  the License, or (at your option) any later version. You should have received a copy of the GNU 
//  Affero General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "library.h"
#include "container.h"
#include "manifest.h"
#include "package.h"
#include "zip_archive.h"
#include <sstream>
#include <fstream>
#include <list>

// file format is CSV, unencrypted

EPUB3_BEGIN_NAMESPACE

unique_ptr<Library> Library::_singleton(nullptr);

Library::Library(const string& path)
{
    if ( !Load(path) )
        throw std::invalid_argument("The provided Locator doesn't appear to contain library data.");
}
Library::~Library()
{
}
bool Library::Load(const string& path)
{
    std::ifstream stream(path.stl_str());
    
    std::stringstream ss;
    std::string tmp;
    while ( std::getline(stream, tmp).good() )
    {
        try
        {
            ss << tmp;
            
            string thisPath;
            std::list<std::string> uidList;
            while ( !ss.eof() )
            {
                std::getline(ss, tmp, ss.widen(','));
                if ( thisPath.empty() )
                {
                    // first item is a path to a local item
                    thisPath = tmp;
                }
                else
                {
                    // remaining items are unique IDs
                    uidList.emplace_back(tmp);
                }
            }
            
            _containers[thisPath] = nullptr;
            for ( auto uid : uidList )
            {
                _packages[uid] = std::make_pair(thisPath, nullptr);
            }
        }
        catch (...)
        {
            return false;
        }
    }
    
    return true;
}
Library* Library::MainLibrary(const string& path)
{
    static std::once_flag __guard;
    std::call_once(__guard, [&](){ _singleton.reset(new Library(path)); });
    return _singleton.get();
}
string Library::PathForEPubWithUniqueID(const string &uniqueID) const
{
    auto found = _packages.find(uniqueID);
    if ( found == _packages.end() )
        return string::EmptyString;
    
    return found->second.first;
}
string Library::PathForEPubWithPackageID(const string &packageID) const
{
    string uniqueIDStart(packageID + "@");
    for ( auto &pair : _packages )
    {
        if ( pair.first == packageID || pair.first.find(uniqueIDStart) == 0 )
            return pair.second.first;
    }
    
    return string::EmptyString;
}
void Library::AddPublicationsInContainer(shared_ptr<Container> container, const string& path)
{
    // store the container
    auto existing = _containers.find(path);
    if ( existing == _containers.end() )
        _containers[path] = container;
    
    for ( auto pkg : container->Packages() )
    {
#if EPUB_HAVE(CXX_MAP_EMPLACE)
        _packages.emplace(pkg->UniqueID(), LookupEntry(std::make_pair(path, pkg)));
#else
        _packages[pkg->UniqueID()] = LookupEntry({path, pkg});
#endif
    }
}
void Library::AddPublicationsInContainerAtPath(const ePub3::string &path)
{
    ContainerPtr p = Container::OpenContainer(path);
    if ( p )
        AddPublicationsInContainer(p, path);
}
IRI Library::EPubURLForPublication(shared_ptr<Package> package) const
{
    return EPubURLForPublicationID(package->UniqueID());
}
IRI Library::EPubURLForPublicationID(const string &identifier) const
{
    return IRI(IRI::gEPUBScheme, identifier, "/");
}
shared_ptr<Package> Library::PackageForEPubURL(const IRI &url, bool allowLoad)
{
    // is it an epub URL?
    if ( url.Scheme() != IRI::gEPUBScheme )
        return nullptr;
    
    string ident = url.Host();
    auto entry = _packages.find(ident);
    if ( entry == _packages.end() )
        return nullptr;
    
    if ( entry->second.second != nullptr || !allowLoad )
        return entry->second.second;
    
    AddPublicationsInContainerAtPath(entry->second.first);
    
    // returns a package ptr or nullptr
    return entry->second.second;
}
IRI Library::EPubCFIURLForManifestItem(ManifestItemPtr item) const
{
    IRI packageURL = EPubURLForPublication(item->Owner());
    packageURL.SetContentFragmentIdentifier(item->Owner()->CFIForManifestItem(item));
    return packageURL;
}
shared_ptr<ManifestItem> Library::ManifestItemForCFI(const IRI &urlWithCFI, CFI* pRemainingCFI)
{
    CFI cfi = urlWithCFI.ContentFragmentIdentifier();
    if ( cfi.Empty() )
        return nullptr;
    
    PackagePtr pkg = PackageForEPubURL(urlWithCFI);
    if ( pkg == nullptr )
        return nullptr;
    
    return pkg->ManifestItemForCFI(cfi, pRemainingCFI);
}
unique_ptr<ByteStream> Library::ReadStreamForEPubURL(const IRI &url, CFI *pRemainingCFI)
{
    CFI cfi = url.ContentFragmentIdentifier();
    if ( cfi.Empty() )
    {
        // it references a content document directly
        PackagePtr pkg = PackageForEPubURL(url);
        if ( pkg )
            return pkg->ReadStreamForItemAtPath(url.Path());
    }
    else
    {
        ManifestItemPtr item = ManifestItemForCFI(url, pRemainingCFI);
        if ( item != nullptr )
            return item->Reader();
    }
    
    return nullptr;
}
bool Library::WriteToFile(const string& path) const
{
    std::ofstream stream(path.stl_str());
    for ( auto item : _containers )
    {
        // works like an auto_ptr, scoping the allocation
        ContainerPtr pContainer = item.second;
        
        if ( !pContainer )
            pContainer = Container::OpenContainer(item.first);
        if ( !pContainer )
            continue;
        
        stream << item.first;
        for ( auto pkg : pContainer->Packages() )
        {
            stream << "," << pkg->UniqueID();
        }
        
        stream << std::endl;
    }
    
    return true;
}

EPUB3_END_NAMESPACE
