#include <charconv>
#include <cmath>
#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

#include "CGL/viewer.h"
#include "application.hpp"

namespace {

void PrintUsage(const char* binary_name) {
  std::cout << "Usage: " << binary_name << " [options]\n"
            << "  -m <FLOAT>          Mass per node (positive)\n"
            << "  -g <FLOAT> <FLOAT>  Gravity vector (x, y)\n"
            << "  -s <INT>            Steps per simulation frame (positive)\n"
            << "  -h, --help          Show this help\n";
}

float ParseFloat(std::string_view text) {
  float value = 0.0F;
  const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
  if (error != std::errc{} || end != text.data() + text.size() || !std::isfinite(value)) {
    throw std::invalid_argument("Invalid number: " + std::string(text));
  }
  return value;
}

int ParseSteps(std::string_view text) {
  int value = 0;
  const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
  if (error != std::errc{} || end != text.data() + text.size() || value <= 0) {
    throw std::invalid_argument("Steps per frame must be a positive integer: " + std::string(text));
  }
  return value;
}

std::string_view NextArgument(int argc, char** argv, int& index) {
  if (index + 1 >= argc) {
    throw std::invalid_argument("Missing value for " + std::string(argv[index]));
  }
  return argv[++index];
}

}  // namespace

int main(int argc, char** argv) {
  try {
    assignment8::AppConfig config;
    for (int i = 1; i < argc; ++i) {
      const std::string_view option = argv[i];
      if (option == "-h" || option == "--help") {
        PrintUsage(argv[0]);
        return 0;
      }
      if (option == "-m") {
        config.mass = ParseFloat(NextArgument(argc, argv, i));
        if (config.mass <= 0.0F) {
          throw std::invalid_argument("Mass per node must be positive");
        }
      } else if (option == "-g") {
        const float gravity_x = ParseFloat(NextArgument(argc, argv, i));
        const float gravity_y = ParseFloat(NextArgument(argc, argv, i));
        config.gravity = CGL::Vector2D(gravity_x, gravity_y);
      } else if (option == "-s") {
        config.steps_per_frame = ParseSteps(NextArgument(argc, argv, i));
      } else {
        throw std::invalid_argument("Unknown option: " + std::string(option));
      }
    }

    CGL::Viewer viewer;
    auto application = std::make_unique<assignment8::Application>(config);
    // CGL Viewer deletes its renderer on destruction; ownership is transferred here.
    viewer.set_renderer(application.release());
    viewer.init();
    viewer.start();
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    PrintUsage(argv[0]);
    return 1;
  }
}
