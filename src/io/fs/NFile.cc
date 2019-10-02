#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "NFile.h"

using namespace std;
using namespace neut;

NFile::NFile(const string path) : path(path) {
  exists = filesystem::exists(path);
  dir = filesystem::is_directory(path);
}

void NFile::read(ios_base::openmode mode) {
  ifstream ifs;
  ios_base::openmode _openmode = ios_base::ate | mode; // always seek to the end

  ifs.open(path, _openmode);
  if (ifs.fail()) {
    throw runtime_error(strerror(errno));
  }

  ifstream::pos_type pos = ifs.tellg();
  vector<char> tmp(pos);

  ifs.seekg(0, ios::beg);
  ifs.read(&tmp[0], pos); // read pos bytes from the stream into the vector

  data = tmp;
}

void NFile::write(vector<char> data, ios_base::openmode mode) {
  ofstream output(path, mode);
  if (output.fail()) {
    throw runtime_error(strerror(errno));
  }

  // write data to the disk
  output.write(data.data(), data.size());
}
