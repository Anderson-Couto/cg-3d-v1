#ifndef WINDOW_HPP_
#define WINDOW_HPP_

#include <vector>
#include <random>
#include <bitset>
#include "abcgOpenGL.hpp"
#include "dices.hpp"
#include "gamedata.hpp"

class Window : public abcg::OpenGLWindow {
 protected:
  void onEvent(SDL_Event const &event) override;
  void onCreate() override;
  void onPaint() override;
  void onPaintUI() override;
  void onResize(glm::ivec2 const &size) override;
  void onDestroy() override;

 private:
  GLuint m_program{};
  std::vector<Vertex> m_vertices;
  std::vector<GLuint> m_indices;
  int m_verticesToDraw{};
  GameData m_gameData;

  Dices m_dices;
  int quantity{1};

  glm::ivec2 m_viewportSize{};

  void loadModelFromFile(std::string_view path);
  void toDefault();
};

#endif