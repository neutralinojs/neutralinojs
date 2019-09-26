#ifndef NFILE_H
#define NFILE_H

#include <string>
#include <vector>
#include <filesystem>

using namespace std;

namespace neut {
class NFile {
private:
  vector<char> data;
  string path;
  bool dir;
  bool exists;

public:
  NFile(const string path);
  void read(ios_base::openmode mode);
  void write(const vector<char> data, ios_base::openmode mode);
  string getString();
  vector<char> getBytes();
  size_t length();
  bool isDir();
  bool isExists();
};

inline bool NFile::isExists() { return exists; }

inline bool NFile::isDir() { return dir; }

} // namespace neut

#endif
