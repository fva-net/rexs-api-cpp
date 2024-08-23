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

#ifndef REXSAPI_TYPES_HXX
#define REXSAPI_TYPES_HXX

#include <rexsapi/ConversionHelper.hxx>
#include <rexsapi/Exception.hxx>
#include <rexsapi/Format.hxx>

#if defined(_MSVC_LANG)
#if _MSVC_LANG >= 202002L
#define REXS_HAS_CHRONO_DATE
#endif
#endif

#if defined(__GNUC__) && __cplusplus >= 202002L
#if __GNUC__ > 14 || (__GNUC_ == 14 && __GNUC_MINOR__ > 1)
#define REXS_HAS_CHRONO_DATE
#endif
#endif

#if defined(__clang__)
#undef REXS_HAS_CHRONO_DATE
#endif

#if defined(__APPLE__)
#undef REXS_HAS_CHRONO_DATE
#endif


#if defined(REXS_HAS_CHRONO_DATE)
#include <chrono>
namespace rexs_date = std::chrono;
#else
#include <date/date.h>
namespace rexs_date = date;
#endif
#include <sstream>
#include <vector>

namespace rexsapi
{
  /**
   * @brief Represents all allowed REXS value types.
   *
   */
  enum class TValueType : uint8_t {
    FLOATING_POINT,           //!< floating_point
    BOOLEAN,                  //!< boolean
    INTEGER,                  //!< integer
    ENUM,                     //!< enum
    STRING,                   //!< string
    FILE_REFERENCE,           //!< file_reference
    FLOATING_POINT_ARRAY,     //!< floating_point_array
    BOOLEAN_ARRAY,            //!< boolean_array
    INTEGER_ARRAY,            //!< integer_array
    STRING_ARRAY,             //!< string_array
    ENUM_ARRAY,               //!< enum_array
    REFERENCE_COMPONENT,      //!< reference_component
    FLOATING_POINT_MATRIX,    //!< floating_point_matrix
    INTEGER_MATRIX,           //!< integer_matrix
    BOOLEAN_MATRIX,           //!< boolean_matrix
    STRING_MATRIX,            //!< string_matrix
    ARRAY_OF_INTEGER_ARRAYS,  //!< array_of_integer_arrays
    DATE_TIME                 //!< date_time
  };

  /**
   * @brief Creates a value type from a string.
   *
   * @param type The string representation of a value type
   * @return TValueType for the corresponding string
   * @throws TException if an invalid type was specified
   */
  static TValueType typeFromString(const std::string& type);

  /**
   * @brief Returns a string representation of a value type.
   *
   * @param type The type to represent as string
   * @return std::string representation of the value type
   */
  static std::string toTypeString(TValueType type);


  /**
   * @brief Represents a bool type that can be used as a bool in a std::vector.
   *
   */
  struct Bool {
    Bool() = default;

    /// deliberately not marked as "explicit"
    Bool(bool value)
    : m_Value{value}
    {
    }

    /**
     * @brief Returns the bool value of the underlying bool.
     *
     * @return true
     * @return false
     */
    bool operator*() const
    {
      return m_Value;
    }

    explicit operator bool() const
    {
      return m_Value;
    }

    friend bool operator==(const Bool& lhs, const Bool& rhs)
    {
      return lhs.m_Value == rhs.m_Value;
    }

    friend bool operator!=(const Bool& lhs, const Bool& rhs)
    {
      return lhs.m_Value != rhs.m_Value;
    }

    bool m_Value{false};
  };


  /**
   * @brief Represents the REXS matrix type.
   *
   * @tparam T The C++ type for this matrix. Currently, integer, boolean, floating point and string are allowed.
   */
  template<typename T>
  struct TMatrix {
    using value_type = T;

    TMatrix() = default;

    explicit TMatrix(std::vector<std::vector<T>> v)
    : m_Values{std::move(v)}
    {
    }

    /**
     * @brief Converts a matrix to a different matrix type.
     *
     * @param m The matrix to convert. Has to have an underlying type that can be converted
     *          to this matrix type.
     */
    template<typename S>
    TMatrix(const TMatrix<S>& m)  /// deliberately not marked as "explicit"
    {
      for (const auto& row : m.m_Values) {
        std::vector<T> tmp;
        tmp.resize(row.size());
        std::transform(row.begin(), row.end(), tmp.begin(), [](auto x) {
          return static_cast<T>(x);
        });
        m_Values.emplace_back(std::move(tmp));
      }
    }

    TMatrix(const TMatrix<T>& m) = default;
    TMatrix(TMatrix<T>&& m) noexcept = default;
    TMatrix& operator=(const TMatrix<T>& m) = default;
    TMatrix& operator=(TMatrix<T>&& m) noexcept = default;

    ~TMatrix() = default;

    /**
     * @brief Checks if a martrix is valid.
     *
     * A valid matrix has the same column count for every row.
     *
     * @return true the matrix has the same column count for every row
     * @return false the matrix is invalid
     */
    bool validate() const noexcept
    {
      if (m_Values.size()) {
        size_t n = m_Values[0].size();
        for (const auto& row : m_Values) {
          if (row.size() != n) {
            return false;
          }
        }
      }
      return true;
    }

    friend bool operator==(const TMatrix<T>& lhs, const TMatrix<T>& rhs) noexcept
    {
      return lhs.m_Values == rhs.m_Values;
    }

    /**
     * @brief The store for the actual matrix values.
     *
     * Currently, the only way to access the matrix values.
     *
     */
    std::vector<std::vector<T>> m_Values;
  };


  /**
   * @brief Represents the REXS date_time type.
   *
   */
  class TDatetime
  {
  public:
    using time_point = std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>;

    /**
     * @brief Construct a new TDatetime object from a string.
     *
     * @param datetime The string has to be in ISO8601 format `yyyy-mm-ddThh:mm:ss[+/-]<offset to UTC>`. 
                       The offset requires a : between hours and minutes.
     * @throws std::exception if the string cannot be parsed or the date time is invalid
     */
    explicit TDatetime(const std::string& datetime)
    {
      std::istringstream in{datetime};
      in >> rexs_date::parse("%FT%T%Ez", m_Timepoint);

      if (!in.good()) {
        throw std::runtime_error{"illegal date specified: " + datetime};
      }
    }

    /**
     * @brief Construct a new TDatetime object from a std::chrono::time_point.
     *
     * @param datetime A time point
     */
    explicit TDatetime(time_point datetime) noexcept
    : m_Timepoint{datetime}
    {
    }

    friend bool operator==(const TDatetime& lhs, const TDatetime& rhs) noexcept
    {
      return lhs.m_Timepoint == rhs.m_Timepoint;
    }

    /**
     * @brief Returns a new TDatetime object constructed with the current date and time.
     *
     * @return A new TDatetime object set to the current date and time
     */
    static TDatetime now() noexcept
    {
      return TDatetime{std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now())};
    }

    /**
     * @brief Returns the UTC string representation in ISO8601 format `yyyy-mm-ddThh:mm:ss[+/-]<offset to UTC>`.
     *        The offset will always be +00:00.
     *
     * @return The UTC string representation
     */
    inline std::string asUTCString() const
    {
  #if defined(REXS_HAS_CHRONO_DATE)
      return std::format("{:%FT%T%Ez}", m_Timepoint);
  #else
      return date::format("%FT%T%Ez", m_Timepoint);
  #endif
    }

    /**
     * @brief Returns the locale string representation in ISO8601 format `yyyy-mm-ddThh:mm:ss[+/-]<offset to UTC>`.
     *        The offset will be set to the current timezone offset from UTC.
     *
     * @return The locale string representation
     */
    inline std::string asLocaleString() const
    {
      return getTimeStringISO8601(m_Timepoint);
    }

    /**
     * @brief Returns the time point.
     *
     * @return time_point
     */
    inline time_point asTimepoint() const noexcept
    {
      return m_Timepoint;
    }

  private:
    time_point m_Timepoint;
  };


  /**
   * @brief Represents all currently allowed REXS relation types.
   *
   */
  enum class TRelationType {
    ASSEMBLY,              //!< assembly
    CENTRAL_SHAFT,         //!< shaft
    CONNECTION,            //!< connection
    CONTACT,               //!< contact
    COUPLING,              //!< coupling
    FLANK,                 //!< flank
    MANUFACTURING_STEP,    //!< manufacturing_step
    ORDERED_ASSEMBLY,      //!< ordered_assembly
    ORDERED_REFERENCE,     //!< ordered_reference
    PLANET_CARRIER_SHAFT,  //!< plant_carrier_shaft
    PLANET_PIN,            //!< planet_pin
    PLANET_SHAFT,          //!< planet_shaft
    REFERENCE,             //!< reference
    SIDE,                  //!< side
    STAGE,                 //!< stage
    STAGE_GEAR_DATA        //!< stage_gear_data
  };

  /**
   * @brief Creates a relation type from a string.
   *
   * @param type The string representation of a relation type
   * @return TRelationType for the corresponding string
   * @throws TException if an invalid type was specified
   */
  static TRelationType relationTypeFromString(const std::string& type);

  /**
   * @brief Returns a string representation of a relation type.
   *
   * @param type The type to represent as string
   * @return std::string representation of the relation type
   */
  static std::string toRelationTypeString(TRelationType type);


  /**
   * @brief Represents all currently allowed REXS relation roles.
   *
   */
  enum class TRelationRole {
    ASSEMBLY,                //!< assembly
    GEAR,                    //!< gear
    GEAR_1,                  //!< gear_1
    GEAR_2,                  //!< gear_2
    INNER_PART,              //!< inner_part
    LEFT,                    //!< left
    MANUFACTURING_SETTINGS,  //!< manufacturing_settings
    ORIGIN,                  //!< origin
    OUTER_PART,              //!< outer_part
    PART,                    //!< part
    PLANETARY_STAGE,         //!< planetary_stage
    REFERENCED,              //!< referenced
    RIGHT,                   //!< right
    SHAFT,                   //!< shaft
    SIDE_1,                  //!< side_1
    SIDE_2,                  //!< side_2
    STAGE,                   //!< stage
    STAGE_GEAR_DATA,         //!< stage_gear_data
    TOOL,                    //!< tool
    WORKPIECE                //!< workpiece
  };

  /**
   * @brief Creates a relation role from a string.
   *
   * @param role The string representation of a relation role
   * @return TRelationRole for the corresponding string
   * @throws TException if an invalid role was specified
   */
  static TRelationRole relationRoleFromString(const std::string& role);

  /**
   * @brief Returns a string representation of a relation role.
   *
   * @param role The role to represent as string
   * @return std::string representation of the relation role
   */
  static std::string toRelationRoleString(TRelationRole role);


  /**
   * @brief Defines if a role is a top or sub level type.
   *
   */
  enum class TRelationRoleType {
    TOP_LEVEL,  //!< top level role
    SUB_LEVEL   //!< sub level role
  };

  /**
   * @brief Returns the role type for a specific role.
   *
   * @param role The role to get the type for
   * @return TRelationRoleType corresponding to the role
   */
  static TRelationRoleType getRoleType(TRelationRole role);


  namespace detail
  {
    enum class TDecoderResult { SUCCESS, WRONG_TYPE, FAILURE, NO_VALUE };
  }

  /////////////////////////////////////////////////////////////////////////////
  // Implementation
  /////////////////////////////////////////////////////////////////////////////

  static inline TValueType typeFromString(const std::string& type)
  {
    if (type == "floating_point") {
      return TValueType::FLOATING_POINT;
    }
    if (type == "boolean") {
      return TValueType::BOOLEAN;
    }
    if (type == "integer") {
      return TValueType::INTEGER;
    }
    if (type == "enum") {
      return TValueType::ENUM;
    }
    if (type == "string") {
      return TValueType::STRING;
    }
    if (type == "file_reference") {
      return TValueType::FILE_REFERENCE;
    }
    if (type == "boolean_array") {
      return TValueType::BOOLEAN_ARRAY;
    }
    if (type == "floating_point_array") {
      return TValueType::FLOATING_POINT_ARRAY;
    }
    if (type == "integer_array") {
      return TValueType::INTEGER_ARRAY;
    }
    if (type == "enum_array") {
      return TValueType::ENUM_ARRAY;
    }
    if (type == "string_array") {
      return TValueType::STRING_ARRAY;
    }
    if (type == "reference_component") {
      return TValueType::REFERENCE_COMPONENT;
    }
    if (type == "floating_point_matrix") {
      return TValueType::FLOATING_POINT_MATRIX;
    }
    if (type == "integer_matrix") {
      return TValueType::INTEGER_MATRIX;
    }
    if (type == "boolean_matrix") {
      return TValueType::BOOLEAN_MATRIX;
    }
    if (type == "string_matrix") {
      return TValueType::STRING_MATRIX;
    }
    if (type == "array_of_integer_arrays") {
      return TValueType::ARRAY_OF_INTEGER_ARRAYS;
    }
    if (type == "date_time") {
      return TValueType::DATE_TIME;
    }
    throw TException{fmt::format("unknown value type '{}'", type)};
  }

  static inline std::string toTypeString(TValueType type)
  {
    switch (type) {
      case TValueType::FLOATING_POINT:
        return "floating_point";
      case TValueType::BOOLEAN:
        return "boolean";
      case TValueType::INTEGER:
        return "integer";
      case TValueType::STRING:
        return "string";
      case TValueType::ENUM:
        return "enum";
      case TValueType::FILE_REFERENCE:
        return "file_reference";
      case TValueType::FLOATING_POINT_ARRAY:
        return "floating_point_array";
      case TValueType::BOOLEAN_ARRAY:
        return "boolean_array";
      case TValueType::INTEGER_ARRAY:
        return "integer_array";
      case TValueType::ENUM_ARRAY:
        return "enum_array";
      case TValueType::STRING_ARRAY:
        return "string_array";
      case TValueType::REFERENCE_COMPONENT:
        return "reference_component";
      case TValueType::FLOATING_POINT_MATRIX:
        return "floating_point_matrix";
      case TValueType::INTEGER_MATRIX:
        return "integer_matrix";
      case TValueType::BOOLEAN_MATRIX:
        return "boolean_matrix";
      case TValueType::STRING_MATRIX:
        return "string_matrix";
      case TValueType::ARRAY_OF_INTEGER_ARRAYS:
        return "array_of_integer_arrays";
      case TValueType::DATE_TIME:
        return "date_time";
    }
    throw TException{fmt::format("unknown value type '{}'", static_cast<int64_t>(type))};
  }

  static inline std::string toRelationTypeString(TRelationType type)
  {
    switch (type) {
      case TRelationType::ASSEMBLY:
        return "assembly";
      case TRelationType::CENTRAL_SHAFT:
        return "central_shaft";
      case TRelationType::CONNECTION:
        return "connection";
      case TRelationType::CONTACT:
        return "contact";
      case TRelationType::COUPLING:
        return "coupling";
      case TRelationType::FLANK:
        return "flank";
      case TRelationType::MANUFACTURING_STEP:
        return "manufacturing_step";
      case TRelationType::ORDERED_ASSEMBLY:
        return "ordered_assembly";
      case TRelationType::ORDERED_REFERENCE:
        return "ordered_reference";
      case TRelationType::PLANET_CARRIER_SHAFT:
        return "planet_carrier_shaft";
      case TRelationType::PLANET_PIN:
        return "planet_pin";
      case TRelationType::PLANET_SHAFT:
        return "planet_shaft";
      case TRelationType::REFERENCE:
        return "reference";
      case TRelationType::SIDE:
        return "side";
      case TRelationType::STAGE:
        return "stage";
      case TRelationType::STAGE_GEAR_DATA:
        return "stage_gear_data";
    }
    throw TException{"unknown relation type"};
  }


  static inline TRelationType relationTypeFromString(const std::string& type)
  {
    if (type == "assembly") {
      return TRelationType::ASSEMBLY;
    }
    if (type == "central_shaft") {
      return TRelationType::CENTRAL_SHAFT;
    }
    if (type == "connection") {
      return TRelationType::CONNECTION;
    }
    if (type == "contact") {
      return TRelationType::CONTACT;
    }
    if (type == "coupling") {
      return TRelationType::COUPLING;
    }
    if (type == "flank") {
      return TRelationType::FLANK;
    }
    if (type == "manufacturing_step") {
      return TRelationType::MANUFACTURING_STEP;
    }
    if (type == "ordered_assembly") {
      return TRelationType::ORDERED_ASSEMBLY;
    }
    if (type == "ordered_reference") {
      return TRelationType::ORDERED_REFERENCE;
    }
    if (type == "planet_carrier_shaft") {
      return TRelationType::PLANET_CARRIER_SHAFT;
    }
    if (type == "planet_pin") {
      return TRelationType::PLANET_PIN;
    }
    if (type == "planet_shaft") {
      return TRelationType::PLANET_SHAFT;
    }
    if (type == "reference") {
      return TRelationType::REFERENCE;
    }
    if (type == "side") {
      return TRelationType::SIDE;
    }
    if (type == "stage") {
      return TRelationType::STAGE;
    }
    if (type == "stage_gear_data") {
      return TRelationType::STAGE_GEAR_DATA;
    }
    throw TException{fmt::format("unknown relation type '{}'", type)};
  }


  static inline std::string toRelationRoleString(TRelationRole role)
  {
    switch (role) {
      case TRelationRole::ASSEMBLY:
        return "assembly";
      case TRelationRole::GEAR:
        return "gear";
      case TRelationRole::GEAR_1:
        return "gear_1";
      case TRelationRole::GEAR_2:
        return "gear_2";
      case TRelationRole::INNER_PART:
        return "inner_part";
      case TRelationRole::LEFT:
        return "left";
      case TRelationRole::MANUFACTURING_SETTINGS:
        return "manufacturing_settings";
      case TRelationRole::ORIGIN:
        return "origin";
      case TRelationRole::OUTER_PART:
        return "outer_part";
      case TRelationRole::PART:
        return "part";
      case TRelationRole::PLANETARY_STAGE:
        return "planetary_stage";
      case TRelationRole::REFERENCED:
        return "referenced";
      case TRelationRole::RIGHT:
        return "right";
      case TRelationRole::SHAFT:
        return "shaft";
      case TRelationRole::SIDE_1:
        return "side_1";
      case TRelationRole::SIDE_2:
        return "side_2";
      case TRelationRole::STAGE:
        return "stage";
      case TRelationRole::STAGE_GEAR_DATA:
        return "stage_gear_data";
      case TRelationRole::TOOL:
        return "tool";
      case TRelationRole::WORKPIECE:
        return "workpiece";
    }
    throw TException{"unknown relation role"};
  }

  static inline TRelationRole relationRoleFromString(const std::string& role)
  {
    if (role == "assembly") {
      return TRelationRole::ASSEMBLY;
    }
    if (role == "gear") {
      return TRelationRole::GEAR;
    }
    if (role == "gear_1") {
      return TRelationRole::GEAR_1;
    }
    if (role == "gear_2") {
      return TRelationRole::GEAR_2;
    }
    if (role == "inner_part") {
      return TRelationRole::INNER_PART;
    }
    if (role == "left") {
      return TRelationRole::LEFT;
    }
    if (role == "manufacturing_settings") {
      return TRelationRole::MANUFACTURING_SETTINGS;
    }
    if (role == "origin") {
      return TRelationRole::ORIGIN;
    }
    if (role == "outer_part") {
      return TRelationRole::OUTER_PART;
    }
    if (role == "part") {
      return TRelationRole::PART;
    }
    if (role == "planetary_stage") {
      return TRelationRole::PLANETARY_STAGE;
    }
    if (role == "referenced") {
      return TRelationRole::REFERENCED;
    }
    if (role == "right") {
      return TRelationRole::RIGHT;
    }
    if (role == "shaft") {
      return TRelationRole::SHAFT;
    }
    if (role == "stage") {
      return TRelationRole::STAGE;
    }
    if (role == "side_1") {
      return TRelationRole::SIDE_1;
    }
    if (role == "side_2") {
      return TRelationRole::SIDE_2;
    }
    if (role == "stage_gear_data") {
      return TRelationRole::STAGE_GEAR_DATA;
    }
    if (role == "tool") {
      return TRelationRole::TOOL;
    }
    if (role == "workpiece") {
      return TRelationRole::WORKPIECE;
    }
    throw TException{fmt::format("unknown relation role '{}'", role)};
  }


  static inline TRelationRoleType getRoleType(TRelationRole role)
  {
    switch (role) {
      case TRelationRole::ASSEMBLY:
      case TRelationRole::GEAR:
      case TRelationRole::ORIGIN:
      case TRelationRole::PLANETARY_STAGE:
      case TRelationRole::STAGE:
      case TRelationRole::WORKPIECE:
        return TRelationRoleType::TOP_LEVEL;
      case TRelationRole::GEAR_1:
      case TRelationRole::GEAR_2:
      case TRelationRole::INNER_PART:
      case TRelationRole::LEFT:
      case TRelationRole::MANUFACTURING_SETTINGS:
      case TRelationRole::OUTER_PART:
      case TRelationRole::PART:
      case TRelationRole::REFERENCED:
      case TRelationRole::RIGHT:
      case TRelationRole::SHAFT:
      case TRelationRole::SIDE_1:
      case TRelationRole::SIDE_2:
      case TRelationRole::STAGE_GEAR_DATA:
      case TRelationRole::TOOL:
        return TRelationRoleType::SUB_LEVEL;
    }
    throw TException{"unknown relation role"};
  }
}

#endif
