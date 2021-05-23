#ifndef BOXER_H
#define BOXER_H

#if defined(BOXER_DLL) && defined(BOXER_BUILD_DLL)
   /*!
    * BOXER_DLL must be defined by applications that are linking against the DLL version of the Boxer library.
    * BOXER_BUILD_DLL is defined when compiling the DLL version of the library.
    */
   #error "You may not have both BOXER_DLL and BOXER_BUILD_DLL defined"
#endif

/*!
 * BOXERAPI is used to declare public API classes / functions for export from the DLL / shared library / dynamic library
 */
#if defined(_WIN32) && defined(BOXER_BUILD_DLL)
   // We are building Boxer as a Win32 DLL
   #define BOXERAPI __declspec(dllexport)
#elif defined(_WIN32) && defined(BOXER_DLL)
   // We are calling Boxer as a Win32 DLL
   #define BOXERAPI __declspec(dllimport)
#elif defined(__GNUC__) && defined(BOXER_BUILD_DLL)
   // We are building Boxer as a shared / dynamic library
   #define BOXERAPI __attribute__((visibility("default")))
#else
   // We are building or calling Boxer as a static library
   #define BOXERAPI
#endif

namespace boxer {

/*!
 * Options for styles to apply to a message box
 */
enum class Style {
   Info,
   Warning,
   Error,
   Question
};

/*!
 * Options for buttons to provide on a message box
 */
enum class Buttons {
   OK,
   OKCancel,
   YesNo,
   Quit
};

/*!
 * Possible responses from a message box. 'None' signifies that no option was chosen, and 'Error' signifies that an
 * error was encountered while creating the message box.
 */
enum class Selection {
   OK,
   Cancel,
   Yes,
   No,
   Quit,
   None,
   Error
};

/*!
 * The default style to apply to a message box
 */
const Style kDefaultStyle = Style::Info;

/*!
 * The default buttons to provide on a message box
 */
const Buttons kDefaultButtons = Buttons::OK;

/*!
 * Blocking call to create a modal message box with the given message, title, style, and buttons
 */
BOXERAPI Selection show(const char *message, const char *title, Style style, Buttons buttons);

/*!
 * Convenience function to call show() with the default buttons
 */
inline Selection show(const char *message, const char *title, Style style) {
   return show(message, title, style, kDefaultButtons);
}

/*!
 * Convenience function to call show() with the default style
 */
inline Selection show(const char *message, const char *title, Buttons buttons) {
   return show(message, title, kDefaultStyle, buttons);
}

/*!
 * Convenience function to call show() with the default style and buttons
 */
inline Selection show(const char *message, const char *title) {
   return show(message, title, kDefaultStyle, kDefaultButtons);
}

} // namespace boxer

#endif
