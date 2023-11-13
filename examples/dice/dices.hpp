#ifndef DICES_HPP_
#define DICES_HPP_

#include "abcgOpenGL.hpp"
#include <random>
#include <list>

class Window;

struct Vertex {
  glm::vec3 position;
  glm::vec3 color;

  bool operator==(const Vertex& other) const {
    return position == other.position;
  }
};

class Dices {
  public:
    void create(GLuint program, int quantity, std::vector<Vertex>, std::vector<GLuint>,int);
    void paint(glm::ivec2 size, float deltaTime);
    void destroy();

  private:
    friend Window;

    GLuint m_program{};
    int m_verticesToDraw{};
    double m_deltaTime{};
    glm::ivec2 m_viewportSize{};

    std::vector<Vertex> m_vertices;
    std::vector<GLuint> m_indices;

    struct Dice {
      GLuint m_VAO{};
      GLuint m_VBO{};
      GLuint m_EBO{};

      glm::vec3 m_angle{};
      glm::ivec3 m_rotation{};
      glm::vec3 velocidadeAngular{};
      glm::vec2 velocidadeDirecional{};
      glm::vec3 translation{};
      glm::bvec2 movimentoDado{true, true};

      float myTime{};
      bool dadoGirando{false};
      bool dadoColidindo{false};
      int quadros; 
      int maxQuadros;
    };
    std::array<glm::vec3, 7> angulosRetos{
      glm::vec3{0.0f,0.0f,0.0f},
      glm::vec3{125.0f,120.0f,45.0f},
      glm::vec3{345.0f,170.0f,15.0f},
      glm::vec3{75.0f,190.0f,13.0f},
      glm::vec3{75.0f,20.0f,77.0f},
      glm::vec3{347.0f,342.0f,75.0f},
      glm::vec3{105.0f,300.0f,45.0f}
    };

    std::vector<Dice> m_dices;

    std::default_random_engine m_randomEngine;

    Dices::Dice inicializarDado();
    void jogarDado(Dice &); 
    void pararDado(Dice&); 
    void alterarSpin(Dice&); 
    void checkCollisions(Dice&);
};

#endif