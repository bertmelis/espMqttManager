/*
Copyright (c) 2022 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#include "SemVer.h"

namespace espMqttManagerHelpers {

int isNewVersion(const char* newVersion, const char* currentVersion) {
  const char* begin = newVersion;
  char* end = nullptr;
  uint32_t newMajor = std::strtoul(begin, &end, 10);
  if (end == begin) {
    return -1;
  }
  begin = end + 1;
  uint32_t newMinor = std::strtol(begin, &end, 10);
  if (end == begin) {
    return -1;
  }
  begin = end + 1;
  uint32_t newPatch = std::strtol(begin, &end, 10);
  if (end == begin) {
    return -1;
  }

  begin = currentVersion;
  end = nullptr;
  uint32_t oldMajor = std::strtoul(begin, &end, 10);
  if (end == begin) {
    return -1;
  }
  begin = end + 1;
  uint32_t oldMinor = std::strtol(begin, &end, 10);
  if (end == begin) {
    return -1;
  }
  begin = end + 1;
  uint32_t oldPatch = std::strtol(begin, &end, 10);
  if (end == begin) {
    return -1;
  }

  if (newMajor > oldMajor) return 1;
  if (newMajor == oldMajor) {
    if (newMinor > oldMinor) return 1;
    if (newMinor == oldMinor) {
      if (newPatch > oldPatch) return 1;
      if (newPatch == oldPatch) return 0;
    }
  }
  return -1;
}

}  // end namespace espMqttManagerHelpers
