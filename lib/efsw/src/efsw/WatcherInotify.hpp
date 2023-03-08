#ifndef EFSW_WATCHERINOTIFY_HPP
#define EFSW_WATCHERINOTIFY_HPP

#include <efsw/FileInfo.hpp>
#include <efsw/FileWatcherImpl.hpp>

namespace efsw {

class WatcherInotify : public Watcher {
  public:
	WatcherInotify();

	WatcherInotify( WatchID id, std::string directory, FileWatchListener* listener, bool recursive,
					WatcherInotify* parent = NULL );

	bool inParentTree( WatcherInotify* parent );

	WatcherInotify* Parent;
	WatchID InotifyID;

	FileInfo DirInfo;
};

} // namespace efsw

#endif
