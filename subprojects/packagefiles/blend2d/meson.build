project('blend2d', 'cpp',
  default_options : ['cpp_std=c++20']
)

inc = include_directories('.', is_system : true)
cxx = meson.get_compiler('cpp')
m_dep = cxx.find_library('m', required : false)
thread_dep = dependency('threads')
asmjit_dep = dependency('asmjit')

base_args = [
  '-DBL_STATIC',
  '-fno-exceptions',
  '-fno-rtti',
  '-fno-math-errno',
  '-fno-threadsafe-statics',
  '-fno-trapping-math',
  '-fno-finite-math-only',
  '-w',
]

if host_machine.cpu_family() in ['x86', 'x86_64']
  base_args += ['-DBL_BUILD_OPT_AVX512']
endif

src_base = files(
  'blend2d/core/api-globals.cpp',
  'blend2d/core/api-nocxx.cpp',
  'blend2d/core/array.cpp',
  'blend2d/core/bitarray.cpp',
  'blend2d/core/bitset.cpp',
  'blend2d/core/compopinfo.cpp',
  'blend2d/core/context.cpp',
  'blend2d/core/filesystem.cpp',
  'blend2d/core/font.cpp',
  'blend2d/core/fontdata.cpp',
  'blend2d/core/fontface.cpp',
  'blend2d/core/fontfeaturesettings.cpp',
  'blend2d/core/fontmanager.cpp',
  'blend2d/core/fonttagdataids.cpp',
  'blend2d/core/fonttagdatainfo.cpp',
  'blend2d/core/fonttagset.cpp',
  'blend2d/core/fontvariationsettings.cpp',
  'blend2d/core/format.cpp',
  'blend2d/core/glyphbuffer.cpp',
  'blend2d/core/gradient.cpp',
  'blend2d/core/image.cpp',
  'blend2d/core/imagecodec.cpp',
  'blend2d/core/imagedecoder.cpp',
  'blend2d/core/imageencoder.cpp',
  'blend2d/core/imagescale.cpp',
  'blend2d/core/matrix.cpp',
  'blend2d/core/object.cpp',
  'blend2d/core/path.cpp',
  'blend2d/core/pathstroke.cpp',
  'blend2d/core/pattern.cpp',
  'blend2d/core/pixelconverter.cpp',
  'blend2d/core/random.cpp',
  'blend2d/core/runtime.cpp',
  'blend2d/core/runtimescope.cpp',
  'blend2d/core/string.cpp',
  'blend2d/core/trace.cpp',
  'blend2d/core/var.cpp',
  'blend2d/codec/bmpcodec.cpp',
  'blend2d/codec/jpegcodec.cpp',
  'blend2d/codec/jpeghuffman.cpp',
  'blend2d/codec/jpegops.cpp',
  'blend2d/codec/pngcodec.cpp',
  'blend2d/codec/pngops.cpp',
  'blend2d/codec/pngops_asimd.cpp',
  'blend2d/codec/qoicodec.cpp',
  'blend2d/compression/checksum.cpp',
  'blend2d/compression/checksum_asimd.cpp',
  'blend2d/compression/checksum_asimd_crypto.cpp',
  'blend2d/compression/deflatedecoder.cpp',
  'blend2d/compression/deflatedecoderfast.cpp',
  'blend2d/compression/deflatedecoderutils.cpp',
  'blend2d/compression/deflatedefs.cpp',
  'blend2d/compression/deflateencoder.cpp',
  'blend2d/geometry/sizetable.cpp',
  'blend2d/opentype/otcff.cpp',
  'blend2d/opentype/otcmap.cpp',
  'blend2d/opentype/otcore.cpp',
  'blend2d/opentype/otface.cpp',
  'blend2d/opentype/otglyf.cpp',
  'blend2d/opentype/otglyf_asimd.cpp',
  'blend2d/opentype/otglyfsimddata.cpp',
  'blend2d/opentype/otkern.cpp',
  'blend2d/opentype/otlayout.cpp',
  'blend2d/opentype/otmetrics.cpp',
  'blend2d/opentype/otname.cpp',
  'blend2d/pipeline/pipedefs.cpp',
  'blend2d/pipeline/piperuntime.cpp',
  'blend2d/pipeline/jit/compoppart.cpp',
  'blend2d/pipeline/jit/fetchgradientpart.cpp',
  'blend2d/pipeline/jit/fetchpart.cpp',
  'blend2d/pipeline/jit/fetchpatternpart.cpp',
  'blend2d/pipeline/jit/fetchpixelptrpart.cpp',
  'blend2d/pipeline/jit/fetchsolidpart.cpp',
  'blend2d/pipeline/jit/fetchutilscoverage.cpp',
  'blend2d/pipeline/jit/fetchutilsinlineloops.cpp',
  'blend2d/pipeline/jit/fetchutilspixelaccess.cpp',
  'blend2d/pipeline/jit/fetchutilspixelgather.cpp',
  'blend2d/pipeline/jit/fillpart.cpp',
  'blend2d/pipeline/jit/pipecompiler.cpp',
  'blend2d/pipeline/jit/pipecomposer.cpp',
  'blend2d/pipeline/jit/pipefunction.cpp',
  'blend2d/pipeline/jit/pipeprimitives.cpp',
  'blend2d/pipeline/jit/pipegenruntime.cpp',
  'blend2d/pipeline/jit/pipepart.cpp',
  'blend2d/pipeline/reference/fixedpiperuntime.cpp',
  'blend2d/pixelops/interpolation.cpp',
  'blend2d/pixelops/funcs.cpp',
  'blend2d/raster/rastercontext.cpp',
  'blend2d/raster/rastercontextops.cpp',
  'blend2d/raster/renderfetchdata.cpp',
  'blend2d/raster/rendertargetinfo.cpp',
  'blend2d/raster/workdata.cpp',
  'blend2d/raster/workermanager.cpp',
  'blend2d/raster/workerproc.cpp',
  'blend2d/raster/workersynchronization.cpp',
  'blend2d/support/arenaallocator.cpp',
  'blend2d/support/arenahashmap.cpp',
  'blend2d/support/math.cpp',
  'blend2d/support/scopedallocator.cpp',
  'blend2d/support/zeroallocator.cpp',
  'blend2d/tables/tables.cpp',
  'blend2d/threading/futex.cpp',
  'blend2d/threading/thread.cpp',
  'blend2d/threading/threadpool.cpp',
  'blend2d/threading/uniqueidgenerator.cpp',
  'blend2d/unicode/unicode.cpp',
)

simd_objs = []

if host_machine.cpu_family() in ['x86', 'x86_64']
  lib_sse2 = static_library('blend2d_sse2',
    files(
      'blend2d/core/matrix_sse2.cpp',
      'blend2d/core/pixelconverter_sse2.cpp',
      'blend2d/codec/jpegops_sse2.cpp',
      'blend2d/codec/pngops_sse2.cpp',
      'blend2d/compression/checksum_sse2.cpp',
      'blend2d/pixelops/interpolation_sse2.cpp',
    ),
    include_directories : inc,
    cpp_args : base_args + ['-msse2'],
    dependencies : [asmjit_dep],
  )

  lib_ssse3 = static_library('blend2d_ssse3',
    files('blend2d/core/pixelconverter_ssse3.cpp'),
    include_directories : inc,
    cpp_args : base_args + ['-mssse3'],
    dependencies : [asmjit_dep],
  )

  lib_sse4_2 = static_library('blend2d_sse4_2',
    files(
      'blend2d/compression/checksum_sse4_2.cpp',
      'blend2d/opentype/otglyf_sse4_2.cpp',
    ),
    include_directories : inc,
    cpp_args : base_args + ['-mpopcnt', '-mpclmul', '-msse4.2'],
    dependencies : [asmjit_dep],
  )

  lib_avx = static_library('blend2d_avx',
    files(
      'blend2d/core/matrix_avx.cpp',
      'blend2d/codec/pngops_avx.cpp',
    ),
    include_directories : inc,
    cpp_args : base_args + ['-mpopcnt', '-mpclmul', '-mavx'],
    dependencies : [asmjit_dep],
  )

  lib_avx2 = static_library('blend2d_avx2',
    files(
      'blend2d/core/pixelconverter_avx2.cpp',
      'blend2d/compression/deflatedecoderfast_avx2.cpp',
      'blend2d/opentype/otglyf_avx2.cpp',
      'blend2d/pixelops/interpolation_avx2.cpp',
    ),
    include_directories : inc,
    cpp_args : base_args + [
      '-mpopcnt',
      '-mpclmul',
      '-mbmi',
      '-mbmi2',
      '-mavx2',
      '-DBL_TARGET_OPT_POPCNT',
      '-DBL_TARGET_OPT_BMI2',
    ],
    dependencies : [asmjit_dep],
  )

  simd_objs = [
    lib_sse2.extract_all_objects(recursive : true),
    lib_ssse3.extract_all_objects(recursive : true),
    lib_sse4_2.extract_all_objects(recursive : true),
    lib_avx.extract_all_objects(recursive : true),
    lib_avx2.extract_all_objects(recursive : true),
  ]
endif

blend2d = static_library('blend2d', src_base,
  objects : simd_objs,
  include_directories : inc,
  cpp_args : base_args,
  dependencies : [asmjit_dep, m_dep, thread_dep],
)

blend2d_dep = declare_dependency(
  include_directories : inc,
  compile_args : ['-DBL_STATIC'],
  link_with : blend2d,
  dependencies : [asmjit_dep, m_dep, thread_dep],
)
