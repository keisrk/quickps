#include <getopt.h>

#include <cstdio>
#include <fstream>
#include <iostream>
#include <libquickps/logger.hpp>
#include <libquickps/quickps.hpp>
#include <sstream>

namespace {
using namespace quickps;

enum {
  OPT_VERBOSE = 1000,
};

static const char *desc = R"(qpsc PaperScript compiler

-h,--help    Prints this help message.
-i,--input   Input JS file path. Its content should be a valid PaperScript.
             If absent or specified as "--input -", reads from stdin.
-o,--output  Output image file path (required). Supported file extensions are .png, .svg.
-s,--size    Geometry width and height of image. Defaults to "640x480".
-I,--image   Pre-load image for document.getElementById(id) in the format "id=path"
             (or "path", where id defaults to the file stem). Can be repeated.
             Also supports --img:<id>=<path> syntax.
--verbose    Enable verbose debug logging.
)";

static const struct option options[] = {
    {"help", no_argument, 0, 'h'},
    {"input", required_argument, 0, 'i'},
    {"output", required_argument, 0, 'o'},
    {"size", required_argument, 0, 's'},
    {"image", required_argument, 0, 'I'},
    {"verbose", no_argument, 0, OPT_VERBOSE},
    {0, 0, 0, 0}};

void AddImageSpec(
    std::string_view spec,
    std::vector<std::pair<std::string, std::filesystem::path>> &images) {
  auto eq = spec.find('=');
  if (eq != std::string_view::npos) {
    std::string id(spec.substr(0, eq));
    std::filesystem::path path(spec.substr(eq + 1));
    images.emplace_back(std::move(id), std::move(path));
  } else {
    std::filesystem::path path(spec);
    std::string id = path.stem().string();
    images.emplace_back(std::move(id), std::move(path));
  }
}

bool Run(
    std::istream &input_stream, std::filesystem::path &output_path, int width,
    int height,
    const std::vector<std::pair<std::string, std::filesystem::path>> &images) {
  QuickPs qps;
  qps.Init();

  for (const auto &[id, path] : images) {
    if (!std::filesystem::exists(path)) {
      log::error("Image file does not exist: {}", path.string());
      qps.Terminate();
      return false;
    }
  }

  bool success = true;
  try {
    std::ostringstream code;
    code << input_stream.rdbuf();

    ScopeConfig config{
        .width = width,
        .height = height,
        .output = output_path,
        .images = images,
    };
    auto scope = qps.CreatePaperScope();
    auto canvas = scope.DefineScope(config);
    auto engine = scope.SetUp(canvas);
    auto exe = engine.Compile(code.str());
    engine.Execute(exe);
    if (output_path.extension() == ".svg") {
      std::ofstream output(output_path);
      if (!output.is_open()) {
        log::error("Failed to open output file: {}", output_path.string());
        return false;
      }
      output << engine.ExportSVG();
    } else {
      auto *canvas_ptr = qps.ctx().GetOpaque<canvas::Canvas>(canvas.value);
      canvas_ptr->Write(output_path);
    }
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

}  // namespace

int main(int argc, char **argv) {
  int opt;
  int width = 640;
  int height = 480;
  std::filesystem::path input_path("-");
  std::filesystem::path output_path;
  std::vector<std::pair<std::string, std::filesystem::path>> images;

  std::vector<char *> filtered_argv;
  filtered_argv.reserve(argc);
  for (int i = 0; i < argc; ++i) {
    std::string_view arg(argv[i]);
    if (arg.starts_with("--img:")) {
      auto spec = arg.substr(6);
      AddImageSpec(spec, images);
    } else {
      filtered_argv.push_back(argv[i]);
    }
  }
  int filtered_argc = static_cast<int>(filtered_argv.size());

  while ((opt = getopt_long(filtered_argc, filtered_argv.data(),
                            "hi:o:s:I:", options, NULL)) != -1) {
    switch (opt) {
      case 'i':
        input_path = optarg;
        break;

      case 'o':
        output_path = optarg;
        break;

      case 'I':
        AddImageSpec(optarg, images);
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
    if (!Run(std::cin, output_path, width, height, images)) {
      return EXIT_FAILURE;
    }
  } else {
    std::ifstream input(input_path);
    if (!input.is_open()) {
      log::error("Failed to open input file: {}", input_path.string());
      return EXIT_FAILURE;
    }
    if (!Run(input, output_path, width, height, images)) {
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
