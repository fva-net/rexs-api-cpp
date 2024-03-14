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

#ifndef REXSAPI_MODEL_MERGER_HXX
#define REXSAPI_MODEL_MERGER_HXX

#include <rexsapi/Model.hxx>


namespace rexsapi
{
  /*
    - All ids are unique over both models
  */
  class TModelMerger
  {
  public:
    TModelMerger() = default;

    std::optional<TModel> merge(TResult& result, const TModel& mainModel, const TModel& referencedModel) const
    {
      if (mainModel.getInfo().getVersion() != referencedModel.getInfo().getVersion()) {
        result.addError(
          TError{TErrorLevel::ERR, fmt::format("cannot reference components from different rexs versions")});
        return {};
      }

      TComponents components;


      return TModel(mainModel.getInfo(), mainModel.getComponents(), mainModel.getRelations(),
                    mainModel.getLoadSpectrum());
    }
  };
}

#endif
