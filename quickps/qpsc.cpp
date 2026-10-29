#include <cstdio>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <sstream>
#include <libquickps/logger.hpp>
#include <libquickps/quickps.hpp>

namespace {
using namespace quickps;

enum {
  OPT_VERBOSE = 1000,
};

static const char *desc = R"(qpsc PaperScript compiler

-h,--help    Prints this help message.
-i,--input   Input JS file path. Its content should be a valid PaperScript.
             If absent or specified as "--input -", reads from stdin.
-o,--output  Output image file path (required). Supported file extension is .png.
-s,--size    Geometry width and height of image. Defaults to "640x480".
--verbose    Enable verbose debug logging.
)";

static const struct option options[] = {
    {"help", no_argument, 0, 'h'},
    {"input", required_argument, 0, 'i'},
    {"output", required_argument, 0, 'o'},
    {"size", required_argument, 0, 's'},
    {"verbose", no_argument, 0, OPT_VERBOSE},
    {0, 0, 0, 0}};

bool Run(std::istream &input_stream, std::filesystem::path &output_path,
         int width, int height) {
  QuickPs qps;
  qps.Init();

  bool success = true;
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
  } catch ([[maybe_unused]] const quickjs::Exception &e) {
    std::cerr << qps.ctx();
    success = false;
  } catch (const std::exception &e) {
    log::error("{}", e.what());
    success = false;
  }

  qps.Terminate();
  return success;
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
      if (2 == std::sscanf(optarg, "%dx%d", &width, &height) && width > 0 &&
          height > 0 && width <= 16384 && height <= 16384) {
        break;
      } else {
        [[fallthrough]];
      }

    case OPT_VERBOSE:
      quickps::Logger::instance().set_level(quickps::LogLevel::Debug);
      break;

    case 'h':
    default:
      std::cout << desc;
      exit(EXIT_SUCCESS);
    }
  }

  if (output_path.empty()) {
    log::error("Output file path (-o, --output) is required.");
    return EXIT_FAILURE;
  }

  if (input_path == "-") {
    if (!Run(std::cin, output_path, width, height)) {
      return EXIT_FAILURE;
    }
  } else {
    std::ifstream input(input_path);
    if (!input.is_open()) {
      log::error("Failed to open input file: {}", input_path.string());
      return EXIT_FAILURE;
    }
    if (!Run(input, output_path, width, height)) {
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
