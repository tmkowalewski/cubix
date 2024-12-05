#import <Cocoa/Cocoa.h>

#include "CocoaUtils.h"
#include "TSystem.h"

void setDockIcon(const char *iconPath) {
    NSString *cocoaStr = [NSString stringWithCString : iconPath encoding : NSASCIIStringEncoding];
    NSImage *image = [[[NSImage alloc] initWithContentsOfFile : cocoaStr] autorelease];
    [NSApp setApplicationIconImage : image];
}
