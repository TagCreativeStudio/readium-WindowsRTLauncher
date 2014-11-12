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

namespace Readium_XAML.Common
{
    class WebViewStreamResolver : IUriToStreamResolver
    {
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

            if (split.Length == 0)
            {
                return null;
            }
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
            else
            {
                var streamTask = GetZipStreamFromPackageAsync(relativePath);
                return streamTask.AsAsyncOperation<IInputStream>();
            }
        }

        private async Task<IInputStream> GetFileStreamFromApplicationUriAsync(Uri appDataUri)
        {
            var storageFile = await StorageFile.GetFileFromApplicationUriAsync(appDataUri);

            var fileStream = await storageFile.OpenAsync(FileAccessMode.Read);
            return fileStream.GetInputStreamAt(0);
        }

        private async Task<IInputStream> GetZipStreamFromPackageAsync(string relativePath)
        {
            IClosableStream stream = this.package.ReadStreamForRelativePath(relativePath);
            uint readSize = (uint)stream.Size;

            DataReader reader = new DataReader(stream);

            uint actualReadBytes = await reader.LoadAsync(readSize);

            IBuffer contentBuffer = reader.DetachBuffer();

            stream.Close();

            InMemoryRandomAccessStream memoryStream = new InMemoryRandomAccessStream();
            DataWriter writer = new DataWriter(memoryStream.GetOutputStreamAt(0));
            writer.WriteBuffer(contentBuffer);

            uint wroteBytes = await writer.StoreAsync();

            return memoryStream.GetInputStreamAt(0);
        }
    }
}
