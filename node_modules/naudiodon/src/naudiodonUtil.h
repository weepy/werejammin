/*
  Aerostat Beam Coder - Node.js native bindings for FFmpeg.
  Copyright (C) 2019  Streampunk Media Ltd.

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.

  https://www.streampunk.media/ mailto:furnace@streampunk.media
  14 Ormiscaig, Aultbea, Achnasheen, IV22 2JJ  U.K.
*/

#ifndef BEAMCODER_UTIL_H
#define BEAMCODER_UTIL_H

#include <chrono>
#include <stdio.h>
#include <string>
#include <unordered_map>
#include <algorithm>
#include "node_api.h"

#define DECLARE_NAPI_METHOD(name, func) { name, 0, func, 0, 0, 0, napi_default, 0 }

// Handling NAPI errors - use "napi_status status;" where used
#define CHECK_STATUS if (checkStatus(env, status, __FILE__, __LINE__ - 1) != napi_ok) return nullptr
#define PASS_STATUS if (status != napi_ok) return status
#define ACCEPT_STATUS(s) if ((status != s) && (status != napi_ok)) return status

napi_status checkStatus(napi_env env, napi_status status,
  const char * file, uint32_t line);

// High resolution timing
#define HR_TIME_POINT std::chrono::high_resolution_clock::time_point
#define NOW std::chrono::high_resolution_clock::now()
long long microTime(std::chrono::high_resolution_clock::time_point start);

// Argument processing
napi_status checkArgs(napi_env env, napi_callback_info info, const char* methodName,
  napi_value* args, size_t argc, napi_valuetype* types);

// Async error handling
#define NAUDIODON_ERROR_START 6000
#define NAUDIODON_INVALID_ARGS 6001
#define NAUDIODON_SUCCESS 0

struct carrier {
  virtual ~carrier() {}
  napi_ref passthru = nullptr;
  int32_t status = NAUDIODON_SUCCESS;
  std::string errorMsg;
  long long totalTime;
  napi_deferred _deferred;
  napi_async_work _request = nullptr;
};

void tidyCarrier(napi_env env, carrier* c);
int32_t rejectStatus(napi_env env, carrier* c, char* file, int32_t line);

#define REJECT_STATUS if (rejectStatus(env, c, (char*) __FILE__, __LINE__) != NAUDIODON_SUCCESS) return;
#define REJECT_RETURN if (rejectStatus(env, c, (char*) __FILE__, __LINE__) != NAUDIODON_SUCCESS) return promise;
#define FLOATING_STATUS if (status != napi_ok) { \
  printf("Unexpected N-API status not OK in file %s at line %d value %i.\n", \
    __FILE__, __LINE__ - 1, status); \
}

#define NAPI_THROW_ERROR(msg) { \
  char errorMsg[256]; \
  sprintf(errorMsg, "%s", msg); \
  napi_throw_error(env, nullptr, errorMsg); \
  return nullptr; \
}

napi_status naud_set_uint32(napi_env env, napi_value target, const char* name, uint32_t value);
napi_status naud_get_uint32(napi_env env, napi_value target, const char* name, uint32_t* value);
napi_status naud_set_int32(napi_env env, napi_value target, const char* name, int32_t value);
napi_status naud_get_int32(napi_env env, napi_value target, const char* name, int32_t* value);
napi_status naud_set_int64(napi_env env, napi_value target, const char* name, int64_t value);
napi_status naud_get_int64(napi_env env, napi_value target, const char* name, int64_t* value);
napi_status naud_set_double(napi_env env, napi_value target, const char* name, double value);
napi_status naud_get_double(napi_env env, napi_value target, const char* name, double* value);
napi_status naud_set_string_utf8(napi_env env, napi_value target, const char* name, const char* value);
napi_status naud_get_string_utf8(napi_env env, napi_value target, const char* name, char** value);
napi_status naud_set_bool(napi_env env, napi_value target, const char* name, bool value);
napi_status naud_get_bool(napi_env env, napi_value target, const char* name, bool* present, bool* value);
napi_status naud_set_null(napi_env env, napi_value target, const char* name);
napi_status naud_is_null(napi_env env, napi_value props, const char* name, bool* isNull);
napi_status naud_delete_named_property(napi_env env, napi_value props, const char* name, bool* deleted);

#endif // BEAMCODER_UTIL_H
