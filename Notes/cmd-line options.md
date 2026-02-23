
# Basic Options

| Action | Short   | Long                 | Description                                                |
| :----: | ------- | -------------------- | ---------------------------------------------------------- |
|   󱏘   | -o PATH |                      | Prefix PATH to all generated files                         |
|   󱏘   | -I PATH |                      | Search for includes in the specified path                  |
|   󰜺   | -M      |                      | Print make rules for generated files                       |
|   󱏘   |         | --version            | Print the version number of flatc and exit                 |
|   󱏘   | -h      | --help               | Prints this help text and exit                             |
|   󱏘   |         | --file-names-only    | Print out generated file names without writing to the file |
|   󱏘   |         | --no-warnings        | Inhibit all warnings messages                              |
|   󱏘   |         | --warnings-as-errors | Treat all warnings as errors                               |

# Input File Options
## Binary
| flag            | description                                                                                       |
| --------------- | ------------------------------------------------------------------------------------------------- |
| --raw-binary    | Allow binaries without file_identifier to be read. This may crash flatc given a mismatched schema |
| --size-prefixed | Input binaries are size prefixed buffers                                                          |
## Schema's( fbs, bfbs, JSON, proto)
| Action | flag                    | description                                                                          |
| :----: | ----------------------- | ------------------------------------------------------------------------------------ |
|   󱏘   | --require-explicit-ids  | When parsing schemas, require explicit ids (id: x)                                   |
|       | --conform FILE          | Specify a schema the following schemas should be a evolution of. Gives errors if not |
|       | --conform-includes PATH | Include path for the schema given with --conform PATH                                |
### JSON
| flag             | description                                                                                                                      |
| ---------------- | -------------------------------------------------------------------------------------------------------------------------------- |
| --allow-non-utf8 | Pass non-UTF-8 input through parser and emit nonstandard \\x escapes in JSON. (Default is to raisparse error on non-UTF-8 input. |
| --strict-json    | Strict JSON: field names must be / will be quoted, ntrailing commas in tables/vectors                                            |
### .proto

| flag                            | description                                                                                                                                                                                                                                                                               |
| ------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| --proto                         | Input is a .proto, translate to .fb                                                                                                                                                                                                                                                       |
| --proto-id-gap                  | Action that should be taken when a gap between protobu ids found. Supported values: * 'nop' - do not car about gap * 'warn' - A warning message will be show about the gap in protobuf ids(default) * 'error' - A  error message will be shown and the fbs generation wil  be interrupted |
| --proto-namespace-suffix SUFFIX | Add this namespace to any flatbuffers generated fro protobufs                                                                                                                                                                                                                             |
| --keep-proto-id                 | Keep protobuf field ids in generated fbs file                                                                                                                                                                                                                                             |
| --oneof-union                   | Translate .proto oneofs to flatbuffer unions                                                                                                                                                                                                                                              |
# Operational Modes

## Generate Schema

| Short | Long         | Description          |
| ----- | ------------ | -------------------- |
|       | --jsonschema | Generate Json schema |
## Generate Wire-Format Binary from Data

| Short | Long     | Description                                           |
| ----- | -------- | ----------------------------------------------------- |
| -b    | --binary | Generate wire format binaries for any data definition |
| -t    | --json   | Generate text output for any data definition          |

| Short | Long                      | Description                                                                                                     |
| ----- | ------------------------- | --------------------------------------------------------------------------------------------------------------- |
|       | --defaults-json           | Output fields whose value is the default when writin JSON                                                       |
|       | --schema                  | Serialize schemas instead of JSON (use with -b)                                                                 |
|       | --unknown-json            | Allow fields in JSON that are not defined in the schema. These fields will be discarded when generatin binaries |
|       | --flexbuffers             | Used with \"binary\" and \"json\" options, it generate data using schema-less FlexBuffers                       |
|       | --force-defaults          | Emit default values in binary output from JSON                                                                  |
|       | --annotate SCHEMA         | Annotate the provided BINARY_FILE with the specifie SCHEMA file                                                 |
|       | --annotate-sparse-vectors | Don't annotate every vector element                                                                             |
## Generate Code
This is really the only side of the situation I care about.

| Action | Short | Long         | Description                                            |
| ------ | ----- | ------------ | ------------------------------------------------------ |
| 󱏘     | -c    | --cpp        | Generate C++ headers for tables/struct                 |
|       | -n    | --csharp     | Generate C# classes for tables/struct                  |
| 󰜺     | -d    | --dart       | Generate Dart classes for tables/struct                |
| 󱏘     |       | --gdscript   | Generate GDScript files for tables/struct              |
| 󰜺     | -g    | --go         | Generate Go files for tables/struct                    |
| 󰜺     | -j    | --java       | Generate Java classes for tables/struct                |
| 󰜺     |       | --kotlin     | Generate Kotlin classes for tables/struct              |
| 󰜺     |       | --kotlin-kmp | Generate Kotlin multiplatform classes fo tables/struct |
| 󰜺     |       | --lobster    | Generate Lobster files for tables/struct               |
|       | -l    | --lua        | Generate Lua files for tables/struct                   |
| 󰜺     |       | --nim        | Generate Nim files for tables/struct                   |
| 󰜺     |       | --php        | Generate PHP files for tables/struct                   |
|       | -p    | --python     | Generate Python files for tables/struct                |
|       | -r    | --rust       | Generate Rust files for tables/struct                  |
|       |       | --swift      | Generate Swift files for tables/struct                 |
| 󰜺     | -T    | --ts         | Generate TypeScript code for tables/struct             |

# Output Options

## Output File

| action | flag                     | description                                                                                          |
| :----: | :----------------------- | ---------------------------------------------------------------------------------------------------- |
|   󱏘   | --filename-ext EXT       | The extension appended to the generated file names Default is language-specific (e.g., '.h' for C++) |
|   󱏘   | --filename-suffix SUFFIX | The suffix appended to the generated file name  (Default is '_generated')                            |


| action | flag                          | description                                                                                                                                                                                                          |
| :----: | :---------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
|   󱏘   | --root-type T                 | Select or override the default root_type                                                                                                                                                                             |
|       | --reflect-names               | Add minimal type/name reflection                                                                                                                                                                                     |
|       | --reflect-types               | Add minimal type reflection to code generation                                                                                                                                                                       |
|   󱏘   | --gen-all                     | Generate not just code for the current schema files but for all files it includes as well. If the language uses a single file for output (by default the case for C++ and JS), all code will end up in this one file |
|   󱏘   | --gen-includes                | (deprecated), this is the default behavior. If the original behavior is required (no include statements use --no-includes                                                                                            |
|   󱏘   | --gen-mutable                 | Generate accessors that can mutate buffers in-place                                                                                                                                                                  |
|   󱏘   | --gen-onefile                 | Generate a single output file for C#, Go, Java, Kotli and Python. Implies --no-include                                                                                                                               |
|   󱏘   | --include-prefix PATH         | Prefix this PATH to any generated include statements                                                                                                                                                                 |
|   󱏘   | --keep-prefix                 | Keep original prefix of schema include statement                                                                                                                                                                     |
|       | --gen-json-emit               | Generates encoding code which emits Flatbuffers int JSON                                                                                                                                                             |
|   󰜺   | --no-emit-min-max-enum-values | Disable generation of MIN and MAX enumerated values for scoped enums and prefixed enums                                                                                                                              |
|        | --natural-utf8                | Output strings with UTF-8 as human-readable strings. By default, UTF-8 characters are printed as \\uXXX escapes                                                                                                      |
## Object API

| action | flag                   | description                                                                                           |
| :----: | :--------------------- | ----------------------------------------------------------------------------------------------------- |
|   󱏘   | --gen-object-api       | Generate an additional object-based API                                                               |
|   󱏘   | --object-prefix PREFIX | Customize class prefix for C++ object-based API                                                       |
|   󱏘   | --object-suffix SUFFIX | Customize class suffix for C++ object-based API  Default Value is \"T\"                               |
|   󱏘   | --gen-compare          | Generate operator == for object-based API types                                                       |
|       | --force-empty          | When serializing from object API representation, force  strings and vectors to empty rather than null |
## GRPC

| action | flag                          | description                                                                     |
| :----: | ----------------------------- | ------------------------------------------------------------------------------- |
|       | --grpc                        | Generate GRPC interfaces for the specified languages                            |
|       | --grpc-additional-header      | Additional headers to prepend to the generated files                            |
|       | --grpc-callback-api           | Generate gRPC code using the callback (reactor) AP instead of legacy sync/async |
|       | --grpc-filename-suffix SUFFIX | The suffix for the generated file names (Default i  '.fb')                      |
|       | --grpc-python-typed-handlers  | The handlers will use the generated classes rather tha  raw bytes               |
|       | --grpc-search-path PATH       | Prefix to any gRPC includes                                                     |
|   󰜺   | --grpc-use-system-headers     | Use <> for headers included from the generated code                             |
## BinaryFBS

| action | flag                  | description                                                                                                                                                                                                                                                |
| :----: | --------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
|       | --bfbs-absolute-paths | Uses absolute paths instead of relative paths in the BFBS output                                                                                                                                                                                           |
|       | --bfbs-builtins       | Add builtin attributes to the binary schema files                                                                                                                                                                                                          |
|       | --bfbs-comments       | Add doc comments to the binary schema files                                                                                                                                                                                                                |
|       | --bfbs-filenames PATH | Sets the root path where reflection filenames i  reflection.fbs are relative to. The 'root' is denote  with`//`. E.g. if PATH=/a/b/c then /a/d/e.fbs will b  serialized as //../d/e.fbs. (PATH defaults to th  directory of the first provided schema file |
|       | --bfbs-gen-embed      | Generate code to embed the bfbs schema to the source                                                                                                                                                                                                       |
## Languages

| action | flag                         | description                                                                                                                 |
| :----: | :--------------------------- | --------------------------------------------------------------------------------------------------------------------------- |
|       | --gen-name-strings           | Generate type name functions for C++ and Rust                                                                               |
|   󰜺   | --gen-nullable               | Add Clang _Nullable for C++ pointer. or @Nullable for Java                                                                  |
|       | --no-includes                | Don't generate include statements for included schema the generated file depends on (C++, Python Proto-to-Fbs)              |
|       | --no-leak-private-annotation | Prevents multiple type of annotations within a Fb SCHEMA file. Currently this is required to generate private types in Rust |
### C++

| action | flag                         | description                                                                                                                                                                                                                                             |
| :----: | ---------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
|       | --cpp-field-case-style STYLE | Generate C++ fields using selected case style Supported STYLE values: * 'unchanged' - leave unchange(default) * 'upper' - schema snake_case emitUpperCamel; * 'lower' - schema snake_case emitlowerCamelC++17 features in generated code (experimental) |
|   󰜺   | --cpp-include                | Adds an #include in generated file                                                                                                                                                                                                                      |
|   󰜺   | --cpp-ptr-type T             | Set object API pointer type (default std::unique_ptr)                                                                                                                                                                                                   |
|   󰜺   | --cpp-static-reflection      | When using C++17, generate extra code to provid compile-time (static) reflection of Flatbuffers types Requires --cpp-std to be \"c++17\" or higher                                                                                                      |
|   󰜺   | --cpp-std CPP_STD            | Generate a C++ code using features of selected C+  standard. Supported CPP_STD values: * 'c++0x' generate code compatible with old compilers; 'c++11'  use C++11 code generator (default); * 'c++17' - us                                               |
|       | --cpp-str-flex-ctor          | Don't construct custom string types by passinstd::string from Flatbuffers, but (char* + length)                                                                                                                                                         |
|   󱏘   | --cpp-str-type T             | Set object API string type (default std::string) T::c_str(), T::length() and T::empty() must b supported. The custom type also needs to bconstructible from std::string (see th option to change this behavior)                                         |
|   󰜺   | --scoped-enums               | Use C++11 style scoped and strongly typed enums. Also implies --no-prefix                                                                                                                                                                               |
|   󰜺   | --no-prefix                  | Don't prefix enum values with the enum type in C++                                                                                                                                                                                                      |
### C\#

| action | flag                     | description                                                                           |
| :----: | ------------------------ | ------------------------------------------------------------------------------------- |
|       | --cs-gen-json-serializer | Allows (de)serialization of JSON text in the Object  API. (requires --gen-object-api) |
|       | --cs-global-alias        | Prepend \"global::\" to all user generated csharp classe and structs                  |

### GDScript

| action | flag             | description                                            |
| :----: | ---------------- | ------------------------------------------------------ |
|   󱏘   | --gdscript-debug | Generate an additional debug code to help troubleshoot |
### Go

| action | flag               | description                                                                                               |
| :----: | ------------------ | --------------------------------------------------------------------------------------------------------- |
|   󰜺   | --go-import IMPORT | Generate the overriding import for flatbuffers i Golang (default is \"github.com/google/flatbuffers/go\") |
|   󰜺   | --go-module-name   | Prefix local import paths of generated go code with th module nam                                         |
|   󰜺   | --go-namespace     | Generate the overriding namespace in Golang                                                               |

### Java

| action | flag                    | description                                                                                                                       |
| :----: | ----------------------- | --------------------------------------------------------------------------------------------------------------------------------- |
|   󰜺   | --java-checkerframework | Add @Pure for Java                                                                                                                |
|       | --java-package-prefix   | Add a prefix to the generated package name for Java                                                                               |
|       | --json-nested-bytes     | Allow a nested_flatbuffer field to be parsed as  vector of bytes in JSON, which is unsafe unless checke  by a verifier afterwards |
|   󰜺   | --gen-generated         | Add @Generated annotation for Java                                                                                                |
### Kotlin

| action | flag            | description                                                                                    |
| :----: | :-------------- | ---------------------------------------------------------------------------------------------- |
|   󰜺   | --gen-jvmstatic | Add @JvmStatic annotation for Kotlin methods icompanion object for interop from Java to Kotlin |
### Python

| action | flag                            | description                                                       |
| :----: | ------------------------------- | ----------------------------------------------------------------- |
|       | --python-decode-obj-api-strings | Decode bytes to strings for the Python Object AP                  |
|   󰜺   | --python-gen-numpy              | Whether to generate numpy helpers                                 |
|       | --python-no-type-prefix-suffix  | Skip emission of Python functions that are prefixe  with typename |
|   󰜄   | --python-typing                 | Generate Python type annotation                                   |
|   󰜄   | --python-version                | Generate code for the given Python version                        |
### Rust

| action | flag                    | description                                                   |
| :----: | ----------------------- | ------------------------------------------------------------- |
|   󰜺   | --rust-module-root-file | Generate rust code in individual files with a modul root file |
|   󰜺   | --rust-serialize        | Implement serde::Serialize on generated Rust types            |
### Swift

| action | flag                        | description                                 |
| :----: | --------------------------- | ------------------------------------------- |
|   󰜺   | --swift-implementation-only | Adds a @_implementationOnly to swift import |
### TypeScript

| action | flag                 | description                                |
| :----: | -------------------- | ------------------------------------------ |
|   󰜺   | --ts-entry-points    | (deprecated) Alias for --gen-all           |
|   󰜺   | --ts-flat-files      | (deprecated) Alias for --gen-all           |
|   󰜺   | --ts-omit-entrypoint | Omit emission of namespace entrypoint file |

# Usage String

FILE's may be schemas (must end in .fbs), binary schemas (must end in .bfbs) or
JSON files (conforming to preceding schema). BINARY_FILEs after the -- must be binary flatbuffer format files.
Output files are named using the base file name of the input, and written to the current directory or the path given by -o

example:

C:\\Godot\\Projects\\Enetheru.flatbuffers-project\\addons\\enetheru.flatbuffers-extesion\\bin\\flatc.exe -c -b schema1.fbs schema2.fbs data.json

# Key
- 󱏘  󰡖 󰄲 - Likely, planned, in progress, completed
- 󰩴 󰩳 - Favourites
- 󰅗 󰅘 - cant or wont implement
- 󰐖 󰜄 - add
- 󰍵 󰛲 - remove
- 󰋒 󰋓 - love
- 󰎔   - new
- 󰀧 󰳤 - alert
- 󰜺   - not applicable.
-    - needs more research
