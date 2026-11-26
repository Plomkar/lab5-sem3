#pragma once

#include <memory>
#include <string>

#include "world_map.hpp"

namespace wg {

class IMapRenderer {
public:
    IMapRenderer() = default;
    IMapRenderer(const IMapRenderer&) = delete;
    IMapRenderer& operator=(const IMapRenderer&) = delete;
    virtual ~IMapRenderer() = default;

    virtual std::string render(const WorldMap& map) const = 0;
};

class AsciiRenderer final : public IMapRenderer {
public:
    std::string render(const WorldMap& map) const override;
};

class RendererDecorator : public IMapRenderer {
public:
    explicit RendererDecorator(std::unique_ptr<IMapRenderer> inner);

protected:
    const IMapRenderer& inner() const { return *inner_; }

private:
    std::unique_ptr<IMapRenderer> inner_;
};

class FrameDecorator final : public RendererDecorator {
public:
    using RendererDecorator::RendererDecorator;
    std::string render(const WorldMap& map) const override;
};

class ColorDecorator final : public RendererDecorator {
public:
    using RendererDecorator::RendererDecorator;
    std::string render(const WorldMap& map) const override;
};

class LegendDecorator final : public RendererDecorator {
public:
    LegendDecorator(std::unique_ptr<IMapRenderer> inner, bool colored);
    std::string render(const WorldMap& map) const override;

private:
    bool colored_;
};

std::string stripAnsi(const std::string& text);

}  // namespace wg
