#include "window.hpp"

#include <fmt/core.h>
#include <imgui.h>
#include <tiny_obj_loader.h>

#include <cppitertools/itertools.hpp>
#include <glm/gtx/fast_trigonometry.hpp>
#include <glm/gtx/hash.hpp>
#include <unordered_map>

using namespace std;
template <> struct std::hash<Vertex> {
  size_t operator()(Vertex const &vertex) const noexcept {
    auto const h1{std::hash<glm::vec3>()(vertex.position)};
    return h1;
  }
};

void Window::onEvent(SDL_Event const &event) {
  // Keyboard events
  if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
    m_gameData.m_input.set(static_cast<size_t>(Input::Roll));
  }

  if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
      m_gameData.m_input.set(static_cast<size_t>(Input::Roll));
  }

  // Mouse events
  if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_SPACE) {
    m_gameData.m_input.reset(static_cast<size_t>(Input::Roll));
  }
  
  if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
      m_gameData.m_input.reset(static_cast<size_t>(Input::Roll));
  }

}

void Window::onCreate() {
  abcg::glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  abcg::glEnable(GL_DEPTH_TEST);
  auto const assetsPath{abcg::Application::getAssetsPath()};
  m_program = 
      abcg::createOpenGLProgram({{.source = assetsPath + "dice.vert",
                                  .stage = abcg::ShaderStage::Vertex},
                                 {.source = assetsPath + "dice.frag",
                                  .stage = abcg::ShaderStage::Fragment}});

  loadModelFromFile(assetsPath + "dice.obj");
  toDefault();

  #if !defined(__EMSCRIPTEN__)
    abcg::glEnable(GL_PROGRAM_POINT_SIZE);
  #endif

  m_verticesToDraw = m_indices.size();
  m_dices.create(m_program, quantity, m_vertices, m_indices, m_verticesToDraw);
}

void Window::onPaint() {
    abcg::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    abcg::glViewport(0, 0, m_viewportSize.x, m_viewportSize.y);
    
    m_dices.paint(m_viewportSize, getDeltaTime());
}

void Window::onPaintUI() {
  abcg::OpenGLWindow::onPaintUI();
  {
    ImGui::SetNextWindowPos(ImVec2(m_viewportSize.x - 138, m_viewportSize.y - 40));
    ImGui::SetNextWindowSize(ImVec2(128, 35));
    ImGui::Begin("Button window", nullptr, ImGuiWindowFlags_NoDecoration);

    ImGui::PushItemWidth(200);
    if(m_gameData.m_input[static_cast<size_t>(Input::Roll)]){
      for(auto &dice : m_dices.m_dices){
        m_dices.jogarDado(dice);
      } 
    }

    ImGui::PopItemWidth();
    {
      static std::size_t currentIndex{};
      const std::vector<std::string> comboItems{"1", "2", "3", "4", "5"};

      ImGui::PushItemWidth(70);
      if (ImGui::BeginCombo("Dados",
                            comboItems.at(currentIndex).c_str())) {
        for (const auto index : iter::range(comboItems.size())) {
          const bool isSelected{currentIndex == index};
          if (ImGui::Selectable(comboItems.at(index).c_str(), isSelected))
            currentIndex = index;
          if (isSelected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
      ImGui::PopItemWidth();
      if(quantity != (int)currentIndex + 1){
        quantity = currentIndex + 1;
        onCreate();
      }
    }

    
    ImGui::End();
  }
  abcg::glFrontFace(GL_CW);
}

void Window::onResize(glm::ivec2 const &size) {
  m_viewportSize = size;
}

void Window::onDestroy() {
  abcg::glDeleteProgram(m_program);
  m_dices.destroy();
}

void Window::loadModelFromFile(std::string_view path) {
  tinyobj::ObjReader reader;

  if (!reader.ParseFromFile(path.data())) {
    if (!reader.Error().empty()) {
      throw abcg::RuntimeError(
          fmt::format("Failed to load model {} ({})", path, reader.Error()));
    }
    throw abcg::RuntimeError(fmt::format("Failed to load model {}", path));
  }

  if (!reader.Warning().empty()) {
    fmt::print("Warning: {}\n", reader.Warning());
  }

  const auto& attrib{reader.GetAttrib()};
  const auto& shapes{reader.GetShapes()};

  m_vertices.clear();
  m_indices.clear();

  std::unordered_map<Vertex, GLuint> hash{};

  for (const auto& shape : shapes) { 
    for (const auto offset : iter::range(shape.mesh.indices.size())) {
      const tinyobj::index_t index{shape.mesh.indices.at(offset)};
      const int startIndex{3 * index.vertex_index};
      const float vx{attrib.vertices.at(startIndex + 0)};
      const float vy{attrib.vertices.at(startIndex + 1)};
      const float vz{attrib.vertices.at(startIndex + 2)};
      const auto material_id = shape.mesh.material_ids.at(offset/3);
      
      Vertex vertex{};
      vertex.position = {vx, vy, vz};
      vertex.color = {(float)material_id, (float)material_id, (float)material_id};
      
      if (hash.count(vertex) == 0) {
        hash[vertex] = m_vertices.size();
        m_vertices.push_back(vertex);
      }
      m_indices.push_back(hash[vertex]);
    }
  }
}

void Window::toDefault() {
  glm::vec3 max(std::numeric_limits<float>::lowest());
  glm::vec3 min(std::numeric_limits<float>::max());
  for (const auto& vertex : m_vertices) {
    max.x = std::max(max.x, vertex.position.x);
    max.y = std::max(max.y, vertex.position.y);
    max.z = std::max(max.z, vertex.position.z);
    min.x = std::min(min.x, vertex.position.x);
    min.y = std::min(min.y, vertex.position.y);
    min.z = std::min(min.z, vertex.position.z);
  }

  
  const auto center{(min + max) / 2.0f};
  const auto scaling{2.0f / glm::length(max - min)};
  for (auto& vertex : m_vertices) {
    vertex.position = (vertex.position - center) * scaling;
  }
}
