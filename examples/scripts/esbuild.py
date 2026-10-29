#!/usr/bin/env python

import argparse
import hashlib
import json
import os
import shutil
import sys
import tarfile
from pathlib import Path
from dataclasses import dataclass
from typing import List, Optional
from subprocess import run
from tempfile import NamedTemporaryFile
from urllib.request import urlopen

VERSION = "@FALLBACK_VERSION@"
OS = "@SYSTEM@"
ARCH = "64" if "@CPU_FAMILY@" == "x86_64" else "@CPU_FAMILY@"
URL = f"https://registry.npmjs.org/esbuild-{OS}-{ARCH}/-/esbuild-{OS}-{ARCH}-{VERSION}.tgz"

EXPECTED_BINARY_SHA256 = {
    ("linux", "64", "0.15.18"): "bada46be9dec28c6a189593aa239e0edfcfd814ec07d282a71eff4db1bb086f6",
}
EXPECTED_TWGL_SHA256 = "5a63b7265ed1b4af018ccbccdc350da8ac4d7c4993fa1d61f2ce1cd217621105"


def _sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


@dataclass
class DepFile:
    target: Path
    dependencies: List[Path]

    @classmethod
    def from_json(cls, meta: object):
        (target, target_meta) = next(iter(meta["outputs"].items()))
        dependencies = list(target_meta["inputs"].keys())
        return cls(target, dependencies)

    def __str__(self) -> str:
        """gcc-style dependencies in the form of Makefiles."""
        lines = []
        lines.append(f"{self.target}:")

        for dep in self.dependencies:
            lines.append(f"{dep}")

        return (" ").join(lines)


class EsBuild:
    CACHE_DIR = Path.home() / ".cache" / "quickps" / f"esbuild-{OS}-{ARCH}-{VERSION}"
    FALLBACK_PATH = CACHE_DIR / "package" / "bin" / "esbuild"

    @classmethod
    def _verify_binary(cls, path: Path):
        expected = EXPECTED_BINARY_SHA256.get((OS, ARCH, VERSION))
        if expected is not None:
            actual = _sha256_file(path)
            if actual != expected:
                path.unlink(missing_ok=True)
                raise RuntimeError(
                    f"SHA-256 mismatch for {path}: expected {expected}, got {actual}"
                )

    @classmethod
    def download(cls):
        cls.CACHE_DIR.mkdir(parents=True, mode=0o700, exist_ok=True)
        tmp_path: Optional[Path] = None
        try:
            with urlopen(URL) as response, NamedTemporaryFile(
                dir=cls.CACHE_DIR, delete=False
            ) as tmp:
                tmp_path = Path(tmp.name)
                shutil.copyfileobj(response, tmp)

            with tarfile.open(tmp_path) as tar:
                tar.extract("package/bin/esbuild", path=cls.CACHE_DIR, filter="data")

            cls._verify_binary(cls.FALLBACK_PATH)
        finally:
            if tmp_path is not None:
                tmp_path.unlink(missing_ok=True)

    @classmethod
    def find(cls):
        esbuild = shutil.which("esbuild")
        if esbuild:
            return cls(Path(esbuild))
        elif cls.FALLBACK_PATH.exists():
            cls._verify_binary(cls.FALLBACK_PATH)
            return cls(cls.FALLBACK_PATH)
        else:
            cls.download()
            return cls(cls.FALLBACK_PATH)

    def __init__(self, esbuild: Path):
        self.esbuild = esbuild

    def version(self):
        run([self.esbuild, "--version"]).check_returncode()

    def run(
        self,
        metafile: Path,
        depfile: Path,
        args: List[str],
        nodepath: Optional[Path] = None,
    ):
        if nodepath is not None:
            twgl_path = Path(nodepath) / "twgl.js"
            if twgl_path.exists():
                actual = _sha256_file(twgl_path)
                if actual != EXPECTED_TWGL_SHA256:
                    raise RuntimeError(
                        f"SHA-256 mismatch for {twgl_path}: expected {EXPECTED_TWGL_SHA256}, got {actual}"
                    )

        _args = [self.esbuild, *args, f"--metafile={metafile}"]
        _env = None if nodepath is None else {**os.environ, "NODE_PATH": str(nodepath)}
        run(_args, env=_env).check_returncode()

        with metafile.open() as input, depfile.open("w") as output:
            meta = json.load(input)
            dep = DepFile.from_json(meta)
            print(dep, file=output)


def main():
    parser = argparse.ArgumentParser(
        prog="esbuild.py",
        description="Bundle JS files.",
    )
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument("--version", action="store_true")
    group.add_argument("--depfile", type=Path)
    parser.add_argument("--metafile", type=Path)
    parser.add_argument("--nodepath", type=Path)
    (args, esbuild_args) = parser.parse_known_args()

    if args.version:
        EsBuild.find().version()
        sys.exit(0)

    depfile = args.depfile
    metafile = (
        depfile.parent / (depfile.stem + ".meta.json")
        if args.metafile is None
        else args.metafile
    )

    EsBuild.find().run(metafile, depfile, esbuild_args, nodepath=args.nodepath)


if __name__ == "__main__":
    main()
