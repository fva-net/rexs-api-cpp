# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.1.0]

### Changed

- Switched matrix encoding from row-major to column-major order (#9)
- JSON model files will now be written with a UTF-8 BOM (#11)
- The model registry can be configured to load the latest available model version, if the requested version is not
  known. If no appropriate language can be found, english will be chosen. (#8)
- In relaxed mode latest available model version loading will be active (#8)
- Updated all thirdparty components (#7)
- Add changelog to the package (#16)

## [1.0.0] - 2022-11-09

### Added

- First release
