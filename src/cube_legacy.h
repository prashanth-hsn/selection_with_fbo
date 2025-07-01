#pragma once

#pragma once

#include <string>
#include <glm/glm.hpp>
#include <vector>
#include <set>
struct Color {
    std::string name; // Name of the color for easier identification
    double r;         // Red component
    double g;         // Green component
    double b;         // Blue component

    // Constructor to easily initialize Color objects
    Color(const std::string& name, double red, double green, double blue)
        : name(name), r(red), g(green), b(blue) {}
};


class CubeLegacy {
private:
    std::vector<Color> colorPalette;
public:
    CubeLegacy();

    ~CubeLegacy() = default;

    void render(const std::vector<glm::vec3>& positions, const std::set<int>& selected, bool selection_mode);
};
