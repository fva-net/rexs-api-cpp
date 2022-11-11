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

#include <rexsapi/Model.hxx>


class ComponentFinder
{
public:
  explicit ComponentFinder(const rexsapi::TModel& model)
  : m_Model{model}
  {
  }

  const rexsapi::TComponent& findComponent(const std::string& name) const
  {
    auto it =
      std::find_if(m_Model.getComponents().begin(), m_Model.getComponents().end(), [&name](const auto& component) {
        return component.getName() == name;
      });
    if (it == m_Model.getComponents().end()) {
      throw rexsapi::TException{fmt::format("no component with name={} found", name)};
    }
    return *it;
  }

private:
  const rexsapi::TModel& m_Model;
};


class AttributeFinder
{
public:
  explicit AttributeFinder(const rexsapi::TComponent& component)
  : m_Component{component}
  {
  }

  rexsapi::TAttributes findCustomAttributes() const
  {
    rexsapi::TAttributes attributes;

    std::for_each(m_Component.getAttributes().begin(), m_Component.getAttributes().end(),
                  [&attributes](const auto& attribute) {
                    if (attribute.isCustomAttribute()) {
                      attributes.emplace_back(attribute);
                    }
                  });

    return attributes;
  }

private:
  const rexsapi::TComponent& m_Component;
};
