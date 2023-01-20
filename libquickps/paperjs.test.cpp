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
  EXPECT_EQ(buf.size(), 0x59f9f);
}

TEST_F(PaperJsTest, LoadByteCode) {
  const auto &buf = paper::GetByteCode();
  const auto &ctx = Runtime::CreateContext();

  auto module = ctx->LoadModule<paper::ByteCode>(buf);
  EXPECT_FALSE(module.IsException());
  EXPECT_EQ(JS_VALUE_GET_TAG(module.cobj()), JS_TAG_MODULE);
  JSModuleDef *m = static_cast<JSModuleDef *>(module.cobj().u.ptr);
  EXPECT_NE(m, nullptr);
  auto module_name_obj = JS_GetModuleName(ctx->cobj(), m);
  const char *module_name = JS_AtomToCString(ctx->cobj(), module_name_obj);
  EXPECT_STREQ(module_name, "paper.js");

  JS_FreeCString(ctx->cobj(), module_name);
  JS_FreeAtom(ctx->cobj(), module_name_obj);

  auto module_eval = Value(JS_EvalFunction(ctx->cobj(), module.cobj()));
  EXPECT_FALSE(module_eval.IsException());
  EXPECT_EQ(JS_VALUE_GET_TAG(module_eval.cobj()), JS_TAG_UNDEFINED);
}

TEST_F(PaperJsTest, PaperScope) {
  const auto &buf = paper::GetByteCode();
  const auto &ctx = Runtime::CreateContext();
  auto module = ctx->LoadModule<paper::ByteCode>(buf);
  auto module_eval = Value(JS_EvalFunction(ctx->cobj(), module.cobj()));
  EXPECT_FALSE(module_eval.IsException());

  auto global_this = Value(JS_GetGlobalObject(ctx->cobj()));
  EXPECT_FALSE(global_this.IsException());

  auto paper =
      Value(JS_GetPropertyStr(ctx->cobj(), global_this.cobj(), "paper"));
  EXPECT_FALSE(paper.IsException());
  EXPECT_EQ(JS_VALUE_GET_TAG(paper.cobj()), JS_TAG_OBJECT);

  auto paper_scope =
      Value(JS_GetPropertyStr(ctx->cobj(), paper.cobj(), "PaperScope"));
  EXPECT_FALSE(paper_scope.IsException());
  EXPECT_EQ(JS_VALUE_GET_TAG(paper_scope.cobj()), JS_TAG_OBJECT);

  auto inject =
      Value(JS_GetPropertyStr(ctx->cobj(), paper_scope.cobj(), "inject"));
  EXPECT_FALSE(inject.IsException());
  EXPECT_TRUE(JS_IsFunction(ctx->cobj(), inject.cobj()));

  auto paper_scope_instance =
      Value(JS_CallConstructor(ctx->cobj(), paper_scope.cobj(), 0, nullptr));
  EXPECT_FALSE(paper_scope_instance.IsException());
  EXPECT_EQ(JS_VALUE_GET_TAG(paper_scope_instance.cobj()), JS_TAG_OBJECT);

  auto setup = Value(
      JS_GetPropertyStr(ctx->cobj(), paper_scope_instance.cobj(), "setup"));
  EXPECT_FALSE(setup.IsException());
  EXPECT_TRUE(JS_IsFunction(ctx->cobj(), setup.cobj()));

  JS_FreeValue(ctx->cobj(), global_this.cobj());
  JS_FreeValue(ctx->cobj(), paper.cobj());
  JS_FreeValue(ctx->cobj(), paper_scope.cobj());
  JS_FreeValue(ctx->cobj(), inject.cobj());
  JS_FreeValue(ctx->cobj(), paper_scope_instance.cobj());
  JS_FreeValue(ctx->cobj(), setup.cobj());
}

TEST_F(PaperJsTest, PaperScript) {
  const auto &buf = paper::GetByteCode();
  const auto &ctx = Runtime::CreateContext();
  auto module = ctx->LoadModule<paper::ByteCode>(buf);
  auto module_eval = Value(JS_EvalFunction(ctx->cobj(), module.cobj()));
  EXPECT_FALSE(module_eval.IsException());

  auto global_this = Value(JS_GetGlobalObject(ctx->cobj()));
  EXPECT_FALSE(global_this.IsException());

  auto paper =
      Value(JS_GetPropertyStr(ctx->cobj(), global_this.cobj(), "paper"));
  EXPECT_FALSE(paper.IsException());
  EXPECT_EQ(JS_VALUE_GET_TAG(paper.cobj()), JS_TAG_OBJECT);

  auto paper_script =
      Value(JS_GetPropertyStr(ctx->cobj(), paper.cobj(), "PaperScript"));
  EXPECT_FALSE(paper_script.IsException());
  EXPECT_EQ(JS_VALUE_GET_TAG(paper_script.cobj()), JS_TAG_OBJECT);

  auto compile =
      Value(JS_GetPropertyStr(ctx->cobj(), paper_script.cobj(), "compile"));
  EXPECT_FALSE(compile.IsException());
  EXPECT_TRUE(JS_IsFunction(ctx->cobj(), compile.cobj()));

  auto execute =
      Value(JS_GetPropertyStr(ctx->cobj(), paper_script.cobj(), "execute"));
  EXPECT_FALSE(execute.IsException());
  EXPECT_TRUE(JS_IsFunction(ctx->cobj(), execute.cobj()));

  JS_FreeValue(ctx->cobj(), global_this.cobj());
  JS_FreeValue(ctx->cobj(), paper.cobj());
  JS_FreeValue(ctx->cobj(), paper_script.cobj());
  JS_FreeValue(ctx->cobj(), compile.cobj());
  JS_FreeValue(ctx->cobj(), execute.cobj());
}

} // namespace
