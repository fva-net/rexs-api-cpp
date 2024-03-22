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
  namespace detail
  {
    class TAttributeFinder
    {
    public:
      explicit TAttributeFinder(const rexsapi::TComponent& component)
      : m_Component{component}
      {
      }

      [[nodiscard]] rexsapi::TAttributes findCustomAttributes() const;

      [[nodiscard]] std::optional<std::reference_wrapper<const TAttribute>> findAttributeById(const std::string& id) const;

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

      [[nodiscard]] std::optional<std::reference_wrapper<const TComponent>> findComponentByExternalId(int64_t id) const;

      [[nodiscard]] std::optional<std::reference_wrapper<const TComponent>> findComponentByInternalId(uint64_t id) const;

      [[nodiscard]] std::vector<TAttribute> findAllAttributesByAttributeId(const std::string& attribute) const;

    private:
      const rexsapi::TComponents& m_Components;
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
      findRelationsByReferenceId(TResult& result, uint64_t id, bool mainLevel = true) const;

    private:
      const TModel& m_Model;
      const TRexsVersion& m_Version;
      const TRelationTypeChecker m_Checker;
      TExternalSubcomponentsChecker m_SubcomponentChecker;
    };
  }

  
  /**
   * @brief The model merger creates a new model from a main model and a referenced model for externally referenced
   * components by the main model.
   *
   * It will clone all the relations, components, attributes, and load spectrum from the main model. The merger will
   * try to resolve all externally referenced components using the given referenced model. If a referenced component
   * is not found in the referenced model and the "data_source" attribute does not match, the attributes "data_source"
   * and "referenced_component_id" will be retained and have to be resolved later. Referenced components that cannot
   * be found will flag an error otherwise. Referenced relations from the referenced model will be cloned into the new
   * model. Additionally, not already set attributes of the referenced component will be cloned into the referencing
   * component. Validation according to the REXS permissible components rules will be performed.
   *
   * A successfully resolved referencing component will not contain the "data_source" and "referenced_component_id"
   * attributes anymore.
   *
   * @attention Only referenced components with a valid "data_source" attribute will be processed. Globally stored
   * data sources are not supported.
   *
   */
  class TModelMerger
  {
  public:
    /**
     * @brief Constructs a new TModelMerger object.
     *
     * @param mode Defines how to handle encountered issues while processing the models
     * @param registry Will load the REXS database version and language corresponding to the version information of the
     *                 main model
     */
    explicit TModelMerger(TMode mode, const database::TModelRegistry& registry)
    : m_Mode{mode}
    , m_Registry{registry}
    {
    }

    /**
     * @brief Merges the main and the referenced model into a new model with resolved references.
     *
     * If any problems like not resolvable references come up, errors will be added accordingly to the result. The
     * mode will control the type of added error. REXS permissible components rules will be checked.
     *
     * @attention The mainModel and the referencedModel both have to have the same rexs version.
     * @param result Describes the outcome of the operation. Will contain messages upon issues encountered.
     * @param mainModel The model that contains external references to be resolved
     * @param dataSource The data source the referenced model was loaded from. Has to exactly match the "data_source"
     *                   attributes given in the mainModel to actually resolve the referenced component.
     * @param referencedModel The model used as data source for the resolving
     */
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

      const detail::TComponentFinder componentFinder{referencedModel.getComponents()};
      const detail::TRelationFinder relationFinder{m_Mode.getMode(), referencedModel, referencedModel.getInfo().getVersion()};

      struct ReferencedRelation {
        uint64_t mainModelId;
        uint64_t referencedModelId;
        TRelations referencedRelations;
      };
      std::vector<ReferencedRelation> referencedRelations;

      for (const auto& component : mainModel.getComponents()) {
        const detail::TAttributeFinder atrributeFinder{component};
        bool addComponent = true;
        const auto refAttribute = atrributeFinder.findAttributeById("referenced_component_id");
        if (refAttribute) {
          const auto dataSourceAttribute = atrributeFinder.findAttributeById("data_source");
          if (dataSourceAttribute && dataSourceAttribute.value().get().getValueAsString() == dataSource) {
            const auto refComponentId = refAttribute.value().get().getValue().getValue<TIntType>();
            const auto refComponent = componentFinder.findComponentByExternalId(refComponentId);
            if (refComponent) {
              const auto& referencedComponent = refComponent.value().get();
              if (referencedComponent.getType() != component.getType()) {
                result.addError(
                  TError{TErrorLevel::CRIT,
                         fmt::format("referenced component {} in data_source '{}' has wrong type '{}' instead of '{}'",
                                     refComponentId, dataSource, referencedComponent.getType(), component.getType())});
                return {};
              }
              const auto compRelations =
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

              TAttributes attributes = getFilteredAttributes(component.getAttributes());
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

      // TODO: clone the load spectrum

      return modelBuilder.build(mainModel.getInfo());
    }

  private:
    TAttributes getFilteredAttributes(const TAttributes& attributes) const
    {
      TAttributes filteredAttributes;

      std::for_each(attributes.begin(), attributes.end(), [&filteredAttributes](const auto& attribute) {
        if (attribute.getAttributeId() != "data_source" && attribute.getAttributeId() != "referenced_component_id") {
          filteredAttributes.emplace_back(attribute);
        }
      });

      return filteredAttributes;
    }

    void insertComponent(TModelBuilder& modelBuilder, const TComponent& component, const TAttributes& attributes) const
    {
      modelBuilder.addComponent(component.getType(), std::to_string(component.getInternalId()))
        .name(component.getName());
      for (const auto& attr : attributes.empty() ? component.getAttributes() : attributes) {
        // TODO: handle custom attributes
        if (attr.getValueType() == TValueType::REFERENCE_COMPONENT &&
            attr.getAttributeId() != "referenced_component_id") {
          modelBuilder.addAttribute(attr.getAttributeId())
            .unit(attr.getUnit().getName())
            .reference(std::to_string(attr.getValue().getValue<rexsapi::TIntType>()));
        } else if (attr.isCustomAttribute()) {
          modelBuilder.addCustomAttribute(attr.getAttributeId(), attr.getValueType()).unit(attr.getUnit().getName()).value(attr.getValue());
        } else {
          modelBuilder.addAttribute(attr.getAttributeId()).unit(attr.getUnit().getName()).value(attr.getValue());
        }
      }
    }

    const detail::TModeAdapter m_Mode;
    const database::TModelRegistry& m_Registry;
  };

  namespace detail
  {
    inline rexsapi::TAttributes TAttributeFinder::findCustomAttributes() const
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

    inline std::optional<std::reference_wrapper<const TAttribute>> TAttributeFinder::findAttributeById(const std::string& id) const
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


    inline std::optional<std::reference_wrapper<const TComponent>> TComponentFinder::findComponentByExternalId(int64_t id) const
    {
      auto it = std::find_if(m_Components.begin(), m_Components.end(), [&id](const auto& component) {
        return component.getExternalId() == static_cast<uint64_t>(id);
      });
      if (it != m_Components.end()) {
        return *it;
      }

      return {};
    }

    inline std::optional<std::reference_wrapper<const TComponent>> TComponentFinder::findComponentByInternalId(uint64_t id) const
    {
      auto it = std::find_if(m_Components.begin(), m_Components.end(), [&id](const auto& component) {
        return component.getInternalId() == id;
      });
      if (it != m_Components.end()) {
        return *it;
      }

      return {};
    }

    inline std::vector<TAttribute> TComponentFinder::findAllAttributesByAttributeId(const std::string& attribute) const
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

    inline std::vector<std::reference_wrapper<const TRelation>>
    TRelationFinder::findRelationsByReferenceId(TResult& result, uint64_t id, bool mainLevel) const
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
  }
}

#endif
