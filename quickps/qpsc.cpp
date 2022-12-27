#include <boost/program_options.hpp>
#include <filesystem>
#include <iostream>
#include <string>

#define PROJECT_NAME "quickps"

int main(int argc, char **argv) {
  using namespace boost::program_options;

  options_description desc("Allowed options");
  desc.add_options()("help,h", "print usage message")(
      "input,i", value<std::string>(), "input JS file");
  variables_map vm;
  store(parse_command_line(argc, argv, desc), vm);

  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 0;
  }

  std::cout << "This is project " << PROJECT_NAME << ".\n";
  return 0;
}
