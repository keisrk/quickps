#ifndef QUICKPS_QUICKPS_H_
#define QUICKPS_QUICKPS_H_

namespace quickps {

class QuickPs {};

class PaperScope {
  static PaperScope Create();
  void Inject();
  void SetUp();
};

class PaperScript {
  void Compile();
  void Execute();
};

} // namespace quickps

#endif // QUICKPS_QUICKPS_H_

/*
int main(int argc, char **argv)
{
  JSRuntime *rt;
  JSContext *ctx;
  rt = JS_NewRuntime();
  js_std_set_worker_new_context_func(JS_NewCustomContext);
  js_std_init_handlers(rt);
  ctx = JS_NewCustomContext(rt);
  js_std_add_helpers(ctx, argc, argv);
  js_std_eval_binary(ctx, qjsc_hello, qjsc_hello_size, 0);
  js_std_loop(ctx);
  JS_FreeContext(ctx);
  JS_FreeRuntime(rt);
  return 0;
}
*/
