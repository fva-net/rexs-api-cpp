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

#ifndef REXSAPI_MODEL_BUILDER_HXX
#define REXSAPI_MODEL_BUILDER_HXX


#include <rexsapi/Model.hxx>
#include <rexsapi/RelationTypeChecker.hxx>
#include <rexsapi/database/Model.hxx>

#include <set>

namespace rexsapi
{
  /**
   * @brief Represents a components unique id.
   *
   * Can be either a generated or a custom user defined id.
   *
   * Component ids should not be created manually but by using the TModelBuilder. This ensures that the ids are truley
   * unique and tracked by the builder.
   */
  class TComponentId
  {
  public:
    /**
     * @brief Constructs a new TComponentId object.
     *
     * A generated or user supplied id.
     *
     * @param id The components unique id
     */
    explicit TComponentId(uint64_t id)
    : m_Id{id}
    {
    }

    /**
     * @brief Constructs a new TComponentId object.
     *
     * A custom user defined id.
     *
     * @param id The components unique id
     */
    explicit TComponentId(std::string id)
    : m_Id{std::move(id)}
    {
    }

    friend bool operator==(const TComponentId& lhs, const TComponentId& rhs) noexcept
    {
      return lhs.m_Id == rhs.m_Id;
    }

    friend bool operator<(const TComponentId& lhs, const TComponentId& rhs) noexcept
    {
      return lhs.m_Id < rhs.m_Id;
    }

    std::size_t hash() const noexcept
    {
      return std::hash<Id>{}(m_Id);
    }

    std::string asString() const;

    /**
     * @brief Checks if this TComponentId has an integer id value
     *
     * @return true
     * @return false
     */
    bool isInteger() const
    {
      return m_Id.index() == 0;
    }

    /**
     * @brief Returns the integer id
     *
     * @exception Will throw std::bad_variant_access if the TComponentId id is not of type integer
     *
     * @return uint64_t
     */
    uint64_t integer() const
    {
      return std::get<uint64_t>(m_Id);
    }

  private:
    using Id = std::variant<uint64_t, std::string>;
    Id m_Id;
  };
}

namespace std
{
  /**
   * @brief Calculates the hash value for a TComponentId.
   *
   * @tparam  TComponentId
   */
  template<>
  struct hash<rexsapi::TComponentId> {
    std::size_t operator()(const rexsapi::TComponentId& id) const noexcept
    {
      return id.hash();
    }
  };
}

namespace rexsapi
{
  namespace detail
  {
    struct TAttributeEntry {
      const database::TAttribute* m_Attribute{nullptr};
      std::string m_AttributeId{};
      std::optional<TValueType> m_ValueType{};
      std::optional<TUnit> m_Unit{};
      TValue m_Value{};
      TCodeType m_CodeType{TCodeType::None};
      std::optional<TComponentId> m_Reference{};

      bool isCustom() const noexcept
      {
        return m_Attribute == nullptr;
      }

      bool operator==(const std::string& attribute) const noexcept
      {
        if (m_Attribute) {
          return m_Attribute->getAttributeId() == attribute;
        }
        return m_AttributeId == attribute;
      }

      ::rexsapi::TAttribute createAttribute(const std::unordered_map<TComponentId, uint64_t>& m_ComponentMapping) const;
      ::rexsapi::TAttribute createAttribute() const;
    };

    struct TComponentEntry {
      TComponentId m_Id;
      std::optional<uint64_t> m_ExternalId;
      const database::TComponent* m_DatabaseComponent{nullptr};
      std::string m_Name{};
      std::vector<TAttributeEntry> m_Attributes{};
    };

    class TComponents
    {
    public:
      explicit TComponents(const database::TModel& databaseModel)
      : m_DatabaseModel{databaseModel}
      {
      }

      const database::TModel& databaseModel() const noexcept
      {
        return m_DatabaseModel;
      }

      void addComponent(TComponentId id, std::optional<uint64_t> externalId = {});

      void addComponent(const std::string& component, std::optional<uint64_t> id,
                        std::optional<uint64_t> externalId = {});

      void addComponent(const std::string& component, std::string id, std::optional<uint64_t> externalId = {});

      const auto& components() const noexcept
      {
        return m_Components;
      }

      void name(std::string name)
      {
        lastComponent().m_Name = std::move(name);
      }

      TComponentId id() const
      {
        return lastComponent().m_Id;
      }

      void addAttribute(const std::string& attribute)
      {
        checkDuplicateAttribute(lastComponent(), attribute);
        lastComponent().m_Attributes.emplace_back(
          detail::TAttributeEntry{&m_DatabaseModel.findAttributeById(attribute)});
      }

      void addCustomAttribute(const std::string& attribute, TValueType type)
      {
        checkDuplicateAttribute(lastComponent(), attribute);
        lastComponent().m_Attributes.emplace_back(detail::TAttributeEntry{nullptr, attribute, type});
      }

      void reference(const TComponentId& id)
      {
        TValueType type;
        std::string attributeId;
        if (lastAttribute().isCustom()) {
          type = *lastAttribute().m_ValueType;
          attributeId = lastAttribute().m_AttributeId;
        } else {
          type = lastAttribute().m_Attribute->getValueType();
          attributeId = lastAttribute().m_Attribute->getAttributeId();
        }

        if (type != TValueType::REFERENCE_COMPONENT) {
          throw TException{fmt::format("a reference can only be set on attributes of value type REFERENCE_COMPONENT "
                                       "for attribute id={} of component id={}",
                                       attributeId, lastComponent().m_Id.asString())};
        }
        lastAttribute().m_Reference = id;
      }

      void reference(std::string id)
      {
        lastAttribute().m_Reference = TComponentId{std::move(id)};
      }

      void unit(const std::string& unit);

      void value(TValue val)
      {
        TValueType type;
        std::string attributeId;
        if (lastAttribute().isCustom()) {
          type = *lastAttribute().m_ValueType;
          attributeId = lastAttribute().m_AttributeId;
        } else {
          type = lastAttribute().m_Attribute->getValueType();
          attributeId = lastAttribute().m_Attribute->getAttributeId();
        }

        // TODO: maybe add new value type REFERENCE_EXTERNAL_COMPONENT
        if (type == TValueType::REFERENCE_COMPONENT && attributeId != "referenced_component_id") {
          throw TException{
            fmt::format("a reference has to be set using the reference method for attribute id={} of component id={}",
                        attributeId, lastComponent().m_Id.asString())};
        }

        if (!val.matchesValueType(type)) {
          throw TException{
            fmt::format("value of attribute id={} of component id={} does not have the correct value type", attributeId,
                        lastComponent().m_Id.asString())};
        }
        lastAttribute().m_Value = std::move(val);
      }

      void coded(TCodeType type)
      {
        lastAttribute().m_CodeType = type;
      }

    private:
      TComponentId getNextComponentId() noexcept;

      void checkDuplicateComponent(const TComponentId& component) const;

      void checkDuplicateComponent(const std::string& id = "") const;

      static void checkDuplicateAttribute(const detail::TComponentEntry& component, const std::string& attribute);

      void checkComponent() const;

      void checkAttribute() const;

      TComponentEntry& lastComponent() &
      {
        checkComponent();
        return m_Components.back();
      }

      const TComponentEntry& lastComponent() const&
      {
        checkComponent();
        return m_Components.back();
      }

      TAttributeEntry& lastAttribute() &
      {
        checkAttribute();
        return m_Components.back().m_Attributes.back();
      }

      uint64_t m_ComponentId{0};
      const database::TModel& m_DatabaseModel;
      std::vector<TComponentEntry> m_Components;
    };
  }


  /**
   * @brief Builder used to build REXS model TComponent collections
   *
   * Creates components and their attributes. Mainly used to be passed into a TModelBuilder for reuse of the builder.
   *
   * In general, the builder acts like a stack of components and attributes. Adding a component puts that particular
   * component on top of the stack and makes it the active component. All following method calls will act upon the
   * active component, until the next component is added. The same is true for attributes. Adding an attribute puts that
   * attribute on the attribute stack and makes it to the active attribute. All following method calls concerning
   * attributes will act on the active attribute, until the next attribute is added. For convenience, most methods
   * return a reference to the builder enabling the chaining for calls leading to more dense and easy to read code.
   *
   * Every component and attribute is checked against a REXS database model to make sure the resulting components are
   * REXS standard compliant. The builder will throw exceptions on any issues.
   */
  class TComponentBuilder
  {
  public:
    /**
     * @brief Constructs a new TComponentBuilder object
     *
     * @param databaseModel The REXS database model used to check and validate the added components and attributes for
     * standard compliance. The resulting model will have the same REXS version as the database model.
     */
    explicit TComponentBuilder(const database::TModel& databaseModel)
    : m_Components{databaseModel}
    {
    }

    /**
     * @brief Adds a new component to the builder and makes it the active component
     *
     * Internally, the builder will create a new unique component id and assign it to the component. The id can be
     * retrieved by calling TComponentBuilder::id.
     *
     * @param component The component type to add, e.g. "gear_unit". The type has to be allowed by the REXS database
     * model.
     * @param id Optional id to use for component. Will be used as the generated component id and all references. The
     * builder will make sure the ids are unique. If user assigned integer ids are used, you are not allowed to mix them
     * with string ids. Either all or none of the components shall have an explicit user assigned integer id.
     * @param externalId Optional originating id
     * @return TComponentBuilder& to the builder for chaining calls
     * @throws TException if the component type is not allowed by the REXS database model
     */
    TComponentBuilder& addComponent(const std::string& component, std::optional<uint64_t> id = {},
                                    std::optional<uint64_t> externalId = {}) &;

    /**
     * @brief Adds a new component to the builder and makes it the active component
     *
     * Uses a user supplied custom id as unique id. Will make sure that the id is unique for all components of this
     * builder. The id can be retrieved by calling TComponentBuilder::id.
     *
     * @param component The component type to add, e.g. "gear_unit". The type has to be allowed by the REXS database
     * model.
     * @param id A user supplied custom id. Has to be unique for all components of this builder. This
     * method is not allowed if a component has been supplied a user defined integer id previously.
     * @param externalId Optional originating id
     * @return TComponentBuilder& to the builder for chaining calls
     * @throws TException if the component type is not allowed by the REXS database model
     * @throws TException if the user supplied id has already been use for another component
     */
    TComponentBuilder& addComponent(const std::string& component, std::string id,
                                    std::optional<uint64_t> externalId = {}) &;

    /**
     * @brief Retrieves the component id of the active component
     *
     * @return TComponentId of the active component
     * @throws TException if there is no active component
     */
    [[nodiscard]] TComponentId id() const;

    /**
     * @brief Sets a name for the active component
     *
     * @param name The name to set for the active component. Can be an arbitrary string.
     * @return TComponentBuilder& to the builder for chaining calls
     * @throws TException if there is no active component
     */
    TComponentBuilder& name(std::string name) &;

    /**
     * @brief Adds a new attribute to the builder and makes it the active attribute
     *
     * Adds the attribute to the active component.
     *
     * @param attributeId The attribute id to add, e.g. "reference_component_for_position". The attribute id has to be
     * allowed by the REXS database model.
     * @return TComponentBuilder& to the builder for chaining calls
     * @throws TException if there is no active component
     * @throws TException if the attributeId is not allowed by the REXS database model
     */
    TComponentBuilder& addAttribute(const std::string& attributeId) &;

    /**
     * @brief Adds a new custom attribute to the builder makes it the active attribute
     *
     * Adds the custom attribute to the active component.
     *
     * @param attribute The custom attribute id. Should always start with the prefix *custom_*.
     * @param type Corresponds to one of the REXS datatypes
     * @return TComponentBuilder& to the builder for chaining calls
     * @throws TException if there is no active component
     */
    TComponentBuilder& addCustomAttribute(const std::string& attribute, TValueType type) &;

    /**
     * @brief Set a reference to a component on the active attribute
     *
     * Needs an active attribute with a value of type TValueType::REFERENCE_COMPONENT.
     *
     * @param id The component id of the referenced component. The component id has to be one of the already added
     * components to this builder. The active component id can be retrieved by calling TComponentBuilder::id. Actually,
     * the id is allowed to reference the currently active component.
     * @return TComponentBuilder& to the builder for chaining calls
     * @throws TException if there is no active attribute
     * @throws TException if the attributes value type is not TValueType::REFERENCE_COMPONENT
     */
    TComponentBuilder& reference(const TComponentId& id) &;

    /**
     * @brief Set a reference to a component on the active attribute
     *
     * Needs an active attribute with a value of type TValueType::REFERENCE_COMPONENT.
     *
     * @param id The custom user supplied id of the referenced component. The component id has to be one of the already
     * added components to this builder. Actually, the id is allowed to reference the currently active component. This
     * method is not allowed if a component has been supplied a user defined integer id previously.
     * @return TComponentBuilder& to the builder for chaining calls
     * @throws TException if there is no active attribute
     * @throws TException if the attributes value type is not TValueType::REFERENCE_COMPONENT
     */
    TComponentBuilder& reference(std::string id) &;

    /**
     * @brief Set the unit of the active attribute
     *
     * @param unit The unit name to set for the active attribute. Can be anything if the active attribute is a custom
     * attribute. Otherwise the unit has to match the active attributes unit fetched from the REXS model database.
     * @return TComponentBuilder& to the builder for chaining calls
     * @throws TException if there is no active attribute
     * @throws TException if unit does not match the active attributes unit
     */
    TComponentBuilder& unit(const std::string& unit) &;

    /**
     * @brief Set the value of the active attribute
     *
     * @tparam T The C++ type of the value to set. Has to match the attributes value type.
     * @param val The actual value to set
     * @return TComponentBuilder& to the builder for chaining calls
     * @throws TException if there is no active attribute
     * @throws TException if value does not match the active attributes value type
     * @throws TException if the attributes value type is TValueType::REFERENCE_COMPONENT. Use
     * TComponentBuilder::reference instead.
     */
    template<typename T>
    TComponentBuilder& value(T val) &;

    /**
     * @brief Specifies that the active attributes value shall be stored in a coded form
     *
     * Does only have an effect on array or matrix types. Will be ignored for other types.
     * Will store the array or matrix value in base64 coded form. Can lead to truncation of values if
     * TCodeType::Optimized is chosen.
     *
     * @param type Specifies how exactly the value shall be coded. If in doubt always take the default.
     * @return TComponentBuilder& to the builder for chaining calls
     * @throws TException if there is no active attribute
     */
    TComponentBuilder& coded(TCodeType type = TCodeType::Default) &;

    /**
     * @brief Finalizes the component builder and creates the REXS model TComponent collection
     *
     * Called as final step to create the REXS model components collection. Can be called multiple times and generates
     * the same collection of components as long as no other mehods have been called in between on this builder.
     *
     * Establishes an internal mapping between component ids and components in the returned component collection used by
     * the TModelBuilder.
     *
     * @return TComponents collection of all added REXS model components with their attributes. Will be empty if no
     * components have been added.
     * @throws TException if anything goes wrong while building the components and attributes
     */
    [[nodiscard]] TComponents build();

    /**
     * @brief Retrieves the REXS model component corrresponding to the given component id
     *
     * Should only be used with a component collection created by this builder and prior to call any other methods on
     * the builder after creating the collection with TComponentBuilder::build.
     *
     * _In general, it should not be used in user code_.
     *
     * @param components The component collection created by calling TComponentBuilder::build on the same builder
     * @param id The component id to retrieve the REXS model component for
     * @return const TComponent& The REXS model component associated to the given component id
     * @throws TException if there was no component associated to the given component id
     */
    const TComponent& getComponentForId(const TComponents& components, const TComponentId& id) const&;

  private:
    uint64_t getComponentForId(const TComponentId& id) const;

    friend class TModelBuilder;
    detail::TComponents m_Components;
    std::unordered_map<TComponentId, uint64_t> m_ComponentMapping;
  };


  /**
   * @brief Builder used to build REXS model TLoadCase instances for a TLoadSpectrum
   *
   * Creates load components and their attributes.
   *
   * As the TComponentBuilder, this builder has an internal active component and attribute and all called methods act on
   * the corresponding active element.
   *
   * Every component and attribute is checked against a REXS database model to make sure the resulting components are
   * REXS standard compliant. The builder will throw exceptions on any issues.
   *
   * Load case builder should not be created manually but by using the TModelBuilder.
   */
  class TLoadCaseBuilder
  {
  public:
    /**
     * @brief Constructs a new TLoadCaseBuilder object
     *
     * @param databaseModel The REXS database model used to check and validate the added components and attributes for
     * standard compliance. The resulting model will have the same REXS version as the database model.
     */
    explicit TLoadCaseBuilder(const database::TModel& databaseModel)
    : m_Components{databaseModel}
    {
    }

    ~TLoadCaseBuilder() = default;

    TLoadCaseBuilder(const TLoadCaseBuilder&) = delete;
    TLoadCaseBuilder& operator=(const TLoadCaseBuilder&) = delete;
    TLoadCaseBuilder(TLoadCaseBuilder&&) noexcept = default;
    TLoadCaseBuilder& operator=(TLoadCaseBuilder&&) = delete;

    /**
     * @brief Adds an existing component to the builder and makes it the active component
     *
     * @param id The component id of an existing component. Has to be a component id added previously to the
     * TModelBuilder used to create this load case builder in order to ensure referencial integrity.
     * @return TLoadCaseBuilder& to the builder for chaining calls
     * @throws TException if the component has been already added to this load case
     */
    TLoadCaseBuilder& addComponent(TComponentId id) &;

    /**
     * @brief Adds an existing component to the builder and makes it the active component
     *
     * @param id The component id of an existing component. Has to be a component id added previously to the
     * TModelBuilder used to create this load case builder in order to ensure referencial integrity. This
     * method is not allowed if a component has been supplied a user defined integer id previously.
     * @return TLoadCaseBuilder& to the builder for chaining calls
     * @throws TException if the component has been already added to this load case
     */
    TLoadCaseBuilder& addComponent(std::string id) &;

    /**
     * @brief Adds a new load attribute to the builder and makes it the active attribute
     *
     * Adds the attribute to the active component.
     *
     * @param attributeId The attribute id to add, e.g. "operating_time". The attribute id has to be
     * allowed by the REXS database model.
     * @return TLoadCaseBuilder& to the builder for chaining calls
     * @throws TException if there is no active component
     * @throws TException if the attributeId is not allowed by the REXS database model
     */
    TLoadCaseBuilder& addAttribute(const std::string& attributeId) &;

    /**
     * @brief Adds a new custom load attribute to the builder makes it the active attribute
     *
     * Adds the custom attribute to the active component.
     *
     * @param attribute The custom attribute id. Should always start with the prefix *custom_*.
     * @param type Corresponds to one of the REXS datatypes
     * @return TLoadCaseBuilder& to the builder for chaining calls
     * @throws TException if there is no active component
     */
    TLoadCaseBuilder& addCustomAttribute(const std::string& attribute, TValueType type) &;

    /**
     * @brief Set the unit of the active attribute
     *
     * @param unit The unit name to set for the active attribute. Can be anything if the active attribute is a custom
     * attribute. Otherwise the unit has to match the active attributes unit fetched from the REXS model database.
     * @return TLoadCaseBuilder& to the builder for chaining calls
     * @throws TException if there is no active attribute
     * @throws TException if unit does not match the active attributes unit
     */
    TLoadCaseBuilder& unit(const std::string& unit) &;

    /**
     * @brief Set the value of the active attribute
     *
     * @tparam T The C++ type of the value to set. Has to match the attributes value type.
     * @param val The actual value to set
     * @return TLoadCaseBuilder& to the builder for chaining calls
     * @throws TException if there is no active attribute
     * @throws TException if value does not match the active attributes value type
     * @throws TException if the attributes value type is TValueType::REFERENCE_COMPONENT. Component references are
     * currently not allowed for load case components.
     */
    template<typename T>
    TLoadCaseBuilder& value(T val) &;

    /**
     * @brief Specifies that the active attributes value shall be stored in a coded form
     *
     * Does only have an effect on array or matrix types. Will be ignored for other types.
     * Will store the array or matrix value in base64 coded form. Can lead to truncation of values if
     * TCodeType::Optimized is chosen.
     *
     * @param type Specifies how exactly the value shall be coded. If in doubt always take the default.
     * @return TLoadCaseBuilder& to the builder for chaining calls
     * @throws TException if there is no active attribute
     */
    TLoadCaseBuilder& coded(TCodeType type = TCodeType::Default) &;

    /**
     * @brief Finalizes the load case builder and creates the REXS model TLoadCase
     *
     * @param components The TComponent collection build with the component builder owning the used component ids
     * @param componentBuilder The same component builder used to create the TComponent collection with. No other
     * methods shall be called on the component builder after the call to TComponentBuilder::build.
     * @return TLoadCase The TLoadCase created with the components and attributes defined for this builder
     * @throws TException if anything goes wrong creating the TLoadCase
     */
    TLoadCase build(const TComponents& components, const TComponentBuilder& componentBuilder) const;

  private:
    detail::TComponents m_Components;
  };


  /**
   * @brief Builder used to build REXS model TAccumulation instances for a TLoadSpectrum
   *
   * Creates accumulation components and their attributes.
   *
   * As the TComponentBuilder, this builder has an internal active component and attribute and all called methods act on
   * the corresponding active element.
   *
   * Every component and attribute is checked against a REXS database model to make sure the resulting components are
   * REXS standard compliant. The builder will throw exceptions on any issues.
   *
   * Accumulation builder should not be created manually but by using the TModelBuilder.
   */
  class TAccumulationBuilder
  {
  public:
    /**
     * @brief Constructs a new TAccumulationBuilder object
     *
     * @param databaseModel The REXS database model used to check and validate the added components and attributes for
     * standard compliance. The resulting model will have the same REXS version as the database model.
     */
    explicit TAccumulationBuilder(const database::TModel& databaseModel)
    : m_Components{databaseModel}
    {
    }

    ~TAccumulationBuilder() = default;

    TAccumulationBuilder(const TAccumulationBuilder&) = delete;
    TAccumulationBuilder& operator=(const TAccumulationBuilder&) = delete;
    TAccumulationBuilder(TAccumulationBuilder&&) noexcept = default;
    TAccumulationBuilder& operator=(TAccumulationBuilder&&) = delete;

    /**
     * @brief Adds an existing component to the builder and makes it the active component
     *
     * @param id The component id of an existing component. Has to be a component id added previously to the
     * TModelBuilder used to create this load case builder in order to ensure referencial integrity.
     * @return TAccumulationBuilder& to the builder for chaining calls
     * @throws TException if the component has been already added to this accumulation
     */
    TAccumulationBuilder& addComponent(TComponentId id) &;

    /**
     * @brief Adds an existing component to the builder and makes it the active component
     *
     * @param id The component id of an existing component. Has to be a component id added previously to the
     * TModelBuilder used to create this accumulation builder in order to ensure referencial integrity. This
     * method is not allowed if a component has been supplied a user defined integer id previously.
     * @return TAccumulationBuilder& to the builder for chaining calls
     * @throws TException if the component has been already added to this accumulation
     */
    TAccumulationBuilder& addComponent(std::string id) &;

    /**
     * @brief Adds a new accumulation attribute to the builder and makes it the active attribute
     *
     * Adds the attribute to the active component.
     *
     * @param attributeId The attribute id to add, e.g. "operating_time". The attribute id has to be
     * allowed by the REXS database model.
     * @return TAccumulationBuilder& to the builder for chaining calls
     * @throws TException if there is no active component
     * @throws TException if the attributeId is not allowed by the REXS database model
     */
    TAccumulationBuilder& addAttribute(const std::string& attributeId) &;

    /**
     * @brief Adds a new custom accumulation attribute to the builder makes it the active attribute
     *
     * Adds the custom attribute to the active component.
     *
     * @param attribute The custom attribute id. Should always start with the prefix *custom_*.
     * @param type Corresponds to one of the REXS datatypes
     * @return TAccumulationBuilder& to the builder for chaining calls
     * @throws TException if there is no active component
     */
    TAccumulationBuilder& addCustomAttribute(const std::string& attribute, TValueType type) &;

    /**
     * @brief Set the unit of the active attribute
     *
     * @param unit The unit name to set for the active attribute. Can be anything if the active attribute is a custom
     * attribute. Otherwise the unit has to match the active attributes unit fetched from the REXS model database.
     * @return TAccumulationBuilder& to the builder for chaining calls
     * @throws TException if there is no active attribute
     * @throws TException if unit does not match the active attributes unit
     */
    TAccumulationBuilder& unit(const std::string& unit) &;

    /**
     * @brief Set the value of the active attribute
     *
     * @tparam T The C++ type of the value to set. Has to match the attributes value type.
     * @param val The actual value to set
     * @return TAccumulationBuilder& to the builder for chaining calls
     * @throws TException if there is no active attribute
     * @throws TException if value does not match the active attributes value type
     * @throws TException if the attributes value type is TValueType::REFERENCE_COMPONENT. Component references are
     * currently not allowed for accumulation components.
     */
    template<typename T>
    TAccumulationBuilder& value(T val) &;

    /**
     * @brief Specifies that the active attributes value shall be stored in a coded form
     *
     * Does only have an effect on array or matrix types. Will be ignored for other types.
     * Will store the array or matrix value in base64 coded form. Can lead to truncation of values if
     * TCodeType::Optimized is chosen.
     *
     * @param type Specifies how exactly the value shall be coded. If in doubt always take the default.
     * @return TAccumulationBuilder& to the builder for chaining calls
     * @throws TException if there is no active attribute
     */
    TAccumulationBuilder& coded(TCodeType type = TCodeType::Default) &;

    /**
     * @brief Finalizes the load case builder and creates the REXS model TAccumulation
     *
     * @param components The TComponent collection build with the component builder owning the used component ids
     * @param componentBuilder The same component builder used to create the TComponent collection with. No other
     * methods shall be called on the component builder after the call to TComponentBuilder::build.
     * @return TAccumulation The TAccumulation created with the components and attributes defined for this builder
     * @throws TException if anything goes wrong creating the TAccumulation
     */
    TAccumulation build(const TComponents& components, const TComponentBuilder& componentBuilder) const;

  private:
    detail::TComponents m_Components;
  };


  /**
   * @brief Builder used to build REXS TModel instances.
   *
   * The model builder is the main class to build TModel instances with their dependencies with. It highly abstracts the
   * construction of REXS standard compliant models. It supports all constructs from the REXS standard.
   *
   * The model builder uses a TComponentBuilder internally and thus has the same internal active component and attribute
   * and all called methods act on the corresponding active element. Additionally, the model builder has a stack for
   * relations and references that works in the same way.
   *
   * Every construct is checked against a REXS database model to make sure the resulting model is
   * REXS standard compliant. The builder will throw exceptions on any issues.
   */
  class TModelBuilder
  {
  public:
    /**
     * @brief Constructs a new TModelBuilder object
     *
     * Creates an own TComponentBuilder instance internally.
     *
     * @param databaseModel The REXS database model used to check and validate the added relations, components, and
     * attributes for standard compliance. The resulting model will have the same REXS version as the database model.
     */
    explicit TModelBuilder(const database::TModel& databaseModel)
    : m_ComponentBuilder{databaseModel}
    , m_AccumulationBuilder{databaseModel}
    {
    }

    /**
     * @brief Constructs a new TModelBuilder object
     *
     * Uses an existing TComponentBuilder for creating components and attributes. All elements already created will be
     * incorporated into the model builder.
     *
     * @param componentBuilder The component builder to use. All already added elements will be incorporated into the
     * new model.
     */
    explicit TModelBuilder(TComponentBuilder componentBuilder)
    : m_ComponentBuilder{std::move(componentBuilder)}
    , m_AccumulationBuilder{m_ComponentBuilder.m_Components.databaseModel()}
    {
    }

    /**
     * @brief Adds a new relation and makes it the active relation
     *
     * @param type The relation type for the new relation
     * @return TModelBuilder& to the builder for chaining calls
     */
    TModelBuilder& addRelation(TRelationType type) &;

    /**
     * @brief Set the order on a relation
     *
     * @param order The order has to be >= 1
     * @return TModelBuilder& to the builder for chaining calls
     * @throws TException if there is no active relation
     * @throws TException if the order is not >= 1
     */
    TModelBuilder& order(uint32_t order) &;

    /**
     * @brief Adds a new reference to a relation and makes it the active reference
     *
     * The reference has to refer to a component that has already been added to the model builder.
     *
     * @param role The role of the component in the relation
     * @param id The component id of a component added to the model builder
     * @return TModelBuilder& to the builder for chaining calls
     * @throws TException if there is no active relation
     */
    TModelBuilder& addRef(TRelationRole role, TComponentId id) &;

    /**
     * @brief Adds a new reference to a relation and makes it the active reference
     *
     * The reference has to refer to a component that has been added to the model builder.
     *
     * @param role The role of the component in the relation
     * @param id The user supplied id of a component added to the model builder. This
     * method is not allowed if a component has been supplied a user defined integer id previously.
     * @return TModelBuilder& to the builder for chaining calls
     * @throws TException if there is no active relation
     */
    TModelBuilder& addRef(TRelationRole role, std::string id) &;

    /**
     * @brief Set a hint for the active reference
     *
     * @param hint The hint to set for the active reference. Can be an arbitrary string.
     * @return TModelBuilder& to the builder for chaining calls
     * @throws TException if there is no active relation
     */
    TModelBuilder& hint(std::string hint) &;

    /**
     * @brief Adds a new component to the builder and makes it the active component
     *
     * Internally, the builder will create a new unique component and assign it to the component. The id can be
     * retrieved by calling TModelBuilder::id.
     *
     * @param component The component type to add, e.g. "gear_unit". The type has to be allowed by the REXS database
     * model.
     * @param id Optional id to use for component. Will be used as the generated component id. The builder will make
     * sure the ids are unique. If you use your own ids, you are not allowed to mix them with string ids.
     * @param externalId Optional originating id
     * @return TModelBuilder& to the builder for chaining calls
     * @throws TException if the component type is not allowed by the REXS database model
     */
    TModelBuilder& addComponent(const std::string& component, std::optional<uint64_t> id = {},
                                std::optional<uint64_t> externalId = {}) &;

    /**
     * @brief Adds a new component to the builder and makes it the active component
     *
     * Uses a user supplied custom id as unique id. Will make sure that the id is unique for all components of this
     * builder. The id can be retrieved by calling TModelBuilder::id.
     *
     * @param component The component type to add, e.g. "gear_unit". The type has to be allowed by the REXS database
     * model.
     * @param id A user supplied custom id. Has to be unique for all components of this builder. This
     * method is not allowed if a component has been supplied a user defined integer id previously.
     * @param externalId Optional originating id
     * @return TModelBuilder& to the builder for chaining calls
     * @throws TException if the component type is not allowed by the REXS database model
     * @throws TException if the user supplied id has already been use for another component
     */
    TModelBuilder& addComponent(const std::string& component, std::string id,
                                std::optional<uint64_t> externalId = {}) &;

    /**
     * @brief Retrieves the component id of the active component
     *
     * @return TComponentId of the active component
     * @throws TException if there is no active component
     */
    [[nodiscard]] TComponentId id() const;

    /**
     * @brief Sets a name for the active component
     *
     * @param name The name to set for the active component. Can be an arbitrary string.
     * @return TModelBuilder& to the builder for chaining calls
     * @throws TException if there is no active component
     */
    TModelBuilder& name(std::string name) &;

    /**
     * @brief Adds a new attribute to the builder and makes it the active attribute
     *
     * Adds the attribute to the active component.
     *
     * @param attributeId The attribute id to add, e.g. "reference_component_for_position". The attribute id has to be
     * allowed by the REXS database model.
     * @return TModelBuilder& to the builder for chaining calls
     * @throws TException if there is no active component
     * @throws TException if the attributeId is not allowed by the REXS database model
     */
    TModelBuilder& addAttribute(const std::string& attributeId) &;

    /**
     * @brief Adds a new custom attribute to the builder makes it the active attribute
     *
     * Adds the custom attribute to the active component.
     *
     * @param attribute The custom attribute id. Should always start with the prefix *custom_*.
     * @param type Corresponds to one of the REXS datatypes
     * @return TModelBuilder& to the builder for chaining calls
     * @throws TException if there is no active component
     */
    TModelBuilder& addCustomAttribute(const std::string& attribute, TValueType type) &;

    /**
     * @brief Set a reference to a component on the active attribute
     *
     * Needs an active attribute with a value of type TValueType::REFERENCE_COMPONENT.
     *
     * @param id The component id of the referenced component. The component id has to be one of the already added
     * components to this builder. The active component id can be retrieved by calling TModelBuilder::id. Actually,
     * the id is allowed to reference the currently active component.
     * @return TModelBuilder& to the builder for chaining calls
     * @throws TException if there is no active attribute
     * @throws TException if the attributes value type is not TValueType::REFERENCE_COMPONENT
     */
    TModelBuilder& reference(const TComponentId& id) &;

    /**
     * @brief Set a reference to a component on the active attribute
     *
     * Needs an active attribute with a value of type TValueType::REFERENCE_COMPONENT.
     *
     * @param id The custom user supplied id of the referenced component. The component id has to be one of the already
     * added components to this builder. Actually, the id is allowed to reference the currently active component. This
     * method is not allowed if a component has been supplied a user defined integer id previously.
     * @return TModelBuilder& to the builder for chaining calls
     * @throws TException if there is no active attribute
     * @throws TException if the attributes value type is not TValueType::REFERENCE_COMPONENT
     */
    TModelBuilder& reference(std::string id) &;

    /**
     * @brief Set the unit of the active attribute
     *
     * @param unit The unit name to set for the active attribute. Can be anything if the active attribute is a custom
     * attribute. Otherwise the unit has to match the active attributes unit fetched from the REXS model database.
     * @return TModelBuilder& to the builder for chaining calls
     * @throws TException if there is no active attribute
     * @throws TException if unit does not match the active attributes unit
     */
    TModelBuilder& unit(const std::string& unit) &;

    /**
     * @brief Set the value of the active attribute
     *
     * @tparam T The C++ type of the value to set. Has to match the attributes value type.
     * @param val The actual value to set
     * @return TModelBuilder& to the builder for chaining calls
     * @throws TException if there is no active attribute
     * @throws TException if value does not match the active attributes value type
     * @throws TException if the attributes value type is TValueType::REFERENCE_COMPONENT. Use
     * TModelBuilder::reference instead.
     */
    template<typename T>
    TModelBuilder& value(T val) &;

    /**
     * @brief Specifies that the active attributes value shall be stored in a coded form
     *
     * Does only have an effect on array or matrix types. Will be ignored for other types.
     * Will store the array or matrix value in base64 coded form. Can lead to truncation of values if
     * TCodeType::Optimized is chosen.
     *
     * @param type Specifies how exactly the value shall be coded. If in doubt always take the default.
     * @return TModelBuilder& to the builder for chaining calls
     * @throws TException if there is no active attribute
     */
    TModelBuilder& coded(TCodeType type = TCodeType::Default) &;

    /**
     * @brief Retrieves a load case builder
     *
     * The load case builder will build exactly one load case. In order to create multipe load cases, multiple load case
     * builder are necessary.
     *
     * @attention Don't copy the load case builder, as it will loose its reference
     * @return TLoadCaseBuilder& to a load case builder instance
     */
    [[nodiscard]] TLoadCaseBuilder& addLoadCase() &;

    /**
     * @brief Retrieves the accumulation builder
     *
     * There is exactly one accumulation builder per model.
     *
     * @attention Don't copy the accumulation builder, as it will loose its reference
     * @return TAccumulationBuilder& to the accumulation builder instance
     */
    [[nodiscard]] TAccumulationBuilder& addAccumulation() &;

    /**
     * @brief Finalizes the model builder and creates the REXS TModel instance
     *
     * Called as final step to create the REXS TModel instance. Can be called multiple times and generates
     * the same TModel as long as no other mehods have been called in between on this builder.
     *
     * @param info The meta data for this model
     * @return TModel of all added REXS model components, attributes, load cases, and accumulation
     * @throws TException if anything goes wrong while building the model
     */
    [[nodiscard]] TModel build(TModelInfo info);

    /**
     * @brief Finalizes the model builder and creates the REXS TModel instance
     *
     * Called as final step to create the REXS TModel instance. Can be called multiple times and generates
     * the same TModel as long as no other mehods have been called in between on this builder.
     *
     * @param applicationId The name of the application creating the model
     * @param applicationVersion The version if the application creating the model
     * @param applicationLanguage The optional language used by the application
     * @return TModel of all added REXS model components, attributes, load cases, and accumulation
     * @throws TException if anything goes wrong while building the model
     */
    [[nodiscard]] TModel build(std::string applicationId, std::string applicationVersion,
                               std::optional<std::string> applicationLanguage);

  private:
    void checkRelation() const
    {
      if (m_Relations.empty()) {
        throw TException{"no relations added yet"};
      }
    }

    void checkReference() const
    {
      checkRelation();
      if (m_Relations.back().m_References.empty()) {
        throw TException{"no references added yet"};
      }
    }

    struct ReferenceEntry {
      TRelationRole m_Role;
      TComponentId m_Id;
      std::string m_Hint{};
    };

    struct RelationEntry {
      TRelationType m_Type;
      std::optional<uint32_t> m_Order{};
      std::vector<ReferenceEntry> m_References{};
    };

    TComponentBuilder m_ComponentBuilder;
    std::vector<RelationEntry> m_Relations;
    std::vector<TLoadCaseBuilder> m_LoadCases;
    TAccumulationBuilder m_AccumulationBuilder;
  };


  /////////////////////////////////////////////////////////////////////////////
  // Implementation
  /////////////////////////////////////////////////////////////////////////////

  inline std::string TComponentId::asString() const
  {
    return std::visit(detail::overload{[](const uint64_t& n) -> std::string {
                                         return std::to_string(n);
                                       },
                                       [](const std::string& s) {
                                         return s;
                                       }},
                      m_Id);
  }


  inline ::rexsapi::TAttribute
  detail::TAttributeEntry::createAttribute(const std::unordered_map<TComponentId, uint64_t>& componentMapping) const
  {
    if (m_Value.isEmpty() && !m_Reference.has_value()) {
      throw TException{fmt::format("attribute id={} has an empty value",
                                   m_Attribute != nullptr ? m_Attribute->getAttributeId() : m_AttributeId)};
    }

    TValue val = m_Value;
    val.coded(m_CodeType);

    if (m_Attribute != nullptr) {
      if (m_Unit.has_value() && *m_Unit != m_Attribute->getUnit()) {
        throw TException{
          fmt::format("attribute id={} has wrong unit {}", m_Attribute->getAttributeId(), m_Unit->getName())};
      }

      if (m_Reference.has_value()) {
        auto id = static_cast<TReferenceComponentType>(componentMapping.at(*m_Reference));
        return ::rexsapi::TAttribute{*m_Attribute, TValue{id}};
      }

      return ::rexsapi::TAttribute{*m_Attribute, std::move(val)};
    }

    TUnit unit;
    if (m_Unit.has_value()) {
      unit = *m_Unit;
    }

    if (m_Reference.has_value()) {
      auto id = static_cast<TReferenceComponentType>(componentMapping.at(*m_Reference));
      return ::rexsapi::TAttribute{m_AttributeId, unit, *m_ValueType, TValue{id}};
    }

    return ::rexsapi::TAttribute{m_AttributeId, unit, *m_ValueType, std::move(val)};
  }

  inline ::rexsapi::TAttribute detail::TAttributeEntry::createAttribute() const
  {
    if (m_Value.isEmpty()) {
      throw TException{fmt::format("attribute id={} has an empty value",
                                   m_Attribute != nullptr ? m_Attribute->getAttributeId() : m_AttributeId)};
    }

    TValue val = m_Value;
    val.coded(m_CodeType);

    if (m_Attribute != nullptr) {
      if (m_Unit.has_value() && *m_Unit != m_Attribute->getUnit()) {
        throw TException{
          fmt::format("attribute id={} has wrong unit {}", m_Attribute->getAttributeId(), m_Unit->getName())};
      }
      return ::rexsapi::TAttribute{*m_Attribute, std::move(val)};
    }

    TUnit unit;
    if (m_Unit.has_value()) {
      unit = *m_Unit;
    }

    return ::rexsapi::TAttribute{m_AttributeId, unit, *m_ValueType, std::move(val)};
  }


  inline void detail::TComponents::addComponent(TComponentId id, std::optional<uint64_t> externalId)
  {
    checkDuplicateComponent(id);
    m_Components.emplace_back(detail::TComponentEntry{TComponentId{std::move(id)}, externalId});
  }

  inline void detail::TComponents::addComponent(const std::string& component, std::optional<uint64_t> id,
                                                std::optional<uint64_t> externalId)
  {
    checkDuplicateComponent(component);
    if (id) {
      m_Components.emplace_back(
        detail::TComponentEntry{TComponentId{id.value()}, externalId, &m_DatabaseModel.findComponentById(component)});
    } else {
      m_Components.emplace_back(
        detail::TComponentEntry{getNextComponentId(), externalId, &m_DatabaseModel.findComponentById(component)});
    }
  }

  inline void detail::TComponents::addComponent(const std::string& component, std::string id,
                                                std::optional<uint64_t> externalId)
  {
    checkDuplicateComponent(id);
    m_Components.emplace_back(
      detail::TComponentEntry{TComponentId{std::move(id)}, externalId, &m_DatabaseModel.findComponentById(component)});
  }

  inline TComponentId detail::TComponents::getNextComponentId() noexcept
  {
    return TComponentId{++m_ComponentId};
  }

  inline void detail::TComponents::checkDuplicateComponent(const TComponentId& component) const
  {
    const auto it = std::find_if(m_Components.begin(), m_Components.end(), [&component](const auto& comp) {
      return comp.m_Id == component;
    });
    if (it != m_Components.end()) {
      throw TException{fmt::format("component id={} already added", component.asString())};
    }
  }

  inline void detail::TComponents::checkDuplicateComponent(const std::string& id) const
  {
    const auto it = std::find_if(m_Components.begin(), m_Components.end(), [&id](const auto& comp) {
      return comp.m_Id == TComponentId{id};
    });
    if (it != m_Components.end()) {
      throw TException{fmt::format("component id={} already added", id)};
    }
  }

  inline void detail::TComponents::checkDuplicateAttribute(const detail::TComponentEntry& component,
                                                           const std::string& attribute)
  {
    const auto it =
      std::find_if(component.m_Attributes.begin(), component.m_Attributes.end(), [&attribute](const auto& att) {
        return att == attribute;
      });
    if (it != component.m_Attributes.end()) {
      throw TException{
        fmt::format("attribute id={} already added to component id={}", attribute, component.m_Id.asString())};
    }
  }

  inline void detail::TComponents::checkComponent() const
  {
    if (m_Components.empty()) {
      throw TException{"no components added yet"};
    }
  }

  inline void detail::TComponents::checkAttribute() const
  {
    checkComponent();
    if (m_Components.back().m_Attributes.empty()) {
      throw TException{"no attributes added yet"};
    }
  }

  inline void detail::TComponents::unit(const std::string& unit)
  {
    if (unit.empty()) {
      return;
    }
    auto& attribute = lastAttribute();
    if (attribute.m_Attribute != nullptr) {
      if (TUnit{attribute.m_Attribute->getUnit()} != TUnit{unit}) {
        throw TException{
          fmt::format("unit {} does not match attribute id={} unit", unit, attribute.m_Attribute->getAttributeId())};
      }
      attribute.m_Unit = TUnit{attribute.m_Attribute->getUnit()};
    } else {
      attribute.m_Unit = TUnit{unit};
    }
  }


  inline TComponentBuilder& TComponentBuilder::addComponent(const std::string& component, std::optional<uint64_t> id,
                                                            std::optional<uint64_t> externalId) &
  {
    if (id) {
      m_Components.addComponent(component, id, externalId);
    } else {
      m_Components.addComponent(component, externalId);
    }
    return *this;
  }

  inline TComponentBuilder& TComponentBuilder::addComponent(const std::string& component, std::string id,
                                                            std::optional<uint64_t> externalId) &
  {
    m_Components.addComponent(component, id, externalId);
    return *this;
  }

  inline TComponentBuilder& TComponentBuilder::name(std::string name) &
  {
    m_Components.name(std::move(name));
    return *this;
  }

  inline TComponentBuilder& TComponentBuilder::addAttribute(const std::string& attribute) &
  {
    m_Components.addAttribute(attribute);
    return *this;
  }

  inline TComponentBuilder& TComponentBuilder::addCustomAttribute(const std::string& attribute, TValueType type) &
  {
    m_Components.addCustomAttribute(attribute, type);
    return *this;
  }

  inline TComponentBuilder& TComponentBuilder::reference(const TComponentId& id) &
  {
    m_Components.reference(id);
    return *this;
  }

  inline TComponentBuilder& TComponentBuilder::reference(std::string id) &
  {
    m_Components.reference(std::move(id));
    return *this;
  }

  inline TComponentBuilder& TComponentBuilder::unit(const std::string& unit) &
  {
    m_Components.unit(unit);
    return *this;
  }

  template<typename T>
  inline TComponentBuilder& TComponentBuilder::value(T val) &
  {
    m_Components.value(TValue{std::move(val)});
    return *this;
  }

  inline TComponentBuilder& TComponentBuilder::coded(TCodeType type) &
  {
    m_Components.coded(type);
    return *this;
  }

  inline TComponentId TComponentBuilder::id() const
  {
    return m_Components.id();
  }

  inline TComponents TComponentBuilder::build()
  {
    m_ComponentMapping.clear();
    uint64_t internalComponentId{0};
    TComponents components;

    bool assignIds{false};
    for (const auto& component : m_Components.components()) {
      if (!component.m_Id.isInteger()) {
        assignIds = true;
      }
    }

    for (const auto& component : m_Components.components()) {
      if (assignIds) {
        m_ComponentMapping[component.m_Id] = ++internalComponentId;
      } else {
        m_ComponentMapping[component.m_Id] = component.m_Id.integer();
      }
    }

    for (const auto& component : m_Components.components()) {
      TAttributes attributes;
      for (const auto& attribute : component.m_Attributes) {
        if (!attribute.isCustom() &&
            !component.m_DatabaseComponent->hasAttribute(attribute.m_Attribute->getAttributeId())) {
          throw TException{fmt::format("attribute id={} is not part of component {} id={}",
                                       attribute.m_Attribute->getAttributeId(),
                                       component.m_DatabaseComponent->getComponentId(), component.m_Id.asString())};
        }
        attributes.emplace_back(attribute.createAttribute(m_ComponentMapping));
      }
      if (component.m_ExternalId) {
        components.emplace_back(TComponent{component.m_ExternalId.value(), m_ComponentMapping[component.m_Id],
                                           *component.m_DatabaseComponent, component.m_Name, std::move(attributes)});
      } else {
        components.emplace_back(TComponent{m_ComponentMapping[component.m_Id], *component.m_DatabaseComponent,
                                           component.m_Name, std::move(attributes)});
      }
    }

    return components;
  }

  inline const TComponent& TComponentBuilder::getComponentForId(const TComponents& components,
                                                                const TComponentId& id) const&
  {
    const auto cid = getComponentForId(id);
    auto it = std::find_if(components.begin(), components.end(), [cid](const auto& component) {
      return component.getInternalId() == cid;
    });
    if (it == components.end()) {
      throw TException{fmt::format("no component found for id={}", id.asString())};
    }

    return *it;
  }

  inline uint64_t TComponentBuilder::getComponentForId(const TComponentId& id) const
  {
    auto it = m_ComponentMapping.find(id);
    if (it == m_ComponentMapping.end()) {
      return 0;
    }

    return it->second;
  }


  inline TLoadCaseBuilder& TLoadCaseBuilder::addComponent(TComponentId id) &
  {
    m_Components.addComponent(id);
    return *this;
  }

  inline TLoadCaseBuilder& TLoadCaseBuilder::addComponent(std::string id) &
  {
    m_Components.addComponent(TComponentId{id});
    return *this;
  }

  inline TLoadCaseBuilder& TLoadCaseBuilder::addAttribute(const std::string& attribute) &
  {
    m_Components.addAttribute(attribute);
    return *this;
  }

  inline TLoadCaseBuilder& TLoadCaseBuilder::addCustomAttribute(const std::string& attribute, TValueType type) &
  {
    m_Components.addCustomAttribute(attribute, type);
    return *this;
  }

  inline TLoadCaseBuilder& TLoadCaseBuilder::unit(const std::string& unit) &
  {
    m_Components.unit(unit);
    return *this;
  }

  template<typename T>
  inline TLoadCaseBuilder& TLoadCaseBuilder::value(T val) &
  {
    m_Components.value(TValue{std::move(val)});
    return *this;
  }

  inline TLoadCaseBuilder& TLoadCaseBuilder::coded(TCodeType type) &
  {
    m_Components.coded(type);
    return *this;
  }

  namespace detail
  {
    template<typename C>
    static inline C buildComponents(const detail::TComponents& builderComponents,
                                    const rexsapi::TComponents& components,
                                    const rexsapi::TComponentBuilder& componentBuilder)
    {
      TLoadComponents loadComponents;
      for (const auto& component : builderComponents.components()) {
        const auto& referencedComponent = componentBuilder.getComponentForId(components, component.m_Id);
        const auto& databaseComponent =
          builderComponents.databaseModel().findComponentById(referencedComponent.getType());
        rexsapi::TAttributes loadAttributes;

        for (const auto& attribute : component.m_Attributes) {
          if (!attribute.isCustom() && !databaseComponent.hasAttribute(attribute.m_Attribute->getAttributeId())) {
            throw TException{fmt::format("attribute id={} is not part of component {} id={}",
                                         attribute.m_Attribute->getAttributeId(), referencedComponent.getType(),
                                         component.m_Id.asString())};
          }

          loadAttributes.emplace_back(attribute.createAttribute());
        }
        loadComponents.emplace_back(TLoadComponent{referencedComponent, std::move(loadAttributes)});
      }

      return C{std::move(loadComponents)};
    }
  }

  inline TLoadCase TLoadCaseBuilder::build(const TComponents& components,
                                           const TComponentBuilder& componentBuilder) const
  {
    return detail::buildComponents<TLoadCase>(m_Components, components, componentBuilder);
  }


  inline TAccumulationBuilder& TAccumulationBuilder::addComponent(TComponentId id) &
  {
    m_Components.addComponent(id);
    return *this;
  }

  inline TAccumulationBuilder& TAccumulationBuilder::addComponent(std::string id) &
  {
    m_Components.addComponent(TComponentId{id});
    return *this;
  }

  inline TAccumulationBuilder& TAccumulationBuilder::addAttribute(const std::string& attribute) &
  {
    m_Components.addAttribute(attribute);
    return *this;
  }

  inline TAccumulationBuilder& TAccumulationBuilder::addCustomAttribute(const std::string& attribute, TValueType type) &
  {
    m_Components.addCustomAttribute(attribute, type);
    return *this;
  }

  inline TAccumulationBuilder& TAccumulationBuilder::unit(const std::string& unit) &
  {
    m_Components.unit(unit);
    return *this;
  }

  template<typename T>
  inline TAccumulationBuilder& TAccumulationBuilder::value(T val) &
  {
    m_Components.value(TValue{std::move(val)});
    return *this;
  }

  inline TAccumulationBuilder& TAccumulationBuilder::coded(TCodeType type) &
  {
    m_Components.coded(type);
    return *this;
  }

  inline TAccumulation TAccumulationBuilder::build(const TComponents& components,
                                                   const TComponentBuilder& componentBuilder) const
  {
    return detail::buildComponents<TAccumulation>(m_Components, components, componentBuilder);
  }


  inline TModelBuilder& TModelBuilder::addRelation(TRelationType type) &
  {
    m_Relations.emplace_back(RelationEntry{type});
    return *this;
  }

  inline TModelBuilder& TModelBuilder::order(uint32_t order) &
  {
    checkRelation();
    if (order == 0) {
      throw TException{"relation order has to be >= 1"};
    }
    m_Relations.back().m_Order = order;
    return *this;
  }

  inline TModelBuilder& TModelBuilder::addRef(TRelationRole role, TComponentId id) &
  {
    checkRelation();
    m_Relations.back().m_References.emplace_back(ReferenceEntry{role, std::move(id)});
    return *this;
  }

  inline TModelBuilder& TModelBuilder::addRef(TRelationRole role, std::string id) &
  {
    checkRelation();
    m_Relations.back().m_References.emplace_back(ReferenceEntry{role, TComponentId{std::move(id)}});
    return *this;
  }

  inline TModelBuilder& TModelBuilder::hint(std::string hint) &
  {
    checkReference();
    m_Relations.back().m_References.back().m_Hint = std::move(hint);
    return *this;
  }

  inline TModelBuilder& TModelBuilder::addComponent(const std::string& component, std::optional<uint64_t> id,
                                                    std::optional<uint64_t> externalId) &
  {
    m_ComponentBuilder.addComponent(component, id, externalId);
    return *this;
  }

  inline TModelBuilder& TModelBuilder::addComponent(const std::string& component, std::string id,
                                                    std::optional<uint64_t> externalId) &
  {
    m_ComponentBuilder.addComponent(component, std::move(id), externalId);
    return *this;
  }

  inline TModelBuilder& TModelBuilder::name(std::string name) &
  {
    m_ComponentBuilder.name(std::move(name));
    return *this;
  }

  inline TModelBuilder& TModelBuilder::addAttribute(const std::string& attribute) &
  {
    m_ComponentBuilder.addAttribute(attribute);
    return *this;
  }

  inline TModelBuilder& TModelBuilder::addCustomAttribute(const std::string& attribute, TValueType type) &
  {
    m_ComponentBuilder.addCustomAttribute(attribute, type);
    return *this;
  }

  inline TModelBuilder& TModelBuilder::reference(const TComponentId& id) &
  {
    m_ComponentBuilder.reference(id);
    return *this;
  }

  inline TModelBuilder& TModelBuilder::reference(std::string id) &
  {
    m_ComponentBuilder.reference(std::move(id));
    return *this;
  }

  inline TModelBuilder& TModelBuilder::unit(const std::string& unit) &
  {
    m_ComponentBuilder.unit(unit);
    return *this;
  }

  template<typename T>
  inline TModelBuilder& TModelBuilder::value(T val) &
  {
    m_ComponentBuilder.value(std::move(val));
    return *this;
  }

  inline TModelBuilder& TModelBuilder::coded(TCodeType type) &
  {
    m_ComponentBuilder.coded(type);
    return *this;
  }

  inline TLoadCaseBuilder& TModelBuilder::addLoadCase() &
  {
    m_LoadCases.emplace_back(TLoadCaseBuilder{m_ComponentBuilder.m_Components.databaseModel()});
    return m_LoadCases.back();
  }

  inline TAccumulationBuilder& TModelBuilder::addAccumulation() &
  {
    return m_AccumulationBuilder;
  }

  inline TComponentId TModelBuilder::id() const
  {
    return m_ComponentBuilder.id();
  }

  inline TModel TModelBuilder::build(rexsapi::TModelInfo info)
  {
    TRelations relations;
    auto components = m_ComponentBuilder.build();

    if (components.empty()) {
      throw TException{"no components specified for model"};
    }

    std::set<uint64_t> usedComponents;
    for (const auto& relation : m_Relations) {
      TRelationReferences references;
      for (const auto& reference : relation.m_References) {
        const auto& component = m_ComponentBuilder.getComponentForId(components, reference.m_Id);
        usedComponents.emplace(component.getInternalId());
        references.emplace_back(TRelationReference{reference.m_Role, reference.m_Hint, component});
      }
      if (references.empty()) {
        throw TException{"no references specified for relation"};
      }
      relations.emplace_back(rexsapi::TRelation{relation.m_Type, relation.m_Order, std::move(references)});
    }

    if (!relations.empty() && usedComponents.size() != components.size()) {
      throw TException{
        fmt::format("{} components are not used in a relation", components.size() - usedComponents.size())};
    }

    TLoadCases loadCases;
    for (const auto& loadCase : m_LoadCases) {
      auto tmpCase = loadCase.build(components, m_ComponentBuilder);
      if (!tmpCase.getLoadComponents().empty()) {
        loadCases.emplace_back(std::move(tmpCase));
      }
    }
    std::optional<TAccumulation> accumulation;
    {
      auto accu = m_AccumulationBuilder.build(components, m_ComponentBuilder);
      if (!accu.getLoadComponents().empty()) {
        accumulation = std::move(accu);
      }
    }

    TLoadSpectrum spectrum{std::move(loadCases), std::move(accumulation)};

    TModel model{std::move(info), std::move(components), std::move(relations), std::move(spectrum)};
    TRelationTypeChecker checker{TMode::STRICT_MODE};
    TResult result;
    if (!checker.check(result, model)) {
      std::stringstream stream;
      stream << "relation type check failed:\n";
      for (const auto& issue : result.getErrors()) {
        stream << "\t" << issue.getMessage() << "\n";
      }
      throw TException{stream.str()};
    }
    return model;
  }

  inline TModel TModelBuilder::build(std::string applicationId, std::string applicationVersion,
                                     std::optional<std::string> language)
  {
    const rexsapi::TModelInfo info{std::move(applicationId), std::move(applicationVersion),
                                   getTimeStringISO8601(std::chrono::system_clock::now()),
                                   m_ComponentBuilder.m_Components.databaseModel().getVersion(), std::move(language)};
    return build(std::move(info));
  }
}

#endif
