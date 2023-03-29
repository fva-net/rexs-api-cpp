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

#ifndef REXSAPI_MODEL_HXX
#define REXSAPI_MODEL_HXX

#include <rexsapi/LoadSpectrum.hxx>
#include <rexsapi/Relation.hxx>
#include <rexsapi/RexsVersion.hxx>

/**
 * @brief Namespace for REXS model related stuff
 *
 */
namespace rexsapi
{
  /**
   * @brief Represents meta information about a model
   *
   */
  class TModelInfo
  {
  public:
    TModelInfo(std::string applicationId, std::string applicationVersion, std::string date, TRexsVersion version,
               std::optional<std::string> language)
    : m_ApplicationId{std::move(applicationId)}
    , m_ApplicationVersion{std::move(applicationVersion)}
    , m_Date{std::move(date)}
    , m_Version{std::move(version)}
    , m_Language{std::move(language)}
    {
    }

    [[nodiscard]] const std::string& getApplicationId() const& noexcept
    {
      return m_ApplicationId;
    }

    [[nodiscard]] const std::string& getApplicationVersion() const& noexcept
    {
      return m_ApplicationVersion;
    }

    [[nodiscard]] const std::optional<std::string>& getApplicationLanguage() const& noexcept
    {
      return m_Language;
    }

    [[nodiscard]] const std::string& getDate() const& noexcept
    {
      return m_Date;
    }

    [[nodiscard]] const TRexsVersion& getVersion() const& noexcept
    {
      return m_Version;
    }

  private:
    std::string m_ApplicationId;
    std::string m_ApplicationVersion;
    std::string m_Date;
    TRexsVersion m_Version;
    std::optional<std::string> m_Language;
  };


  /**
   * @brief Represents a REXS model loaded from a file or created by the TModelBuilder
   *
   * A TModel instance is the main REXS object that abstracts a complete REXS model as described by the REXS
   * standard. It can be queried for meta information, components, relations, and load spectrum, drilling down to
   * attributes and values using getters from components and relations.
   *
   * Models should not be created manually but by using the TModelBuilder. This ensures that the model is standard
   * compliant.
   */
  class TModel
  {
  public:
    /**
     * @brief Construct a new TModel object
     *
     * Models are immutable objects, once created they cannot be changed.
     *
     * @param info The models meta info. The infos version directly controls the REXS database model, and thus standard,
     * version this model complies to.
     * @param components All components of this model. Will be referenced by the relations and load spectrum. Every
     * component should be referenced in at least one relation.
     * @param relations All relations of this model.
     * @param spectrum The load spectrum of this model. The optionality of the spectrum is implied in the absensce of
     * load cases and accumulations in the spectrum.
     */
    TModel(TModelInfo info, TComponents components, TRelations relations, TLoadSpectrum spectrum)
    : m_Info{std::move(info)}
    , m_Components{std::move(components)}
    , m_Relations{std::move(relations)}
    , m_Spectrum{std::move(spectrum)}
    {
    }

    [[nodiscard]] const TModelInfo& getInfo() const&
    {
      return m_Info;
    }

    [[nodiscard]] const TComponents& getComponents() const&
    {
      return m_Components;
    }

    [[nodiscard]] const TRelations& getRelations() const&
    {
      return m_Relations;
    }

    [[nodiscard]] const TLoadSpectrum& getLoadSpectrum() const&
    {
      return m_Spectrum;
    }

  private:
    TModelInfo m_Info;
    TComponents m_Components;
    TRelations m_Relations;
    TLoadSpectrum m_Spectrum;
  };
}

#endif
