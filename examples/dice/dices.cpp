#include "dices.hpp"
#include <imgui.h>
#include <glm/gtx/fast_trigonometry.hpp>
#include <fmt/core.h>

void Dices::create(GLuint program, int quantity, std::vector<Vertex> vertices, std::vector<GLuint> indices, int verticesToDraw){
  destroy();
  auto seed{std::chrono::steady_clock::now().time_since_epoch().count()};
  m_randomEngine.seed(seed);

  m_program = program;
  m_vertices = vertices;
  m_indices = indices;
  m_verticesToDraw = verticesToDraw;

  m_dices.clear();
  m_dices.resize(quantity);

  for(auto &dice : m_dices) {
    dice = inicializarDado();
  }
  
}

void Dices::paint(glm::ivec2 size, float deltaTime){
  m_deltaTime = deltaTime;
  m_viewportSize = size;

  abcg::glUseProgram(m_program);
    
  for(auto &dice : m_dices){
    abcg::glBindVertexArray(dice.m_VAO);

    if(dice.dadoGirando){
      checkCollisions(dice);

      dice.quadros++;
      if(dice.translation.x >= 1.5f) {
        dice.movimentoDado.x = false;
        alterarSpin(dice);
      }
      else if (dice.translation.x <= -1.5f) {
        dice.movimentoDado.x = true;
        alterarSpin(dice);
      }

      if(dice.translation.y >= 1.5f) {
        dice.movimentoDado.y = false;
        alterarSpin(dice);
      }
      else if (dice.translation.y <= -1.5f) {
        dice.movimentoDado.y = true;
        alterarSpin(dice);
      }
      
      if(dice.movimentoDado.x) {
        dice.translation.x += dice.velocidadeDirecional.x; 
      }
      else{
        dice.translation.x -= dice.velocidadeDirecional.x;
      }
      if(dice.movimentoDado.y) {
        dice.translation.y += dice.velocidadeDirecional.y; 
      }
      else{
        dice.translation.y -= dice.velocidadeDirecional.y;
      }

      if(dice.quadros > dice.maxQuadros){
        pararDado(dice);
      }
    }

    if(dice.m_rotation.x || dice.m_rotation.y ||dice.m_rotation.z){
      dice.myTime = deltaTime;
      
      if(dice.m_rotation.x)
        dice.m_angle.x = glm::wrapAngle(dice.m_angle.x + dice.velocidadeAngular.x * dice.myTime);

      if(dice.m_rotation.y)
        dice.m_angle.y = glm::wrapAngle(dice.m_angle.y + dice.velocidadeAngular.y * dice.myTime);

      if(dice.m_rotation.z)
        dice.m_angle.z = glm::wrapAngle(dice.m_angle.z + dice.velocidadeAngular.z * dice.myTime);
    }

    const GLint rotationXLoc{abcg::glGetUniformLocation(m_program, "rotationX")};
    abcg::glUniform1f(rotationXLoc, dice.m_angle.x);
    const GLint rotationYLoc{abcg::glGetUniformLocation(m_program, "rotationY")};
    abcg::glUniform1f(rotationYLoc, dice.m_angle.y);
    const GLint rotationZLoc{abcg::glGetUniformLocation(m_program, "rotationZ")};
    abcg::glUniform1f(rotationZLoc, dice.m_angle.z);
    const GLint translationLoc{abcg::glGetUniformLocation(m_program, "translation")};
    abcg::glUniform3fv(translationLoc, 1, &dice.translation.x);

    abcg::glDrawElements(GL_TRIANGLES, m_verticesToDraw, GL_UNSIGNED_INT,
                        nullptr);

    abcg::glBindVertexArray(0);
  }
  abcg::glUseProgram(0);
}

Dices::Dice Dices::inicializarDado() {
  Dice dice;

  abcg::glGenBuffers(1, &dice.m_VBO);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, dice.m_VBO);
  abcg::glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices[0]) * m_vertices.size(),
                     m_vertices.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  abcg::glGenBuffers(1, &dice.m_EBO);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dice.m_EBO);
  abcg::glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     sizeof(m_indices[0]) * m_indices.size(), m_indices.data(),
                     GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  abcg::glGenVertexArrays(1, &dice.m_VAO);
  abcg::glBindVertexArray(dice.m_VAO);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, dice.m_VBO);

  GLint positionAttribute{abcg::glGetAttribLocation(m_program, "inPosition")};
  if (positionAttribute >= 0) {
    abcg::glEnableVertexAttribArray(positionAttribute);
    abcg::glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE,
                                sizeof(Vertex), nullptr);
  }

  const GLint colorAttribute{abcg::glGetAttribLocation(m_program, "inColor")};
  if (colorAttribute >= 0) {
    abcg::glEnableVertexAttribArray(colorAttribute);
    GLsizei offset{sizeof(glm::vec3)};
    abcg::glVertexAttribPointer(colorAttribute, 3, GL_FLOAT, GL_FALSE,
                                sizeof(Vertex),
                                reinterpret_cast<void*>(offset));
  }

  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dice.m_EBO);

  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);
  abcg::glBindVertexArray(0);
  
  dice.m_rotation = {0, 0, 0};  
  dice.velocidadeAngular = {0.0f, 0.0f, 0.0f};
  dice.myTime = 0.0f;
  dice.quadros=0;

  std::uniform_real_distribution<float> fdist(-1.5f,1.5f);
  dice.translation = {fdist(m_randomEngine),fdist(m_randomEngine),0.0f};
  pararDado(dice);

  return dice;
}

void Dices::jogarDado(Dice &dice){

  const float FPS = ImGui::GetIO().Framerate;
  std::uniform_int_distribution<int> idist((int)FPS * 2, (int)FPS * 5);
  dice.maxQuadros = idist(m_randomEngine);

  alterarSpin(dice);
  dice.dadoGirando = true;
}

void Dices::pararDado(Dice &dice) {
  auto seed{std::chrono::steady_clock::now().time_since_epoch().count()};
  m_randomEngine.seed(seed);
  dice.quadros = 0;
  dice.dadoGirando = false;
  dice.m_rotation = {0,0,0};

  std::uniform_int_distribution<int> idist(1,6);
  const int numeroDoDado = idist(m_randomEngine);
  dice.m_angle.x = glm::radians(angulosRetos[numeroDoDado].x);
  dice.m_angle.y = glm::radians(angulosRetos[numeroDoDado].y);
}

void Dices::alterarSpin(Dice &dice){
  dice.m_rotation = {0, 0, 0};
  std::uniform_int_distribution<int> idist(0,2);
  dice.m_rotation[idist(m_randomEngine)] = 1;
  const float FPS = ImGui::GetIO().Framerate;
  std::uniform_real_distribution<float> frameDist(FPS * 4, FPS * 8);
  dice.velocidadeAngular = {glm::radians(frameDist(m_randomEngine))
                      ,glm::radians(frameDist(m_randomEngine))
                      ,glm::radians(frameDist(m_randomEngine))};

  std::uniform_real_distribution<float> fdist(m_deltaTime / 200.0f, m_deltaTime / 100.0f);
  dice.velocidadeDirecional.x = fdist(m_randomEngine) * m_viewportSize.x;
  dice.velocidadeDirecional.y = fdist(m_randomEngine) * m_viewportSize.y;
}

void Dices::checkCollisions(Dice &current_dice) {
  for(auto &dice : m_dices) {
    if(&dice != &current_dice)
    {
      const auto distance{
          glm::distance(current_dice.translation, dice.translation)};

      if (distance < 1.2f) {
        if(!current_dice.dadoColidindo) {
          current_dice.dadoColidindo = true;
          current_dice.movimentoDado.x = !current_dice.movimentoDado.x;
          current_dice.movimentoDado.y = !current_dice.movimentoDado.y;
        }
        return;
      }
    }
  }
  current_dice.dadoColidindo = false;
  return;
}

void Dices::destroy(){
  for(auto dice : m_dices){
    abcg::glDeleteBuffers(1, &dice.m_EBO);
    abcg::glDeleteBuffers(1, &dice.m_VBO);
    abcg::glDeleteVertexArrays(1, &dice.m_VAO);
  }
}