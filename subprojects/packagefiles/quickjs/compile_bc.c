#include "quickjs.h"
#include "quickjs-libc.h"
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input.js> <output.bc>\n", argv[0]);
        return 1;
    }

    JSRuntime *rt = JS_NewRuntime();
    if (!rt) {
        fprintf(stderr, "Failed to create JS runtime\n");
        return 1;
    }
    JS_SetStripInfo(rt, 1);
    JSContext *ctx = JS_NewContext(rt);
    if (!ctx) {
        fprintf(stderr, "Failed to create JS context\n");
        JS_FreeRuntime(rt);
        return 1;
    }

    size_t buf_len;
    uint8_t *buf = js_load_file(ctx, &buf_len, argv[1]);
    if (!buf) {
        fprintf(stderr, "Cannot open %s\n", argv[1]);
        JS_FreeContext(ctx);
        JS_FreeRuntime(rt);
        return 1;
    }

    const char *module_name = basename(argv[1]);
    JSValue obj = JS_Eval(ctx, (const char *)buf, buf_len, module_name,
                          JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY);
    js_free(ctx, buf);

    if (JS_IsException(obj)) {
        js_std_dump_error(ctx);
        JS_FreeContext(ctx);
        JS_FreeRuntime(rt);
        return 1;
    }

    size_t bc_len;
    uint8_t *bc = JS_WriteObject(ctx, &bc_len, obj, JS_WRITE_OBJ_BYTECODE);
    JS_FreeValue(ctx, obj);

    if (!bc) {
        js_std_dump_error(ctx);
        JS_FreeContext(ctx);
        JS_FreeRuntime(rt);
        return 1;
    }

    FILE *f = fopen(argv[2], "wb");
    if (!f) {
        perror(argv[2]);
        js_free(ctx, bc);
        JS_FreeContext(ctx);
        JS_FreeRuntime(rt);
        return 1;
    }

    size_t written = fwrite(bc, 1, bc_len, f);
    fclose(f);
    js_free(ctx, bc);
    JS_FreeContext(ctx);
    JS_FreeRuntime(rt);

    if (written != bc_len) {
        fprintf(stderr, "Failed to write complete bytecode\n");
        return 1;
    }

    return 0;
}
