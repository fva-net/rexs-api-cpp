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

#define REXSAPI_MINIZ_IMPL
#include <rexsapi/Rexsapi.hxx>

#include <algorithm>

#include "Cli11.hxx"
#include "ToolsHelper.hxx"

#ifdef _WIN32
static bool setLocale{ true };
#endif


struct Options {
  rexsapi::TMode mode{rexsapi::TMode::STRICT_MODE};
  std::filesystem::path modelDatabasePath;
  std::filesystem::path model;
  rexsapi::TCustomExtensionMappings customExtentionMappings;
  bool statistcs{false};
  bool showAttributes{false};
};

static std::string getVersion()
{
  return fmt::format("model_dumper version {}\n", REXSAPI_VERSION_STRING);
}

static std::optional<Options> getOptions(int argc, char** argv)
{
  Options options;
  std::vector<std::string> customExtensionMappings;

  CLI::App app{getVersion()};
  auto* strictFlag = app.add_flag(
    "--mode-strict",
    [&options](auto) {
      options.mode = rexsapi::TMode::STRICT_MODE;
    },
    "Strict standard handling");
  app
    .add_flag(
      "--mode-relaxed",
      [&options](auto) {
        options.mode = rexsapi::TMode::RELAXED_MODE;
      },
      "Relaxed standard handling")
    ->excludes(strictFlag);
  app.add_option("-d,--database", options.modelDatabasePath, "The model database path")
    ->check(CLI::ExistingDirectory)
    ->required();
  app.add_flag("-s", options.statistcs, "Show some model statistics");
  app.add_flag("-a,--attributes", options.showAttributes, "Show component attributes");
#ifdef _WIN32
  app.add_flag("--no-locale", [](auto) {
    setLocale = false;
    }, "Don't set locale to UTF-8");
#endif
  app.add_option(
    "-m", customExtensionMappings,
    "Custom extension for rexs files. E.g. .rexs.in:xml will use the extension .res.in to import xml files.");
  app.add_option("model", options.model, "The model file to dump")->check(CLI::ExistingFile)->required();

  try {
    app.parse(argc, argv);
  } catch (const CLI::Success& e) {
    app.exit(e);
    return {};
  } catch (const CLI::ParseError& e) {
    std::cerr << getVersion() << std::endl;
    app.exit(e);
    return {};
  }

  options.customExtentionMappings = getCustomMappings(customExtensionMappings);

  return options;
}


class SimpleModelDumper : public rexsapi::TModelVisitor
{
public:
  explicit SimpleModelDumper(const rexsapi::TModel& model, bool dumpStatistics, bool showAttributes,
                             std::ostream& stream)
  : m_Stream{stream}
  , m_ShowAttributes{showAttributes}
  {
    visit(model);
    dump(dumpStatistics);
  }

private:
  void dump(bool dumpStatistics) const
  {
    if (dumpStatistics) {
      m_Stream << "\nStatistics\n==========\n";
      m_Stream << fmt::format("Relations {}\n", m_Relations);
      std::for_each(m_RelationTypes.begin(), m_RelationTypes.end(), [this](const auto& element) {
        const auto& [type, count] = element;
        m_Stream << fmt::format("\t{}: {}\n", type, count);
      });
      m_Stream << fmt::format("Components {}\n", m_Components);
      std::for_each(m_ComponentTypes.begin(), m_ComponentTypes.end(), [this](const auto& element) {
        const auto& [type, count] = element;
        m_Stream << fmt::format("\t{}: {}\n", type, count);
      });
    }
  }

  void onVisit(const rexsapi::TModelInfo& info) override
  {
    m_Stream << fmt::format("ApplId: '{}' ApplVer: {} Date: {} REXSVer: {}\n", info.getApplicationId(),
                            info.getApplicationVersion(), info.getDate(), info.getVersion().asString());
    m_Stream << "\nRelations\n=========\n";
  }

  void onVisit(const rexsapi::TRelation& relation) override
  {
    m_Stream << fmt::format("Type: {}", rexsapi::toRelationTypeString(relation.getType()));
    if (relation.getOrder().has_value()) {
      m_Stream << fmt::format(" Order: {}", *relation.getOrder());
    }
    m_Stream << "\n";
    ++m_Relations;
    auto relationType = rexsapi::toRelationTypeString(relation.getType());
    if (m_RelationTypes.find(relationType) == m_RelationTypes.end()) {
      m_RelationTypes[relationType] = 0;
    }
    ++m_RelationTypes[relationType];
  }

  void onVisit(const rexsapi::TRelationReference& reference) override
  {
    m_Stream << fmt::format("{:<8} Role: {:<20} ComponentId: {:<4} Type: {:<36} Hint: {}\n", "",
                            rexsapi::toRelationRoleString(reference.getRole()),
                            reference.getComponent().getInternalId(), reference.getComponent().getType(),
                            reference.getHint());
  }

  void onVisit(const rexsapi::TComponent& component) override
  {
    if (static bool first = true; first) {
      m_Stream << fmt::format("\nComponents\n==========\n");
      first = false;
    }

    ++m_Components;

    if (m_ComponentTypes.find(component.getType()) == m_ComponentTypes.end()) {
      m_ComponentTypes[component.getType()] = 0;
    }
    ++m_ComponentTypes[component.getType()];

    m_Stream << fmt::format("Id: {:<4} Type: {:<36} Name: {}\n", component.getInternalId(), component.getType(),
                            component.getName());
  }

  void onVisit(const rexsapi::TAttribute& attribute) override
  {
    if (m_ShowAttributes) {
      m_Stream << fmt::format("{:<8} AttId: {:<35} Name: {:<30} Unit: {:<10} Value: {}\n", "",
                              attribute.getAttributeId(), attribute.getName(), attribute.getUnit().getName(),
                              attribute.getValueAsString());
    }
  }

  void onVisit(const rexsapi::TLoadSpectrum&) override
  {
    m_Stream << fmt::format("\nLoadSpectrum\n============\n");
  }

  void onVisit(const rexsapi::TLoadCase&) override
  {
    static uint16_t count{0};
    m_Stream << fmt::format("Case {}\n", ++count);
  }

  void onVisit(const rexsapi::TAccumulation&) override
  {
    m_Stream << fmt::format("\nAccumulation\n============\n");
  }

  void onVisit(const rexsapi::TLoadComponent& loadComponent) override
  {
    m_Stream << fmt::format("ComponentId: {:<4} Type: {:<36}\n", loadComponent.getComponent().getInternalId(),
                            loadComponent.getComponent().getType());
    if (m_ShowAttributes) {
      for (const auto& attribute : loadComponent.getLoadAttributes()) {
        m_Stream << fmt::format("{:<8} AttId: {:<35} Name: {:<30} Unit: {:<10} Value: {}\n", "",
                                attribute.getAttributeId(), attribute.getName(), attribute.getUnit().getName(),
                                attribute.getValueAsString());
      }
    }
    m_Stream << "\n";
  }

  std::ostream& m_Stream;
  bool m_ShowAttributes{false};
  size_t m_Relations{};
  std::unordered_map<std::string, size_t> m_RelationTypes{};
  size_t m_Components{};
  std::unordered_map<std::string, size_t> m_ComponentTypes{};
};


int main(int argc, char** argv)
{
  try {
    auto options = getOptions(argc, argv);
    if (!options) {
      return EXIT_FAILURE;
    }

    const rexsapi::TModelLoader loader{options->modelDatabasePath, options->customExtentionMappings};

    rexsapi::TResult result;
    const auto model = loader.load(options->model, result, options->mode);

#ifdef _WIN32
    if (setLocale) {
      setlocale(LC_ALL, ".UTF8");
    }
#endif

    std::cout << "File " << options->model;
    if (!result) {
      std::cerr << fmt::format("\n  Cannot load model. Found {} issues\n", result.getErrors().size());
    } else {
      std::cout << "\n\n";
      SimpleModelDumper dumper{*model, options->statistcs, options->showAttributes, std::cout};
    }
  } catch (const std::exception& ex) {
    std::cerr << fmt::format("Exception caught: {}\n", ex.what());
  }

  return EXIT_SUCCESS;
}
