/*
Copyright (c) 2023 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

#include <stdlib.h>
#include <stdint.h>

namespace espMqttManagerHelpers {

int isNewVersion(const char* newVersion, const char* currentVersion);

}  // end namespace espMqttManagerHelpers
