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

#ifndef REXSAPI_RELATION_TYPE_CHECKER_HXX
#define REXSAPI_RELATION_TYPE_CHECKER_HXX

#include <rexsapi/Json.hxx>
#include <rexsapi/Mode.hxx>
#include <rexsapi/Model.hxx>

#include <iterator>

namespace rexsapi
{
  namespace detail
  {
    class TRelationTypeMapping
    {
    public:
      struct TRelationRole {
        std::string m_Component{};
        rexsapi::TRelationRole m_Role{};
      };

      struct TRelationTypeEntry {
        TRelationType m_Type{};
        bool m_Ordered{false};
        std::vector<TRelationRole> m_Roles{};
      };

      std::vector<TRelationTypeEntry> m_Entries{};
    };

    using TRelationTypeMappings = std::map<TRexsVersion, TRelationTypeMapping>;

    static inline void from_json(const rexsapi::json& j, TRelationTypeMapping::TRelationRole& role);

    static inline void from_json(const rexsapi::json& j, TRelationTypeMapping::TRelationTypeEntry& entry);

    static inline TRelationTypeMappings parseMappings(TResult& result, std::string_view buffer);

    static inline TRelationTypeMappings loadMappings();
  }


  /**
   * @brief Checks relations for correctness.
   *
   * Checks either a model or a single relation for correctness. Correctness means that a relation has the necessary
   * reference roles mandated by the standard according to the relations (or models) version. Additionally, a relation
   * is checked if it is allowed to have an order attribute.
   */
  class TRelationTypeChecker
  {
  public:
    /**
     * @brief Constructs a new TRelationTypeChecker object.
     *
     * The checker will downgrade non-critical errors to warnings if relaxed mode is used. However, the checking might
     * downgrade errors, but the check methods will nevertheless return false upon any errors.
     *
     * @param mode The mode to use for checking
     */
    explicit TRelationTypeChecker(TMode mode)
    : m_Mode{mode}
    {
    }

    /**
     * @brief Checks all the relations of the given model.
     *
     * @param result Describes the outcome of the check. Will contain messages upon issues encountered.
     * @param model The model to check the relations for. Will use the models version for the check.
     * @return true if all relations are correct
     * @return false if at least one relation is not correct
     */
    bool check(TResult& result, const TModel& model) const;

    /**
     * @brief Checks a single relation.
     *
     * @param result Describes the outcome of the check. Will contain messages upon issues encountered.
     * @param version The version to use for the check
     * @param relation The relation to check
     * @return true if the relation is correct
     * @return false if the realtion is not correct
     */
    bool check(TResult& result, const TRexsVersion& version, const TRelation& relation) const;

  private:
    bool checkRelation(TResult& result, const TRexsVersion& version, const detail::TRelationTypeMapping* mapping,
                       const TRelation& relation) const;

    static bool checkRole(const detail::TRelationTypeMapping::TRelationRole& role,
                          const TRelationReferences& references);

    const detail::TRelationTypeMapping* findVersion(const TRexsVersion& version) const;

    detail::TModeAdapter m_Mode;
    detail::TRelationTypeMappings m_Mappings{detail::loadMappings()};
  };


  /////////////////////////////////////////////////////////////////////////////
  // Implementation
  /////////////////////////////////////////////////////////////////////////////

  inline bool TRelationTypeChecker::check(TResult& result, const TModel& model) const
  {
    const auto& version = model.getInfo().getVersion();
    const auto* mapping = findVersion(version);
    if (mapping == nullptr) {
      result.addError(
        TError{TErrorLevel::ERR, fmt::format("cannot find configuration for version {}", version.asString())});
      return false;
    }

    bool res{true};
    for (const auto& relation : model.getRelations()) {
      res |= checkRelation(result, version, mapping, relation);
    }

    return res;
  }

  inline bool TRelationTypeChecker::check(TResult& result, const TRexsVersion& version, const TRelation& relation) const
  {
    const auto* mapping = findVersion(version);
    if (mapping == nullptr) {
      result.addError(
        TError{TErrorLevel::ERR, fmt::format("cannot find configuration for version {}", version.asString())});
      return false;
    }

    return checkRelation(result, version, mapping, relation);
  }

  inline bool TRelationTypeChecker::checkRelation(TResult& result, const TRexsVersion& version,
                                                  const detail::TRelationTypeMapping* mapping,
                                                  const TRelation& relation) const
  {
    auto it = std::find_if(mapping->m_Entries.begin(), mapping->m_Entries.end(),
                           [type = relation.getType()](const auto& entry) {
                             return entry.m_Type == type;
                           });

    if (it == mapping->m_Entries.end()) {
      result.addError(TError{m_Mode.adapt(TErrorLevel::ERR),
                             fmt::format("relation type '{}' is not allowed for version {}",
                                         toRelationTypeString(relation.getType()), version.asString())});
      return false;
    }

    const auto& relationType = *it;

    if (relation.getOrder().has_value() && !relationType.m_Ordered) {
      result.addError(TError{m_Mode.adapt(TErrorLevel::ERR), fmt::format("relation type '{}' does not allow ordering",
                                                                         toRelationTypeString(relation.getType()))});
    }

    for (const auto& role : relationType.m_Roles) {
      if (!checkRole(role, relation.getReferences())) {
        result.addError(
          TError{m_Mode.adapt(TErrorLevel::ERR),
                 fmt::format("role '{}:{}' is missing for relation '{}'", role.m_Component,
                             toRelationRoleString(role.m_Role), toRelationTypeString(relation.getType()))});
      }
    }
    if (relationType.m_Roles.size() != relation.getReferences().size()) {
      result.addError(TError{m_Mode.adapt(TErrorLevel::ERR),
                             fmt::format("too many roles in relation '{}'", toRelationTypeString(relation.getType()))});
    }

    return !result.hasIssues();
  }

  inline bool TRelationTypeChecker::checkRole(const detail::TRelationTypeMapping::TRelationRole& role,
                                              const TRelationReferences& references)
  {
    for (const auto& reference : references) {
      if (role.m_Role == reference.getRole()) {
        return true;
      }
    }

    return false;
  }

  inline const detail::TRelationTypeMapping* TRelationTypeChecker::findVersion(const TRexsVersion& version) const
  {
    for (auto it = std::make_reverse_iterator(m_Mappings.end()); it != std::make_reverse_iterator(m_Mappings.begin());
         ++it) {
      const auto& [key, value] = *it;
      if (version >= TRexsVersion{key}) {
        return &value;
      }
    }
    return nullptr;
  }

  namespace detail
  {
    static inline void from_json(const rexsapi::json& j, TRelationTypeMapping::TRelationRole& role)
    {
      j.at("component").get_to(role.m_Component);
      role.m_Role = relationRoleFromString(j.at("role").get<std::string>());
    }

    static inline void from_json(const rexsapi::json& j, TRelationTypeMapping::TRelationTypeEntry& entry)
    {
      if (j.contains("ordered")) {
        j.at("ordered").get_to(entry.m_Ordered);
      }
      for (const auto& role : j["roles"]) {
        entry.m_Roles.emplace_back(role.get<TRelationTypeMapping::TRelationRole>());
      }
    }

    static inline TRelationTypeMappings parseMappings(TResult& result, std::string_view buffer)
    {
      TRelationTypeMappings mappings;

      try {
        const rexsapi::json j = rexsapi::json::parse(buffer);
        for (const auto& [key, value] : j.items()) {
          auto version = TRexsVersion{key};

          TRelationTypeMapping mapping;
          for (const auto& [relationType, entry] : value.items()) {
            TRelationTypeMapping::TRelationTypeEntry relationTypeEntry =
              entry.get<TRelationTypeMapping::TRelationTypeEntry>();
            relationTypeEntry.m_Type = relationTypeFromString(relationType);

            mapping.m_Entries.emplace_back(std::move(relationTypeEntry));
          }
          mappings.emplace(version, mapping);
        }
      } catch (const std::exception& ex) {
        result.addError(TError{TErrorLevel::CRIT, fmt::format("cannot parse json document: {}", ex.what())});
      }

      return mappings;
    }

    static inline TRelationTypeMappings loadMappings()
    {
      TResult result;

      const auto* relationTypes = R"(
{
  "1.0": {
    "assembly": {
      "ordered": false,
      "roles": [
        { "component": "K1", "role": "assembly" },
        { "component": "K2", "role": "part" }
      ]
    },
    "ordered_assembly": {
      "ordered": true,
      "roles": [
        { "component": "K1", "role": "assembly" },
        { "component": "K2", "role": "part" }
      ]
    },
    "stage": {
      "ordered": false,
      "roles": [
        { "component": "S", "role": "stage" },
        { "component": "R1", "role": "gear_1" },
        { "component": "R2", "role": "gear_2" }
      ]
    },
    "stage_gear_data": {
      "ordered": false,
      "roles": [
        { "component": "S", "role": "stage" },
        { "component": "R", "role": "gear" },
        { "component": "D", "role": "stage_gear_data" }
      ]
    },
    "side": {
      "ordered": false,
      "roles": [
        { "component": "M", "role": "assembly" },
        { "component": "IP", "role": "inner_part" },
        { "component": "OP", "role": "outer_part" }
      ]
    },
    "coupling": {
      "ordered": false,
      "roles": [
        { "component": "C", "role": "assembly" },
        { "component": "S1", "role": "side_1" },
        { "component": "S2", "role": "side_2" }
      ]
    },
    "flank": {
      "ordered": false,
      "roles": [
        { "component": "R", "role": "gear" },
        { "component": "C1", "role": "left" },
        { "component": "C2", "role": "right" }
      ]
    },
    "reference": {
      "ordered": false,
      "roles": [
        { "component": "R", "role": "origin" },
        { "component": "P", "role": "referenced" }
      ]
    },
    "ordered_reference": {
      "ordered": true,
      "roles": [
        { "component": "R", "role": "origin" },
        { "component": "P", "role": "referenced" }
      ]
    }
  },
  "1.1": {
    "assembly": {
      "ordered": false,
      "roles": [
        { "component": "K1", "role": "assembly" },
        { "component": "K2", "role": "part" }
      ]
    },
    "ordered_assembly": {
      "ordered": true,
      "roles": [
        { "component": "K1", "role": "assembly" },
        { "component": "K2", "role": "part" }
      ]
    },
    "stage": {
      "ordered": false,
      "roles": [
        { "component": "S", "role": "stage" },
        { "component": "R1", "role": "gear_1" },
        { "component": "R2", "role": "gear_2" }
      ]
    },
    "stage_gear_data": {
      "ordered": false,
      "roles": [
        { "component": "S", "role": "stage" },
        { "component": "R", "role": "gear" },
        { "component": "D", "role": "stage_gear_data" }
      ]
    },
    "side": {
      "ordered": false,
      "roles": [
        { "component": "M", "role": "assembly" },
        { "component": "IP", "role": "inner_part" },
        { "component": "OP", "role": "outer_part" }
      ]
    },
    "flank": {
      "ordered": false,
      "roles": [
        { "component": "R", "role": "gear" },
        { "component": "C1", "role": "left" },
        { "component": "C2", "role": "right" }
      ]
    },
    "reference": {
      "ordered": false,
      "roles": [
        { "component": "R", "role": "origin" },
        { "component": "P", "role": "referenced" }
      ]
    },
    "ordered_reference": {
      "ordered": true,
      "roles": [
        { "component": "R", "role": "origin" },
        { "component": "P", "role": "referenced" }
      ]
    },
    "planet_shaft": {
      "ordered": false,
      "roles": [
        { "component": "C1", "role": "planetary_stage" },
        { "component": "C2", "role": "shaft" }
      ]
    },
    "central_shaft": {
      "ordered": false,
      "roles": [
        { "component": "C1", "role": "planetary_stage" },
        { "component": "C2", "role": "shaft" }
      ]
    },
    "planet_carrier_shaft": {
      "ordered": false,
      "roles": [
        { "component": "C1", "role": "planetary_stage" },
        { "component": "C2", "role": "shaft" }
      ]
    },
    "planet_pin": {
      "ordered": false,
      "roles": [
        { "component": "C1", "role": "planetary_stage" },
        { "component": "C2", "role": "shaft" }
      ]
    }
  },
  "1.3": {
    "assembly": {
      "ordered": false,
      "roles": [
        { "component": "K1", "role": "assembly" },
        { "component": "K2", "role": "part" }
      ]
    },
    "ordered_assembly": {
      "ordered": true,
      "roles": [
        { "component": "K1", "role": "assembly" },
        { "component": "K2", "role": "part" }
      ]
    },
    "stage": {
      "ordered": false,
      "roles": [
        { "component": "S", "role": "stage" },
        { "component": "R1", "role": "gear_1" },
        { "component": "R2", "role": "gear_2" }
      ]
    },
    "stage_gear_data": {
      "ordered": false,
      "roles": [
        { "component": "S", "role": "stage" },
        { "component": "R", "role": "gear" },
        { "component": "D", "role": "stage_gear_data" }
      ]
    },
    "side": {
      "ordered": false,
      "roles": [
        { "component": "M", "role": "assembly" },
        { "component": "IP", "role": "inner_part" },
        { "component": "OP", "role": "outer_part" }
      ]
    },
    "flank": {
      "ordered": false,
      "roles": [
        { "component": "R", "role": "gear" },
        { "component": "C1", "role": "left" },
        { "component": "C2", "role": "right" }
      ]
    },
    "reference": {
      "ordered": false,
      "roles": [
        { "component": "R", "role": "origin" },
        { "component": "P", "role": "referenced" }
      ]
    },
    "manufacturing_step": {
      "ordered": true,
      "roles": [
        { "component": "F", "role": "workpiece" },
        { "component": "T", "role": "tool" },
        { "component": "M", "role": "manufacturing_settings" }
      ]
    },
    "planet_shaft": {
      "ordered": false,
      "roles": [
        { "component": "C1", "role": "planetary_stage" },
        { "component": "C2", "role": "shaft" }
      ]
    },
    "central_shaft": {
      "ordered": false,
      "roles": [
        { "component": "C1", "role": "planetary_stage" },
        { "component": "C2", "role": "shaft" }
      ]
    },
    "planet_carrier_shaft": {
      "ordered": false,
      "roles": [
        { "component": "C1", "role": "planetary_stage" },
        { "component": "C2", "role": "shaft" }
      ]
    },
    "planet_pin": {
      "ordered": false,
      "roles": [
        { "component": "C1", "role": "planetary_stage" },
        { "component": "C2", "role": "shaft" }
      ]
    },
    "contact": {
      "ordered": false,
      "roles": [
        { "component": "S", "role": "assembly" },
        { "component": "A", "role": "side_1" },
        { "component": "B", "role": "side_2" }
      ]
    }
  }
}
    )";

      TRelationTypeMappings mappings = parseMappings(result, relationTypes);
      if (!result) {
        throw TException{fmt::format("cannot load relation types mapping")};
      }

      return mappings;
    }
  }
}

#endif
