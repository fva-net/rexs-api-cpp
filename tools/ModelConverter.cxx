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
  std::filesystem::path outputPath;
  rexsapi::TFileType type{rexsapi::TFileType::UNKNOWN};
  rexsapi::TCustomExtensionMappings customExtentionMappings;
};

static std::string getVersion()
{
  return fmt::format("model_converter version {}\n", REXSAPI_VERSION_STRING);
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
  app.add_flag("-r", recurse, "Recurse into sub-directories");
  app
    .add_option_function<std::string>(
      "-f,--format",
      [&options](const std::string& value) {
        options.type = rexsapi::fileTypeFromString(value);
      },
      "Select output format")
    ->check(CLI::IsMember({"xml", "json"}))
    ->required();
  app.add_option(
    "-m", customExtensionMappings,
    "Custom extension for rexs files. E.g. .rexs.in:xml will use the extension .res.in to import xml files.");
  app.add_option("-d,--database", options.modelDatabasePath, "The model database path")
    ->check(CLI::ExistingDirectory)
    ->required();
  app.add_option("-o,--output", options.outputPath, "Output directory for converted models")->required();
  app.add_option("models", options.models, "The model files to convert")
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

  if (!std::filesystem::exists(options.outputPath)) {
    std::cout << "Output directory does not exist, creating\n";
    std::error_code ec;
    if (!std::filesystem::create_directories(options.outputPath, ec) || ec) {
      std::cerr << "Cannot create output directory\n";
      return {};
    }
  }
  if (!std::filesystem::is_directory(options.outputPath)) {
    std::cerr << "Output is not a directory\n";
    return {};
  }

  options.models = getModels(recurse, options.models);
  options.customExtentionMappings = getCustomMappings(customExtensionMappings);

  return options;
}

static std::filesystem::path stripRexsExtension(std::filesystem::path path)
{
  if (path.stem().has_extension()) {
    path.replace_extension("");
  }
  return path.replace_extension("");
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
      if (!model) {
        std::cerr << "Error: could not load model " << modelFile << std::endl;
      } else {
        result.reset();
        auto file{stripRexsExtension(modelFile.filename())};
        switch (options->type) {
          case rexsapi::TFileType::JSON:
            rexsapi::TModelSaver{}.store(result, *model, options->outputPath / file.replace_extension(".rexsj"),
                                         rexsapi::TSaveType::JSON);
            break;
          case rexsapi::TFileType::XML:
            rexsapi::TModelSaver{}.store(result, *model, options->outputPath / file.replace_extension(".rexs"),
                                         rexsapi::TSaveType::XML);
            break;
          default:
            throw rexsapi::TException{fmt::format("Format is not implemented ({})", file.extension().string())};
        }
        if (!result) {
          std::cerr << fmt::format("Could not store {} to {}", modelFile.string(),
                                   std::filesystem::canonical(options->outputPath / file).string())
                    << std::endl;
        } else {
          std::cout << fmt::format("Converted {} to {}", modelFile.string(),
                                   std::filesystem::canonical(options->outputPath / file).string())
                    << std::endl;
        }
      }
    }
  } catch (const std::exception& ex) {
    std::cerr << "Exception caught: " << ex.what() << std::endl;
  }

  return 0;
}
