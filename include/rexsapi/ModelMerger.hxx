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


#include <rexsapi/ModelBuilder.hxx>
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

      TModelBuilder modelBuilder{databaseModel};


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
            const auto refComponentId = refAttribute.value().get().getValue().getValue<TIntType>();
            const auto& refComponent = componentFinder.findComponentByExternalId(refComponentId);
            if (refComponent) {
              const auto& referencedComponent = refComponent.value().get();
              if (referencedComponent.getType() != component.getType()) {
                result.addError(
                  TError{TErrorLevel::CRIT,
                         fmt::format("referenced component {} in data_source '{}' has wrong type '{}' instead of '{}'",
                                     refComponentId, dataSource, referencedComponent.getType(), component.getType())});
                return {};
              }
              const auto& compRelations =
                relationFinder.findRelationsByReferenceId(referencedComponent.getInternalId());
              referencedRelations.emplace_back(
                ReferencedRelation{component.getInternalId(), referencedComponent.getInternalId(), {}});
              std::set<TComponent> relationComponents;
              std::for_each(compRelations.begin(), compRelations.end(), [&](const auto& relation) {
                std::set<TComponent> tmp;
                bool isRefContained = false;
                std::for_each(relation.get().getReferences().begin(), relation.get().getReferences().end(),
                              [&](const auto& reference) {
                                if (reference.getComponent().getInternalId() != referencedComponent.getInternalId()) {
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

              for (const auto& comp : relationComponents) {
                insertComponent(modelBuilder, comp, {});
              }

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

              insertComponent(modelBuilder, component, attributes);
              addComponent = false;
            } else {
              result.addError(
                TError{TErrorLevel::CRIT, fmt::format("cannot find referenced component {} in data_source '{}'",
                                                      refComponentId, dataSource)});
              return {};
            }
          } else {
            // TODO: We don't have a data_source attribute, so look into the global database
          }
        }
        if (addComponent) {
          insertComponent(modelBuilder, component, {});
        }
      }

      /*
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
       */

      return modelBuilder.build(mainModel.getInfo());
    }

  private:
    void insertComponent(TModelBuilder& modelBuilder, const TComponent& component, const TAttributes& attributes) const
    {
      modelBuilder.addComponent(component.getType(), std::to_string(component.getInternalId()))
        .name(component.getName());
      for (const auto& attr : attributes.empty() ? component.getAttributes() : attributes) {
        // TODO: handle custom attributes
        // TODO: maybe add new value type REFERENCE_EXTERNAL_COMPONENT
        if (attr.getValueType() == TValueType::REFERENCE_COMPONENT &&
            attr.getAttributeId() != "referenced_component_id") {
          modelBuilder.addAttribute(attr.getAttributeId())
            .unit(attr.getUnit().getName())
            .reference(std::to_string(attr.getValue().getValue<rexsapi::TIntType>()));
        } else {
          modelBuilder.addAttribute(attr.getAttributeId()).unit(attr.getUnit().getName()).value(attr.getValue());
        }
      }
    }

    const database::TModelRegistry& m_Registry;
  };
}

#endif
