project('asmjit', 'cpp',
  default_options : ['cpp_std=c++20']
)

inc = include_directories('.', is_system : true)
cxx = meson.get_compiler('cpp')
thread_dep = dependency('threads')
rt_dep = cxx.find_library('rt', required : false)

src = files(
  'asmjit/axl/arena.cpp',
  'asmjit/axl/arena_bit_set.cpp',
  'asmjit/axl/arena_hash.cpp',
  'asmjit/axl/arena_vector.cpp',
  'asmjit/core/arch_traits.cpp',
  'asmjit/core/assembler.cpp',
  'asmjit/core/builder.cpp',
  'asmjit/core/code_holder.cpp',
  'asmjit/core/code_writer.cpp',
  'asmjit/core/compiler.cpp',
  'asmjit/core/const_pool.cpp',
  'asmjit/core/cpu_info.cpp',
  'asmjit/core/debug_utils.cpp',
  'asmjit/core/emit_helper.cpp',
  'asmjit/core/emitter.cpp',
  'asmjit/core/emitter_utils.cpp',
  'asmjit/core/environment.cpp',
  'asmjit/core/error.cpp',
  'asmjit/core/error_handler.cpp',
  'asmjit/core/formatter.cpp',
  'asmjit/core/func.cpp',
  'asmjit/core/func_args_context.cpp',
  'asmjit/core/inst.cpp',
  'asmjit/core/inst_db.cpp',
  'asmjit/core/jit_allocator.cpp',
  'asmjit/core/jit_runtime.cpp',
  'asmjit/core/logger.cpp',
  'asmjit/core/pauth.cpp',
  'asmjit/core/ra_local.cpp',
  'asmjit/core/ra_pass.cpp',
  'asmjit/core/ra_stack.cpp',
  'asmjit/core/string.cpp',
  'asmjit/core/target.cpp',
  'asmjit/core/type.cpp',
  'asmjit/core/virt_mem.cpp',
  'asmjit/arm/arm_formatter.cpp',
  'asmjit/arm/a64_assembler.cpp',
  'asmjit/arm/a64_builder.cpp',
  'asmjit/arm/a64_compiler.cpp',
  'asmjit/arm/a64_emit_helper.cpp',
  'asmjit/arm/a64_formatter.cpp',
  'asmjit/arm/a64_func.cpp',
  'asmjit/arm/a64_inst_api.cpp',
  'asmjit/arm/a64_inst_db.cpp',
  'asmjit/arm/a64_ra_pass.cpp',
  'asmjit/x86/x86_assembler.cpp',
  'asmjit/x86/x86_builder.cpp',
  'asmjit/x86/x86_compiler.cpp',
  'asmjit/x86/x86_emit_helper.cpp',
  'asmjit/x86/x86_formatter.cpp',
  'asmjit/x86/x86_func.cpp',
  'asmjit/x86/x86_inst_api.cpp',
  'asmjit/x86/x86_inst_db.cpp',
  'asmjit/x86/x86_ra_pass.cpp',
  'asmjit/ujit/uni_compiler_a64.cpp',
  'asmjit/ujit/uni_compiler_x86.cpp',
  'asmjit/ujit/vec_const_table.cpp',
)

public_args = [
  '-DASMJIT_STATIC',
  '-DASMJIT_NO_FOREIGN',
  '-DASMJIT_NO_STDCXX',
  '-DASMJIT_ABI_NAMESPACE=abi_bl',
]

private_args = public_args + [
  '-fno-exceptions',
  '-fno-rtti',
  '-fno-math-errno',
  '-fno-threadsafe-statics',
  '-w',
]

asmjit = static_library('asmjit', src,
  include_directories : inc,
  cpp_args : private_args,
  dependencies : [thread_dep, rt_dep],
)

asmjit_dep = declare_dependency(
  include_directories : inc,
  compile_args : public_args,
  link_with : asmjit,
  dependencies : [thread_dep, rt_dep],
)
