#!/usr/bin/env python

import argparse
import json
import shutil
import sys
import tarfile
from pathlib import Path
from dataclasses import dataclass
from typing import List, Optional
from subprocess import run
from tempfile import NamedTemporaryFile, gettempdir
from urllib.request import urlopen

VERSION = "@FALLBACK_VERSION@"
OS = "@SYSTEM@"
ARCH = "64" if "@CPU_FAMILY@" == "x86_64" else "@CPU_FAMILY@"
URL = f"https://registry.npmjs.org/esbuild-{OS}-{ARCH}/-/esbuild-{OS}-{ARCH}-{VERSION}.tgz"


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
    FALLBACK_PATH = Path(gettempdir()) / "package" / "bin" / "esbuild"

    @staticmethod
    def download():
        with urlopen(URL) as response, NamedTemporaryFile(delete=False) as tmp:
            shutil.copyfileobj(response, tmp)

        with tarfile.open(tmp.name) as tar:
            tar.extract("package/bin/esbuild", path=gettempdir())

    @classmethod
    def find(cls):
        esbuild = shutil.which("esbuild")
        if esbuild:
            return cls(esbuild)
        elif cls.FALLBACK_PATH.exists():
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
        _args = [self.esbuild, *args, f"--metafile={metafile}"]
        _env = None if nodepath is None else {"NODE_PATH": nodepath}
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
