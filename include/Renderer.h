#pragma once

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <vector>

static constexpr unsigned int WINDOW_WIDTH = 800;
static constexpr unsigned int WINDOW_HEIGHT = 800;

static constexpr float POINT_SIZE = 10.f;

class Renderer {
public:
    Renderer()
    : window{sf::VideoMode{{WINDOW_WIDTH, WINDOW_HEIGHT}}, "3D Renderer"}
    , view(window.getDefaultView())
    {}

    void run() {
        // set origin to center of screen
        // and keep bounds of screen space between [-1, 1]
        view.setCenter({0, 0});
        view.setSize({2.0f, -2.0f}); // flip y coordinate to match +y = up
        window.setView(view);

        sf::Clock clock;
        float theta = 0;
        bool movingAway = true;
        float dz = 0.0f;

        float min_dz = 1.5f;
        float max_dz = 8.0f;

        float speed = 0.30f;
        float moveSpeed = 1.f;
        while (window.isOpen()) {
            handleEvents(window);

            // update
            float dt = clock.restart().asSeconds();
            theta += 2*M_PI * speed * dt;
            if (dz >= max_dz) movingAway = false;
            else if (dz <= min_dz) movingAway = true;
            dz += (movingAway ? moveSpeed : -moveSpeed) * dt;

            // render
            window.clear();
            for (const auto& face : vertFaces) {
                for (size_t i = 0; i < face.size(); ++i) {
                    drawLine(
                        project(translate_z(rotate_yz(rotate_xz(vertices[face[i]], theta),theta), dz)),
                        project(translate_z(rotate_yz(rotate_xz(vertices[face[(i + 1) % face.size()]], theta),theta), dz))
                    );
                }
            }
            window.display();
        }
    }

private:
    static sf::Vector3f translate_z(const sf::Vector3f& pos, float z) {
        return {pos.x, pos.y, pos.z + z};
    }

    static sf::Vector3f rotate_xz(const sf::Vector3f& pos, float theta) {
        float s = std::sinf(theta);
        float c = std::cosf(theta);
        float x = pos.x * c - pos.z * s;
        float z = pos.x * s + pos.z * c;
        return {x, pos.y, z};
    }

    static sf::Vector3f rotate_yz(const sf::Vector3f& pos, float theta) {
        float s = std::sinf(theta);
        float c = std::cosf(theta);
        float y = pos.y * c - pos.z * s;
        float z = pos.y * s + pos.z * c;
        return {pos.x, y, z};
    }


    static sf::Vector2f project(const sf::Vector3f& pos) {
        float z = std::max(0.001f, pos.z);
        return {pos.x / z, pos.y / z};
    }

    void drawLine(const sf::Vector2f& a, const sf::Vector2f& b) {
        sf::VertexArray va(sf::PrimitiveType::Lines, 2);
        va[0].position = a;
        va[1].position = b;
        window.draw(va);
    }

    void handleEvents(sf::RenderWindow& window) {
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
        }
    }

    std::vector<sf::Vector3f> vertices {
        { -0.5f,  0.5f,  0.5f}, // top left
        {  0.5f,  0.5f,  0.5f}, // top right
        {  0.5f, -0.5f,  0.5f}, // bottom right
        { -0.5f, -0.5f,  0.5f}, // bottom left

        { -0.5f,  0.5f, -0.5f}, // top left
        {  0.5f,  0.5f, -0.5f}, // top right
        {  0.5f, -0.5f, -0.5f}, // bottom right
        { -0.5f, -0.5f, -0.5f}, // bottom left
    };

    std::vector<std::vector<int>> vertFaces {
        {0, 1, 2, 3},
        {4, 5, 6, 7},
        {0, 4},
        {1, 5},
        {2, 6},
        {3, 7},
    };

    sf::RenderWindow window;
    sf::View view;
};
