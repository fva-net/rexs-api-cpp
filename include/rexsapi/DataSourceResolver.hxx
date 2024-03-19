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

#ifndef REXSAPI_DATA_SOURCE_RESOLVER_HXX
#define REXSAPI_DATA_SOURCE_RESOLVER_HXX

#include <rexsapi/Mode.hxx>
#include <rexsapi/Model.hxx>
#include <rexsapi/Result.hxx>


namespace rexsapi
{
  class TDataSourceResolver
  {
  public:
    virtual ~TDataSourceResolver() = default;

    virtual std::optional<TModel> load(const std::string& data_source, TResult& result,
                                       TMode mode = TMode::STRICT_MODE) const = 0;
  };
}

#endif
