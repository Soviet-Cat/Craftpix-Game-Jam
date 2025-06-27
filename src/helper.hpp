#include <string>
#include <sstream>
#include <fstream>
#include <ostream>
#include <iostream>

#include "../lib/glm/glm.hpp"

std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        std::cout << "Failed to open file: " << path << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    if (content.empty()) {
        std::cout << "File is empty: " << path << std::endl;
    }
    return content;
}

glm::vec2 lerp(const glm::vec2& a, const glm::vec2& b, float t)
{
    return a + (b - a) * t;
}