#!/usr/bin/env python

import argparse
import json
import platform
import shutil
import tarfile
from pathlib import Path
from dataclasses import dataclass
from typing import List
from subprocess import run
from tempfile import NamedTemporaryFile, gettempdir
from urllib.request import urlopen


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
    FALLBACK_VERSION = "0.15.18"
    FALLBACK_PATH = Path(gettempdir()) / "package" / "bin" / "esbuild"

    @staticmethod
    def download(version: str):
        platform_data = platform.platform()
        if "Linux" in platform_data:
            os = "linux"
        elif "Darwin" in platform_data:
            os = "darwin"
        elif "Windows" in platform_data:
            os = "windows"
        else:
            raise Exception(f"Unsupported platform: {platform_data}")

        if "x86_64" in platform_data:
            arch = "64"
        elif "arm64" in platform_data:
            arch = "arm64"
        elif "arm" in platform_data:
            arch = "arm"
        else:
            if os == "windows":
                arch = "64"
            else:
                raise Exception(f"Unsupported platform: {platform_data}")

        url = f"https://registry.npmjs.org/esbuild-{os}-{arch}/-/esbuild-{os}-{arch}-{version}.tgz"

        with urlopen(url) as response, NamedTemporaryFile(delete=False) as tmp:
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

        cls.download(cls.FALLBACK_VERSION)
        return cls(cls.FALLBACK_PATH)

    def __init__(self, esbuild: Path):
        self.esbuild = esbuild

    def run(self, metafile: Path, depfile: Path, args: List[str]):
        _args = [self.esbuild, *args, f"--metafile={metafile}"]
        run(_args).check_returncode()

        with metafile.open() as input, depfile.open("w") as output:
            meta = json.load(input)
            dep = DepFile.from_json(meta)
            print(dep, file=output)


def main():
    parser = argparse.ArgumentParser(
        prog="esbuild.py",
        description="Bundle JS files.",
    )
    parser.add_argument("--depfile", type=Path, required=True)
    parser.add_argument("--metafile", type=Path)
    (args, esbuild_args) = parser.parse_known_args()
    depfile = args.depfile
    metafile = (
        depfile.parent / (depfile.stem + ".meta.json")
        if args.metafile is None
        else args.metafile
    )
    EsBuild.find().run(metafile, depfile, esbuild_args)


if __name__ == "__main__":
    main()
