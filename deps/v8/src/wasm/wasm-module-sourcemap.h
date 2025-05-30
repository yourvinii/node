// Copyright 2019 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_WASM_WASM_MODULE_SOURCEMAP_H_
#define V8_WASM_WASM_MODULE_SOURCEMAP_H_

#if !V8_ENABLE_WEBASSEMBLY
#error This header should only be included if WebAssembly is enabled.
#endif  // !V8_ENABLE_WEBASSEMBLY

#include <string>
#include <vector>

#include "include/v8-local-handle.h"
#include "src/base/macros.h"

namespace v8 {

class String;

namespace internal {
namespace wasm {
// The class is for decoding and managing source map generated by a WebAssembly
// toolchain (e.g. Emscripten). This implementation mostly complies with the
// specification (https://sourcemaps.info/spec.html), with the following
// accommodations:
// 1. "names" field is an empty array in current source maps of Wasm, hence it
// is not handled;
// 2. The semicolons divides "mappings" field into groups, each of which
// represents a line in the generated code. As *.wasm is in binary format, there
// is one "line" of generated code, and ";" is treated as illegal symbol in
// "mappings".
// 3. Though each comma-separated section may contains 1, 4 or 5 fields, we only
// consider "mappings" with 4 fields, i.e. start line of generated code, index
// into "sources" fields, start line of source code and start column of source
// code.
class V8_EXPORT_PRIVATE WasmModuleSourceMap {
 public:
  WasmModuleSourceMap(v8::Isolate* v8_isolate,
                      v8::Local<v8::String> src_map_str);

  // Member valid_ is true only if the source map complies with specification
  // and can be correctly decoded.
  bool IsValid() const { return valid_; }

  // Given a function located at [start, end) in Wasm Module, this function
  // checks if this function has its corresponding source code.
  bool HasSource(size_t start, size_t end) const;

  // Given a function's base address start and an address addr within, this
  // function checks if the address can be mapped to an offset in this function.
  // For example, we have the following memory layout for Wasm functions, foo
  // and bar, and O1, O2, O3 and O4 are the decoded offsets of source map:
  //
  // O1 --- O2 ----- O3 ----- O4
  // --->|<-foo->|<--bar->|<-----
  // --------------A-------------
  //
  // Address A of function bar should be mapped to its nearest lower offset, O2.
  // However, O2 is an address of function foo, thus, this mapping is treated as
  // invalid.
  bool HasValidEntry(size_t start, size_t addr) const;

  // This function is responsible for looking up an offset's corresponding line
  // number in source file. It should only be called when current function is
  // checked with IsValid, HasSource and HasValidEntry.
  size_t GetSourceLine(size_t wasm_offset) const;

  // This function is responsible for looking up an offset's corresponding
  // source file name. It should only be called when current function is checked
  // with IsValid, HasSource and HasValidEntry.
  std::string GetFilename(size_t wasm_offset) const;

  size_t EstimateCurrentMemoryConsumption() const;

 private:
  std::vector<size_t> offsets;
  std::vector<std::string> filenames;
  std::vector<size_t> file_idxs;
  std::vector<size_t> source_row;
  // As column number in source file is always 0 in source map generated by
  // WebAssembly toolchain, we will not store this value.

  bool valid_ = false;

  bool DecodeMapping(const std::string& s);
};
}  // namespace wasm
}  // namespace internal
}  // namespace v8
#endif  // V8_WASM_WASM_MODULE_SOURCEMAP_H_
