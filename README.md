![Supported Platforms](https://img.shields.io/badge/platforms-Linux%20%7C%20Windows%20%7C%20Mac-blue.svg)
![License: Apache 2](https://img.shields.io/badge/license-Apache%202-blue)
![Language: C++17](https://img.shields.io/badge/language-C%2B%2B17-blue.svg)
![Version:](https://img.shields.io/badge/version-1.0.0-green)
[![GitHub Build Status](https://github.com/fva-net/rexs-api-cpp/workflows/CMake%20Build%20Matrix/badge.svg)](https://github.com/fva-net/rexs-api-cpp/actions)
[![Coverage Status](https://coveralls.io/repos/github/BearinxSimulationSuite/REXSapi/badge.svg?branch=main)](https://coveralls.io/github/BearinxSimulationSuite/REXSapi?branch=main)

# rexs-api-cpp

The REXSapi library is a C++ implementation of the [REXS specification](https://www.rexs.info/rexs_de.html). It features high level abstractions for REXS models and provides classes to load, access, create, and store REXS models. Additionally, the API tries to check the model for standard compliance and will report on found issues.

# Status

The project is now in beta phase and the API should be stable. You can checkout the library and play with it. Probably still a bit early for production use, though.

# Supported REXS Versions

The library uses REXS database model files in order to validate REXS model files. Database model files can be downloaded from the [REXS database page](https://database.rexs.info/). Currently, the implementation supports versions 1.0 to 1.4, but newer database files should also work. Version 1.0 to 1.4 database model files in english and german can also be found in the models directory of this project.

The library supports REXS model files in xml and json format. Compressed REXS zip archives can also be loaded. The loading and storing mechanism can be easily extended to support other sources besides files for model loading and storing.

# Supported Platforms

The library can be used on
- Linux
- Windows
- Mac OS X

You need a C++17 compatible compiler to use the library.

# Getting Started

In order to use REXSapi it is most convinient to just include the `Rexsapi.hxx` header. Mind that you have to include this header along the `REXSAPI_MINIZ_IMPL` define right before the include in *exactly* one compilation unit (cpp file) in order to add the miniz implementation to the project.

```c++
#define REXSAPI_MINIZ_IMPL
#include <rexsapi/Rexsapi.hxx>
```

## Load a REXS Model File

Loading a REXS model file is straight forward. You need the REXS database model files for the API to validate the model.

```c++
const rexsapi::TModelLoader loader{"/path/to/rexs/database/models"};
rexsapi::TResult result;
const std::optional<rexsapi::TModel> model = 
        loader.load("/path/to/your/rexs/model/file", result, rexsapi::TMode::STRICT_MODE);
if (result) {
  std::cout << "sucessfully loaded REXS model\n";
} else {
  std::cerr << "failed to load REXS model\n";
  for (const auto& issue : result.getErrors()) {
    std::cerr << issue.getMessage() << "\n";
  }
}
```

The `TModelLoader` class can load json and xml REXS model files. If successful, the result will convert to true and the model optional will contain a model. In case of a failure, the result will contain a collection of messages describing the issues. The issues can either be errors or warnings. It is perfectly possible that the result converts to false, a failure, but the model optional contains a model. This means that the model could be loaded in general, but that there are issues with the model like incorrect value types, missing references, etc.

## Working With a REXS Model

The Model itself provides methods for accessing every aspect of a model.

```c++
const rexsapi::TModel model = loadModel();
for (const auto& relation : model.getRelations()) {
  for (const auto& ref : relation.getReferences()) {
    for (const auto& comp : ref.getComponents()) {
      std::cout << comp.getType() << "\n";
      for (const auto& att : comp.getAttributes()) {
        std::cout << "\t" << att.getAttributeId() << "\n";
      }
    }
  }
}
```

Alternatively, if the model shall be processed in a complete way, the `TModelVisitor` class can be used. It allows access to the complete model without the need for explicit iteration. Check the `ModelVisitorTest` test and the `model_dumper` tool for examples.

## Create a REXS Model

The most reliable way to create a REXS model is to use the `TModelBuilder` class. It can create every aspect of a model, be it components, relations, or load spectrum, of a REXS model and highly abstracts the construction of REXS standard compliant models. The `TModelBuilder` needs a specific REXS database model for checking and validating the model. Most model builder methods return a reference to the model builder in order to allow chaining of method calls resulting in dense easy to read code.

```c++
const auto registry = rexsapi::createModelRegistry("/path/to/rexs/database/models");
const auto& databaseModel = registry.getModel(rexsapi::TRexsVersion{"1.4"}, "en");
rexsapi::TModelBuilder modelBuilder{databaseModel};
modelBuilder.addComponent("concept_bearing", "bearing-id").name("Bearing No. 1");
modelBuilder.addAttribute("axial_force_absorption").value("negative");
modelBuilder.addAttribute("support_vector").unit("mm").value(
        rexsapi::TFloatArrayType{70.0, 0.0, 0.0}).coded();
modelBuilder.addComponent("gear_casing", "gear-casing-id");
        
... [more attributes and components]

modelBuilder.addRelation(rexsapi::TRelationType::SIDE)
    .addRef(rexsapi::TRelationRole::ASSEMBLY, "bearing-id")
    .addRef(rexsapi::TRelationRole::INNER_PART, "gear-casing-id");
    .addRef(rexsapi::TRelationRole::OUTER_PART, "shaft-id");
    
... [more relations]

auto model = modelBuilder.build("REXSApi Model Builder", "1.0", "en");
```
First you add components and attributes to the model. The last added component or attribute are the so called active component or attribute. All following method calls always affect the currently active component or attribute. The same is true for relations. Attributes need real C++ types as values.

If all necessary components have been added, you can start to add relations that reference these components via unique string ids. Alternatively, you can also use automatically generated ids that can be retrieved from the model builder after adding a component. You have to specify the correct roles for a specific relation type. The model builder will check all components, attributes, and relations against the chosen REXS database model version and will throw exceptions on any error in order to guarantee the construction of a standard compliant model.

## Save a REXS Model to a File

Saving a REXS model to a file is straight forward using the `TModelSaver` convenience class.

```c++
const rexsapi::TModel model = createModel();
rexsapi::TResult result;
rexsapi::TModelSaver{}.store(result, model, "/path/to/your/rexs/model/file",
        rexsapi::TSaveType::XML);
if (result) {
  std::cout << "sucessfully stored REXS model\n";
} else {
  std::cerr << "failed to store REXS model\n";
}
```

The `TModelSaver` class can store REXS models as xml or json. If successful, the result will convert to true and the model is stored. In case of a failure, the result will contain a collection of messages describing the issues. The file path to store the model to does not need any extension, the `TModelSaver` will assign the correct extension automatically.

# Tools

The library comes packaged with three tools: `model_converter`, `model_checker`, and `model_dumper`. The tools can come in handy when working with rexs model files and can also serve as examples how to use the library.

## model_checker

The `model_checker` checks files for compatibility with the REXSapi library. You can check complete directories with one go. The library expects REXS model files to be conformant to the REXS specification. However, the library supports a so called _relaxed_ mode where most errors to the specfication are turned into warnings in order to process files even if they are not 100% compliant to the specifcation. The tool will print the found issues to the console. You can use the tools output to fix problems in the files.

### Options

| Option | Description |
|:--|:--|
| --help, -h | Show usage and options |
| --mode-strict | This is the default mode. Files will be checked to comply strictly to the standard. |
| --mode-relaxed | This mode will relax the checking and produce warnings instead of errors for non-standard constructs. |
| --warnings, -w | Enables the printing of warnings to the console. Otherwise, only errors will be printed. |
| -r | If directories are specified as arguments, recurse into sub-directories. |
| -m | Custom file extension mapping of the form ".rexs.in:xml". Will load files with the extension ".rexs.in" as xml files. Can be specified multiple times, but has to precede some other option or be terminated with --.
| --database, -d | The path to the model database files including the schemas (json and xml). |
| | Files and directories to look for model files to process. |

```bash
> ./model_checker --mode-relaxed -d ../models -m .rexs.in:xml -- FVA-Industriegetriebe_2stufig_1-4.rexs
File ".FVA-Industriegetriebe_2stufig_1-4.rexs" processed with 10 warnings
```

## model_converter

The `model_converter` can convert REXS model files between xml and json format. Files can be converted in any direction, even into the same format. You can convert complete directories with one go. As with the `model_checker`, the tool supports a relaxed mode for loading non-standard complying model files. If files do not conform to the standard, converting them may result in removed elements.

### Options
| Option | Description |
|:--|:--|
| --help, -h | Show usage and options |
| --mode-strict | This is the default mode. Files will be checked to comply strictly to the standard. |
| --mode-relaxed | This mode will relax the checking and produce warnings instead of errors for non-standard constructs. |
| --format, -f | The output format of the tool. Either json or xml. |
| -r | If directories are specified as arguments, recurse into sub-directories. |
| -m | Custom file extension mapping of the form ".rexs.in:xml". Will load files with the extension ".rexs.in" as xml files. Can be specified multiple times, but has to precede some other option or be terminated with --.
| --output, -o | The output path to write converted file to. |
| --database, -d | The path to the model database files including the schemas (json and xml). |
| | Files and directories to look for model files to process. |

```bash
> ./model_converter --mode-relaxed -f json -d ../models -m .rexs.in:xml --output /out FVA-Industriegetriebe_2stufig_1-4.rexs
Converted FVA-Industriegetriebe_2stufig_1-4.rexs to /out/FVA-Industriegetriebe_2stufig_1-4.rexsj
```

## model_dumper

The `model_dumper` prints the loaded model to the console. As with the `model_checker`, the tool supports a relaxed mode for loading non-standard complying model files. The tool can be used to check if the specific model can be loaded with the REXSapi correctly.

### Options
| Option | Description |
|:--|:--|
| --help, -h | Show usage and options |
| --mode-strict | This is the default mode. Files will be checked to comply strictly to the standard. |
| --mode-relaxed | This mode will relax the checking and produce warnings instead of errors for non-standard constructs. |
| -m | Custom file extension mapping of the form ".rexs.in:xml". Will load files with the extension ".rexs.in" as xml files. Can be specified multiple times, but has to precede some other option or be terminated with --.
| -s | Show some statistics of the model |
| --attributes, -a | Show the component attributes |
| --database, -d | The path to the model database files including the schemas (json and xml). |
| | The REXS model file to dump. |

```bash
> ./model_dumper --mode-relaxed -d ../models FVA-Industriegetriebe_2stufig_1-4.rexs
File "/Users/lfuerste/Development/REXSapi/test/example_models/FVA-Industriegetriebe_2stufig_1-4.rexs"

ApplId: 'Bearinx' ApplVer: 12.0.9241 (sandbox development) Date: 2022-04-21T11:42:31+01:00 REXSVer: 1.4

Relations
...
```

# Integration

The library is header only and can be easily integrated into existing projects. Using CMake is the recommended way to use the library. However, the library also comes as a zip package which can be used without CMake. You have to set the C++ standard of your project to C++17 in order to build with the library. 

The library has dependencies to other open source software. This dependencies will be either automatically downloaded by CMake or are prepackaged in the zip package. You can also use your own versions of the thirdparty libraries, but make sure the versions are compatible to the versions used by the REXSapi.

## CMake

Just clone the git repository and add REXSapi as a sub directory in an appropriate CMakeLists.txt file. Then use the provided rexsapi interface as library. If you want to build with the examples, tools or the tests, you can set `BUILD_WITH_EXAMPLES`, `BUILD_WITH_TESTS`, and/or `BUILD_WITH_TOOLS` to `ON`.

```cmake
set(CMAKE_CXX_STANDARD 17)
add_executable(test
  main.cxx
)
set(BUILD_WITH_EXAMPLES ON)
add_subdirectory(REXSapi)
target_link_libraries(test PRIVATE rexsapi)
```

Alternatively, you can use CMakes FetchContent module to download REXSapi and make the projects interface available.

```cmake
include(FetchContent)
FetchContent_Declare(
  rexsapi
  GIT_REPOSITORY https://github.com/fva-net/rexs-api-cpp.git
  GIT_TAG origin/v1.0.0
)

FetchContent_MakeAvailable(rexsapi)
```

## Package

If you do not want to use CMake, you can download a REXSapi zip package. The package contains all necessary header files, including all dependencies. In order to build a REXSapi project, unzip the archive and add the resulting directories `include` and `deps/include` directory as additional header search directories to your build.

# Build Instructions

The library is header only. A build is only necessary if you want to run the tests or build the examples or tools.

## Linux

- You will need the following software packages
  - g++ 9.3.0 or higher
  - cmake 3.16.3 or higher
- To install the dependencies on ubuntu
  - Call `sudo apt-get install cmake g++`
- Create a build directory in the source directory of REXSapi and change to it
- Call `cmake -DCMAKE_BUILD_TYPE=Release ..`
- Call `cmake --build . --config Release`
- To run the tests
  - Call `ctest -VV -C Release`

## Windows

- You will need the following software packages
  - Visual Studio 2019 or higher
- CMake support is build into Visual Studio 2019
- Open the local folder of the REXSapi project
- Visual Studio will configure the project automatically

## Mac
- You will need the following software packages
  - XCode 12.4 or higher
  - cmake 3.16.3 or higher
- To install cmake on Mac
  - Call `brew install cmake`
- Create a build directory in the source directory of REXSapi and change to it
- Call `cmake -GXcode -DCMAKE_BUILD_TYPE=Release ..`
- Call `cmake --build . --config Release`
- To run the tests
  - Call `ctest -VV -C Release`

## External Dependencies

REXSapi uses the following thirdparty open source software

- [cli11 2.2.0](https://github.com/CLIUtils/CLI11)
- [fmt 8.1.1](https://github.com/fmtlib/fmt)
- [nlohmann json 3.10.5](https://github.com/nlohmann/json)
- [miniz 2.2.0](https://github.com/richgel999/miniz)
- [pugixml 1.12.1](https://github.com/zeux/pugixml)
- [valijson 0.6](https://github.com/tristanpenman/valijson)
- [doctest 2.4.9](https://github.com/doctest/doctest)

# License
REXsapi is licensed under the Apache-2.0 license.

See the LICENSE file for more information.
