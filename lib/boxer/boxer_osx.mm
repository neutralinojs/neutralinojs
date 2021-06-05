#include <lib/boxer/boxer.h>
#import <Cocoa/Cocoa.h>

namespace boxer {

namespace {

NSString* const kOkStr = @"OK";
NSString* const kCancelStr = @"Cancel";
NSString* const kYesStr = @"Yes";
NSString* const kNoStr = @"No";
NSString* const kQuitStr = @"Quit";

NSAlertStyle getAlertStyle(Style style) {
#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_12
   switch (style) {
      case Style::Info:
         return NSAlertStyleInformational;
      case Style::Warning:
         return NSAlertStyleWarning;
      case Style::Error:
         return NSAlertStyleCritical;
      case Style::Question:
         return NSAlertStyleWarning;
      default:
         return NSAlertStyleInformational;
   }
#else
   switch (style) {
      case Style::Info:
         return NSInformationalAlertStyle;
      case Style::Warning:
         return NSWarningAlertStyle;
      case Style::Error:
         return NSCriticalAlertStyle;
      case Style::Question:
         return NSWarningAlertStyle;
      default:
         return NSInformationalAlertStyle;
   }
#endif
}

void setButtons(NSAlert *alert, Buttons buttons) {
   switch (buttons) {
      case Buttons::OK:
         [alert addButtonWithTitle:kOkStr];
         break;
      case Buttons::OKCancel:
         [alert addButtonWithTitle:kOkStr];
         [alert addButtonWithTitle:kCancelStr];
         break;
      case Buttons::YesNo:
         [alert addButtonWithTitle:kYesStr];
         [alert addButtonWithTitle:kNoStr];
         break;
     case Buttons::Quit:
         [alert addButtonWithTitle:kQuitStr];
       break;
      default:
         [alert addButtonWithTitle:kOkStr];
   }
}

Selection getSelection(int index, Buttons buttons) {
   switch (buttons) {
      case Buttons::OK:
         return index == NSAlertFirstButtonReturn ? Selection::OK : Selection::None;
      case Buttons::OKCancel:
         if (index == NSAlertFirstButtonReturn) {
            return Selection::OK;
         } else if (index == NSAlertSecondButtonReturn) {
            return Selection::Cancel;
         } else {
            return Selection::None;
         }
      case Buttons::YesNo:
         if (index == NSAlertFirstButtonReturn) {
            return Selection::Yes;
         } else if (index == NSAlertSecondButtonReturn) {
            return Selection::No;
         } else {
            return Selection::None;
         }
      case Buttons::Quit:
         return index == NSAlertFirstButtonReturn ? Selection::Quit : Selection::None;
      default:
         return Selection::None;
   }
}

} // namespace

Selection show(const char *message, const char *title, Style style, Buttons buttons) {
   NSAlert *alert = [[NSAlert alloc] init];

   [alert setMessageText:[NSString stringWithCString:title
                                   encoding:[NSString defaultCStringEncoding]]];
   [alert setInformativeText:[NSString stringWithCString:message
                                       encoding:[NSString defaultCStringEncoding]]];

   [alert setAlertStyle:getAlertStyle(style)];
   setButtons(alert, buttons);

   // Force the alert to appear on top of any other windows
   [[alert window] setLevel:NSModalPanelWindowLevel];

   Selection selection = getSelection([alert runModal], buttons);
   [alert release];

   return selection;
}

} // namespace boxer
