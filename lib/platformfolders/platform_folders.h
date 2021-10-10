/*
Its is under the MIT license, to encourage reuse by cut-and-paste.

The original files are hosted here: https://github.com/sago007/PlatformFolders

Copyright (c) 2015 Poul Sander

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation files
(the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef SAGO_PLATFORM_FOLDERS_H
#define SAGO_PLATFORM_FOLDERS_H

#include <vector>
#include <string>

/**
 * The namespace I use for common function. Nothing special about it.
 */
namespace sago {

#ifndef DOXYGEN_SHOULD_SKIP_THIS
namespace internal {
#if !defined(_WIN32) && !defined(__APPLE__)
void appendExtraFoldersTokenizer(const char* envName, const char* envValue, std::vector<std::string>& folders);
#endif
#ifdef _WIN32
std::string win32_utf16_to_utf8(const wchar_t* wstr);
#endif
}
#endif  //DOXYGEN_SHOULD_SKIP_THIS

/**
 * Retrives the base folder for storing data files.
 * You must add the program name yourself like this:
 * @code{.cpp}
 * string data_home = getDataHome()+"/My Program Name/";
 * @endcode
 * On Windows this defaults to %APPDATA% (Roaming profile)
 * On Linux this defaults to ~/.local/share but can be configured
 * @return The base folder for storing program data.
 */
std::string getDataHome();

/**
 * Retrives the base folder for storing config files.
 * You must add the program name yourself like this:
 * @code{.cpp}
 * string data_home = getConfigHome()+"/My Program Name/";
 * @endcode
 * On Windows this defaults to %APPDATA% (Roaming profile)
 * On Linux this defaults to ~/.config but can be configured
 * @return The base folder for storing config data.
 */
std::string getConfigHome();

/**
 * Retrives the base folder for storing cache files.
 * You must add the program name yourself like this:
 * @code{.cpp}
 * string data_home = getCacheDir()+"/My Program Name/";
 * @endcode
 * On Windows this defaults to %APPDATALOCAL%
 * On Linux this defaults to ~/.cache but can be configured
 * @return The base folder for storing data that do not need to be backed up.
 */
std::string getCacheDir();

/**
 * This will append extra folders that your program should be looking for data files in.
 * This does not normally include the path returned by GetDataHome().
 * If you want all the folders you should do something like:
 * @code{.cpp}
 * vector<string> folders;
 * folders.push_back(getDataHome());
 * appendAdditionalDataDirectories(folders);
 * for (string s& : folders) {
 *     s+="/My Program Name/";
 * }
 * @endcode
 * You must apply "/My Program Name/" to all the strings.
 * The string at the lowest index has the highest priority.
 * @param homes A vector that extra folders will be appended to.
 */
void appendAdditionalDataDirectories(std::vector<std::string>& homes);

/**
 * This will append extra folders that your program should be looking for config files in.
 * This does not normally include the path returned by GetConfigHome().
 * If you want all the folders you should do something like:
 * @code{.cpp}
 * std::vector<std::string> folders;
 * folders.push_back(sago::getConfigHome());
 * sago::appendAdditionalConfigDirectories(folders);
 * for (std::string s& : folders) {
 *     s+="/My Program Name/";
 * }
 * @endcode
 * You must apply "/My Program Name/" to all the strings.
 * The string at the lowest index has the highest priority.
 * @param homes A vector that extra folders will be appended to.
 */
void appendAdditionalConfigDirectories(std::vector<std::string>& homes);

/**
 * The folder that represents the desktop.
 * Normally you should try not to use this folder.
 * @return Absolute path to the user's desktop
 */
std::string getDesktopFolder();

/**
 * The folder to store user documents to
 * @return Absolute path to the "Documents" folder
 */
std::string getDocumentsFolder();

/**
 * The folder where files are downloaded.
 * @return Absolute path to the folder where files are downloaded to.
 */
std::string getDownloadFolder();

/**
 * The folder where files are downloaded.
 * @note This is provided for backward compatibility. Use getDownloadFolder instead.
 * @return Absolute path to the folder where files are downloaded to.
 */
std::string getDownloadFolder1();

/**
 * The folder for storing the user's pictures.
 * @return Absolute path to the "Picture" folder
 */
std::string getPicturesFolder();

/**
 * This returns the folder that can be used for sharing files with other users on the same system.
 * @return Absolute path to the "Public" folder
 */
std::string getPublicFolder();

/**
 * The folder where music is stored
 * @return Absolute path to the music folder
 */
std::string getMusicFolder();

/**
 * The folder where video is stored
 * @return Absolute path to the video folder
 */
std::string getVideoFolder();

/**
 * A base folder for storing saved games.
 * You must add the program name to it like this:
 * @code{.cpp}
 * string saved_games_folder = sago::getSaveGamesFolder1()+"/My Program Name/";
 * @endcode
 * @note Windows: This is an XP compatible version and returns the path to "My Games" in Documents. Vista and later has an official folder.
 * @note Linux: XDF does not define a folder for saved games. This will just return the same as GetDataHome()
 * @return The folder base folder for storing save games.
 */
std::string getSaveGamesFolder1();

/**
 * A base folder for storing saved games.
 * You must add the program name to it like this:
 * @code{.cpp}
 * string saved_games_folder = sago::getSaveGamesFolder2()+"/My Program Name/";
 * @endcode
 * @note PlatformFolders provide different folders to for saved games as not all operating systems has support for Saved Games yet.
 * It is recommended to pick the highest number (currently getSaveGamesFolder2) at the time your product enters production and stick with it
 * @note Windows: This returns the "Saved Games" folder. This folder exist in Vista and later
 * @note Linux: XDF does not define a folder for saved games. This will just return the same as GetDataHome()
 * @return The folder base folder for storing save games.
 */
std::string getSaveGamesFolder2();

#ifndef DOXYGEN_SHOULD_SKIP_THIS

/**
 * This class contains methods for finding the system depended special folders.
 * For Windows these folders are either by convention or given by CSIDL.
 * For Linux XDG convention is used.
 * The Linux version has very little error checking and assumes that the config is correct
 */
class PlatformFolders {
public:
	PlatformFolders();
	~PlatformFolders();
	/**
	 * The folder that represents the desktop.
	 * Normally you should try not to use this folder.
	 * @return Absolute path to the user's desktop
	 */
	std::string getDesktopFolder() const;
	/**
	 * The folder to store user documents to
	 * @return Absolute path to the "Documents" folder
	 */
	std::string getDocumentsFolder() const;
	/**
	 * The folder for storing the user's pictures.
	 * @return Absolute path to the "Picture" folder
	 */
	std::string getPicturesFolder() const;
	/**
	 * Use sago::getPublicFolder() instead!
	 */
	std::string getPublicFolder() const;
	/**
	 * The folder where files are downloaded.
	 * @note Windows: This version is XP compatible and returns the Desktop. Vista and later has a dedicated folder.
	 * @return Absolute path to the folder where files are downloaded to.
	 */
	std::string getDownloadFolder1() const;
	/**
	 * The folder where music is stored
	 * @return Absolute path to the music folder
	 */
	std::string getMusicFolder() const;
	/**
	 * The folder where video is stored
	 * @return Absolute path to the video folder
	 */
	std::string getVideoFolder() const;
	/**
	 * The base folder for storing saved games.
	 * You must add the program name to it like this:
	 * @code{.cpp}
	 * PlatformFolders pf;
	 * string saved_games_folder = pf.getSaveGamesFolder1()+"/My Program Name/";
	 * @endcode
	 * @note Windows: This is an XP compatible version and returns the path to "My Games" in Documents. Vista and later has an official folder.
	 * @note Linux: XDF does not define a folder for saved games. This will just return the same as GetDataHome()
	 * @return The folder base folder for storing save games.
	 */
	std::string getSaveGamesFolder1() const;
private:
	PlatformFolders(const PlatformFolders&);
	PlatformFolders& operator=(const PlatformFolders&);
#if !defined(_WIN32) && !defined(__APPLE__)
	struct PlatformFoldersData;
	PlatformFoldersData* data;
#endif
};

#endif // skip doxygen


}  //namespace sago

#endif  /* PLATFORM_FOLDERS_H */
