#include <cstdio>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <libquickps/quickps.hpp>

namespace {
using namespace quickps;

static const char *desc = R"(qpsc PaperScript compiler

-h,--help    Prints this help message.
-i,--input   Input JS file path. Its content should be a valid PaperScript.
             If absent or specified as "--input -", reads from stdin.
-o,--output  Output image file path. Supported file extensions are {.png, .svg, .json}.
             If absent, outputs SVG data to stdout.
-s,--size    Geometry width and height of image. Defaults to "640x480".
)";

static const struct option options[] = {{"help", no_argument, 0, 'h'},
                                        {"input", required_argument, 0, 'i'},
                                        {"output", required_argument, 0, 'o'},
                                        {"size", required_argument, 0, 's'},
                                        {0, 0, 0, 0}};

void Run(std::istream &input_stream, std::filesystem::path &output_path,
         int width, int height) {
  QuickPs qps;
  qps.Init();

  try {
    std::ostringstream code;
    code << input_stream.rdbuf();
    auto scope = qps.CreatePaperScope();
    auto canvas = scope.Inject().CreateCanvas(width, height);
    auto engine = scope.SetUp(canvas);
    auto exe = engine.Compile(code.str());
    engine.Execute(exe);
    auto *canvas_ptr = qps.ctx().GetOpaque<canvas::Canvas>(canvas.value);
    canvas_ptr->Write(output_path);
  } catch ([[maybe_unused]] quickjs::Exception &e) {
    std::cerr << qps.ctx();
    throw;
  }

  qps.Terminate();
}

} // namespace

int main(int argc, char **argv) {
  int opt;
  int width = 640;
  int height = 480;
  std::filesystem::path input_path("-");
  std::filesystem::path output_path;

  while ((opt = getopt_long(argc, argv, "hi:o:s:", options, NULL)) != -1) {
    switch (opt) {
    case 'i':
      input_path = optarg;
      break;

    case 'o':
      output_path = optarg;
      break;

    case 's':
      if (2 == std::sscanf(optarg, "%dx%d", &width, &height)) {
        break;
      } else {
        [[fallthrough]];
      }

    case 'h':
    default:
      std::cout << desc;
      exit(EXIT_SUCCESS);
    }
  }

  if (input_path == "-") {
    Run(std::cin, output_path, width, height);
  } else {
    std::ifstream input(input_path);
    Run(input, output_path, width, height);
  }

  return EXIT_SUCCESS;
}
