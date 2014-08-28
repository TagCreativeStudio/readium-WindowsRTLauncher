//
//  filter_chain.cpp
//  ePub3
//
//  Created by Jim Dovey on 2013-08-27.
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

#include "filter_chain.h"
#include "../ePub/manifest.h"
#include "filter.h"
#include "byte_buffer.h"
#include "make_unique.h"
#include <iostream>

#define ASYNC_BUF_SIZE 4096*4

#if !EPUB_OS(WINDOWS)
# define memcpy_s(dst, dstLen, src, srcLen) memcpy(dst, src, srcLen)
#endif

EPUB3_BEGIN_NAMESPACE

std::unique_ptr<thread_pool> FilterChain::_filterThreadPool(nullptr);

class FilterChainSyncStream : public ByteStream
{
private:
	typedef std::pair<ContentFilterPtr, std::unique_ptr<FilterContext>>	_FilterNode;

	std::unique_ptr<ByteStream>		_input;
	std::vector<_FilterNode>		_filters;

	bool							_needs_cache;
	ByteBuffer						_cache;
	ByteBuffer						_read_cache;

public:
	FilterChainSyncStream(std::unique_ptr<ByteStream>&& input, std::vector<ContentFilterPtr>& filters, ConstManifestItemPtr manifestItem);
	virtual ~FilterChainSyncStream() {}

	virtual size_type BytesAvailable() const _NOEXCEPT OVERRIDE
	{
		if (_needs_cache && _input->AtEnd()) {
			return _cache.GetBufferSize();
		} else {
			return _input->BytesAvailable();
		}
	}
	virtual size_type SpaceAvailable() const _NOEXCEPT OVERRIDE
	{
		return 0;
	}
	virtual bool IsOpen() const _NOEXCEPT OVERRIDE
	{
		return _input->IsOpen();
	}
	virtual void Close() OVERRIDE
	{
		_input->Close();
	}
	virtual size_type ReadBytes(void* bytes, size_type len) OVERRIDE;
	virtual size_type WriteBytes(const void* bytes, size_type len) OVERRIDE
	{
		throw std::system_error(std::make_error_code(std::errc::operation_not_supported));
	}

	virtual bool AtEnd() const _NOEXCEPT OVERRIDE
	{
		if (_needs_cache && _input->AtEnd()) {
			return _cache.IsEmpty();
		} else {
			return _input->AtEnd();
		}
	}
	virtual int Error() const _NOEXCEPT OVERRIDE
	{
		return _input->Error();
	}

private:
	size_type ReadBytesFromCache(void* bytes, size_type len);
	void CacheBytes();
	size_type FilterBytes(void* bytes, size_type len);

};

FilterChainSyncStream::FilterChainSyncStream(std::unique_ptr<ByteStream>&& input, std::vector<ContentFilterPtr>& filters, ConstManifestItemPtr manifestItem)
: _input(std::move(input)), _filters(), _needs_cache(false), _cache(), _read_cache()
{
	_cache.SetUsesSecureErasure();
	_read_cache.SetUsesSecureErasure();

	for (auto& filter : filters)
	{
		_filters.emplace_back(filter, std::unique_ptr<FilterContext>(filter->MakeFilterContext(manifestItem)));
		if (filter->RequiresCompleteData() && !_needs_cache)
			_needs_cache = true;
	}
}
ByteStream::size_type FilterChainSyncStream::ReadBytes(void* bytes, size_type len)
{
	if (_needs_cache)
	{
		if (_cache.GetBufferSize() == 0 && _input->AtEnd() == false)
			CacheBytes();

		return ReadBytesFromCache(bytes, len);
	}

	if (_read_cache.GetBufferSize() > 0)
	{
		size_type toMove = std::min(len, _read_cache.GetBufferSize());
		::memcpy_s(bytes, len, _read_cache.GetBytes(), toMove);
		_read_cache.RemoveBytes(toMove);
		return toMove;
	}
	else 
	{
		size_type result = _input->ReadBytes(bytes, len);
		result = FilterBytes(bytes, result);
		size_type toMove = std::min(len, _read_cache.GetBufferSize());
		::memcpy_s(bytes, len, _read_cache.GetBytes(), toMove);
		_read_cache.RemoveBytes(toMove);
		return toMove;
	}
}
ByteStream::size_type FilterChainSyncStream::FilterBytes(void* bytes, size_type len)
{
	size_type result = len;
	ByteBuffer buf(reinterpret_cast<uint8_t*>(bytes), len);
	buf.SetUsesSecureErasure();

	for (auto& pair : _filters)
	{
		size_type filteredLen = 0;
		void* filteredData = pair.first->FilterData(pair.second.get(), buf.GetBytes(), buf.GetBufferSize(), &filteredLen);
		if (filteredData == nullptr || filteredLen == 0) {
			if (filteredData != nullptr && filteredData != bytes)
				delete[] reinterpret_cast<uint8_t*>(filteredData);
			throw std::logic_error("ChainLinkProcessor: ContentFilter::FilterData() returned no data!");
		}

		if (filteredData != bytes)
		{
			buf = ByteBuffer(reinterpret_cast<uint8_t*>(filteredData), filteredLen);
			result = filteredLen;
			delete[] reinterpret_cast<uint8_t*>(filteredData);
		}
	}
	_read_cache = std::move(buf);

	return result;
}
ByteStream::size_type FilterChainSyncStream::ReadBytesFromCache(void* bytes, size_type len)
{
	size_type numToRead = std::min(len, size_type(_cache.GetBufferSize()));
	::memcpy_s(bytes, len, _cache.GetBytes(), numToRead);
	_cache.RemoveBytes(numToRead);
	return numToRead;
}
void FilterChainSyncStream::CacheBytes()
{
	// read everything from the input stream
#define _TMP_BUF_LEN 16*1024
	uint8_t buf[_TMP_BUF_LEN] = {};
	while (_input->AtEnd() == false)
	{
		size_type numRead = _input->ReadBytes(buf, _TMP_BUF_LEN);
		if (numRead == 0)
			break;
		if (numRead > 0)
			_cache.AddBytes(buf, numRead);
	}

	// filter everything completely
	size_type filtered = FilterBytes(_cache.GetBytes(), _cache.GetBufferSize());

	if (filtered > 0)
	{
		_cache = std::move(_read_cache);
		_read_cache.RemoveBytes(_read_cache.GetBufferSize());
	}

	// this potentially contains decrypted data, so use secure erasure
	_cache.SetUsesSecureErasure();
}

std::shared_ptr<AsyncByteStream> FilterChain::GetFilteredOutputStreamForManifestItem(ConstManifestItemPtr item) const
{
    std::unique_ptr<AsyncByteStream> rawInput = item->AsyncReader();
    
    // transfer ownership to a new shared_ptr
    ChainLink input = ChainLink(rawInput.release());
    shared_vector<ChainLinkProcessor> thisChain;
    
    AsyncPipe::Pair linkPipe;
    
    for ( ContentFilterPtr filter : _filters )
    {
        if ( filter->TypeSniffer()(item) )
        {
            if ( !thisChain.empty() )
                thisChain.back()->SetOutputLink(linkPipe.first);
            
            thisChain.push_back(ChainLinkProcessor::New(filter, input, item));
            linkPipe = AsyncPipe::LinkedPair();
            input = linkPipe.second;
        }
    }
    
    // if no filters apply, read raw bytes
    if ( thisChain.empty() )
        return input;
    
    // otherwise, attach the output pipe
    thisChain.back()->SetOutputLink(linkPipe.first);

	static std::once_flag __once;
	std::call_once(__once, [](){
		_filterThreadPool.reset(new thread_pool(thread_pool::Automatic));
	});
    
    // set it to run on a runloop somewhere...
    _filterThreadPool->add([thisChain]() mutable {
        RunLoopPtr myRunLoop = RunLoop::CurrentRunLoop();
        
        auto pos = thisChain.rbegin();
        auto end = thisChain.rend();
        while ( pos != end )
        {
            (*pos)->ScheduleProcessor(myRunLoop);
            ++pos;
        }
        
        // as a file/zip-based async stream, the raw input stream will automatically
        // read bytes from the underlying resource on a shared background thread,
        // thus feeding the chain that follows
        
        // Note that the input stream is returned to us already opened.
        
        // run until all sources have been removed
        myRunLoop->Run();
    });
    
    // return the output pipe
    return linkPipe.second;
}
std::shared_ptr<ByteStream> FilterChain::GetSyncFilteredOutputStreamForManifestItem(ConstManifestItemPtr item) const
{
	std::unique_ptr<ByteStream> rawInput = item->Reader();
	if (rawInput->IsOpen() == false)
		return nullptr;

	std::vector<ContentFilterPtr> thisChain;
	for (ContentFilterPtr filter : _filters)
	{
		if (filter->TypeSniffer()(item))
			thisChain.push_back(filter);
	}

	return std::make_shared<FilterChainSyncStream>(std::move(rawInput), thisChain, item);
}

FilterChain::ChainLinkProcessor::ChainLinkProcessor(ContentFilterPtr filter, ChainLink input, ConstManifestItemPtr item)
  : _filter(filter),
    _context(filter->MakeFilterContext(item)),
    _input(input),
    _output(nullptr),
    _collectionBuffer()
{
}
FilterChain::ChainLinkProcessor::~ChainLinkProcessor()
{
}

void FilterChain::ChainLinkProcessor::ScheduleProcessor(RunLoopPtr runLoop)
{
    if ( _filter->RequiresCompleteData() )
        _collectionBuffer.SetUsesSecureErasure();
    
    std::weak_ptr<ChainLinkProcessor> weakSelf(Ptr());
    _input->SetEventHandler([this, weakSelf](AsyncEvent evt, AsyncByteStream* stream) {
        // we use this variable ONLY to ensure that the 'this' ptr is still valid
        auto strongSelf = weakSelf.lock();
        if ( !bool(strongSelf) )
            return;
        
        switch ( evt )
        {
            case AsyncEvent::HasBytesAvailable:
            {
                // we *know* that all links on a given chain fire events serially on
                // the same thread, so we don't need to dick about with locks here
                
                // there are bytes available to read -- pump as many through as we can
                ssize_t numMoved = FunnelBytes();
                if ( numMoved == 0 && !_output->IsOpen() )
                    stream->Close();
                
                break;
            }
                
            case AsyncEvent::ErrorOccurred:
                std::cerr << "ChainLinkProcessor input stream error: " << stream->Error() << std::endl;
                _output->Close();
                break;
                
            case AsyncEvent::EndEncountered:
            {
                if ( _filter->RequiresCompleteData() )
                {
                    size_t filteredLen = 0;
                    void* filteredData = _filter->FilterData(_context.get(), _collectionBuffer.GetBytes(), _collectionBuffer.GetBufferSize(), &filteredLen);
                    if ( filteredData == nullptr || filteredLen == 0 ) {
                        throw std::logic_error("ChainLinkProcessor: ContentFilter::FilterData() returned no data!");
                    }
                    
                    // (securely) remove all bytes from the buffer
                    _collectionBuffer.RemoveBytes(_collectionBuffer.GetBufferSize());
                    
                    // forward the data
                    uint8_t *outputData = reinterpret_cast<uint8_t*>(filteredData);
                    size_t offset = 0;
                    
                    while ( offset < filteredLen )
                    {
                        offset += _output->WriteBytes(&outputData[offset], filteredLen-offset);
                        if ( offset < filteredLen )
                            std::this_thread::yield();
                    }
                }
                
                _output->Close();
                break;
            }
            default:
                break;
        }
    });
    
    // if this is the last link in the chain, this will ultimately be replaced by the
    // SDK client. If it's only an intermediary link, however, we need to ensure that
    // notification of read-space-available messages is passed back down the chain to
    // the actual resource stream, otherwise the whole process may stall...
    _output->SetEventHandler([this](AsyncEvent evt, AsyncByteStream* stream) {
        switch ( evt )
        {
            case AsyncEvent::HasSpaceAvailable:
            {
                // pull more data through into the output stream
                if ( FunnelBytes() <= 0 && (_input->AtEnd() || !_input->IsOpen()) )
                {
                    stream->Close();
                    return;
                }
                
                break;
            }
                
            case AsyncEvent::ErrorOccurred:
                std::cerr << "ChainLinkProcessor input stream error: " << stream->Error() << std::endl;
                stream->Close();
				if (bool(_input))
					_input->Close();
                break;
                
            case AsyncEvent::EndEncountered:
				if (bool(_input))
					_input->Close();
                break;
                
            default:
                break;
        }
    });
    
    _input->SetTargetRunLoop(runLoop);
    _output->SetTargetRunLoop(runLoop);
}

ssize_t FilterChain::ChainLinkProcessor::FunnelBytes()
{
    uint8_t buf[ASYNC_BUF_SIZE];
    size_t bytesToMove = std::min(_input->BytesAvailable(), _output->SpaceAvailable());
    
    while ( bytesToMove > 0 )
    {
        size_t thisChunk = std::min(size_t(ASYNC_BUF_SIZE), bytesToMove);
        thisChunk = _input->ReadBytes(buf, thisChunk);      // consumes read bytes from the buffer
        
        if ( _filter->RequiresCompleteData() )
        {
            _collectionBuffer.AddBytes(buf, thisChunk);
        }
        else
        {
            size_t filteredLen = 0;
            void* filteredData = _filter->FilterData(_context.get(), buf, thisChunk, &filteredLen);
            if ( filteredData == nullptr || filteredLen == 0 ) {
				if (filteredData != nullptr && filteredData != buf)
					delete[] reinterpret_cast<char*>(filteredData);
                throw std::logic_error("ChainLinkProcessor: ContentFilter::FilterData() returned no data!");
            }

            _output->WriteBytes(filteredData, filteredLen);

			if (filteredData != buf)
				delete[] reinterpret_cast<char*>(filteredData);
        }
        
        bytesToMove -= thisChunk;
    }
    
    return (ssize_t)bytesToMove;
}

EPUB3_END_NAMESPACE
