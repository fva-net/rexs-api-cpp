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

#include <functional>
#include <set>


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
    explicit TComponentFinder(const rexsapi::TComponents& components)
    : m_Components{components}
    {
    }

    std::optional<std::reference_wrapper<const TComponent>> findComponentByExternalId(int64_t id) const
    {
      auto it = std::find_if(m_Components.begin(), m_Components.end(), [&id](const auto& component) {
        return component.getExternalId() == static_cast<uint64_t>(id);
      });
      if (it != m_Components.end()) {
        return *it;
      }

      return {};
    }

    std::optional<std::reference_wrapper<const TComponent>> findComponentByInternalId(uint64_t id) const
    {
      auto it = std::find_if(m_Components.begin(), m_Components.end(), [&id](const auto& component) {
        return component.getInternalId() == id;
      });
      if (it != m_Components.end()) {
        return *it;
      }

      return {};
    }

  private:
    const rexsapi::TComponents& m_Components;
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
    explicit TModelMerger(const database::TModelRegistry& registry)
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

      const auto language = mainModel.getInfo().getApplicationLanguage();
      const auto& databaseModel = m_Registry.getModel(mainModel.getInfo().getVersion(), language.value_or("en"));

      TComponents components;
      const TComponentFinder componentFinder{referencedModel.getComponents()};
      const TRelationFinder relationFinder{referencedModel};
      TRelations relations;

      struct ReferencedRelation {
        uint64_t mainModelId;
        uint64_t referencedModelId;
        TRelations referencedRelations;
      };
      std::vector<ReferencedRelation> referencedRelations;

      for (const auto& component : mainModel.getComponents()) {
        const TAttributeFinder atrributeFinder{component};
        bool addComponent = true;
        const auto refAttribute = atrributeFinder.findAttributeById("referenced_component_id");
        if (refAttribute) {
          const auto& dataSourceAttribute = atrributeFinder.findAttributeById("data_source");
          if (dataSourceAttribute && dataSourceAttribute.value().get().getValueAsString() == dataSource) {
            const auto& refComponent =
              componentFinder.findComponentByExternalId(refAttribute.value().get().getValue().getValue<TIntType>());
            if (refComponent) {
              const auto& referencedId = refComponent.value().get().getInternalId();
              const auto& compRelations = relationFinder.findRelationsByReferenceId(referencedId);
              referencedRelations.emplace_back(ReferencedRelation{component.getInternalId(), referencedId, {}});
              std::set<TComponent> relationComponents;
              std::for_each(compRelations.begin(), compRelations.end(), [&](const auto& relation) {
                std::set<TComponent> tmp;
                bool isRefContained = false;
                std::for_each(relation.get().getReferences().begin(), relation.get().getReferences().end(),
                              [&](const auto& reference) {
                                if (reference.getComponent().getInternalId() != referencedId) {
                                  tmp.emplace(reference.getComponent());
                                } else {
                                  isRefContained = true;
                                  referencedRelations.back().referencedRelations.emplace_back(relation);
                                }
                              });
                if (isRefContained) {
                  relationComponents.insert(tmp.begin(), tmp.end());
                }
              });

              components.insert(components.end(), relationComponents.begin(), relationComponents.end());

              TAttributes attributes = component.getAttributes();
              std::for_each(refComponent.value().get().getAttributes().begin(),
                            refComponent.value().get().getAttributes().end(), [&attributes](const auto& attribute) {
                              auto it =
                                std::find_if(attributes.begin(), attributes.end(), [&attribute](const auto& attr) {
                                  return attribute.getAttributeId() == attr.getAttributeId();
                                });
                              if (it == attributes.end()) {
                                attributes.emplace_back(attribute);
                              }
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

      const TComponentFinder newModelComponentFinder{components};
      for (const auto& relation : mainModel.getRelations()) {
        TRelationReferences references;
        std::for_each(relation.getReferences().begin(), relation.getReferences().end(),
                      [&references, &newModelComponentFinder](const auto& reference) {
                        references.emplace_back(
                          reference.getRole(), reference.getHint(),
                          newModelComponentFinder.findComponentByInternalId(reference.getComponent().getInternalId())
                            .value()
                            .get());
                      });
        relations.emplace_back(relation.getType(), relation.getOrder(), references);
      }

      for (const auto& refRelation : referencedRelations) {
        std::for_each(refRelation.referencedRelations.begin(), refRelation.referencedRelations.end(),
                      [&relations, &refRelation, &newModelComponentFinder](const auto& relation) {
                        TRelationReferences references;
                        std::for_each(relation.getReferences().begin(), relation.getReferences().end(),
                                      [&refRelation, &references, &newModelComponentFinder](const auto& reference) {
                                        auto id = reference.getComponent().getInternalId();
                                        if (id == refRelation.referencedModelId) {
                                          id = refRelation.mainModelId;
                                        }
                                        references.emplace_back(
                                          reference.getRole(), reference.getHint(),
                                          newModelComponentFinder.findComponentByInternalId(id).value().get());
                                      });
                        relations.emplace_back(relation.getType(), relation.getOrder(), references);
                      });
      }

      return TModel(mainModel.getInfo(), std::move(components), std::move(relations), mainModel.getLoadSpectrum());
    }

  private:
    const database::TModelRegistry& m_Registry;
  };
}

#endif
