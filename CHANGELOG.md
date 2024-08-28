# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [2.1.0]

### Fixed

- Code now builds with C++20 and C++23 (#53)
- Clang format style file fixed (#54)

## [2.0.0]

### Fixed

- Fixed README.md
- Fixed doxygen docu regarding brief description not rendering correctly in Visual Studio (#25)

### Changed

- Updated thirdparty components (#26)
- Upgraded cmake version to 3.22 (#27)
- Upgraded github CI os and compiler (#27)
- Upgraded github CI actions (#28)
- Except for the TModelInfo all other TModel constructor parameters have to be forcibly moved now
- The rexsapi tools currently do not support the resolving of external component references and should be
  used with --mode-relaxed if references lead to problems
- TModelLoader now takes an optional TDataSourceResolver to resolve external component references (#31)
- Analogous to the TModelLoader, TJsonModelLoader and TXMLModelLoader also take an optional TDataSourceResolver

### Added

- External ids will be used for better error messages for easier finding a specific erroneous component in a 
  model file (#2)
- Components can have an external id, which represents exactly the components id specified in a model file (#4)
- Added model database file for 1.6 models (#30)
- References into external data sources can now be used for REXS models >= version 1.5 (#31)
- External sub components checker (#31)
- New check method for main components to TRelationTypeChecker (#36)
- TModelMerger to merge multiple TModel instances into a new instance (#36)
- The TModelBuilder can now use user supplied integers as ids

## [1.1.0]

### Changed

- Switched matrix encoding from row-major to column-major order (#9)
- JSON model files will now be written with a UTF-8 BOM (#11)
- The model registry can be configured to load the latest available model version, if the requested version is not
  known. If no appropriate language can be found, english will be chosen. (#8)
- In relaxed mode latest available model version loading will be active (#8)
- Added model database file for 1.5 models (#12)
- Added new date_time data type (#20)
- Updated all thirdparty components (#7)
- Add changelog to the package (#16)

## [1.0.0] - 2022-11-09

### Added

- First release
