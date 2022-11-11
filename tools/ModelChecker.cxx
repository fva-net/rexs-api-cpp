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

#include "Cli11.hxx"
#include "ToolsHelper.hxx"


struct Options {
  rexsapi::TMode mode{rexsapi::TMode::STRICT_MODE};
  std::filesystem::path modelDatabasePath;
  std::vector<std::filesystem::path> models;
  bool showWarnings{false};
  rexsapi::TCustomExtensionMappings customExtentionMappings;
};

static std::string getVersion()
{
  return fmt::format("model_checker version {}\n", REXSAPI_VERSION_STRING);
}

static std::optional<Options> getOptions(int argc, char** argv)
{
  Options options;
  bool recurse{false};
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
  app.add_flag("-w,--warnings", options.showWarnings, "Show all warnings");
  app.add_flag("-r", recurse, "Recurse into sub-directories");
  app.add_option("-d,--database", options.modelDatabasePath, "The model database path")
    ->check(CLI::ExistingDirectory)
    ->required();
  app.add_option(
    "-m", customExtensionMappings,
    "Custom extension for rexs files. E.g. .rexs.in:xml will use the extension .res.in to import xml files.");
  app.add_option("models", options.models, "The model files or directories to check")
    ->check(CLI::ExistingFile | CLI::ExistingDirectory)
    ->required();

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

  options.models = getModels(recurse, options.models);
  options.customExtentionMappings = getCustomMappings(customExtensionMappings);

  return options;
}


int main(int argc, char** argv)
{
  try {
    auto options = getOptions(argc, argv);
    if (!options) {
      return EXIT_FAILURE;
    }

    const rexsapi::TModelLoader loader{options->modelDatabasePath, options->customExtentionMappings};

    bool start{true};
    for (const auto& modelFile : options->models) {
      if (start) {
        start = false;
      } else {
        std::cout << std::endl;
      }
      rexsapi::TResult result;
      const auto model = loader.load(modelFile, result, options->mode);

      std::cout << "File " << modelFile;
      if (!result) {
        std::cout << std::endl << fmt::format("  Found {} issues", result.getErrors().size()) << std::endl;
      } else {
        std::cout << " processed";
        if (result.hasIssues() && options->showWarnings) {
          std::cout << fmt::format(", but has the following {} warnings", result.getErrors().size());
        } else if (result.hasIssues() && !options->showWarnings) {
          std::cout << fmt::format(" with {} warnings", result.getErrors().size());
        } else {
          std::cout << " successfully";
        }
        std::cout << std::endl;
      }
      for (const auto& error : result.getErrors()) {
        if (error.isWarning() && !options->showWarnings) {
          continue;
        }
        std::cout << "  " << error.getMessage() << std::endl;
      }
    }
  } catch (const std::exception& ex) {
    std::cerr << "Exception caught: " << ex.what() << std::endl;
  }

  return EXIT_SUCCESS;
}
