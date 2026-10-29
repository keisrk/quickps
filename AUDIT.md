# Security Audit Report: `quickps` & `subprojects/paperjs`

**Audit Scope**: `quickps/`, `libquickps/`, `examples/`, build scripts (`meson.build`, `subprojects/*.wrap`), and `subprojects/paperjs/` (including `subprojects/packagefiles/paperjs/`).  
**Out of Scope**: `QuickJS`, `AsmJit`, and `Blend2D` upstream internals.

---

## Executive Summary

| ID | Severity | Component | Vulnerability Title | CWE |
| :--- | :--- | :--- | :--- | :--- |
| **CPP-01** | **High** | `libquickps/quickjsmm` & `canvas/facade` | Out-of-Bounds `argv` Read & Unchecked Argument Dereference in `ValueIter` | CWE-125, CWE-476 |
| **CPP-02** | **High** | `libquickps/quickjsmm/esm.hpp` | C++ Exceptions Thrown Across QuickJS C Stack Frames (ABI Undefined Behavior / DoS) | CWE-248, CWE-755 |
| **BLD-01** | **High** | `examples/scripts/esbuild.py` | Unverified Binary Download, Predictable `/tmp` Execution (TOCTOU), and Unfiltered Tar Extraction | CWE-494, CWE-377, CWE-22 |
| **BLD-02** | **High** | `subprojects/paperjs/meson.build` & `examples/meson.build` | Unverified External JS Dependencies Fetched via `curl --config` | CWE-494, CWE-829 |
| **BLD-03** | **High** | `subprojects/paperjs/scripts/prepro.py` | Heap Metadata Corruption (`v.u.ptr - 4`), C-String Leaks, Path Traversal, and Multi-Macro Substitution Bug | CWE-787, CWE-401, CWE-22 |
| **PJS-01** | **High** | `subprojects/paperjs/src/core/Base.js` & `Tween.js` | Prototype & Property Pollution via `Base#set()`, `Base.importJSON()`, and `Tween` | CWE-1321, CWE-502 |
| **PJS-02** | **High** | `subprojects/paperjs/src/core/PaperScript.js` & `quickps.cpp` | Dynamic Code Injection (`options.url`/`scope`) and Double Compilation of PaperScript | CWE-94, CWE-95 |
| **CPP-03** | **Medium** | `quickps/qpsc.cpp` & `libquickps/canvas/context.cpp` | Unbounded Canvas Dimensions & Unbounded Clip Mask Stack Allocation (OOM DoS) | CWE-789, CWE-400 |
| **CPP-04** | **Medium** | `libquickps/quickjsmm/context.hpp` | Missing Destructor Invocation (`~T()`) in `Runtime::Dtor<T>` Causing Native Resource Leaks | CWE-401 |
| **CPP-05** | **Medium** | `libquickps/quickjsmm/context.cpp` & `value.hpp` | Null Pointer Dereference on `JS_ToCString` Failure and Double Memory Leak in `Context::Set` | CWE-476, CWE-401 |
| **PJS-03** | **Medium** | `subprojects/paperjs/src/svg/SvgImport.js` | Unbounded Mutual Recursion (Stack Exhaustion DoS) & External Resource Fetching (SSRF) in `importSVG` | CWE-674, CWE-918 |
| **PJS-04** | **Medium** | `subprojects/paperjs/src/**` & `prepro.py` | Regular Expression Denial of Service (ReDoS) in SVG/Font/Agent Parsers | CWE-1333 |
| **CPP-06** | **Low** | `libquickps/canvas/color.cpp` | Uncaught `std::out_of_range` / `std::invalid_argument` on Malformed CSS Colors | CWE-248 |
| **CPP-07** | **Low** | `quickps/qpsc.cpp` & `libquickps/quickps.cpp` | Unchecked File Streams, Ignored `write_to_file` Errors, and Unchecked `LoadModule` Exception | CWE-252 |
| **BLD-04** | **Low** | `subprojects/paperjs.wrap` & `esbuild.py` | Mutable Git Tag (`v0.16.0`) in Wrap File and Subprocess `os.environ` Stripping | CWE-829, CWE-426 |

---

## Part 1: C++ Core (`quickps/` & `libquickps/`)

### [CPP-01] Out-of-Bounds `argv` Read & Unchecked Argument Dereference in `ValueIter`
- **Severity**: **High**
- **CWE**: CWE-125 (Out-of-bounds Read), CWE-476 (NULL Pointer Dereference)
- **Locations**:
  - `libquickps/quickjsmm/value.hpp:98-112`
  - `libquickps/quickjsmm/value.cpp:56-64`
  - `libquickps/canvas/facade.cpp:202-580`
  - `libquickps/canvas/context.cpp:851-922`

**Vulnerability Analysis**:
1. **Out-of-Bounds Read on Increment**: In `ValueIter::operator++()` (`libquickps/quickjsmm/value.cpp:60-64`):
   ```cpp
   ValueIter &ValueIter::operator++() {
     count_++;
     current_.emplace(js_values_[count_]);
     return *this;
   }
   ```
   When `count_` is incremented to `argc_` (the end iterator position), `js_values_[count_]` unconditionally reads **one `JSValue` (16 bytes) past the end** of QuickJS's `argv` array.
2. **Missing Pre-Increment Bounds Check in Facade Methods**: Nearly every function in `canvas::facade` (`libquickps/canvas/facade.cpp`) and `New<canvas::Canvas>` (`libquickps/canvas/context.cpp:912-922`) extracts positional arguments *before* checking `first == last`:
   ```cpp
   auto x = first++->Get<double>(ctx);
   auto y = first++->Get<double>(ctx);
   if (first != last)
     throw qjs::Exception();
   ```
   - If a JS script calls `ctx.scale()` with **0 arguments**, the first `first++->Get<double>(ctx)` dereferences an empty `std::optional<Value> current_`, throwing `std::bad_optional_access`.
   - Worse, if a JS script calls `ctx.scale(1.0)` with **1 argument**, the first `first++` increments `count_` to `1 == argc_` and emplaces out-of-bounds stack memory (`js_values_[1]`) into `current_`. The second `first++->Get<double>(ctx)` then passes that uninitialized 16-byte stack value (`js_values_[1]`) into `JS_ToFloat64` (or `JS_ToCString`), causing **type confusion, arbitrary memory disclosure, or a segmentation fault**.

**Remediation**:
- Guard `ValueIter::operator++()` with bounds checks:
  ```cpp
  ValueIter &ValueIter::operator++() {
    if (count_ < argc_) {
      count_++;
    }
    if (count_ < argc_) {
      current_.emplace(js_values_[count_]);
    } else {
      current_.reset();
    }
    return *this;
  }
  ```
- In `ValueIter::operator*()` and `ValueIter::operator->()`, check `if (!current_.has_value()) throw Exception();` so insufficient arguments throw a clean `qjs::Exception` instead of reading out-of-bounds memory.

---

### [CPP-02] C++ Exceptions Thrown Across QuickJS C Stack Frames
- **Severity**: **High**
- **CWE**: CWE-248 (Uncaught Exception), CWE-755 (Improper Handling of Exceptional Conditions)
- **Locations**:
  - `libquickps/quickjsmm/esm.hpp:54-153`
  - `libquickps/canvas/facade.cpp:40-43`

**Vulnerability Analysis**:
All C callbacks registered with QuickJS in `EntryBuilder` (`libquickps/quickjsmm/esm.hpp:54-125`, `generic`, `getter`, `setter`) and `ClassBuilder` (`libquickps/quickjsmm/esm.hpp:146-153`, `ctor`) invoke C++ functions that throw `quickps::quickjs::Exception` (or standard library exceptions) without a `try / catch` block at the C callback boundary.
Because QuickJS is compiled as C code, unwinding a C++ exception across QuickJS stack frames (`js_call_c_function`, `JS_SetPropertyInternal`, `JS_EvalFunction`) skips QuickJS's internal `JS_FreeValue` cleanup and `rt->current_stack_frame` restoration, leaving dangling objects in `rt->gc_obj_list` and crashing the host process (`Assertion list_empty(&rt->gc_obj_list) failed`).

**Remediation**:
- Catch `const std::exception &` inside all C callback lambdas in `libquickps/quickjsmm/esm.hpp` and `AddColorStop` in `libquickps/canvas/facade.cpp`, converting them into QuickJS exceptions via `JS_ThrowInternalError` / `JS_ThrowTypeError` (returning `JS_EXCEPTION`).

---

### [CPP-03] Unbounded Canvas Dimensions & Unbounded Clip Mask Stack Allocation (OOM DoS)
- **Severity**: **Medium**
- **CWE**: CWE-789 (Memory Allocation with Excessive Size Value), CWE-400 (Uncontrolled Resource Consumption)
- **Locations**:
  - `quickps/qpsc.cpp:68-73`
  - `libquickps/canvas/context.cpp:38-39, 80-83, 650-653`

**Vulnerability Analysis**:
1. Neither `qpsc.cpp:69` (`std::sscanf(optarg, "%dx%d", &width, &height)`) nor `Canvas::Canvas(int width, int height)` (`libquickps/canvas/context.cpp:650-653`) validates that `width > 0 && height > 0` or caps maximum canvas dimensions (e.g., `width <= 16384 && height <= 16384`).
2. `Context::Save()` (`libquickps/canvas/context.cpp:80-83`) pushes `clip_mask_` onto `clip_stack_` without a depth limit, and `DrawWithClipMask` (`libquickps/canvas/context.cpp:38-39`) allocates a full `w * h * 4` byte `BLImage` on every draw call when clipping is active.

**Remediation**:
- Validate `width` and `height` in `qpsc.cpp` and `Canvas::Canvas` (e.g., `1 <= width <= 16384`, `1 <= height <= 16384`).
- Cap `clip_stack_` depth in `Context::Save()` (e.g., `kMaxSaveDepth = 1024`).

---

### [CPP-04] Missing Destructor Invocation (`~T()`) in `Runtime::Dtor<T>`
- **Severity**: **Medium**
- **CWE**: CWE-401 (Missing Release of Memory after Effective Lifetime)
- **Location**: `libquickps/quickjsmm/context.hpp:29-60`

**Vulnerability Analysis**:
`Runtime::Ctor<T>` (`libquickps/quickjsmm/context.hpp:52-60`) constructs `T` via placement new (`new (buf) T(args...)`), which initializes non-trivial members (`BLImage surface_` in `Canvas`; `BLContext context_`, `BLPath path_`, `std::optional<BLImage> clip_mask_`, and `std::vector<std::optional<BLImage>> clip_stack_` in `Context`).
However, `Runtime::Dtor<T>` (`libquickps/quickjsmm/context.hpp:29-32`) only frees the outer struct buffer:
```cpp
template <class T> static void Dtor(T *ptr) {
  auto rt = Runtime::GetInstance();
  js_free_rt(rt.cobj_.value(), ptr);
}
```
Because `ptr->~T()` is not called before `js_free_rt`, all heap memory owned by `Canvas` and `Context` members (`BLImage` pixel buffers, `BLContext`, `BLPath`, `std::vector`) is leaked whenever a `Canvas` or `Context` is finalized.

**Remediation**:
- Call `if (ptr) { ptr->~T(); js_free_rt(rt.cobj_.value(), ptr); }` in `Runtime::Dtor<T>`.

---

### [CPP-05] Null Pointer Dereference & Memory Leaks in String Conversions
- **Severity**: **Medium**
- **CWE**: CWE-476 (NULL Pointer Dereference), CWE-401 (Memory Leak)
- **Locations**:
  - `libquickps/quickjsmm/context.cpp:54-57`
  - `libquickps/quickjsmm/value.hpp:60-64`

**Vulnerability Analysis**:
1. In `Context::Set(std::string &ref, Value val)` (`libquickps/quickjsmm/context.cpp:54-57`):
   ```cpp
   void Context::Set(std::string &ref, Value val) {
     auto js_str = JS_ToString(cobj_, val.cobj());
     ref.assign(JS_ToCString(cobj_, js_str));
   }
   ```
   Neither `js_str` (`JS_FreeValue`) nor the C-string returned by `JS_ToCString` (`JS_FreeCString`) is freed, and if `JS_ToCString` returns `nullptr`, `ref.assign(nullptr)` triggers undefined behavior.
2. In `Value::Get<std::string>` (`libquickps/quickjsmm/value.hpp:60-64`):
   ```cpp
   auto c_str = JS_ToCString(ctx.cobj(), cobj_);
   std::string result(c_str);
   JS_FreeCString(ctx.cobj(), c_str);
   ```
   If `JS_ToCString` returns `nullptr` (e.g., when converting an object whose `toString()` throws), constructing `std::string(nullptr)` is undefined behavior.

**Remediation**:
- Check `if (!c_str) throw Exception();` before constructing `std::string(c_str)`, and free both `c_str` and `js_str` in `Context::Set`.

---

### [CPP-06] Uncaught `std::out_of_range` / `std::invalid_argument` in `Color::FromString`
- **Severity**: **Low**
- **CWE**: CWE-248 (Uncaught Exception)
- **Location**: `libquickps/canvas/color.cpp:52-243`

**Vulnerability Analysis**:
`Color::FromName` (`libquickps/canvas/color.cpp:246`) calls `kNamedColors.at(str)`. Any unrecognized color name or unsupported CSS color syntax throws `std::out_of_range`. Similarly, large numeric values matching `\d+` in `rgb(...)` or `hsl(...)` can throw `std::out_of_range` in `std::stoi`/`std::stof`.

**Remediation**:
- Use `kNamedColors.find(str)` and `try / catch` around `std::stoi`/`std::stof`, returning a `std::optional<Color>` (or fallback transparent black) so invalid CSS colors are safely ignored per the HTML5 Canvas 2D specification.

---

### [CPP-07] Unchecked File I/O in `qpsc` & Unchecked `LoadModule` Return Value
- **Severity**: **Low**
- **CWE**: CWE-252 (Unchecked Return Value)
- **Locations**:
  - `quickps/qpsc.cpp:85-87`
  - `libquickps/canvas/context.cpp:665-667`
  - `libquickps/quickps.cpp:32-40`

**Vulnerability Analysis**:
- `qpsc.cpp:85-87` does not check `if (!input)` after opening `std::ifstream input(input_path)`.
- `Canvas::Write` (`libquickps/canvas/context.cpp:665-667`) ignores the `BLResult` returned by `surface_.write_to_file(png.c_str())`.

---

## Part 2: Build System & Supply-Chain Security

### [BLD-01] Unverified Download, Predictable `/tmp` Execution (TOCTOU), and Tar Extraction in `esbuild.py`
- **Severity**: **High**
- **CWE**: CWE-494 (Download of Code Without Integrity Check), CWE-377 (Insecure Temporary File), CWE-22 (Tar Slip)
- **Location**: `examples/scripts/esbuild.py:18-63`

**Vulnerability Analysis**:
1. **Local Privilege Escalation / Arbitrary Code Execution via `/tmp/package/bin/esbuild` (CWE-377)**:
   `EsBuild.FALLBACK_PATH` (`examples/scripts/esbuild.py:45`) is hardcoded to `Path(gettempdir()) / "package" / "bin" / "esbuild"` (`/tmp/package/bin/esbuild`). In `EsBuild.find()` (`examples/scripts/esbuild.py:56-63`):
   ```python
   elif cls.FALLBACK_PATH.exists():
       return cls(cls.FALLBACK_PATH)
   ```
   On a shared machine or CI host, any local user can pre-create `/tmp/package/bin/esbuild`, and the build will execute it without downloading or verifying anything.
2. **No Cryptographic Integrity Check (CWE-494)**: The `.tgz` downloaded from `registry.npmjs.org` is not verified against a SHA-256 digest.
3. **Unfiltered `tar.extract` & Leaked Tempfile**: `NamedTemporaryFile(delete=False)` is never unlinked, and `tar.extract()` does not pass `filter="data"`.

**Remediation**:
- Store downloaded build tools in a project-local or user-private cache directory (`0700` permissions) rather than world-writable `/tmp`.
- Verify the SHA-256 digest of the downloaded archive against pinned hashes before extraction, pass `filter="data"` to `tar.extract()`, and unlink the temporary tarball in a `finally` block.

---

### [BLD-02] Unverified External JS Dependencies Fetched via `curl --config`
- **Severity**: **High**
- **CWE**: CWE-494 (Download of Code Without Integrity Check), CWE-829 (Inclusion of Functionality from Untrusted Control Sphere)
- **Locations**:
  - `subprojects/paperjs/meson.build:12-30` (and `subprojects/packagefiles/paperjs/meson.build:12-30`)
  - `examples/meson.build:10-16`
  - `subprojects/paperjs/node_modules/acorn/acorn.config`
  - `subprojects/paperjs/node_modules/straps/straps.config`
  - `subprojects/paperjs/node_modules/nodom/nodom.config`
  - `examples/src/twgl.config`

**Vulnerability Analysis**:
`acorn.js`, `straps.js`, `nodom.js`, and `twgl.js` are fetched at build time using `curl --config @INPUT@ --output @OUTPUT@` without SHA-256 verification and without `--fail --proto '=https' --tlsv1.2`. The downloaded JS is compiled into QuickJS bytecode (`paper.bc`) and embedded directly into `libpaperjs`.

**Remediation**:
- Verify SHA-256 checksums on all downloaded JS artifacts (e.g., via Meson `wrap-file` with `source_hash` or a checksum-verifying fetch script) and pass `--fail --proto '=https' --tlsv1.2` to `curl`.

---

### [BLD-03] Heap Metadata Corruption (`v.u.ptr - 4`), C-String Leaks, and Multi-Macro Substitution Bug in `prepro.py`
- **Severity**: **High**
- **CWE**: CWE-787 (Out-of-bounds Write), CWE-401 (Memory Leak), CWE-22 (Path Traversal)
- **Location**: `subprojects/paperjs/scripts/prepro.py:99-215` (and `subprojects/packagefiles/paperjs/scripts/prepro.py:99-215`)

**Vulnerability Analysis**:
1. **Out-of-Bounds Heap Write in `free_value` (`prepro.py:99-104`)**:
   ```python
   def free_value(self, v: JsValue):
       if v.tag < 0:
           rc = c_int32.from_address(v.u.ptr - 4)
           rc.value -= 1
           if rc.value <= 0:
               getattr(self.qjs, "__JS_FreeValue")(self.ctx, v)
   ```
   In QuickJS (`subprojects/quickjs/quickjs.h:213`), `JSRefCountHeader` (`struct { int ref_count; }`) is located at **`v.u.ptr` (offset `0`)**, *not* `v.u.ptr - 4`. Accessing `v.u.ptr - 4` modifies the 4 bytes **preceding** the `malloc` allocation (part of the allocator's chunk size header), corrupting heap metadata and miscalculating the object's reference count.
2. **Multiple `/*#=*/` Macro Substitution Corruption (`prepro.py:178-183`)**:
   `s = SUBSTITUTE.search(line)` evaluates only the **first** `/*#=*/` macro on a line, and `line = SUBSTITUTE.sub(repl, line)` replaces **all** `/*#=*/` macros on that line with the first result. In `subprojects/paperjs/src/text/TextItem.js:125`:
   `if (flags & (/*#=*/ChangeFlag.GEOMETRY | /*#=*/ChangeFlag.STYLE))`
   is compiled into `if (flags & (6 | 6))` instead of `if (flags & (6 | 64))`.
3. **Missing `JS_FreeCString` (`prepro.py:108`)**:
   Because `JS_ToCStringLen2.restype = c_char_p`, ctypes converts the returned `const char *` into a Python `bytes` object and discards the pointer address without ever calling `JS_FreeCString`.

**Remediation**:
- In `free_value`, read `c_int32.from_address(v.u.ptr)` at offset `0` rather than `v.u.ptr - 4`.
- In `Prepro.process`, replace all macros per line individually via `line = SUBSTITUTE.sub(lambda m: self._engine.evaluate(m["js"]), line)`.
- Change `JS_ToCStringLen2.restype` to `c_void_p`, extract bytes via `string_at`, and call `JS_FreeCString`.

---

## Part 3: Paper.js Runtime Security (`subprojects/paperjs/src/`)

### [PJS-01] Prototype & Property Pollution via `Base#set()`, `Base.importJSON()`, and `Tween`
- **Severity**: **High**
- **CWE**: CWE-1321 (Prototype Pollution), CWE-502 (Deserialization of Untrusted Data)
- **Locations**:
  - `subprojects/paperjs/src/core/Base.js:390-423, 520-563`
  - `subprojects/paperjs/src/anim/Tween.js:437-464`
  - `subprojects/paperjs/src/svg/SvgImport.js:565-570, 787-799`

**Vulnerability Analysis**:
- `Base.filter` (`Base.js:390-423`) and `Tween._setProperty` (`Tween.js:459-464`) copy properties onto target objects without rejecting `__proto__`, `constructor`, or `prototype`.
- `Base.deserialize` (`Base.js:542`) resolves `type = Base.exports[type]` without checking `Object.prototype.hasOwnProperty.call(Base.exports, type)` or excluding `Base.exports.PaperScript`.

**Remediation**:
- Block `__proto__`, `prototype`, and `constructor` keys in `Base.filter`, `Base.set`, and `Tween._setProperty`.
- Restrict `Base.deserialize` to own, serializable classes on `Base.exports` (excluding `PaperScript`).

---

### [PJS-02] Dynamic Code Injection (`options.url` / `scope`) and Double Compilation of PaperScript
- **Severity**: **High**
- **CWE**: CWE-94 (Code Injection), CWE-95 (Eval Injection)
- **Locations**:
  - `subprojects/paperjs/src/core/PaperScript.js:599-781`
  - `libquickps/quickps.cpp:115-135`

**Vulnerability Analysis**:
- In `libquickps/quickps.cpp:115-135`, `PaperScript::Compile` extracts only the `.code` string from `PaperScript.compile(source)` and passes that string to `PaperScript.execute(exe.code, scope)` (`quickps.cpp:133`). Because `PaperScript.execute` (`PaperScript.js:676`) checks `typeof code === 'object' ? code : compile(code, options)`, the already-compiled JS code is **compiled a second time**.
- In `PaperScript.js:599`, `options.url`, `options.handlers`, and `scope` property names are concatenated directly into the generated `Function(...)` source string without sanitization.

**Remediation**:
- Preserve the compiled object returned by `PaperScript.compile()` in `PaperExecutable` so `PaperScript.execute()` does not re-compile already-transformed code.
- Sanitize `options.url` (strip `\r` and `\n`) and validate identifier names in `options.handlers` and `scope`.

---

### [PJS-03] Unbounded Mutual Recursion (Stack Exhaustion DoS) & External Resource Fetching in `SvgImport.js`
- **Severity**: **Medium**
- **CWE**: CWE-674 (Uncontrolled Recursion), CWE-918 (SSRF)
- **Locations**:
  - `subprojects/paperjs/src/svg/SvgImport.js:192-196, 748-757, 979-994`

**Vulnerability Analysis**:
- `getDefinitionById` (`SvgImport.js:748-757`) does not mark `id` as visited before recursively calling `importNode(node, options, false)`. Circular `<linearGradient href="#...">` or `<clipPath clip-path="url(#...)">` definitions cause infinite mutual recursion and exhaust the call stack.

**Remediation**:
- Track active definition IDs in a visited set inside `getDefinitionById` and enforce a maximum DOM recursion depth in `importNode`.

---

### [PJS-04] Regular Expression Denial of Service (ReDoS)
- **Severity**: **Medium**
- **CWE**: CWE-1333 (Inefficient Regular Expression Complexity)
- **Locations**:
  - `subprojects/paperjs/src/core/PaperScope.js:89`
  - `subprojects/paperjs/src/svg/SvgImport.js:523`
  - `subprojects/paperjs/src/svg/SvgExport.js:526`
  - `subprojects/paperjs/src/anim/Tween.js:412`
  - `subprojects/paperjs/scripts/prepro.py:25`

**Vulnerability Analysis**:
- `PaperScope.js:89`, `SvgImport.js:523` (`^\s*drop-shadow\((.*)\)\s*$`), `SvgExport.js:526` (`^(.*?)\s*\b(italic|oblique)\b\s*(.*)$`), `Tween.js:412`, and `prepro.py:25` contain overlapping or unanchored greedy wildcards that exhibit quadratic/polynomial backtracking on malformed inputs.

---

### [BLD-04] Mutable Git Tag in `paperjs.wrap` & Environment Stripping in `esbuild.py`
- **Severity**: **Low**
- **CWE**: CWE-829, CWE-426
- **Locations**:
  - `subprojects/paperjs.wrap:1-4`: Uses mutable `revision = v0.16.0` instead of an immutable 40-char commit SHA.
  - `examples/scripts/esbuild.py:79`: Replaces `os.environ` entirely with `{"NODE_PATH": nodepath}` instead of merging `{**os.environ, "NODE_PATH": str(nodepath)}`.
