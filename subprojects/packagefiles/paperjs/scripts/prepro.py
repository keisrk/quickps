import argparse
import logging
import re
from ctypes import (
    CDLL,
    byref,
    c_bool,
    c_double,
    c_int32,
    c_int64,
    c_uint64,
    c_char_p,
    c_size_t,
    c_ubyte,
    c_void_p,
    POINTER,
    Structure,
    Union,
)
from ctypes.util import find_library
from pathlib import Path
from typing import List, Optional


INCLUDE = re.compile(r"^\s*/\*#\*\/\s*include\('(?P<path>.+)'(,\s*\{.+\})?\);?$")
SUBSTITUTE = re.compile(r"/\*#=\*/\s*(?P<js>\([^)]+\)|[\w\.]+)")
JS_EVAL_TYPE_GLOBAL = 0
JS_EVAL_TYPE_MODULE = 1 << 0
JS_EVAL_FLAG_COMPILE_ONLY = 1 << 5
JS_WRITE_OBJ_BYTECODE = 1 << 0
JS_TAG_STRING = -7


class JsValueUnion(Union):
    _fields_ = [
        ("uint64", c_uint64),
        ("float64", c_double),
        ("ptr", c_void_p),
    ]


class JsValue(Structure):
    _fields_ = [
        ("u", JsValueUnion),
        ("tag", c_int64),
    ]


class QuickJs:
    qjs: Optional[CDLL] = None

    @classmethod
    def setup(cls, dll_path: Optional[Path] = None):
        if dll_path is None:
            dll_path = find_library("quickjs")

        cls.qjs = CDLL(dll_path)
        cls.qjs.JS_NewRuntime.restype = c_void_p
        cls.qjs.JS_SetStripInfo.argtypes = [c_void_p, c_int32]
        cls.qjs.JS_NewContext.argtypes = [c_void_p]
        cls.qjs.JS_NewContext.restype = c_void_p
        cls.qjs.JS_IsError.restype = c_bool
        cls.qjs.JS_IsError.argtypes = [c_void_p, JsValue]
        cls.qjs.JS_Eval.restype = JsValue
        cls.qjs.JS_Eval.argtypes = [c_void_p, c_char_p, c_size_t, c_char_p, c_int32]
        cls.qjs.JS_ToCStringLen2.restype = c_char_p
        cls.qjs.JS_ToCStringLen2.argtypes = [
            c_void_p,
            c_void_p,
            JsValue,
            c_int32,
        ]
        cls.qjs.JS_WriteObject.restype = POINTER(c_ubyte)
        cls.qjs.JS_WriteObject.argtypes = [
            c_void_p,
            POINTER(c_size_t),
            JsValue,
            c_int32,
        ]
        cls.qjs.JS_FreeContext.argtypes = [c_void_p]
        cls.qjs.JS_FreeRuntime.argtypes = [c_void_p]
        cls.qjs.JS_FreeCString.argtypes = [c_void_p, c_void_p]
        getattr(cls.qjs, "__JS_FreeValue").argtypes = [c_void_p, JsValue]
        cls.qjs.js_free.argtypes = [c_void_p, c_void_p]

    def __init__(self):
        self.filename: Optional[str] = None

    def __enter__(self):
        self.rt = self.qjs.JS_NewRuntime()
        self.qjs.JS_SetStripInfo(self.rt, 1)
        self.ctx = self.qjs.JS_NewContext(self.rt)
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.qjs.JS_FreeContext(self.ctx)
        self.qjs.JS_FreeRuntime(self.rt)

    def free_value(self, v: JsValue):
        if v.tag < 0:
            rc = c_int32.from_address(v.u.ptr - 4)
            rc.value -= 1
            if rc.value <= 0:
                getattr(self.qjs, "__JS_FreeValue")(self.ctx, v)

    def evaluate(self, js: str):
        ret = self._evaluate(js)
        result = self.qjs.JS_ToCStringLen2(self.ctx, None, ret, 0)

        if result is None:
            return

        if ret.tag == JS_TAG_STRING:
            s = result.decode("utf-8")
            out = f"'{s}'"
        else:
            out = result.decode("utf-8")

        self.free_value(ret)
        return out

    def compile(self, js: str) -> bytes:
        ret = self._evaluate(
            js, eval_flag=JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY
        )
        length = c_size_t(0)
        buf = self.qjs.JS_WriteObject(
            self.ctx, byref(length), ret, JS_WRITE_OBJ_BYTECODE
        )
        self.free_value(ret)

        if buf is None or length.value == 0:
            raise Exception("JS module failed to compile")

        data = bytes(buf[: length.value])
        self.qjs.js_free(self.ctx, buf)
        return data

    def _evaluate(self, js: str, eval_flag=JS_EVAL_TYPE_GLOBAL):
        if self.filename is None:
            raise Exception("Unknown filename")

        _filename = self.filename.encode("utf-8")
        _js = js.encode("utf-8")
        ret = self.qjs.JS_Eval(
            self.ctx, c_char_p(_js), len(_js), c_char_p(_filename), eval_flag
        )

        if self.qjs.JS_IsError(self.ctx, ret):
            raise Exception("JS threw an exception.")
        else:
            return ret


class Prepro:
    def __init__(self, engine: QuickJs, output, node_modules: List[Path]):
        self._engine = engine
        self._input_queue = []
        self._output = output
        self._node_modules = node_modules

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        for input in self._input_queue:
            input.__exit__(exc_type, exc_value, traceback)

        self._output.close()

    def evaluate(self, input_path: Path):
        code = "".join(self._load(input_path))
        self._engine.evaluate(code)

    def process(self, input_path: Path):
        for line in self._load(input_path):

            s = SUBSTITUTE.search(line)
            if s:
                logging.info(f"Evaluating '{s['js']}'.")
                repl = self._engine.evaluate(s["js"])
                logging.info(f"Evaluated to '{repl}'.")
                line = SUBSTITUTE.sub(repl, line)

            self._output.write(line)

    def _load(self, input_path: Path):
        self._input_queue.append(input_path.open().__enter__())
        self._engine.filename = input_path.name

        while len(self._input_queue):
            input = self._input_queue.pop()

            for line in input:

                m = INCLUDE.match(line)
                if m:
                    logging.info(f"Loading {m['path']}.")
                    self._input_queue.append(input)
                    next_path = self._get_module_path(input_path, m["path"])
                    self._input_queue.append(next_path.open().__enter__())
                    self._engine.filename = next_path.name
                    break

                yield line

    def _get_module_path(self, input_path: Path, module_path: str):
        if "node_modules" in module_path:
            mod = Path(module_path).name

            for nmod in self._node_modules:
                if nmod.name == mod:
                    return nmod

        return input_path.parent / module_path


def bundle(engine: QuickJs, input_path: Path, args):
    node_modules = args.node_module

    with Prepro(engine, args.output, node_modules) as prepro:
        for eval in args.evaluate:
            logging.info(f"Evaluating {eval}.")
            prepro.evaluate(eval)

        logging.info(f"Processing {input_path}.")
        prepro.process(input_path)


def compile(engine: QuickJs, input_path: Path, args):
    engine.filename = input_path.name
    byte_code = engine.compile(input_path.read_text())
    args.output.write_bytes(byte_code)


def main():
    parser = argparse.ArgumentParser(
        prog="Prepro.py",
        description="Bundle JS files.",
    )
    subparsers = parser.add_subparsers(dest="command")

    prepro = subparsers.add_parser("bundle", help="Bundle JS files")
    prepro.add_argument("input", type=Path)
    prepro.add_argument("--dll", type=Path)
    prepro.add_argument("--output", type=argparse.FileType("w"), default="-")
    prepro.add_argument("--evaluate", type=Path, action="append")
    prepro.add_argument("--node-module", type=Path, action="append")

    quickjs = subparsers.add_parser("compile", help="Compile JS module into bytecode")
    quickjs.add_argument("input", type=Path)
    quickjs.add_argument("--dll", type=Path)
    quickjs.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    logging.info("Initializing QuickJS engine.")
    QuickJs.setup(args.dll)
    input = args.input

    with QuickJs() as engine:
        if args.command == "bundle":
            bundle(engine, input, args)
        elif args.command == "compile":
            compile(engine, input, args)


if __name__ == "__main__":
    logging.basicConfig(level=logging.DEBUG)
    main()
