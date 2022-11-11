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

#ifndef REXSAPI_DATABASE_MODEL_HXX
#define REXSAPI_DATABASE_MODEL_HXX

#include <rexsapi/Exception.hxx>
#include <rexsapi/Format.hxx>
#include <rexsapi/RexsVersion.hxx>
#include <rexsapi/Types.hxx>
#include <rexsapi/database/Attribute.hxx>
#include <rexsapi/database/Component.hxx>
#include <rexsapi/database/Unit.hxx>

#include <unordered_map>

/** @file */

/**
 * @brief Namespace for all REXS database model related stuff
 *
 */
namespace rexsapi::database
{
  /**
   * @brief Represents the current status of an imported REXS model
   *
   */
  enum class TStatus {
    RELEASED,       //!< REXS model has been released
    IN_DEVELOPMENT  //!< REXS model is still in development
  };

  /**
   * @brief Converts a string into a TStatus
   *
   * @param status The status string to convert. Allowed values are: "RELEASED" and "IN_DEVELOPMENT"
   * @return TStatus The converted status
   * @throws TEXception if the status string is not an allowed value
   */
  inline static TStatus statusFromString(const std::string& status)
  {
    if (status == "RELEASED") {
      return TStatus::RELEASED;
    }
    if (status == "IN_DEVELOPMENT") {
      return TStatus::IN_DEVELOPMENT;
    }
    throw TException{fmt::format("status '{}' unkown", status)};
  }


  /**
   * @brief Represents a specific REXS database model version.
   *
   * Models should not be created manually, but imported from the REXS database using the
   * TModelRegistry.
   *
   * Each model corresponds to a model loaded from a specific REXS database version. There will be one model for each
   * language and version.
   *
   * The model is the main access point for querying the REXS database. It is used internally by the API for the REXS
   * model import and creation and determines which elements, be it components, attributes, or other, a REXS model is
   * allowed to use in order to be standard compliant.
   */
  class TModel
  {
  public:
    /**
     * @brief Constructs a new TModel object
     *
     * @param version The REXS version of this model
     * @param language The language of this model. All parts of the model like attributes and components will have names
     * corresponding to the models language.
     * @param date  The date this database model has been created
     * @param status The status of the model
     */
    TModel(TRexsVersion version, std::string language, std::string date, TStatus status)
    : m_Version{std::move(version)}
    , m_Language{std::move(language)}
    , m_Date{std::move(date)}
    , m_Status{status}
    {
    }

    ~TModel() = default;

    TModel(const TModel&) = delete;
    TModel& operator=(const TModel&) = delete;
    TModel(TModel&&) = default;
    TModel& operator=(TModel&&) = delete;

    [[nodiscard]] const TRexsVersion& getVersion() const noexcept
    {
      return m_Version;
    }

    [[nodiscard]] const std::string& getLanguage() const noexcept
    {
      return m_Language;
    }

    [[nodiscard]] const std::string& getDate() const noexcept
    {
      return m_Date;
    }

    /**
     * @brief Checks if this model has been released
     *
     * @return true if the model has been released
     * @return false if the model has not been released
     */
    [[nodiscard]] bool isReleased() const noexcept
    {
      return m_Status == TStatus::RELEASED;
    }

    bool addUnit(TUnit&& unit);

    /**
     * @brief Retrieves a unit by id
     *
     * @param unitId The unit id to retrieve. Corresponds to the _unit_ "id" attribute of the REXS database model.
     * @return const TUnit& to the found unit
     * @throws TExecption if the given unit id is not known to this model
     */
    [[nodiscard]] const TUnit& findUnitById(uint64_t unitId) const;

    /**
     * @brief Retrieves a unit by name
     *
     * @param name The unit name to retrieve. Corresponds to the _unit_ "name" attribute of the REXS database model.
     * @return const TUnit& to the found unit
     * @throws TExecption if the given unit name is not known to this model
     */
    [[nodiscard]] const TUnit& findUnitByName(const std::string& name) const;

    bool addType(uint64_t id, TValueType type);

    /**
     * @brief Retrievs a value type by id
     *
     * @param valueTypeId The value type id to retrieve. Corresponds to the _valueType_ "id" attribute of the REXS
     * database model.
     * @return const TValueType& to the found value type
     * @throws TException if the given value type id is not known to this model
     */
    [[nodiscard]] const TValueType& findValueTypeById(uint64_t valueTypeId) const;

    bool addAttribute(TAttribute&& attribute);

    /**
     * @brief Retrieve an attribute by id
     *
     * @param attributeId The attribute id to retrieve. Corresponds to the _attribute_ "attributeId" attribute of the
     * REXS database model.
     * @return const TAttribute& to the found attribute
     * * @throws TException if the given attribute id is not known to this model
     */
    [[nodiscard]] const TAttribute& findAttributeById(const std::string& attributeId) const;

    /**
     * @brief Checks if an attribute with the given id is part of the model
     *
     * @param attributeId The attribute id to check for. Corresponds to the _attribute_ "attributeId" attribute of the
     * REXS database model.
     * @return true if the attribute is part of the model
     * @return false if an attribute with the given id could not be found
     */
    [[nodiscard]] bool hasAttributeWithId(const std::string& attributeId) const noexcept;

    bool addComponent(TComponent&& component);

    /**
     * @brief Retrieve a component by id
     *
     * @param componentId The component id to retrieve. Corresponds to the _component_ "componentId" attribute of the
     * REXS database model.
     * @return const TComponent& to the found component
     * * @throws TException if the given component id is not known to this model
     */
    [[nodiscard]] const TComponent& findComponentById(const std::string& componentId) const;

  private:
    TRexsVersion m_Version;
    std::string m_Language;
    std::string m_Date;
    TStatus m_Status;
    std::unordered_map<uint64_t, TUnit> m_Units;
    std::unordered_map<uint64_t, TValueType> m_Types;
    std::unordered_map<std::string, TAttribute> m_Attributes;
    std::unordered_map<std::string, TComponent> m_Components;
  };


  /////////////////////////////////////////////////////////////////////////////
  // Implementation
  /////////////////////////////////////////////////////////////////////////////

  inline bool TModel::addUnit(TUnit&& unit)
  {
    const auto [_, added] = m_Units.try_emplace(unit.getId(), std::move(unit));
    return added;
  }

  inline const TUnit& TModel::findUnitById(uint64_t id) const
  {
    const auto it = m_Units.find(id);
    if (it == m_Units.end()) {
      throw TException{fmt::format("unit with id '{}' not found in database", std::to_string(id))};
    }

    return it->second;
  }

  inline const TUnit& TModel::findUnitByName(const std::string& name) const
  {
    const auto it = std::find_if(m_Units.begin(), m_Units.end(), [&name](const auto& item) {
      const auto& [_, unit] = item;
      return unit.getName() == name;
    });
    if (it == m_Units.end()) {
      throw TException{fmt::format("unit '{}' not found in database", name)};
    }

    return it->second;
  }

  inline bool TModel::addType(uint64_t id, TValueType type)
  {
    const auto [_, added] = m_Types.try_emplace(id, type);
    return added;
  }

  inline const TValueType& TModel::findValueTypeById(uint64_t id) const
  {
    const auto it = m_Types.find(id);
    if (it == m_Types.end()) {
      throw TException{fmt::format("value type '{}' not found in database", std::to_string(id))};
    }

    return it->second;
  }

  inline bool TModel::addAttribute(TAttribute&& attribute)
  {
    const auto [_, added] = m_Attributes.try_emplace(attribute.getAttributeId(), std::move(attribute));
    return added;
  }

  inline const TAttribute& TModel::findAttributeById(const std::string& id) const
  {
    const auto it = m_Attributes.find(id);
    if (it == m_Attributes.end()) {
      throw TException{fmt::format("attribute '{}' not found in database", id)};
    }

    return it->second;
  }

  inline bool TModel::hasAttributeWithId(const std::string& id) const noexcept
  {
    return m_Attributes.find(id) != m_Attributes.end();
  }

  inline bool TModel::addComponent(TComponent&& component)
  {
    const auto [_, added] = m_Components.try_emplace(component.getComponentId(), std::move(component));
    return added;
  }

  inline const TComponent& TModel::findComponentById(const std::string& id) const
  {
    const auto it = m_Components.find(id);
    if (it == m_Components.end()) {
      throw TException{fmt::format("component '{}' not found in database", id)};
    }

    return it->second;
  }

}

#endif
