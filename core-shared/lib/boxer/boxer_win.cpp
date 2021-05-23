#include <boxer/boxer.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace boxer {

namespace {

UINT getIcon(Style style) {
   switch (style) {
      case Style::Info:
         return MB_ICONINFORMATION;
      case Style::Warning:
         return MB_ICONWARNING;
      case Style::Error:
         return MB_ICONERROR;
      case Style::Question:
         return MB_ICONQUESTION;
      default:
         return MB_ICONINFORMATION;
   }
}

UINT getButtons(Buttons buttons) {
   switch (buttons) {
      case Buttons::OK:
      case Buttons::Quit: // There is no 'Quit' button on Windows :(
         return MB_OK;
      case Buttons::OKCancel:
         return MB_OKCANCEL;
      case Buttons::YesNo:
         return MB_YESNO;
      default:
         return MB_OK;
   }
}

Selection getSelection(int response, Buttons buttons) {
   switch (response) {
      case IDOK:
         return buttons == Buttons::Quit ? Selection::Quit : Selection::OK;
      case IDCANCEL:
         return Selection::Cancel;
      case IDYES:
         return Selection::Yes;
      case IDNO:
         return Selection::No;
      default:
         return Selection::None;
   }
}

} // namespace

Selection show(const char *message, const char *title, Style style, Buttons buttons) {
   UINT flags = MB_TASKMODAL;

   flags |= getIcon(style);
   flags |= getButtons(buttons);

   return getSelection(MessageBox(nullptr, message, title, flags), buttons);
}

} // namespace boxer
