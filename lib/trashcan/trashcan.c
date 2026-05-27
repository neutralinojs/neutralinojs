/* MIT License
 *
 * Copyright (c) 2019 Robert Guetzkow
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. 
 */

/**
 * @file trashcan.c
 * @author Robert Guetzkow
 * @version 1.0.0-alpha
 * @date 2022-04-13
 * @brief libtrashcan - A library for putting a file or directory in the trashcan.
 *
 * This library implements trashcan functionality for Windows, macOS, Linux and *BSD.
 * The code for Linux and *BSD implements FreeDesktop.org Trash specification v1.0. For 
 * Windows and Apple the respective OS API is used.
 *
 * @see https://specifications.freedesktop.org/trash-spec/trashspec-1.0.html
 *
 * @warning This is an alpha version and not considered stable. Severe bugs will likely
 * exist and it is not recommended to be used in production.
 */

#include "trashcan.h"

#ifdef WIN32
#include <Windows.h>
#include <objbase.h>
#include <shellapi.h>
#include <shobjidl.h>
#include <versionhelpers.h>
#include <stdbool.h>

#elif defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#include <objc/runtime.h>
#include <objc/message.h>

#elif defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
#ifdef __linux__
#define _GNU_SOURCE
#include <mntent.h>
#include <sys/random.h>
#else
#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>
#endif

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <libgen.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <inttypes.h>
#include <limits.h>
#else
#error Platform not supported
#endif

/* Macro for generating enum entries */
#define STATUS_ENUM(ID, NAME, STR) NAME = ID,

/* Macro for generating cases in switch statement */
#define STATUS_STR(ID, NAME, STR) case ID: return STR;

/* Macro for shorter notation to assign a variable before jumping to a label */
#define HANDLE_ERROR(VAR, VAL, LABEL)\
	VAR = VAL;\
	goto LABEL;\

#ifdef WIN32

#define STATUS_CODES(X) \
	X(0, LIBTRASHCAN_SUCCESS, "Successful.")\
	X(-1, LIBTRASHCAN_ALLOC, "Failed to allocate memory for full path.")\
	X(-2, LIBTRASHCAN_FULLPATH, "Failed to get full path.")\
	X(-3, LIBTRASHCAN_COMINIT, "Failed to initialize COM.")\
	X(-4, LIBTRASHCAN_INSTANCE, "Failed to create FileOperation instance.")\
	X(-5, LIBTRASHCAN_FLAGS, "Failed to set operation flags.")\
	X(-6, LIBTRASHCAN_PARSE, "Failed to parse path.")\
	X(-7, LIBTRASHCAN_SETOP, "Failed to prepare delete operation.")\
	X(-8, LIBTRASHCAN_EXECOP, "Failed to delete file or directory.")\
	X(-9, LIBTRASHCAN_WCHARLEN, "Failed to retrieve wchar_t length.")\
	X(-10, LIBTRASHCAN_WCHARALLOC, "Failed to allocated *wchar_t.")\
	X(-11, LIBTRASHCAN_WCHARCONV, "Failed to convert *char to *wchar_t.")\


enum
{
	STATUS_CODES(STATUS_ENUM)
};

/* For backward compatibility define newer flags for Windows Vista and 7 so that it compiles properly. */
#ifndef FOFX_ADDUNDORECORD
#define FOFX_ADDUNDORECORD 0x20000000
#endif

#ifndef FOFX_RECYCLEONDELETE
#define FOFX_RECYCLEONDELETE 0x00080000
#endif

/**
 * @brief Moves a file or a directory (and its content) to the recycling bin.
 *
 * @warning Do not change the current working directory when using this in a multithreaded application!
 *
 * @param path Path to the file or directory that shall be moved to the recycling bin. This needs
 * to be a wchar_t* because the Windows API requires wide characters.
 * @param init_com If true, initializes the COM library at the beginning using `CoUninitialize()`
 * and uninitializes it at the end with `CoUninitialize()`. If init_com is false the COM library
 * isn't loaded and has to be initialized by the code calling this function. This option is useful
 * to avoid initializing the COM library multiple times.
 * @return 0 when successful, negative otherwise.
 */
int trashcan_soft_delete_core(const wchar_t *path, bool init_com)
{
	int status = LIBTRASHCAN_SUCCESS;
	HRESULT hr = 0;
	IFileOperation *pfo;
	IShellItem *pSI;

	ULONG full_path_len = GetFullPathNameW(path, 0, NULL, NULL); /* Retrieve the size of the full path */

	wchar_t *full_path = malloc(full_path_len * sizeof(wchar_t));
	if (full_path == NULL) { HANDLE_ERROR(status, LIBTRASHCAN_ALLOC, error_0) }

	if (full_path_len - 1 != GetFullPathNameW(path, full_path_len, full_path, NULL)) { HANDLE_ERROR(status, LIBTRASHCAN_FULLPATH, error_1) }

	if (init_com)
	{
		hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	}

	if (FAILED(hr)) { HANDLE_ERROR(status, LIBTRASHCAN_COMINIT, error_1) }

	hr = CoCreateInstance(&CLSID_FileOperation, NULL, CLSCTX_ALL, &IID_IFileOperation, (void**)&pfo);
	if (FAILED(hr)) { HANDLE_ERROR(status, LIBTRASHCAN_INSTANCE, error_2) }

	if (IsWindows8OrGreater())
	{
		hr = pfo->lpVtbl->SetOperationFlags(pfo, FOFX_ADDUNDORECORD | FOFX_RECYCLEONDELETE | FOF_SILENT | FOF_NOERRORUI | FOFX_EARLYFAILURE);
	}
	else
	{
		hr = pfo->lpVtbl->SetOperationFlags(pfo, FOF_ALLOWUNDO | FOF_SILENT | FOF_NOERRORUI | FOFX_EARLYFAILURE);
	}

	if (FAILED(hr)) { HANDLE_ERROR(status, LIBTRASHCAN_FLAGS, error_3) }

	hr = SHCreateItemFromParsingName(full_path, NULL, &IID_IShellItem, (void**)&pSI);
	if (FAILED(hr)) { HANDLE_ERROR(status, LIBTRASHCAN_PARSE, error_3) }

	hr = pfo->lpVtbl->DeleteItem(pfo, pSI, NULL);
	if (FAILED(hr)) { HANDLE_ERROR(status, LIBTRASHCAN_SETOP, error_4) }

	hr = pfo->lpVtbl->PerformOperations(pfo);
	if (FAILED(hr)) { HANDLE_ERROR(status, LIBTRASHCAN_EXECOP, error_4) }

error_4:
	pSI->lpVtbl->Release(pSI);
error_3:
	pfo->lpVtbl->Release(pfo);
error_2:
	if (init_com)
	{
		CoUninitialize(); /* Has to be uninitialized when CoInitializeEx returns either S_OK or S_FALSE */
	}
error_1:
	free(full_path);
error_0:
	return status;
}

/**
 * @brief Moves a file or a directory (and its content) to the recycling bin.
 *
 * @warning Do not change the current working directory when using this in a multithreaded application!
 *
 * @param path Path to the file or directory that shall be moved to the recycling bin.
 * @param code_page The code page to use when interpreting path as multibyte sequence. Allowed values
 * are CP_ACP, CP_MACCP, CP_OEMCP, CP_SYMBOL, CP_THREAD_ACP, CP_UTF7 and CP_UTF8.
 * https://docs.microsoft.com/en-us/windows/desktop/api/stringapiset/nf-stringapiset-multibytetowidechar
 * @param init_com If true, initializes the COM library at the beginning using `CoUninitialize()`
 * and uninitializes it at the end with `CoUninitialize()`. If init_com is false the COM library
 * isn't loaded and has to be initialized by the code calling this function. This option is useful
 * to avoid initializing the COM library multiple times.
 * @return 0 when successful, negative otherwise.
 */
int trashcan_soft_delete_com(const char *path, unsigned int code_page, bool init_com)
{
	int status = LIBTRASHCAN_SUCCESS;
	wchar_t *wcs = NULL;

	int mbslen = MultiByteToWideChar(code_page, 0, path, -1, NULL, 0);
	if (mbslen == 0) { HANDLE_ERROR(status, LIBTRASHCAN_WCHARLEN, error_0) }

	wcs = calloc(mbslen, sizeof(wchar_t)); /* Length includes zero termination */

	if (wcs == NULL) { HANDLE_ERROR(status, LIBTRASHCAN_WCHARALLOC, error_0) }

	if (MultiByteToWideChar(code_page, 0, path, -1, wcs, mbslen) == 0) { HANDLE_ERROR(status, LIBTRASHCAN_WCHARCONV, error_1) }

	status = trashcan_soft_delete_core(wcs, init_com);

error_1:
	free(wcs);
error_0:
	return status;
}

/**
 * @brief Moves a file or a directory (and its content) to the trash.
 *
 * @warning Do not change the current working directory when using this in a multithreaded application!
 *
 * @warning This function expects an UTF-8 encoded string! If you want to set a Windows code page
 * use `trashcan_soft_delete_com()` instead.
 *
 * @note The COM library is initialized and uninitialized during this function call. If your
 * application already loads the COM library you should use `trashcan_soft_delete_com()` with
 * `init_com` set to `false`.
 *
 * @param path Path to the file or directory that shall be moved to the trash. This path has
 * to be UTF-8 encoded or use a compatible encoding.
 * @return 0 when successful, negative otherwise.
 */
int trashcan_soft_delete(const char *path)
{
	return trashcan_soft_delete_com(path, CP_UTF8, true);
}

#elif defined(__APPLE__)
#define STATUS_CODES(X) \
	X(0, LIBTRASHCAN_SUCCESS, "Successful.")\
	X(-1, LIBTRASHCAN_ERROR, "Error occurred.")\

enum
{
	STATUS_CODES(STATUS_ENUM)
};

/**
 * @brief Moves a file or a directory (and its content) to the trash.
 *
 * @warning Do not change the current working directory when using this in a multithreaded application!
 *
 * @param path Path to the file or directory that shall be moved to the trash.
 * @return 0 when successful, negative otherwise.
 */
int trashcan_soft_delete(const char *path)
{
	int ret = LIBTRASHCAN_ERROR;

	Class NSAutoreleasePoolClass = objc_getClass("NSAutoreleasePool");
	SEL allocSel = sel_registerName("alloc");
	SEL initSel = sel_registerName("init");
	id poolAlloc = ((id(*)(Class, SEL))objc_msgSend)(NSAutoreleasePoolClass, allocSel);
	id pool = ((id(*)(id, SEL))objc_msgSend)(poolAlloc, initSel);

	Class NSStringClass = objc_getClass("NSString");
	SEL stringWithUTF8StringSel = sel_registerName("stringWithUTF8String:");
	id pathString = ((id(*)(Class, SEL, const char*))objc_msgSend)(NSStringClass, stringWithUTF8StringSel, path);

	Class NSFileManagerClass = objc_getClass("NSFileManager");
	SEL defaultManagerSel = sel_registerName("defaultManager");
	id fileManager = ((id(*)(Class, SEL))objc_msgSend)(NSFileManagerClass, defaultManagerSel);

	Class NSURLClass = objc_getClass("NSURL");
	SEL fileURLWithPathSel = sel_registerName("fileURLWithPath:");
	id nsurl = ((id(*)(Class, SEL, id))objc_msgSend)(NSURLClass, fileURLWithPathSel, pathString);

	SEL trashItemAtURLSel = sel_registerName("trashItemAtURL:resultingItemURL:error:");
	BOOL deleteSuccessful = ((BOOL(*)(id, SEL, id, id, id))objc_msgSend)(fileManager, trashItemAtURLSel, nsurl, nil, nil);

	if (deleteSuccessful)
	{
		ret = LIBTRASHCAN_SUCCESS;
	}

	SEL drainSel = sel_registerName("drain");
	((void(*)(id, SEL))objc_msgSend)(pool, drainSel);

	return ret;
}

#elif defined(__linux__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)

#define STATUS_CODES(X) \
	X(0, LIBTRASHCAN_SUCCESS, "Successful.")\
	X(-1, LIBTRASHCAN_REALPATH, "Failed to retrieve real path.")\
	X(-2, LIBTRASHCAN_HOMETRASH, "Failed to retrieve home trash path.")\
	X(-3, LIBTRASHCAN_HOMESTAT, "Failed to lstat home trash path.")\
	X(-4, LIBTRASHCAN_PATHSTAT, "Failed to lstat path.")\
	X(-5, LIBTRASHCAN_MKDIRHOME, "Failed to create home trash dir.")\
	X(-6, LIBTRASHCAN_TOPDIRTRASH, "Failed to retrieve top dir trash path.")\
	X(-7, LIBTRASHCAN_NAME, "Failed to retrieve filename or directory name from path.")\
	X(-8, LIBTRASHCAN_TIME, "Failed to retrieve current time.")\
	X(-9, LIBTRASHCAN_FILENAMES, "Failed to retrieve target filenames.")\
	X(-10, LIBTRASHCAN_TRASHINFO, "Failed to create and write trash info file.")\
	X(-11, LIBTRASHCAN_RENAME, "Failed to move files to trash.")\
	X(-12, LIBTRASHCAN_COLLISION, "Failed to generate unique name.")\
	X(-13, LIBTRASHCAN_DIRCACHE, "Failed to update directory size cache.")\

enum
{
	STATUS_CODES(STATUS_ENUM)
};

/**
 * @brief Determines paths to the home trash directory.
 *
 * @see https://specifications.freedesktop.org/trash-spec/trashspec-1.0.html
 * @see https://specifications.freedesktop.org/basedir-spec/latest/ar01s03.html
 *
 * @param data_home "$XDG_DATA_HOME/.local/share".
 * @param trash_dir Base directory for home trash.
 * @param trash_info_dir Directory where .trashinfo files are stored.
 * @param trash_files_dir Directory where trashed files are stored.
 * @return 0 when successful, negative otherwise.
 */
static int get_home_trash_dir(char **data_home, char **trash_dir, char **trash_info_dir, char **trash_files_dir)
{
	int status = -1;
	*data_home = NULL;
	*trash_dir = NULL;
	*trash_info_dir = NULL;
	*trash_files_dir = NULL;

	const char *xdg_data_home = getenv("XDG_DATA_HOME");
	if (xdg_data_home == NULL)
	{
		/* $XDG_DATA_HOME isn't set, default value is used. */
		const char *home = getenv("HOME");
		if (home == NULL) { goto error_0; }

		/* If root dir, then don't include in concatenated path because otherwise there would be two leading slashes. */
		if (strcmp(home, "/") == 0) { home = ""; }

		if (asprintf(data_home, "%s%s", home, "/.local/share") < 0) { HANDLE_ERROR(*data_home, NULL, error_m1) }
		if (asprintf(trash_dir, "%s%s", home, "/.local/share/Trash") < 0) { HANDLE_ERROR(*trash_dir, NULL, error_m1) }
		if (asprintf(trash_info_dir, "%s%s", *trash_dir, "/info") < 0) { HANDLE_ERROR(*trash_info_dir, NULL, error_m1) }
		if (asprintf(trash_files_dir, "%s%s", *trash_dir, "/files") < 0) { HANDLE_ERROR(*trash_files_dir, NULL, error_m1) }
	}
	else
	{
		/* If root dir, then don't include in concatenated path because otherwise there would be two leading slashes. */
		if (strcmp(xdg_data_home, "/") == 0) { xdg_data_home = ""; }

		if (asprintf(data_home, "%s", xdg_data_home) < 0) { HANDLE_ERROR(*data_home, NULL, error_m1) }
		if (asprintf(trash_dir, "%s%s", xdg_data_home, "/Trash") < 0) { HANDLE_ERROR(*trash_dir, NULL, error_m1) }
		if (asprintf(trash_info_dir, "%s%s", *trash_dir, "/info") < 0) { HANDLE_ERROR(*trash_info_dir, NULL, error_m1) }
		if (asprintf(trash_files_dir, "%s%s", *trash_dir, "/files") < 0) { HANDLE_ERROR(*trash_files_dir, NULL, error_m1) }
	}

	status = 0;

error_0:
	return status;
error_m1:
	free(*trash_files_dir);
	free(*trash_info_dir);
	free(*trash_dir);
	free(*data_home);
	*trash_files_dir = NULL;
	*trash_info_dir = NULL;
	*trash_files_dir = NULL;
	*data_home = NULL;
	return status;
}

/**
 * @brief Find the mountpoint of a device.
 *
 * @param device Device for which the mountpoint is determined.
 * @param mount_dir Address where pointer to the mount directory shall be stored.
 * @return 0 when successful, negative otherwise.
 */
static int get_mountpoint(dev_t device, char **mount_dir)
{
	int status = -1;
	struct stat mnt_stat;
	*mount_dir = NULL;

#ifdef __linux__
	char *file = "/etc/mtab";
	FILE *fptr = NULL;
	struct mntent *mount_entry;

	fptr = setmntent(file, "r");
	if (fptr == NULL) { goto error_0; }

	while ((mount_entry = getmntent(fptr)) != NULL)
	{
		if (lstat(mount_entry->mnt_dir, &mnt_stat)) { goto error_0; }
		if (device == mnt_stat.st_dev)
		{
			break;
		}
	}

	if (mount_entry == NULL) { goto error_0; }
	if (asprintf(mount_dir, "%s", mount_entry->mnt_dir) < 0) { HANDLE_ERROR(*mount_dir, NULL, error_0) }
#else
	size_t i = 0;
	unsigned char mnt_found = 0;
	struct statfs* mounts;
	int num_mounts = getmntinfo(&mounts, MNT_WAIT);
	if (num_mounts < 0) { goto error_0; }

	for (i = 0; i < num_mounts; i++)
	{
		if (lstat(mounts[i].f_mntonname, &mnt_stat)) { goto error_0; }
		if (device == mnt_stat.st_dev)
		{
			mnt_found = 1;
			break;
		}
	}

	if (!mnt_found) { goto error_0; }
	if (asprintf(mount_dir, "%s", mounts[i].f_mntonname) < 0) { HANDLE_ERROR(*mount_dir, NULL, error_0) }
#endif

	status = 0;

error_0:
	return status;
}

/**
 * @brief Determines paths to the $topdir for case (1) and (2) of the specification.
 *
 * @see https://specifications.freedesktop.org/trash-spec/trashspec-1.0.html
 *
 * @param case_num Determines whether case (1) or (2) applies.
 * @param device Device for which the $topdir shall be determined.
 * @param trash_dir Base directory for $topdir.
 * @param trash_info_dir Directory where .trashinfo files are stored.
 * @param trash_files_dir Directory where trashed files are stored.
 * @return 0 when successful, negative otherwise.
 */
static int get_top_trash_dir(unsigned char case_num, dev_t device, char **trash_dir, char **trash_info_dir, char **trash_files_dir)
{
	int status = -1;
	char *mount_dir = NULL;
	*trash_dir = NULL;
	*trash_info_dir = NULL;
	*trash_files_dir = NULL;

	if (get_mountpoint(device, &mount_dir)) { goto error_0; }

	uid_t uid = getuid();

	switch (case_num)
	{
		case 1:
			if (asprintf(trash_dir, "%s%s%ju", mount_dir, "/.Trash/", (uintmax_t)uid) < 0) { HANDLE_ERROR(*trash_dir, NULL, error_m1) }
			break;
		case 2:
			if (asprintf(trash_dir, "%s%s%ju", mount_dir, "/.Trash-", (uintmax_t)uid) < 0) { HANDLE_ERROR(*trash_dir, NULL, error_m1) }
			break;
		default:
			goto error_m1;
	}
	if (asprintf(trash_info_dir, "%s%s", *trash_dir, "/info") < 0) { HANDLE_ERROR(*trash_info_dir, NULL, error_m1) }
	if (asprintf(trash_files_dir, "%s%s", *trash_dir, "/files") < 0) { HANDLE_ERROR(*trash_files_dir, NULL, error_m1) }

	status = 0;

error_0:
	return status;
error_m1:
	free(*trash_dir);
	free(*trash_info_dir);
	free(*trash_files_dir);
	*trash_dir = NULL;
	*trash_info_dir = NULL;
	*trash_files_dir = NULL;
	return status;
}

/**
 * @brief Create directories recursively.
 *
 * @param path Path to create recursively.
 * @param mode File permission mode to use when creating the directories.
 * @return 0 when successful, negative otherwise.
 */
static int mkdir_recursive(const char *path, mode_t mode)
{
	int status = -1;
	char *current_path = NULL;
	char *current_pos = NULL;

	if (path == NULL || *path == '\0')
	{
		goto error_0;
	}

	/* Copy string, since we don't want to modify the original */
	if (asprintf(&current_path, "%s", path) < 0) { goto error_0; }

	current_pos = current_path + 1; /* Start after root dir */

	while (*current_pos != '\0')
	{
		if (*current_pos == '/')
		{
			*current_pos = '\0'; /* Temporary truncation */

			if (mkdir(current_path, mode) != 0)
			{
				if (errno != EEXIST) { goto error_1; }
			}

			*current_pos = '/';
		}
		current_pos++;
	}

	/* Create last dir with name between slash and zero termination */
	if (mkdir(current_path, mode) != 0)
	{
		if (errno != EEXIST) { goto error_1; }
	}

	status = 0;

error_1:
	free(current_path);
error_0:
	return status;
}

/**
 * @brief Create directories if the don't already exist.
 *
 * @param trash_dir Base directory for trash.
 * @param trash_info_dir Directory where .trashinfo files are stored.
 * @param trash_files_dir Directory where trashed files are stored.
 * @param mode File permission mode to use when creating the directories.
 * @return 0 when successful, negative otherwise.
 */
static int create_trash_dir(const char *trash_info_dir, const char *trash_files_dir, mode_t mode)
{
	int status = -1;

	if (mkdir_recursive(trash_info_dir, mode) < 0) { goto error_0; }
	if (mkdir_recursive(trash_files_dir, mode) < 0) { goto error_0; }

	status = 0;

error_0:
	return status;
}

/**
 * @brief Check whether a character is unreserved or not.
 *
 * This is meant to be used for URI escaping as defined in RFC 2396
 * @see https://www.ietf.org/rfc/rfc2396.txt
 *
 * @param c Character that is checked.
 * @return 1 when unreserved, 0 otherwise.
 */
static int is_unreserved(char c)
{
	if (('a' <= c && c <= 'z') ||
		('A' <= c && c <= 'Z') ||
		('0' <= c && c <= '9') ||
		c == '-' ||
		c == '_' ||
		c == '.' ||
		c == '!' ||
		c == '~' ||
		c == '*' ||
		c == '\'' ||
		c == '(' ||
		c == ')')
	{
		return 1;
	}

	return 0;
}

/**
 * @brief Escape a string using URI escaping RFC 2396
 *
 * @see https://www.ietf.org/rfc/rfc2396.txt
 *
 * @param str String to be escaped
 * @param str_escaped Address where pointer to the escaped string is stored.
 * @return 0 when successful, negative otherwise.
 */
static int escape_path(const char *str, char **str_escaped)
{
	/* Implements URI escaping as defined in RFC 2396 except for the reserved '/' which is a legal char in the path and therefore isn't escaped */
	int status = -1;
	size_t idx = 0;
	size_t str_len = strlen(str);
	*str_escaped = calloc(str_len * 3 + 1, sizeof(char)); /* Enough space if every char needs to be escaped. */
	if (*str_escaped == NULL) { goto error_0; }

	for (size_t i=0; i < str_len; i++)
	{
		if (is_unreserved(str[i]) || str[i] == '/')
		{
			(*str_escaped)[idx] = str[i];
			idx++;
		}
		else
		{
			snprintf(&((*str_escaped)[idx]), 4, "%%%02hhX", (unsigned char) str[i]); /* 3 chars + '\0' */
			idx += 3;
		}
	}

	status = 0;

error_0:
	return status;
}

/**
 * @brief Creates a .trashinfo file.
 *
 * @see https://specifications.freedesktop.org/trash-spec/trashspec-1.0.html
 *
 * @param trashinfo_filepath Path to the trash info directory.
 * @param original_filepath Path where the file was stored before deletion.
 * @param timeinfo Time information about when the deletion occured.
 * @return 0 when successful, negative otherwise.
 */
static int create_info_file(const char *trashinfo_filepath, const char *original_filepath, const struct tm *timeinfo)
{
	int status = -1;
	char timestamp[20];
	char *escaped_original_filepath = NULL;
	char *trashinfo_file = NULL;

	strftime(timestamp, sizeof(timestamp), "%FT%T", timeinfo);
	if (escape_path(original_filepath, &escaped_original_filepath) < 0) { goto error_0; }

	if (asprintf(&trashinfo_file, "[Trash Info]\nPath=%s\nDeletionDate=%s\n", escaped_original_filepath, timestamp) < 0) { HANDLE_ERROR(trashinfo_file, NULL, error_0) }

	FILE *fptr = fopen(trashinfo_filepath, "wx");
	if (fptr == NULL)
	{
		if (errno == EEXIST)
		{
			status = 1;
		}
		goto error_0;
	}

	if (fwrite(trashinfo_file, sizeof(char), strlen(trashinfo_file), fptr) != strlen(trashinfo_file))
	{
		goto error_1;
	}

	status = 0;
error_1:
	fclose(fptr);
error_0:
	free(trashinfo_file);
	free(escaped_original_filepath);
	return status;
}

/**
 * @brief Generates a random string of given length. The length has to be a multiple of two.
 *
 * @param filename Address to pointer where filename shall be stored.
 * @param filename_length Length of the filename to be generated (without zero termination).
 * @param 0 when successful, negative otherwise.
 */
static int generate_random_filename(char **filename, size_t filename_length)
{
	int status = -1;

	/* Length has to be multiple of two, because 1 byte => 2 hex chars */
	if (filename_length % 2 != 0) { goto error_0; }

	size_t num_bytes = filename_length / 2;
	unsigned char *buf = malloc(filename_length);
	if (buf == NULL) { goto error_0; }

#ifdef __linux__
	if (getrandom(buf, num_bytes, GRND_RANDOM) < 0) { goto error_1; }
#else
	arc4random_buf(buf, num_bytes);
#endif

	*filename = calloc(filename_length + 1, sizeof(char));
	if (*filename == NULL) { goto error_1; }

	for (size_t i = 0; i < num_bytes; i++)
	{
		snprintf(&((*filename)[i*2]), 3, "%02hhX", buf[i]); /* 2 chars + '\0' */
	}

	status = 0;
error_1:
	free(buf);
error_0:
	return status;
}

/**
 * @brief Determines the path and filename of the deleted file and its .trashinfo file.
 *
 * The generated name is related to the original name. If the limits for the filename allow,
 * the new name is the original name, followed by the timestamp of deletion and a counter.
 *
 * Original path: /home/user/some/path/hello.txt
 * New path     : /home/user/.local/share/Trash/files/hello.txt201904241508300
 * Trashinfo    : /home/user/.local/share/Trash/files/hello.txt201904241508300.trashinfo
 *
 * The counter is intended to be used when name collision occur because two files with the same
 * name have been deleted. The timestamp is used to avoid unnecessary retries when many files
 * with the same name have been deleted. Instead of having to retry with many different counters,
 * incrementing until a unique name is reached, we use the deletion time in the name. This greatly
 * limits the chance of a name collision and results in fewer retries.
 *
 * If the approach above exceeds the limit for the filename, e.g. because the original filename
 * already has the maximum allowed number of character, a random name within limits is generated.
 * This approach is not described in the spec, since it does not allow to make a connection between
 * the original name and the new name. However we believe this is good solution for this edge case,
 * since otherwise filename truncation would be need, which may also make identifying the original
 * filename difficult.
 *
 * @see https://specifications.freedesktop.org/trash-spec/trashspec-1.0.html
 *
 * @param original_name Original path before deletion.
 * @param trash_info_dir Path to the trash info directory.
 * @param trash_files_dir Path to the directory where deleted files shall be stored.
 * @param timeinfo Time when file was deleted.
 * @param counter Counter which should be incremented in case a name collision occurs.
 * @param enforce_random_name Force use of a random name.
 * @param trash_info_file Address to pointer where trash info file shall be stored.
 * @param trashed_file Address to pointer where deleted file shall be stored.
 * @return 0 when successful, negative otherwise.
 */
static int generate_filenames(const char *original_name, const char *trash_info_dir, const char *trash_files_dir, const struct tm *timeinfo, 
								unsigned int counter, unsigned char enforce_random_name, char **trash_info_file, char **trashed_file)
{
	int status = -1;
	long chars_left = 0;
	char timestamp_name[15];
	char *counter_str = NULL;
	char *filename = NULL;
	*trash_info_file = NULL;
	*trashed_file = NULL;

	strftime(timestamp_name, sizeof(timestamp_name), "%Y%m%d%H%M%S", timeinfo);

	if (asprintf(&counter_str, "%x", counter) < 0) { goto error_0; }

	/* Check the maximum allowed filename length to ensure that the deleted file can be renamed. */
	errno = 0;
	long name_max = pathconf(trash_files_dir, _PC_NAME_MAX);
	if (name_max == -1)
	{
		if (errno != 0)
		{
			goto error_1;
		}
		/* If errno is zero, then there is no limit set for _PC_NAME_MAX */
	}
	else 
	{
		/* Check if intended filename would exceed the filename length limit of the filesystem. At least 14 bytes without terminating
		 * null are guaranteed by POSIX (see _POSIX_NAME_MAX). */
		chars_left = name_max - (long)(strlen(timestamp_name) + strlen(original_name) + strlen(counter_str) + strlen(".trashinfo"));
	}

	if (chars_left > 0 && !enforce_random_name)
	{
		/* Filenames for ".trashinfo" file and file or directory moved to the trash bin */
		if (asprintf(trash_info_file, "%s/%s%s%s%s", trash_info_dir, original_name, timestamp_name, counter_str, ".trashinfo") < 0) { HANDLE_ERROR(*trash_info_file, NULL, error_m1) }
		if (asprintf(trashed_file, "%s/%s%s%s", trash_files_dir, original_name, timestamp_name, counter_str) < 0) { HANDLE_ERROR(*trashed_file, NULL, error_m1) }
	}
	else
	{
		/* Generate a random filename within limits. This approach is used to handle small filename limits and name collisions during deletion gracefully. */
		size_t filename_length = (size_t)name_max - strlen(".trashinfo"); /* Length without terminating '\0' */
		if (generate_random_filename(&filename, filename_length) < 0) { HANDLE_ERROR(filename, NULL, error_1) }
		if (asprintf(trash_info_file, "%s/%s%s", trash_info_dir, filename, ".trashinfo") < 0) { HANDLE_ERROR(*trash_info_file, NULL, error_m1) }
		if (asprintf(trashed_file, "%s/%s", trash_files_dir, filename) < 0) { HANDLE_ERROR(*trashed_file, NULL, error_m1) }
	}

	status = 0;

error_1:
	free(filename);
	free(counter_str);
error_0:
	return status;
error_m1:
	free(filename);
	free(counter_str);
	free(*trash_info_file);
	free(*trashed_file);
	*trash_info_file = NULL;
	*trashed_file = NULL;
	return status;
}

/**
 * @brief Calculate the size of a directory and its contained files.
 *
 * @param dir Directory for which the size shall be calculated.
 * @param dir_size Address where the result shall be stored.
 * @return 0 when successful, negative otherwise.
 */
static int get_dir_size(const char *base_dir, uint64_t *dir_size)
{
	int status = -1;
	struct dirent *directory_entry;
	struct stat file_stat;
	char *current_entry = NULL;

	DIR *directory = opendir(base_dir);
	if (directory == NULL) { goto error_0; }

	while ((directory_entry = readdir(directory)) != NULL)
	{
		if ((strcmp(directory_entry->d_name, ".") == 0) || (strcmp(directory_entry->d_name, "..") == 0))
		{
			continue;
		}

		if (asprintf(&current_entry, "%s/%s", base_dir, directory_entry->d_name) < 0) { HANDLE_ERROR(current_entry, NULL, error_1) }

		if (lstat(current_entry, &file_stat)) { goto error_1; }

		if (S_ISDIR(file_stat.st_mode))
		{
			if (get_dir_size(current_entry, dir_size) < 0) { goto error_1; }
		}
		else if (S_ISREG(file_stat.st_mode))
		{
			*dir_size += (uint64_t)file_stat.st_size;
		}

		free(current_entry);
		current_entry = NULL;
	}

	status = 0;

error_1:
	closedir(directory);
error_0:
	free(current_entry);
	return status;
}

/**
 * @brief Create or update the directory size cache.
 *
 * For each directory in $trash/files calculate the size recursively and write
 * a line to a temporary file. After completion replace the old $trash/directorysizes
 * file.
 *
 * @see https://specifications.freedesktop.org/trash-spec/trashspec-1.0.html
 *
 * @param trash_dir Path to the trash base directory.
 * @param trash_info_dir Path to the trash info directory.
 * @param trash_files_dir Path to the directory where deleted file are stored.
 * @return 0 when successful, negative otherwise.
 */
static int create_or_update_dir_size_cache(const char *trash_dir, const char *trash_info_dir, const char *trash_files_dir)
{
	int status = -1;
	char *temp_name = NULL;
	char *dir_size_cache = NULL;
	char *dir_size_cache_temp = NULL;
	char *current_dir = NULL;
	char *current_trashinfo = NULL;
	char *current_line = NULL;

	if (generate_random_filename(&temp_name, _POSIX_NAME_MAX) < 0) { goto error_0; }
	if (asprintf(&dir_size_cache, "%s/%s", trash_dir, "directorysizes") < 0) { HANDLE_ERROR(dir_size_cache, NULL, error_0) }
	if (asprintf(&dir_size_cache_temp, "%s/%s", trash_dir, temp_name) < 0) { HANDLE_ERROR(dir_size_cache_temp, NULL, error_0) }

	FILE *fptr = fopen(dir_size_cache_temp, "w");
	if (fptr == NULL)
	{
		goto error_0;
	}

	struct dirent *directory_entry;
	struct stat trashinfo_stat;
	DIR *directory = opendir(trash_files_dir);
	if (directory == NULL) { goto error_1; }

	while ((directory_entry = readdir(directory)) != NULL)
	{
		if ((strcmp(directory_entry->d_name, ".") == 0) || (strcmp(directory_entry->d_name, "..") == 0))
		{
			continue;
		}
		
		if (directory_entry->d_type == DT_DIR)
		{
			uint64_t dir_size = 0;
			if (asprintf(&current_dir, "%s/%s", trash_files_dir, directory_entry->d_name) < 0) { HANDLE_ERROR(current_dir, NULL, error_2) }
			if (get_dir_size(current_dir, &dir_size) < 0) { goto error_2; }
			if (asprintf(&current_trashinfo, "%s/%s%s", trash_info_dir, directory_entry->d_name, ".trashinfo") < 0) { HANDLE_ERROR(current_trashinfo, NULL, error_2) }
			if (lstat(current_trashinfo, &trashinfo_stat)) { goto skip; } /* lstat can fail if .trashinfo file doesn't exist. */
			if (asprintf(&current_line, "%" PRIu64 " %jd %s\n", dir_size, (intmax_t)trashinfo_stat.st_mtime, directory_entry->d_name) < 0) { HANDLE_ERROR(current_line, NULL, error_2) }

			if (fwrite(current_line, sizeof(char), strlen(current_line), fptr) != strlen(current_line))
			{
				goto error_2;
			}

skip:
			free(current_dir);
			free(current_line);
			free(current_trashinfo);
			current_dir = NULL;
			current_line = NULL;
			current_trashinfo = NULL;
		}
	}

	closedir(directory);
	fclose(fptr);

	if (rename(dir_size_cache_temp, dir_size_cache) != 0)
	{
		remove(dir_size_cache_temp);
		goto error_0;
	}

	status = 0;

	goto error_0;

error_2:
	closedir(directory);
error_1:
	fclose(fptr);
error_0:
	free(current_line);
	free(current_trashinfo);
	free(dir_size_cache_temp);
	free(dir_size_cache);
	free(temp_name);
	return status;
}

/**
 * @brief Moves a file or a directory (and its content) to the trash.
 *
 * @warning Do not change the current working directory when using this in a multithreaded application!
 *
 * @param path Path to the file or directory that shall be moved to the trash.
 * @return 0 when successful, -1 otherwise.
 */
int trashcan_soft_delete(const char *path)
{
	int status = LIBTRASHCAN_SUCCESS;
	char *resolved_path = NULL;
	char *data_home = NULL;
	char *trash_dir = NULL;
	char *trash_info_dir = NULL;
	char *trash_files_dir = NULL;
	char *trash_info_file = NULL;
	char *trashed_file = NULL;

	resolved_path = realpath(path, NULL);
	if (resolved_path == NULL) { HANDLE_ERROR(status, LIBTRASHCAN_REALPATH, error_0) }

	/* Get the paths for the home trash directory. */
	if (get_home_trash_dir(&data_home, &trash_dir, &trash_info_dir, &trash_files_dir) < 0) { HANDLE_ERROR(status, LIBTRASHCAN_HOMETRASH, error_0) }

	/* Create $XDG_DATA_HOME if it doesn't exist */
	if (mkdir_recursive(data_home, S_IRWXU) < 0) { HANDLE_ERROR(status, LIBTRASHCAN_MKDIRHOME, error_1) }

	struct stat trash_stat;
	struct stat path_stat;
	if (lstat(data_home, &trash_stat)) { HANDLE_ERROR(status, LIBTRASHCAN_HOMESTAT, error_1) }
	if (lstat(resolved_path, &path_stat)) { HANDLE_ERROR(status, LIBTRASHCAN_PATHSTAT, error_1) }

	if (trash_stat.st_dev == path_stat.st_dev)
	{
		/* File or directory is on the same devices as the home directory. The trash directory is "$XDG_DATA_HOME/Trash".
		 * Create the directories, if they don't exist. */
		if (create_trash_dir(trash_info_dir, trash_files_dir, S_IRWXU) < 0) { HANDLE_ERROR(status, LIBTRASHCAN_MKDIRHOME, error_1) }
	}
	else
	{
		/* If possible apply case (1) or (2) of the specification */
		unsigned char case_1_failed = 0;
		unsigned char case_num = 1;

		free(trash_dir);
		free(trash_info_dir);
		free(trash_files_dir);
		trash_dir = NULL;
		trash_info_dir = NULL;
		trash_files_dir = NULL;

		if (get_top_trash_dir(case_num, path_stat.st_dev, &trash_dir, &trash_info_dir, &trash_files_dir) < 0) { HANDLE_ERROR(status, LIBTRASHCAN_TOPDIRTRASH, error_1) }
		if (lstat(trash_dir, &trash_stat)) { case_1_failed = 1; } /* ENOENT if directory doesn't exist */
		if (!case_1_failed && (trash_stat.st_mode & S_ISVTX) == 0) { case_1_failed = 1; } /* Make sure sticky bit is set */
		if (!case_1_failed && S_ISLNK(trash_stat.st_mode)) { case_1_failed = 1; } /* Check if symlink */
		if (!case_1_failed && create_trash_dir(trash_info_dir, trash_files_dir, S_IRWXU) < 0) { case_1_failed = 1; } /* Create (sub)directories */

		if (case_1_failed)
		{
			case_num = 2;
			free(trash_dir);
			free(trash_info_dir);
			free(trash_files_dir);
			trash_dir = NULL;
			trash_info_dir = NULL;
			trash_files_dir = NULL;
			if (get_top_trash_dir(case_num, path_stat.st_dev, &trash_dir, &trash_info_dir, &trash_files_dir) < 0) { HANDLE_ERROR(status, LIBTRASHCAN_TOPDIRTRASH, error_1) }
			if (create_trash_dir(trash_info_dir, trash_files_dir, S_IRWXU) < 0) { HANDLE_ERROR(status, LIBTRASHCAN_MKDIRHOME, error_1) }
		}

	}

	/* Extract the original file or directory name */
	char *name = strrchr(resolved_path, '/');
	if (name == NULL) { HANDLE_ERROR(status, LIBTRASHCAN_NAME, error_1) }
	name++; /* Start at char after the slash */

	/* Get current time for trash info timestamp and unique filename creation in the trash dir */
	time_t rawtime;
	struct tm *timeinfo;

	time(&rawtime);
	if (rawtime == (time_t)-1) { HANDLE_ERROR(status, LIBTRASHCAN_TIME, error_1) }
	timeinfo = localtime(&rawtime);

	/* Counter for when collisions occur because at least two files with the same name get deleted at the same time. */
	unsigned int counter = 0;

	unsigned char delete_in_progress = 1;
	unsigned char enforce_random_name = 0;

	while (delete_in_progress)
	{
		if (generate_filenames(name, trash_info_dir, trash_files_dir, timeinfo, counter, enforce_random_name, &trash_info_file, &trashed_file) < 0) { HANDLE_ERROR(status, LIBTRASHCAN_FILENAMES, error_1) }

		int status_info = create_info_file(trash_info_file, resolved_path, timeinfo);

		if (status_info == 0) /* Successful .trashinfo creation */
		{
			/* Move file to trash */
			if (rename(resolved_path, trashed_file) != 0)
			{
				remove(trash_info_file);
				HANDLE_ERROR(status, LIBTRASHCAN_RENAME, error_2)
			}

			if (create_or_update_dir_size_cache(trash_dir, trash_info_dir, trash_files_dir) < 0) { HANDLE_ERROR(status, LIBTRASHCAN_DIRCACHE, error_2) }

			delete_in_progress = 0; /* Done. */
		}
		else if (status_info == 1) /* Name collision occured. Repeat with a different name. */
		{
			counter++;

			/* When even a random filename doesn't allow to create a trash info file without conflict, abort. */
			if (enforce_random_name) { HANDLE_ERROR(status, LIBTRASHCAN_COLLISION, error_2) }

			/* Can't generate unique name because the counter has wrapped. This shouldn't happen unless more files with 
			 * the name get deleted simultaneously than there are numbers in the range of uint. Use random name instead. */
			if (counter == 0) { enforce_random_name = 1; }

			free(trashed_file);
			free(trash_info_file);
			trashed_file = NULL;
			trash_info_file = NULL;
		}
		else /* Some other error */
		{
			HANDLE_ERROR(status, LIBTRASHCAN_TRASHINFO, error_2)
		}
	}

error_2:
	free(trashed_file);
	free(trash_info_file);
error_1:
	free(trash_files_dir);
	free(trash_info_dir);
	free(trash_dir);
	free(resolved_path);
error_0:
	return status;
}

#else
#error Platform not supported
#endif

/**
 * @brief Returns a textual representation of the status code.
 *
 * @param error_code The return value of an API function
 * @return String literal that contains the status message
 */
const char* trashcan_status_msg(int status_code)
{
	switch (status_code)
	{
		STATUS_CODES(STATUS_STR)
	}

	return "Unknown status.";
}

