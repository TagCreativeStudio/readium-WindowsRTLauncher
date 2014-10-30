using Readium;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Storage.Streams;
using Windows.Web;
using Windows.Foundation;
using Windows.Storage;

namespace Redium_XAML.Common
{
    class WebViewStreamResolver : IUriToStreamResolver
    {
        /// <summary>
        /// Readium::Package instance representing an entire ePub file.  UriToStreamAsync may return a stream from this ePub file.
        /// </summary>
        public Package package
        {
            set;
            get;
        }

        public WebViewStreamResolver()
        {

        }

        public WebViewStreamResolver(Package package)
        {
            this.package = package;
        }

        /// <summary>
        /// Takes a URI and determines where the IInputStream for that URI should come from.
        /// </summary>
        /// <param name="uri">URI requested by the webview</param>
        /// <returns>A promise to produce an IInputStream containing the requested resource</returns>
        IAsyncOperation<IInputStream> IUriToStreamResolver.UriToStreamAsync(Uri uri)
        {
            if (uri == null)
            {
                throw new ArgumentNullException("Uri should not be null");
            }

            var relativePath = uri.PathAndQuery;

            Uri appDataUri = new Uri("ms-appx:///Assets" + relativePath);
            Debug.WriteLine("[WebViewStreamResolver] loading: " + appDataUri);

            if (relativePath.StartsWith("/"))
            {
                relativePath = relativePath.Substring(1);
            }

            String[] split = relativePath.Split('/');

            // this occurs when the request uri is 'ms-appx:///Assets/readium-js/'
            // not sure why it happens
            if (split.Length == 0)
            {
                return null;
            }

            // the requested file is a local asset, serve it from the filesystem
            else if (split[0].Equals("readium-js"))
            {
                if (split.Length == 1)
                {
                    return null;
                }
                else
                {
                    var streamTask = GetFileStreamFromApplicationUriAsync(appDataUri);
                    return streamTask.AsAsyncOperation<IInputStream>();
                }
            }

            // the requested file is in the ePub document
            else
            {
                var streamTask = GetZipStreamFromPackageAsync(relativePath);
                return streamTask.AsAsyncOperation<IInputStream>();
            }
        }

        /// <summary>
        /// Used to serve requests for resources from the filesystem
        /// </summary>
        /// <param name="appDataUri">The request URI</param>
        /// <returns>A promise to produce an IInputStream containing the requested resource</returns>
        private async Task<IInputStream> GetFileStreamFromApplicationUriAsync(Uri appDataUri)
        {
            var storageFile = await StorageFile.GetFileFromApplicationUriAsync(appDataUri);

            var fileStream = await storageFile.OpenAsync(FileAccessMode.Read);
            return fileStream.GetInputStreamAt(0);
        }

        /// <summary>
        /// Used to serve requests for resources from an ePub
        /// </summary>
        /// <param name="relativePath">Path to requested resource, relative to the ePub archive's base</param>
        /// <returns>A promise to produce an IInputStream containing the requested resource</returns>
        private async Task<IInputStream> GetZipStreamFromPackageAsync(string relativePath)
        {
            IClosableStream stream = this.package.ReadStreamForRelativePath(relativePath);
            uint readSize = (uint)stream.Size;

            // read the entirety of the resource into memory
            DataReader reader = new DataReader(stream);
            uint actualReadBytes = await reader.LoadAsync(readSize);
            IBuffer contentBuffer = reader.DetachBuffer();

            // close the ZipStream
            stream.Close();

            // create a IInputStream out of our in-memory buffer and return it
            InMemoryRandomAccessStream memoryStream = new InMemoryRandomAccessStream();
            DataWriter writer = new DataWriter(memoryStream.GetOutputStreamAt(0));
            writer.WriteBuffer(contentBuffer);
            uint wroteBytes = await writer.StoreAsync();
            return memoryStream.GetInputStreamAt(0);
        }
    }
}
