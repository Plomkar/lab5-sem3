#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "map_generator.hpp"
#include "renderers.hpp"

namespace {

struct Options {
    wg::GeneratorConfig config;
    bool seedGiven = false;
    bool color = true;
    bool frame = true;
    bool legend = true;
    bool help = false;
    bool interactive = false;
    std::string outputFile;
};

void printUsage() {
    std::cout << "Использование: worldmap [опции]\n"
                 "  -w, --width N       ширина карты, 1..300 (по умолчанию 60)\n"
                 "  -h, --height N      высота карты, 1..200 (по умолчанию 20)\n"
                 "  -s, --seed N        зерно генератора (по умолчанию случайное)\n"
                 "      --scale X       масштаб шума, 1..1000 (по умолчанию 18)\n"
                 "      --octaves N     число октав, 1..8 (по умолчанию 4)\n"
                 "      --persistence X затухание октав, (0; 1] (по умолчанию 0.5)\n"
                 "      --no-color      без цвета\n"
                 "      --no-frame      без рамки\n"
                 "      --no-legend     без легенды и статистики\n"
                 "  -o, --out FILE      сохранить карту в текстовый файл\n"
                 "  -i, --interactive   запросить параметры с клавиатуры\n"
                 "Без аргументов программа запрашивает размеры карты.\n";
}

std::string nextValue(const std::vector<std::string>& args, std::size_t& index) {
    if (index + 1 >= args.size()) {
        throw std::invalid_argument("Для параметра " + args[index] + " не задано значение");
    }
    ++index;
    return args[index];
}

int parseInt(const std::string& option, const std::string& text) {
    try {
        std::size_t used = 0;
        const int value = std::stoi(text, &used);
        if (used == text.size()) {
            return value;
        }
    } catch (const std::exception&) {
    }
    throw std::invalid_argument("Параметр " + option + " ожидает целое число, получено \"" + text + "\"");
}

double parseDouble(const std::string& option, const std::string& text) {
    try {
        std::size_t used = 0;
        const double value = std::stod(text, &used);
        if (used == text.size()) {
            return value;
        }
    } catch (const std::exception&) {
    }
    throw std::invalid_argument("Параметр " + option + " ожидает число, получено \"" + text + "\"");
}

std::uint32_t parseSeed(const std::string& option, const std::string& text) {
    const int value = parseInt(option, text);
    if (value < 0) {
        throw std::invalid_argument("Параметр " + option + " не может быть отрицательным");
    }
    return static_cast<std::uint32_t>(value);
}

Options parseArguments(const std::vector<std::string>& args) {
    Options options;
    if (args.empty()) {
        options.interactive = true;
        return options;
    }
    for (std::size_t i = 0; i < args.size(); ++i) {
        const std::string& arg = args[i];
        if (arg == "-w" || arg == "--width") {
            options.config.width = parseInt(arg, nextValue(args, i));
        } else if (arg == "-h" || arg == "--height") {
            options.config.height = parseInt(arg, nextValue(args, i));
        } else if (arg == "-s" || arg == "--seed") {
            options.config.seed = parseSeed(arg, nextValue(args, i));
            options.seedGiven = true;
        } else if (arg == "--scale") {
            options.config.scale = parseDouble(arg, nextValue(args, i));
        } else if (arg == "--octaves") {
            options.config.octaves = parseInt(arg, nextValue(args, i));
        } else if (arg == "--persistence") {
            options.config.persistence = parseDouble(arg, nextValue(args, i));
        } else if (arg == "--no-color") {
            options.color = false;
        } else if (arg == "--no-frame") {
            options.frame = false;
        } else if (arg == "--no-legend") {
            options.legend = false;
        } else if (arg == "-o" || arg == "--out") {
            options.outputFile = nextValue(args, i);
        } else if (arg == "-i" || arg == "--interactive") {
            options.interactive = true;
        } else if (arg == "--help") {
            options.help = true;
        } else {
            throw std::invalid_argument("Неизвестный параметр: " + arg);
        }
    }
    return options;
}

template <typename T>
T ask(const std::string& question, T defaultValue) {
    std::cout << question << " [" << defaultValue << "]: " << std::flush;
    std::string line;
    if (!std::getline(std::cin, line) || line.empty()) {
        return defaultValue;
    }
    std::istringstream stream(line);
    T value{};
    if (!(stream >> value)) {
        throw std::invalid_argument("Не удалось разобрать значение: " + line);
    }
    return value;
}

void askParameters(Options& options) {
    options.config.width = ask("Ширина карты (1..300)", options.config.width);
    options.config.height = ask("Высота карты (1..200)", options.config.height);
    const long long seed = ask("Зерно генератора (0 - случайное)", 0LL);
    if (seed != 0) {
        options.config.seed = static_cast<std::uint32_t>(seed);
        options.seedGiven = true;
    }
}

std::unique_ptr<wg::IMapRenderer> buildRenderer(const Options& options, bool color) {
    std::unique_ptr<wg::IMapRenderer> renderer = std::make_unique<wg::AsciiRenderer>();
    if (options.frame) {
        renderer = std::make_unique<wg::FrameDecorator>(std::move(renderer));
    }
    if (color) {
        renderer = std::make_unique<wg::ColorDecorator>(std::move(renderer));
    }
    if (options.legend) {
        renderer = std::make_unique<wg::LegendDecorator>(std::move(renderer), color);
    }
    return renderer;
}

int run(Options options) {
    if (options.interactive) {
        askParameters(options);
    }
    if (!options.seedGiven) {
        std::random_device device;
        options.config.seed = device() % 1000000U + 1U;
    }

    const wg::MapGeneratorBuilder builder = [&options] {
        wg::MapGeneratorBuilder result;
        result.width(options.config.width)
            .height(options.config.height)
            .seed(options.config.seed)
            .scale(options.config.scale)
            .octaves(options.config.octaves)
            .persistence(options.config.persistence);
        return result;
    }();
    const wg::MapGenerator generator = builder.build();
    const wg::WorldMap map = generator.generate();

    std::cout << "Карта " << options.config.width << "x" << options.config.height << ", зерно " << options.config.seed
              << ", октав " << options.config.octaves << "\n";
    std::cout << buildRenderer(options, options.color)->render(map);

    if (!options.outputFile.empty()) {
        std::ofstream file(options.outputFile);
        if (!file) {
            throw std::runtime_error("Не удалось открыть файл " + options.outputFile);
        }
        file << buildRenderer(options, false)->render(map);
        std::cout << "Карта сохранена в файл " << options.outputFile << "\n";
    }
    return 0;
}

}  // namespace

int main(int argc, char* argv[]) {
    const std::vector<std::string> args(argv + 1, argv + argc);
    try {
        const Options options = parseArguments(args);
        if (options.help) {
            printUsage();
            return 0;
        }
        return run(options);
    } catch (const std::invalid_argument& error) {
        std::cerr << "Ошибка: " << error.what() << "\n";
        return 1;
    } catch (const std::exception& error) {
        std::cerr << "Ошибка: " << error.what() << "\n";
        return 2;
    }
}
