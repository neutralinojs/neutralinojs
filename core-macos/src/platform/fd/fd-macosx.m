#include "Cocoa/Cocoa.h"
#include "../include/fd.h"
#include <stdio.h>

void fd_opendlg(char** outPath){   
    NSOpenPanel *dialog = [NSOpenPanel openPanel];
    [dialog setAllowsMultipleSelection:NO];

    // Build the filter list
    if ( [dialog runModal] == NSModalResponseOK )
    {
        NSURL *url = [dialog URL];
        const char *utf8Path = [[url path] UTF8String];
        size_t len = strlen(utf8Path);
        *outPath = (char*)malloc(sizeof(char)*(len+1));
        if(!*outPath){
            return;
        }
        memcpy(*outPath, utf8Path, len+1 ); /* copy null term */
    }

    return;
}

void fd_savedlg(char** outPath){   
    NSSavePanel *dialog = [NSSavePanel savePanel];
    [dialog setExtensionHidden:NO];

    // Build the filter list
    if ( [dialog runModal] == NSModalResponseOK )
    {
        NSURL *url = [dialog URL];
        const char *utf8Path = [[url path] UTF8String];
        size_t byteLen = [url.path lengthOfBytesUsingEncoding:NSUTF8StringEncoding] + 1;
        *outPath = (char*)malloc(sizeof(char)*(byteLen));
        if(!*outPath){
            return;
        }
        memcpy(*outPath, utf8Path, byteLen ); /* copy null term */
    }

    return;
}
