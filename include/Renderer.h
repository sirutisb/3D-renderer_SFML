#pragma once

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <print>
#include <vector>

static constexpr unsigned int WINDOW_WIDTH = 1400;
static constexpr unsigned int WINDOW_HEIGHT = 1400;

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

        window.setKeyRepeatEnabled(false);

        sf::Clock clock;
        float theta = 0;
        bool movingAway = true;
        float dz = 0.0f;

        float min_dz = 1.5f;
        float max_dz = 8.0f;

        // float speed = 0.30f;
        float speed = 0.01f;
        float moveSpeed = 1.f;
        while (window.isOpen()) {
            handleEvents();

            // update
            float dt = clock.restart().asSeconds();
            theta += 2*M_PI * speed * dt;
            if (dz >= max_dz) movingAway = false;
            else if (dz <= min_dz) movingAway = true;
            dz += (movingAway ? moveSpeed : -moveSpeed) * dt;


            // camera update
            constexpr float camSpeed = 4.0f;
            constexpr float camRotSpeed = 4.0f;
            camAngle += sf::Vector3f{inputRotate.x, inputRotate.y, inputRotate.z} * camRotSpeed * dt;

            // sf::Vector3f realMoveDir {
            //     inputMove.x * std::cosf(camAngle.y) + inputMove.z * std::sinf(camAngle.y),
            //     inputMove.y,
            //     inputMove.z * std::cosf(camAngle.y) - inputMove.x * std::sinf(camAngle.y)
            // };
            sf::Vector3f realMoveDir = rotateXYZ(inputMove, camAngle);

            camPos += realMoveDir * camSpeed * dt;

            // render
            window.clear();
            for (const auto& face : vertFaces) {
                for (size_t i = 0; i < face.size(); ++i) {
                    // drawLine(
                    //     project(translate_z(rotate_yz(rotate_xz(vertices[face[i]], theta),theta), dz)),
                    //     project(translate_z(rotate_yz(rotate_xz(vertices[face[(i + 1) % face.size()]], theta),theta), dz))
                    // );
                    auto l1 = rotate_yz(rotate_xz(vertices[face[i]], theta), theta);
                    auto l2 = rotate_yz(rotate_xz(vertices[face[(i + 1) % face.size()]], theta),theta);

                    auto c1 = translate(l1, -camPos);
                    auto c2 = translate(l2, -camPos);

                    auto r1 = rotate_yz(rotate_xz(c1, camAngle.y), camAngle.x);
                    auto r2 = rotate_yz(rotate_xz(c2, camAngle.y), camAngle.x);

                    drawLine(
                        project(r1),
                        project(r2)
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

    static sf::Vector3f translate(const sf::Vector3f& pos, const sf::Vector3f& delta) {
        return pos + delta;
    }

    static sf::Vector3f rotateXYZ(const sf::Vector3f& v, const sf::Vector3f& angle) {
        float X = angle.x; // pitch
        float Y = angle.y; // yaw
        float Z = angle.z; // roll

        float cx = std::cos(X), sx = std::sin(X);
        float cy = std::cos(Y), sy = std::sin(Y);
        float cz = std::cos(Z), sz = std::sin(Z);

        float m00 = cy*cz + sx*sy*sz;
        float m01 = cz*sx*sy - cy*sz;
        float m02 = cx*sy;

        float m10 = cx*sz;
        float m11 = cx*cz;
        float m12 = -sx;

        float m20 = cy*sx*sz - cz*sy;
        float m21 = cy*cz*sx + sy*sz;
        float m22 = cx*cy;

        sf::Vector3f r {
            m00*v.x + m01*v.y + m02*v.z,
            m10*v.x + m11*v.y + m12*v.z,
            m20*v.x + m21*v.y + m22*v.z
        };
        return r;
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

    static sf::Vector3f rotate_xy(const sf::Vector3f& pos, float theta) {
        float s = std::sinf(theta);
        float c = std::cosf(theta);
        float x = pos.x * c - pos.y * s;
        float y = pos.x * s + pos.y * c;
        return {x, y, pos.z};
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

    void handleEvents() {
        sf::Vector3f dmv;
        sf::Vector3f drot;
        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            } else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                switch (keyPressed->code) {
                    case sf::Keyboard::Key::A: dmv.x -= 1; break;
                    case sf::Keyboard::Key::D: dmv.x += 1; break;
                    case sf::Keyboard::Key::W: dmv.z += 1; break;
                    case sf::Keyboard::Key::S: dmv.z -= 1; break;

                    case sf::Keyboard::Key::Up: drot.x += 1; break;
                    case sf::Keyboard::Key::Down: drot.x -= 1; break;
                    case sf::Keyboard::Key::Left: drot.y -= 1; break;
                    case sf::Keyboard::Key::Right: drot.y += 1; break;
                    default: break;
                }
            } else if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>()) {
                switch (keyReleased->code) {
                    case sf::Keyboard::Key::A: dmv.x += 1; break;
                    case sf::Keyboard::Key::D: dmv.x -= 1; break;
                    case sf::Keyboard::Key::W: dmv.z -= 1; break;
                    case sf::Keyboard::Key::S: dmv.z += 1; break;

                    case sf::Keyboard::Key::Up: drot.x -= 1; break;
                    case sf::Keyboard::Key::Down: drot.x += 1; break;
                    case sf::Keyboard::Key::Left: drot.y += 1; break;
                    case sf::Keyboard::Key::Right: drot.y -= 1; break;
                    default: break;
                }
            }
        }
        inputRotate += drot;
        inputMove += dmv;
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

    sf::Vector3f camPos;
    sf::Vector3f camAngle;

    sf::Vector3f inputMove;
    sf::Vector3f inputRotate;

    sf::RenderWindow window;
    sf::View view;
};
