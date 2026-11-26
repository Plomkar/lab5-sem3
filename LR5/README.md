# Лабораторная работа №5. Наследование. Полиморфизм

Вариант 8 «Генерация карты»: программа строит карту мира с помощью шума Перлина и выводит её в консоль.
На карте генерируются разные ландшафты: море, пляж, пустыня, равнина, лес, горы и снежные вершины.

Паттерны проектирования: **Фабрика** (`TerrainFactory`), **Строитель** (`MapGeneratorBuilder`), **Декоратор** (`FrameDecorator`,
`ColorDecorator`, `LegendDecorator`). Принципы ООП: наследование, полиморфизм (виртуальные методы `Terrain` и `IMapRenderer`),
инкапсуляция, композиция.

## Структура

```
include/   perlin_noise.hpp terrain.hpp terrain_factory.hpp world_map.hpp map_generator.hpp renderers.hpp
src/       perlin_noise.cpp terrain_factory.cpp world_map.cpp map_generator.cpp renderers.cpp main.cpp
tests/     test_main.cpp (30 тестов, собственный мини-фреймворк)
examples/  готовые карты в текстовых файлах
Makefile
```

## Сборка и запуск

Нужен компилятор с поддержкой C++17 (g++ 9 и новее) и make.

```
make              # сборка build/worldmap
make test         # сборка и запуск тестов
make run          # пример карты 64x22 (зерно 7)
make examples     # заново создать файлы в examples/
```

```
./build/worldmap                      # спросит ширину, высоту и зерно
./build/worldmap -w 80 -h 24 -s 5     # размеры и зерно через параметры
./build/worldmap -w 60 -h 20 --no-color --no-legend
./build/worldmap -w 100 -h 30 -s 2026 --octaves 5 -o map.txt
./build/worldmap --help
```

Одно и то же зерно (`-s`) всегда даёт одну и ту же карту. Если зерно не задано, выбирается случайное и печатается в первой строке вывода.
