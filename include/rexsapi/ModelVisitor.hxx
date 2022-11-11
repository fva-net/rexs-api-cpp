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

#ifndef REXSAPI_MODEL_VISITOR_HXX
#define REXSAPI_MODEL_VISITOR_HXX

#include <rexsapi/Model.hxx>

namespace rexsapi
{
  /**
   * @brief The TModelVisitor can be used for easy model hierarchy traversing
   *
   * It will traverse the model in the following order:
   * - ModelInfo
   * - Relations
   *   - References
   * - Components
   *   - Attributes
   * - Load Spectrum
   *   - Load Cases
   *     - Load Components
   *   - Accumulation
   *     - Load Components
   */
  class TModelVisitor
  {
  public:
    TModelVisitor() = default;

    virtual ~TModelVisitor() = default;

    /**
     * @brief Starts the traversal of the model
     *
     * Calls the appropriate TModelVisitor::onVisit template methods upon traversing the model. Upon return of the
     * method the complete model has been traversed.
     *
     * @param model The model to traverse
     */
    void visit(const TModel& model)
    {
      onVisit(model.getInfo());
      std::for_each(model.getRelations().begin(), model.getRelations().end(), [this](const auto& relation) {
        visit(relation);
      });
      std::for_each(model.getComponents().begin(), model.getComponents().end(), [this](const auto& component) {
        visit(component);
      });
      visit(model.getLoadSpectrum());
    }

  protected:
    TModelVisitor(const TModelVisitor&) = default;
    TModelVisitor& operator=(const TModelVisitor&) = delete;
    TModelVisitor(TModelVisitor&&) noexcept = default;
    TModelVisitor& operator=(TModelVisitor&&) = delete;

    /**
     * @brief Will be called once upon traversal start
     *
     * @param info The models meta data
     */
    virtual void onVisit(const TModelInfo& info) = 0;

    /**
     * @brief Will be called once for every relation
     *
     * Will be called after TModelVisitor::onVisit(const TModelInfo& info). References dont need to be manually
     * iterated, as TModelVisitor::onVisit(const TRelationReference& reference) will be called for every reference of
     * the relation.
     *
     * @param relation The current relation
     */
    virtual void onVisit(const TRelation& relation) = 0;

    /**
     * @brief Will be called once for every reference of a specific relation
     *
     * Will be called right after the correspondig relation has been visited.
     *
     * Will not visit the referenced component or associated attributes.
     *
     * @param reference The current reference
     */
    virtual void onVisit(const TRelationReference& reference) = 0;

    /**
     * @brief Will be called once for every component
     *
     * Will be called after traversing all relations and their corresponding references. Attributes dont need to be
     * manually iterated, as TModelVisitor::onVisit(const TAttribute& attribute) will be called for every attribute of
     * the component.
     *
     * @param component The current component
     */
    virtual void onVisit(const TComponent& component) = 0;

    /**
     * @brief Will be called once for every attribute of a specific component
     *
     * Will be called right after the corresponding component has been visited.
     *
     * @param attribute The current attribute
     */
    virtual void onVisit(const TAttribute& attribute) = 0;

    /**
     * @brief Will be called once for the load spectrum
     *
     * Will only be called, if the model has load cases.
     *
     * Will be called after visiting all components and their corresponding attributes. Load cases dont need to be
     * manually iterated, as TModelVisitor::onVisit(const TLoadCase& loadCase) will be called for every load case of
     * the load spectrum.
     *
     * @param spectrum The current spectrum
     */
    virtual void onVisit(const TLoadSpectrum& spectrum) = 0;

    /**
     * @brief Will be called once for every load case of the load spectrum
     *
     * Will be called after TModelVisitor::onVisit(const TLoadSpectrum& spectrum). Load components dont need to be
     * manually iterated, as TModelVisitor::onVisit(const TLoadComponent& loadComponent) will be called for every load
     * component of the corresponding load case.
     *
     * @param loadCase The current load case
     */
    virtual void onVisit(const TLoadCase& loadCase) = 0;

    /**
     * @brief Will be called once for the load spectrum accumulation
     *
     * Will only be called, if the model has an accumulation.
     *
     * Will be called after visiting all load cases and their corresponding load components. Load components dont need
     * to be manually iterated, as TModelVisitor::onVisit(const TLoadComponent& loadComponent) will be called for every
     * load component of the accumulation.
     *
     * @param accumulation The current accumulation
     */
    virtual void onVisit(const TAccumulation& accumulation) = 0;

    /**
     * @brief Will be called once for every load component of a load case or accumulation
     *
     * Will be called after the corresponding TModelVisitor::onVisit(const TLoadCase& loadCase) or
     * TModelVisitor::onVisit(const TAccumulation& accumulation).
     *
     * Will not visit the referenced component or associated attributes.
     *
     * @param loadComponent The current load component
     */
    virtual void onVisit(const TLoadComponent& loadComponent) = 0;

  private:
    void visit(const TModelInfo& info)
    {
      onVisit(info);
    }

    void visit(const TRelation& relation)
    {
      onVisit(relation);
      std::for_each(relation.getReferences().begin(), relation.getReferences().end(), [this](const auto& reference) {
        visit(reference);
      });
    }

    void visit(const TRelationReference& reference)
    {
      onVisit(reference);
    }

    void visit(const TComponent& component)
    {
      onVisit(component);
      std::for_each(component.getAttributes().begin(), component.getAttributes().end(), [this](const auto& attribute) {
        visit(attribute);
      });
    }

    void visit(const TAttribute& attribute)
    {
      onVisit(attribute);
    }

    void visit(const TLoadSpectrum& spectrum)
    {
      if (spectrum.hasLoadCases()) {
        onVisit(spectrum);
        std::for_each(spectrum.getLoadCases().begin(), spectrum.getLoadCases().end(), [this](const auto& loadCase) {
          visit(loadCase);
        });
      }
      if (spectrum.hasAccumulation()) {
        visit(spectrum.getAccumulation());
      }
    }

    void visit(const TLoadCase& loadCase)
    {
      onVisit(loadCase);
      std::for_each(loadCase.getLoadComponents().begin(), loadCase.getLoadComponents().end(),
                    [this](const auto& loadComponent) {
                      visit(loadComponent);
                    });
    }

    void visit(const TAccumulation& accumulation)
    {
      onVisit(accumulation);
      std::for_each(accumulation.getLoadComponents().begin(), accumulation.getLoadComponents().end(),
                    [this](const auto& loadComponent) {
                      visit(loadComponent);
                    });
    }

    void visit(const TLoadComponent& loadComponent)
    {
      onVisit(loadComponent);
    }
  };
}

#endif
