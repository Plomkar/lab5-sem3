#pragma once

#include <string>

namespace wg {

class Terrain {
public:
    Terrain() = default;
    Terrain(const Terrain&) = delete;
    Terrain& operator=(const Terrain&) = delete;
    virtual ~Terrain() = default;

    virtual char symbol() const = 0;
    virtual std::string name() const = 0;
    virtual int colorCode() const = 0;
    virtual bool isWalkable() const { return true; }
};

class Sea final : public Terrain {
public:
    char symbol() const override { return '~'; }
    std::string name() const override { return "Море"; }
    int colorCode() const override { return 34; }
    bool isWalkable() const override { return false; }
};

class Beach final : public Terrain {
public:
    char symbol() const override { return '.'; }
    std::string name() const override { return "Пляж"; }
    int colorCode() const override { return 93; }
};

class Desert final : public Terrain {
public:
    char symbol() const override { return ':'; }
    std::string name() const override { return "Пустыня"; }
    int colorCode() const override { return 33; }
};

class Plains final : public Terrain {
public:
    char symbol() const override { return '"'; }
    std::string name() const override { return "Равнина"; }
    int colorCode() const override { return 92; }
};

class Forest final : public Terrain {
public:
    char symbol() const override { return '*'; }
    std::string name() const override { return "Лес"; }
    int colorCode() const override { return 32; }
};

class Mountain final : public Terrain {
public:
    char symbol() const override { return '#'; }
    std::string name() const override { return "Горы"; }
    int colorCode() const override { return 90; }
    bool isWalkable() const override { return false; }
};

class Snow final : public Terrain {
public:
    char symbol() const override { return '^'; }
    std::string name() const override { return "Снежные вершины"; }
    int colorCode() const override { return 97; }
    bool isWalkable() const override { return false; }
};

}  // namespace wg
