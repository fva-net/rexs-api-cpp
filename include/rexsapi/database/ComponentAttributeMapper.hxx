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

#ifndef REXSAPI_DATABASE_COMPONENT_ATTRIBUTE_MAPPER_HXX
#define REXSAPI_DATABASE_COMPONENT_ATTRIBUTE_MAPPER_HXX

#include <rexsapi/database/Model.hxx>

/** @file */

/**
 * @brief Classes and functions in the database detail namespace shall not be used by client code.
 *
 */
namespace rexsapi::database::detail
{
  class TComponentAttributeMapper
  {
  public:
    explicit TComponentAttributeMapper(const TModel& model,
                                       std::vector<std::pair<std::string, std::string>>&& attributeMappings)
    : m_Model{model}
    , m_AttributeMappings{std::move(attributeMappings)}
    {
    }

    [[nodiscard]] std::vector<std::reference_wrapper<const TAttribute>>
    getAttributesForComponent(const std::string& id) const;

  private:
    const TModel& m_Model;
    const std::vector<std::pair<std::string, std::string>> m_AttributeMappings{};
  };


  /////////////////////////////////////////////////////////////////////////////
  // Implementation
  /////////////////////////////////////////////////////////////////////////////

  inline std::vector<std::reference_wrapper<const TAttribute>>
  TComponentAttributeMapper::getAttributesForComponent(const std::string& id) const
  {
    std::vector<std::reference_wrapper<const TAttribute>> attributes;

    std::for_each(
      m_AttributeMappings.begin(), m_AttributeMappings.end(), [this, &id, &attributes](const auto& element) {
        if (id == element.first) {
          try {
            attributes.emplace_back(m_Model.findAttributeById(element.second));
          } catch (const TException&) {
            throw TException{fmt::format("attribute id={} not found for component id={}", element.second, id)};
          }
        }
      });

    return attributes;
  }
}

#endif
