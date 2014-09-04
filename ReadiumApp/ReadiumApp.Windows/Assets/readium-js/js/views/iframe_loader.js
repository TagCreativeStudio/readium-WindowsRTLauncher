//  LauncherOSX
//
//  Created by Boris Schneiderman.
// Modified by Daniel Weck
//  Copyright (c) 2012-2013 The Readium Foundation.
//
//  The Readium SDK is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

ReadiumSDK.Views.IFrameLoader = function() {

    var self = this;
    var eventListeners = {};


    this.addIFrameEventListener = function(eventName, callback, context) {

        if(eventListeners[eventName] == undefined) {
            eventListeners[eventName] = [];
        }

        eventListeners[eventName].push({callback: callback, context: context});
    };

    this.updateIframeEvents = function(iframe) {

        _.each(eventListeners, function(value, key){
            for(var i = 0, count = value.length; i< count; i++) {
                $(iframe.contentWindow).off(key);
                $(iframe.contentWindow).on(key, value[i].callback, value[i].context);
            }
        });
    };


    this.loadIframe = function(iframe, src, callback, context) {

        var isWaitingForFrameLoad = true;

        iframe.onload = function() {

            iframe.onload = undefined;

            isWaitingForFrameLoad = false;

            self.updateIframeEvents(iframe);

            try
            {
                iframe.contentWindow.navigator.epubReadingSystem = navigator.epubReadingSystem;
                // console.debug("epubReadingSystem name:"
                //     + iframe.contentWindow.navigator.epubReadingSystem.name
                //     + " version:"
                //     + iframe.contentWindow.navigator.epubReadingSystem.version
                //     + " is loaded to iframe");
            }
            catch(ex)
            {
                console.log("epubReadingSystem INJECTION ERROR! " + ex.message);
            }

            callback.call(context, true);

        };

        //yucks! iframe doesn't trigger onerror event - there is no reliable way to know that iframe finished
        // attempt tot load resource (successfully or not;
        window.setTimeout(function(){

            if(isWaitingForFrameLoad) {

                isWaitingForFrameLoad = false;
                callback.call(context, false);
            }

        }, 8000);

        iframe.src = src;
    };
};
