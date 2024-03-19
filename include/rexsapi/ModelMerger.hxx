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


#include <rexsapi/ExternalSubcomponentsChecker.hxx>
#include <rexsapi/Mode.hxx>
#include <rexsapi/ModelBuilder.hxx>
#include <rexsapi/database/ModelRegistry.hxx>

#include <functional>
#include <set>


namespace rexsapi
{
  class TAttributeFinder
  {
  public:
    explicit TAttributeFinder(const TComponent& component)
    : m_Component{component}
    {
    }

    [[nodiscard]] rexsapi::TAttributes findCustomAttributes() const
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

    [[nodiscard]] std::optional<std::reference_wrapper<const TAttribute>> findAttributeById(const std::string& id) const
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
    const TComponent& m_Component;
  };


  class TComponentFinder
  {
  public:
    explicit TComponentFinder(const TComponents& components)
    : m_Components{components}
    {
    }

    [[nodiscard]] std::optional<std::reference_wrapper<const TComponent>> findComponentByExternalId(int64_t id) const
    {
      auto it = std::find_if(m_Components.begin(), m_Components.end(), [&id](const auto& component) {
        return component.getExternalId() == static_cast<uint64_t>(id);
      });
      if (it != m_Components.end()) {
        return *it;
      }

      return {};
    }

    [[nodiscard]] std::optional<std::reference_wrapper<const TComponent>> findComponentByInternalId(uint64_t id) const
    {
      auto it = std::find_if(m_Components.begin(), m_Components.end(), [&id](const auto& component) {
        return component.getInternalId() == id;
      });
      if (it != m_Components.end()) {
        return *it;
      }

      return {};
    }

    [[nodiscard]] std::vector<TAttribute> findAllAttributesByAttributeId(const std::string& attribute) const
    {
      std::vector<TAttribute> attributes;
      std::for_each(m_Components.begin(), m_Components.end(), [&attribute, &attributes](const auto& component) {
        TAttributeFinder finder{component};
        const auto& attr = finder.findAttributeById(attribute);
        if (attr) {
          attributes.emplace_back(attr.value());
        }
      });

      return attributes;
    }

  private:
    const TComponents& m_Components;
  };


  class TRelationFinder
  {
  public:
    explicit TRelationFinder(TMode mode, const TModel& model, const TRexsVersion& version)
    : m_Model{model}
    , m_Version{version}
    , m_Checker{mode}
    , m_SubcomponentChecker{mode, version}
    {
    }

    [[nodiscard]] std::vector<std::reference_wrapper<const TRelation>>
    findRelationsByReferenceId(TResult& result, uint64_t id, bool mainLevel = true) const
    {
      std::vector<std::reference_wrapper<const TRelation>> relations;
      const TComponent* mainComponent = nullptr;

      std::for_each(m_Model.getRelations().begin(), m_Model.getRelations().end(), [&, this](const auto& relation) {
        auto it = std::find_if(
          relation.getReferences().begin(), relation.getReferences().end(), [&, this](const auto& reference) {
            if (reference.getComponent().getInternalId() == id &&
                m_Checker.isMainComponentRole(result, m_Version, relation.getType(), reference.getRole())) {
              mainComponent = &reference.getComponent();
              return true;
            }
            return false;
          });

        if (it != relation.getReferences().end()) {
          std::for_each(
            relation.getReferences().begin(), relation.getReferences().end(), [&, this](const auto& reference) {
              if (reference.getComponent().getInternalId() != id) {
                if (mainLevel && mainComponent) {
                  m_SubcomponentChecker.isPermissibleSubComponent(result, *mainComponent, reference.getComponent());
                }
                auto subRelations = findRelationsByReferenceId(result, reference.getComponent().getInternalId(), false);
                relations.insert(relations.end(), subRelations.begin(), subRelations.end());
              }
            });
          relations.emplace_back(relation);
        }
      });

      return relations;
    }

  private:
    const TModel& m_Model;
    const TRexsVersion& m_Version;
    const TRelationTypeChecker m_Checker;
    TExternalSubcomponentsChecker m_SubcomponentChecker;
  };


  class TModelMerger
  {
  public:
    explicit TModelMerger(TMode mode, const database::TModelRegistry& registry)
    : m_Mode{mode}
    , m_Registry{registry}
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

      const TComponentFinder componentFinder{referencedModel.getComponents()};
      const TRelationFinder relationFinder{m_Mode.getMode(), referencedModel, referencedModel.getInfo().getVersion()};

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
                relationFinder.findRelationsByReferenceId(result, referencedComponent.getInternalId());
              referencedRelations.emplace_back(
                ReferencedRelation{component.getInternalId(), referencedComponent.getInternalId(), {}});
              std::set<TComponent> relationComponents;
              std::for_each(compRelations.begin(), compRelations.end(), [&](const auto& relation) {
                referencedRelations.back().referencedRelations.emplace_back(relation);
                std::for_each(relation.get().getReferences().begin(), relation.get().getReferences().end(),
                              [&](const auto& reference) {
                                if (reference.getComponent().getInternalId() != referencedComponent.getInternalId()) {
                                  relationComponents.emplace(reference.getComponent());
                                }
                              });
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

      for (const auto& relation : mainModel.getRelations()) {
        modelBuilder.addRelation(relation.getType());
        if (relation.getOrder()) {
          modelBuilder.order(relation.getOrder().value());
        }
        std::for_each(
          relation.getReferences().begin(), relation.getReferences().end(), [&modelBuilder](const auto& reference) {
            modelBuilder.addRef(reference.getRole(), std::to_string(reference.getComponent().getInternalId()))
              .hint(reference.getHint());
          });
      }

      for (const auto& refRelation : referencedRelations) {
        std::for_each(refRelation.referencedRelations.begin(), refRelation.referencedRelations.end(),
                      [&modelBuilder, &refRelation](const auto& relation) {
                        modelBuilder.addRelation(relation.getType());
                        if (relation.getOrder()) {
                          modelBuilder.order(relation.getOrder().value());
                        }

                        std::for_each(
                          relation.getReferences().begin(), relation.getReferences().end(),
                          [&modelBuilder, &refRelation](const auto& reference) {
                            auto id = reference.getComponent().getInternalId();
                            if (id == refRelation.referencedModelId) {
                              id = refRelation.mainModelId;
                            }
                            modelBuilder.addRef(reference.getRole(), std::to_string(id)).hint(reference.getHint());
                          });
                      });
      }

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

    const detail::TModeAdapter m_Mode;
    const database::TModelRegistry& m_Registry;
  };
}

#endif
