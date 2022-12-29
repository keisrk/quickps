#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>
#include <libquickps/quickps.hpp>

using namespace quickps;

void Run(const std::string &input, const std::string &output) {
  QuickPs qps;
  qps.Init();

  try {
    std::ifstream input_stream(input);
    std::stringstream ss;
    ss << input_stream.rdbuf();
    auto code = ss.str();

    auto scope = qps.CreatePaperScope();
    auto canvas = scope.Inject().CreateCanvas(640, 480);
    auto engine = scope.SetUp(canvas);
    auto exe = engine.Compile(code.c_str());
    engine.Execute(exe);
    auto *canvas_ptr = qps.ctx().GetOpaque<canvas::Canvas>(canvas.value);
    std::filesystem::path output_path(output);
    canvas_ptr->Write(output_path);
  } catch ([[maybe_unused]] const std::exception &e) {
    throw;
  }

  qps.Terminate();
}

int main(int argc, char **argv) {
  namespace po = boost::program_options;

  po::options_description desc("Options");
  // clang-format off
  desc.add_options()
    ("help,h", "print usage message")
    ("input,i", po::value<std::string>(), "input JS file")
    ("output,o", po::value<std::string>(), "output PNG file");
  // clang-format on

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 0;
  }

  Run(vm["input"].as<std::string>(), vm["output"].as<std::string>());
  return 0;
}
