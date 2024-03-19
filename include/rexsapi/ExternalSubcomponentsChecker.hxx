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

#ifndef REXSAPI_EXTERNAL_SUBCOMPONENTS_CHECKER_HXX
#define REXSAPI_EXTERNAL_SUBCOMPONENTS_CHECKER_HXX

#include <rexsapi/Component.hxx>
#include <rexsapi/Json.hxx>
#include <rexsapi/Mode.hxx>
#include <rexsapi/RexsVersion.hxx>

#include <optional>
#include <unordered_map>


namespace rexsapi
{
  namespace detail
  {
    class TSubcomponentsMapping
    {
    public:
      struct TSubcomponentsEntry {
        std::vector<std::string> m_Subcomponents{};
      };

      std::unordered_map<std::string, TSubcomponentsEntry> m_Entries{};
    };

    using TSubcomponentsMappings = std::map<TRexsVersion, TSubcomponentsMapping>;

    static inline TSubcomponentsMappings parseSubcomponentsMappings(TResult& result, std::string_view buffer);

    static inline TSubcomponentsMappings loadSubcomponentsMappings();
  }

  class TExternalSubcomponentsChecker
  {
  public:
    explicit TExternalSubcomponentsChecker(TMode mode, const TRexsVersion& version)
    : m_Mode{mode}
    , m_Version{version}
    , m_Mapping{findVersion(version)}
    {
    }

    bool isPermissibleSubComponent(TResult& result, const TComponent& mainComponent,
                                   const TComponent& subComponent) const;

  private:
    static std::optional<detail::TSubcomponentsMapping> findVersion(const TRexsVersion& version);

    const detail::TModeAdapter m_Mode;
    const TRexsVersion m_Version;
    const std::optional<detail::TSubcomponentsMapping> m_Mapping;
  };

  inline std::optional<detail::TSubcomponentsMapping>
  TExternalSubcomponentsChecker::findVersion(const TRexsVersion& version)
  {
    detail::TSubcomponentsMappings mappings{detail::loadSubcomponentsMappings()};

    for (auto it = std::make_reverse_iterator(mappings.end()); it != std::make_reverse_iterator(mappings.begin());
         ++it) {
      const auto& [key, value] = *it;
      if (version >= TRexsVersion{key}) {
        return value;
      }
    }
    return {};
  }

  inline bool TExternalSubcomponentsChecker::isPermissibleSubComponent(TResult& result, const TComponent& mainComponent,
                                                                       const TComponent& subComponent) const
  {
    (void)mainComponent;
    (void)subComponent;
    if (!m_Mapping) {
      result.addError(
        TError(m_Mode.adapt(TErrorLevel::ERR),
               fmt::format("no permissible external subcomponent mapping found for version {}", m_Version.asString())));
      return false;
    }

    auto it = m_Mapping->m_Entries.find(mainComponent.getType());
    if (it == m_Mapping->m_Entries.end()) {
      result.addError(TError(
        m_Mode.adapt(TErrorLevel::ERR),
        fmt::format("no permissible external subcomponent entry found for component '{}'", mainComponent.getType())));
      return false;
    }

    auto itSub =
      std::find(it->second.m_Subcomponents.begin(), it->second.m_Subcomponents.end(), subComponent.getType());
    if (itSub == it->second.m_Subcomponents.end()) {
      result.addError(TError(m_Mode.adapt(TErrorLevel::ERR),
                             fmt::format("external sub component '{}' is not permissible for main component '{}'",
                                         subComponent.getType(), mainComponent.getType())));
    }
    return itSub != it->second.m_Subcomponents.end();
  }

  namespace detail
  {
    static inline TSubcomponentsMappings parseSubcomponentsMappings(TResult& result, std::string_view buffer)
    {
      TSubcomponentsMappings mappings;

      try {
        const rexsapi::json j = rexsapi::json::parse(buffer);
        for (const auto& [key, value] : j.items()) {
          auto version = TRexsVersion{key};

          TSubcomponentsMapping mapping;
          for (const auto& [mainComponent, entry] : value.items()) {
            TSubcomponentsMapping::TSubcomponentsEntry subcomponentEntry;
            for (const auto& subcomponent : entry) {
              subcomponentEntry.m_Subcomponents.emplace_back(subcomponent.get<std::string>());
            }

            mapping.m_Entries.emplace(mainComponent, std::move(subcomponentEntry));
          }
          mappings.emplace(version, mapping);
        }
      } catch (const std::exception& ex) {
        result.addError(TError{TErrorLevel::CRIT, fmt::format("cannot parse json document: {}", ex.what())});
      }

      return mappings;
    }

    static inline TSubcomponentsMappings loadSubcomponentsMappings()
    {
      TResult result;

      const auto* subcomponents = R"(
{
  "1.5": {
    "material": [],
    "sn_curve": [],
    "lubricant": [],
    "bevel_gear_tool": [],
    "cutter_wheel_tool": [],
    "rack_shaped_tool": [],
    "worm_grinding_disc_tool": [],
    "worm_wheel_hob_tool": [],
    "zero_degree_grinding_disk_tool": [],
    "rolling_bearing_with_catalog_geometry": [
      "rolling_bearing_row",
      "rolling_element",
      "material",
      "lubricant",
      "rolling_element_contact"
    ],
    "rolling_bearing_with_detailed_geometry": [
      "rolling_bearing_row",
      "rolling_element",
      "material",
      "lubricant",
      "rolling_element_contact"
    ],
    "shaft": [
      "shaft_section",
      "shaft_shoulder",
      "shaft_shoulder_with_undercut",
      "round_groove",
      "rectangular_groove",
      "v_notch",
      "transverse_bore",
      "fkm_evaluation_point",
      "additional_mass",
      "material"
    ],
    "cylindrical_gear": [
      "cylindrical_gear_flank",
      "material",
      "sn-curve",
      "profile_slope",
      "tip_relief",
      "root_relief",
      "profile_crowning",
      "helix_crowning",
      "helix_slope",
      "end_relief_datum_face",
      "end_relief_non_datum_face",
      "topographical_modification",
      "triangular_tip_relief",
      "triangular_root_relief",
      "profile_twist",
      "profile_deviation",
      "helix_deviation",
      "flank_geometry",
      "cutter_wheel_tool",
      "rack_shaped_tool",
      "zero_degree_grinding_disk_tool",
      "cylindrical_gear_manufacturing_settings"
    ],
    "ring_gear": [
      "cylindrical_gear_flank",
      "material",
      "sn-curve",
      "profile_slope",
      "tip_relief",
      "root_relief",
      "profile_crowning",
      "helix_crowning",
      "helix_slope",
      "end_relief_datum_face",
      "end_relief_non_datum_face",
      "topographical_modification",
      "triangular_tip_relief",
      "triangular_root_relief",
      "profile_twist",
      "profile_deviation",
      "helix_deviation",
      "flank_geometry",
      "cutter_wheel_tool",
      "cylindrical_gear_manufacturing_settings"
    ],
    "bevel_gear": [
      "bevel_gear_flank",
      "material",
      "sn-curve",
      "bevel_gear_tool",
      "bevel_gear_manufacturing_settings"
    ],
    "worm_gear": [
      "worm_gear_flank",
      "material",
      "sn-curve",
      "worm_grinding_disc_tool",
      "worm_gear_manufacturing_settings"
    ],
    "worm_wheel": [
      "worm_gear_flank",
      "material",
      "sn-curve",
      "worm_wheel_hob_tool",
      "worm_gear_manufacturing_settings"
    ]
  }
}
    )";

      auto mappings = parseSubcomponentsMappings(result, subcomponents);
      if (!result) {
        throw TException{fmt::format("cannot load external permissible subcomponent mapping")};
      }

      return mappings;
    }
  }
}

#endif
