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
#include <rexsapi/database/ModelRegistry.hxx>

#include <iostream>


namespace rexsapi
{
  class TAttributeFinder
  {
  public:
    explicit TAttributeFinder(const rexsapi::TComponent& component)
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

    std::optional<std::reference_wrapper<const TAttribute>> findAttributeById(const std::string& id) const
    {
      auto it = std::find_if(m_Component.getAttributes().begin(), m_Component.getAttributes().end(),
                             [&id](const auto& attribute) {
                               return id == attribute.getAttributeId();
                             });
      if (it != m_Component.getAttributes().end()) {
        return *it;
      }

      return {};
    }

  private:
    const rexsapi::TComponent& m_Component;
  };


  class TComponentFinder
  {
  public:
    explicit TComponentFinder(const rexsapi::TModel& model)
    : m_Model{model}
    {
    }

    const rexsapi::TComponent& findComponentByName(const std::string& name) const
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

    std::optional<std::reference_wrapper<const TComponent>> findComponentByExternalId(int64_t id) const
    {
      auto it =
        std::find_if(m_Model.getComponents().begin(), m_Model.getComponents().end(), [&id](const auto& component) {
          return component.getExternalId() == static_cast<uint64_t>(id);
        });
      if (it != m_Model.getComponents().end()) {
        return *it;
      }

      return {};
    }

  private:
    const rexsapi::TModel& m_Model;
  };


  class TRelationFinder
  {
  public:
    explicit TRelationFinder(const rexsapi::TModel& model)
    : m_Model{model}
    {
    }

    // TODO: add recursive lookup for relations of references
    std::vector<std::reference_wrapper<const TRelation>> findRelationsByReferenceId(uint64_t id) const
    {
      std::vector<std::reference_wrapper<const TRelation>> relations;

      std::for_each(m_Model.getRelations().begin(), m_Model.getRelations().end(),
                    [&id, &relations](const auto& relation) {
                      auto it = std::find_if(relation.getReferences().begin(), relation.getReferences().end(),
                                             [&id](const auto& reference) {
                                               return reference.getComponent().getInternalId() == id;
                                             });
                      if (it != relation.getReferences().end()) {
                        relations.emplace_back(relation);
                      }
                    });

      return relations;
    }

  private:
    const rexsapi::TModel& m_Model;
  };

  /*
    - All ids are unique over both models
    - We have to preserve the external id and the data_source
    -
  */
  class TModelMerger
  {
  public:
    TModelMerger(const database::TModelRegistry& registry)
    : m_Registry{registry}
    {
    }

    std::optional<TModel> merge(TResult& result, const TModel& mainModel, const std::string& dataSource,
                                const TModel& referencedModel) const
    {
      if (mainModel.getInfo().getVersion() != referencedModel.getInfo().getVersion()) {
        result.addError(
          TError{TErrorLevel::ERR, fmt::format("cannot reference components from different rexs versions")});
        return {};
      }

      // TODO: find the correct language
      const auto& databaseModel = m_Registry.getModel(mainModel.getInfo().getVersion(), "en");

      TComponents components;
      TComponentFinder componentFinder{referencedModel};
      TRelationFinder relationFinder{referencedModel};
      TRelations relations;

      for (const auto& component : mainModel.getComponents()) {
        TAttributeFinder atrributeFinder{component};
        bool addComponent = true;
        const auto refAttribute = atrributeFinder.findAttributeById("referenced_component_id");
        if (refAttribute) {
          const auto& dataSourceAttribute = atrributeFinder.findAttributeById("data_source");
          if (dataSourceAttribute && dataSourceAttribute.value().get().getValueAsString() == dataSource) {
            const auto& refComponent =
              componentFinder.findComponentByExternalId(refAttribute.value().get().getValue().getValue<TIntType>());
            if (refComponent) {
              std::cout << "Found referenced component " << refComponent.value().get().getName() << std::endl;
              const auto& compRelations =
                relationFinder.findRelationsByReferenceId(refComponent.value().get().getInternalId());
              std::cout << compRelations.size() << std::endl;
              TComponents relationComponents;
              std::for_each(
                compRelations.begin(), compRelations.end(),
                [&relationComponents, id = refComponent.value().get().getInternalId()](const auto& relation) {
                  std::for_each(relation.get().getReferences().begin(), relation.get().getReferences().end(),
                                [&relationComponents, &id](const auto& reference) {
                                  if (reference.getComponent().getInternalId() != id) {
                                    relationComponents.emplace_back(reference.getComponent());
                                  }
                                });
                });
              // TODO: make components collection unique
              components.insert(components.end(), relationComponents.begin(), relationComponents.end());

              TAttributes attributes = component.getAttributes();
              std::for_each(refComponent.value().get().getAttributes().begin(),
                            refComponent.value().get().getAttributes().end(), [&attributes](const auto& attribute) {
                              // TODO: filter already available attributes in main model
                              attributes.emplace_back(attribute);
                            });

              components.emplace_back(component.getExternalId(), component.getInternalId(),
                                      databaseModel.findComponentById(component.getType()), component.getName(),
                                      attributes);
              addComponent = false;
            }
          } else {
            // TODO: We don't have a data_source attribute, so look into the global database
          }
        }
        if (addComponent) {
          components.emplace_back(component);
        }
      }

      return TModel(mainModel.getInfo(), components, relations, mainModel.getLoadSpectrum());
    }

  private:
    const database::TModelRegistry& m_Registry;
  };
}

#endif
