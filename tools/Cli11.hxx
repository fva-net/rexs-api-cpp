/*
 * Copyright Schaeffler Technologies AG & Co. KG (info.de@schaeffler.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef REXSAPI_TOOLS_CLI11_HXX
#define REXSAPI_TOOLS_CLI11_HXX

#if defined(__clang__)
  #pragma clang diagnostic push
#endif

#if defined(__GNUC__)
  #pragma GCC diagnostic push
#endif

#if defined(__clang__)
  #pragma clang diagnostic ignored "-Wfloat-equal"
  #pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"
#endif

#if defined(__GNUC__)
  #pragma GCC diagnostic ignored "-Wfloat-equal"
  #pragma GCC diagnostic ignored "-Wconversion"
#endif

#include <CLI/App.hpp>
#include <CLI/Config.hpp>
#include <CLI/Formatter.hpp>

#if defined(__clang__)
  #pragma clang diagnostic pop
#endif

#if defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif

#endif
