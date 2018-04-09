#ifndef TEMP_FILE_GEN_H
#define TEMP_FILE_GEN_H

#include <climits>
#include <algorithm>
#include <time.h>
#include <string>
#include <iostream>
#include <unordered_set>
#include <mutex>

using namespace std;

class TempFileGen {
private:
  // map to store used temporary file names.
  static unordered_set<long long int> filenames_;

  // mutex for filenames_
  static mutex mutex_;

public:
  static long long int GenFileName() {
    srand(time(NULL));
    long long int file_name = rand() % LONG_MAX;
    mutex_.lock();
    while (!filenames_.insert(rand() % LONG_MAX).second) {
      file_name = rand() % LONG_MAX;
    }
    mutex_.unlock();
    return file_name;
  }

  static void RemoveFile(long long int file_name) {
    mutex_.lock();
    remove(to_string(file_name).c_str());
    filenames_.erase(file_name);
    mutex_.unlock();
  }
};
#endif