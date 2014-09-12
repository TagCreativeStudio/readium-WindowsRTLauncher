//  Created by Boris Schneiderman.
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


/*
 * Renders one page of fixed layout spread
 * @class ReadiumSDK.Views.OnePageView
 */

//Representation of one fixed page
ReadiumSDK.Views.OnePageView = Backbone.View.extend({


    currentSpineItem: undefined,
    spine: undefined,
    contentAlignment: undefined, //expected 'center' 'left' 'right'
    iframeLoader: undefined,
    bookStyles: undefined,

    meta_size : {
        width: 0,
        height: 0
    },


    initialize: function() {

        this.spine = this.options.spine;
        this.contentAlignment = this.options.contentAlignment;
        this.iframeLoader = this.options.iframeLoader;
        this.bookStyles = this.options.bookStyles;

    },

    isDisplaying:function() {

        return this.currentSpineItem != undefined && this.$epubHtml != null && this.$epubHtml.length > 0;
    },

    render: function() {

        if(!this.$iframe) {

            this.template = ReadiumSDK.Helpers.loadTemplate("fixed_page_frame", {});

            this.setElement(this.template);

            this.$el.css("height", "100%");
            this.$el.css("width", "100%");

            this.$el.addClass(this.options.class);
            this.$iframe = $("iframe", this.$el);
        }

        return this;
    },

    remove: function() {

        this.currentSpineItem = undefined;

        //base remove
        Backbone.View.prototype.remove.call(this);
    },


    onIFrameLoad:  function(success) {

        if(success) {
            var epubContentDocument = this.$iframe[0].contentDocument;
            this.$epubHtml = $("html", epubContentDocument);
            this.$epubHtml.css("overflow", "hidden");
            this.applyBookStyles();
            this.updateMetaSize();
//            this.fitToScreen();

            this.trigger(ReadiumSDK.Views.OnePageView.SPINE_ITEM_OPENED, this.$iframe, this.currentSpineItem, true);
        }
    },

    applyBookStyles: function() {

        if(this.$epubHtml) {
            ReadiumSDK.Helpers.setStyles(this.bookStyles.getStyles(), this.$epubHtml);
        }
    },

//    fitToScreen: function() {
//
//        if(!this.isDisplaying()) {
//            return;
//        }
//
//        this.updateMetaSize();
//
//        if(this.meta_size.width <= 0 || this.meta_size.height <= 0) {
//            return;
//        }
//
//
//        var containerWidth = this.$el.width();
//        var containerHeight = this.$el.height();
//
//        var horScale = containerWidth / this.meta_size.width;
//        var verScale = containerHeight / this.meta_size.height;
//
//        var scale = Math.min(horScale, verScale);
//
//        var newWidth = this.meta_size.width * scale;
//        var newHeight = this.meta_size.height * scale;
//
//        var top = Math.floor((containerHeight - newHeight) / 2);
//
//        var left;
//        if(this.contentAlignment == "left") {
//            left = 0;
//        }
//        else if(this.contentAlignment == "right") {
//            left = containerWidth - newWidth;
//        }
//        else { //center
//            left = Math.floor((containerWidth - newWidth) / 2);
//        }
//
//        if(top < 0) top = 0;
//        if(left < 0) left = 0;
//
//        var css = this.generateTransformCSS(scale, left, top);
//        css["width"] = this.meta_size.width;
//        css["height"] = this.meta_size.height;
//
//        this.$epubHtml.css(css);
//    },

    transformContent: function(scale, left, top) {

        var elWidth = Math.floor(this.meta_size.width * scale);
        var elHeight = Math.floor(this.meta_size.height * scale);
                                                    
        this.$el.css("left", left + "px");
        this.$el.css("top", top + "px");
        this.$el.css("width", elWidth + "px");
        this.$el.css("height", elHeight + "px");
                                                    
        this.$iframe.css("width", elWidth + "px");
        this.$iframe.css("height", elHeight + "px");

        var css = this.generateTransformCSS(scale, 0, 0);

        css["width"] = this.meta_size.width;
        css["height"] = this.meta_size.height;

        if(!this.$epubHtml) {
            debugger;
        }

        this.$epubHtml.css(css);
        this.$iframe.css("visibility", "visible");
    },

    generateTransformCSS: function(scale, left, top) {

        var transformString = "translate(" + left + "px, " + top + "px) scale(" + scale + ")";

        //TODO modernizer library can be used to get browser independent transform attributes names (implemented in readium-web fixed_layout_book_zoomer.js)
        var css = {};
        css["-webkit-transform"] = transformString;
        css["-webkit-transform-origin"] = "0 0";

        return css;
    },

    updateMetaSize: function() {

        var contentDocument = this.$iframe[0].contentDocument;

        // first try to read viewport size
        var content = $('meta[name=viewport]', contentDocument).attr("content");

        // if not found try viewbox (used for SVG)
        if(!content) {
            content = $('meta[name=viewbox]', contentDocument).attr("content");
        }

        if(content) {
            var size = this.parseSize(content);
            if(size) {
                this.meta_size.width = size.width;
                this.meta_size.height = size.height;
            }
        }
        else { //try to get direct image size

            var $img = $(contentDocument).find('img');
            var width = $img.width();
            var height = $img.height();

            if( width > 0) {
                this.meta_size.width = width;
                this.meta_size.height = height;
            }
        }

    },

    loadSpineItem: function(spineItem) {

        if(this.currentSpineItem != spineItem) {

            this.currentSpineItem = spineItem;
            var src = this.spine.package.resolveRelativeUrl(spineItem.href);

            //hide iframe until content is scaled
            this.$iframe.css("visibility", "hidden");
            this.iframeLoader.loadIframe(this.$iframe[0], src, this.onIFrameLoad, this);
        }
        else
        {
            this.trigger(ReadiumSDK.Views.OnePageView.SPINE_ITEM_OPENED, this.$iframe, this.currentSpineItem, false);
        }
    },

    parseSize: function(content) {

        var pairs = content.replace(/\s/g, '').split(",");

        var dict = {};

        for(var i = 0;  i  < pairs.length; i++) {
            var nameVal = pairs[i].split("=");
            if(nameVal.length == 2) {

                dict[nameVal[0]] = nameVal[1];
            }
        }

        var width = Number.NaN;
        var height = Number.NaN;

        if(dict["width"]) {
            width = parseInt(dict["width"]);
        }

        if(dict["height"]) {
            height = parseInt(dict["height"]);
        }

        if(!isNaN(width) && !isNaN(height)) {
            return { width: width, height: height} ;
        }

        return undefined;
    },

    getFirstVisibleElementCfi: function(){

        var navigation = new ReadiumSDK.Views.CfiNavigationLogic(this.$el, this.$iframe);
        return navigation.getFirstVisibleElementCfi(0);

    },

    getElement: function(spineItem, selector) {

        if(spineItem != this.currentSpineItem) {
            console.error("spine item is not loaded");
            return undefined;
        }

        var navigation = this.navigationLogic;
        if (!navigation)
        {
            navigation = new ReadiumSDK.Views.CfiNavigationLogic(this.$el, this.$iframe);
        }
        return navigation.getElement(selector);
    },

    getVisibleMediaOverlayElements: function() {
        var navigation = new ReadiumSDK.Views.CfiNavigationLogic(this.$el, this.$iframe);
        return navigation.getVisibleMediaOverlayElements({top:0, bottom: this.$iframe.height()});
    }

});

ReadiumSDK.Views.OnePageView.SPINE_ITEM_OPENED = "SpineItemOpened";
