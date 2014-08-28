//
//  ios_get_progname.m
//  ePub3
//
//  Created by Jim Dovey on 2013-02-07.
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
//

#import <Foundation/NSProcessInfo.h>
#import <dispatch/dispatch.h>

extern const char* _IOSGetProgname(void)
{
    static char __pname[PATH_MAX];
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        NSString* str = [[NSProcessInfo processInfo] processName];
        [str getBytes: __pname maxLength: PATH_MAX-1 usedLength: NULL encoding: NSUTF8StringEncoding options: NSStringEncodingConversionAllowLossy range: NSMakeRange(0, [str length]) remainingRange:NULL];
    });
    return __pname;
}
