#include <array>
#include <gtest/gtest.h>
#include <libquickps/quickjsmm/context.hpp>
#include <paper.h>

namespace {

using namespace quickps::quickjs;

class PaperJsTest : public ::testing::Test {
protected:
  void SetUp() override { Runtime::Init(); }
  void TearDown() override { Runtime::Terminate(); }
};

TEST_F(PaperJsTest, PaperJsGetByteCode) {
  const auto &buf = paper::GetByteCode();
  EXPECT_EQ(buf.size(), 0x59e51);
}

TEST_F(PaperJsTest, LoadByteCode) {
  const auto &buf = paper::GetByteCode();
  const auto &ctx = Runtime::CreateContext();

  auto module = ctx->LoadModule<paper::ByteCode>(buf);
  EXPECT_FALSE(module.IsException());
  EXPECT_EQ(JS_VALUE_GET_TAG(module.value()), JS_TAG_MODULE);
  JSModuleDef *m = static_cast<JSModuleDef *>(module.value().u.ptr);
  EXPECT_NE(m, nullptr);
  auto module_name_obj = JS_GetModuleName(ctx->GetInstance(), m);
  const char *module_name =
      JS_AtomToCString(ctx->GetInstance(), module_name_obj);
  EXPECT_STREQ(module_name, "paper.js");

  JS_FreeCString(ctx->GetInstance(), module_name);
  JS_FreeAtom(ctx->GetInstance(), module_name_obj);

  auto module_eval = Value(JS_EvalFunction(ctx->GetInstance(), module.value()));
  EXPECT_FALSE(module_eval.IsException());
  EXPECT_EQ(JS_VALUE_GET_TAG(module_eval.value()), JS_TAG_UNDEFINED);
}

TEST_F(PaperJsTest, PaperScope) {
  const auto &buf = paper::GetByteCode();
  const auto &ctx = Runtime::CreateContext();
  auto module = ctx->LoadModule<paper::ByteCode>(buf);
  auto module_eval = Value(JS_EvalFunction(ctx->GetInstance(), module.value()));
  EXPECT_FALSE(module_eval.IsException());

  auto global_this = Value(JS_GetGlobalObject(ctx->GetInstance()));
  EXPECT_FALSE(global_this.IsException());

  auto paper = Value(
      JS_GetPropertyStr(ctx->GetInstance(), global_this.value(), "paper"));
  EXPECT_FALSE(paper.IsException());
  EXPECT_EQ(JS_VALUE_GET_TAG(paper.value()), JS_TAG_OBJECT);

  auto paper_scope =
      Value(JS_GetPropertyStr(ctx->GetInstance(), paper.value(), "PaperScope"));
  EXPECT_FALSE(paper_scope.IsException());
  EXPECT_EQ(JS_VALUE_GET_TAG(paper_scope.value()), JS_TAG_OBJECT);

  auto inject = Value(
      JS_GetPropertyStr(ctx->GetInstance(), paper_scope.value(), "inject"));
  EXPECT_FALSE(inject.IsException());
  EXPECT_TRUE(JS_IsFunction(ctx->GetInstance(), inject.value()));

  auto paper_scope_instance = Value(
      JS_CallConstructor(ctx->GetInstance(), paper_scope.value(), 0, nullptr));
  EXPECT_FALSE(paper_scope_instance.IsException());
  EXPECT_EQ(JS_VALUE_GET_TAG(paper_scope_instance.value()), JS_TAG_OBJECT);

  auto setup = Value(JS_GetPropertyStr(ctx->GetInstance(),
                                       paper_scope_instance.value(), "setup"));
  EXPECT_FALSE(setup.IsException());
  EXPECT_TRUE(JS_IsFunction(ctx->GetInstance(), setup.value()));

  JS_FreeValue(ctx->GetInstance(), global_this.value());
  JS_FreeValue(ctx->GetInstance(), paper.value());
  JS_FreeValue(ctx->GetInstance(), paper_scope.value());
  JS_FreeValue(ctx->GetInstance(), inject.value());
  JS_FreeValue(ctx->GetInstance(), paper_scope_instance.value());
  JS_FreeValue(ctx->GetInstance(), setup.value());
}

TEST_F(PaperJsTest, PaperScript) {
  const auto &buf = paper::GetByteCode();
  const auto &ctx = Runtime::CreateContext();
  auto module = ctx->LoadModule<paper::ByteCode>(buf);
  auto module_eval = Value(JS_EvalFunction(ctx->GetInstance(), module.value()));
  EXPECT_FALSE(module_eval.IsException());

  auto global_this = Value(JS_GetGlobalObject(ctx->GetInstance()));
  EXPECT_FALSE(global_this.IsException());

  auto paper = Value(
      JS_GetPropertyStr(ctx->GetInstance(), global_this.value(), "paper"));
  EXPECT_FALSE(paper.IsException());
  EXPECT_EQ(JS_VALUE_GET_TAG(paper.value()), JS_TAG_OBJECT);

  auto paper_script = Value(
      JS_GetPropertyStr(ctx->GetInstance(), paper.value(), "PaperScript"));
  EXPECT_FALSE(paper_script.IsException());
  EXPECT_EQ(JS_VALUE_GET_TAG(paper_script.value()), JS_TAG_OBJECT);

  auto compile = Value(
      JS_GetPropertyStr(ctx->GetInstance(), paper_script.value(), "compile"));
  EXPECT_FALSE(compile.IsException());
  EXPECT_TRUE(JS_IsFunction(ctx->GetInstance(), compile.value()));

  auto execute = Value(
      JS_GetPropertyStr(ctx->GetInstance(), paper_script.value(), "execute"));
  EXPECT_FALSE(execute.IsException());
  EXPECT_TRUE(JS_IsFunction(ctx->GetInstance(), execute.value()));

  JS_FreeValue(ctx->GetInstance(), global_this.value());
  JS_FreeValue(ctx->GetInstance(), paper.value());
  JS_FreeValue(ctx->GetInstance(), paper_script.value());
  JS_FreeValue(ctx->GetInstance(), compile.value());
  JS_FreeValue(ctx->GetInstance(), execute.value());
}

} // namespace
