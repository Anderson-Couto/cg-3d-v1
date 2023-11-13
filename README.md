# Dice

# Membros

Anderson Couto Cardoso (RA 21017415)

## Introdução

Aplicação desenvolvida para o curso Computação Gráfica, com o intuito de mostrar gráficos 3D com Open GL.

## Descrição da Aplicação

Em Dice, o usuário deve selecionar uma quantidade de dados a serem jogados. Após definido, a aplicação inicia ao pressionar a tecla "SPACE" ou clicado com o "BOTÃO ESQUERDO" na tela, e então aquela quantidade de dados irá rotacionar pela tela e sorteará números de 1 a 6 em cada dado.

É possivel acessar a aplicação pelo link: https://anderson-couto.github.io/cg-3d-v1/public/index.html

## Estrutura

Neste projeto, foi utilizada a linguagem C++, assim como as bibliotecas `ABCg` e `ImGui`. Por fim, foi utilizada a API `OpenGL` para renderização gráfica.
A estrutura do codigo ficou da seguinte forma:

- `main.cpp`: Classe base do software;
- `window.hpp`: Define a interface da classe Window, que herda da classe `abcg::OpenGLWindow`. Essa classe é destinada para a criação de uma janela OpenGL e manipulação de eventos relacionados.
- `window.cpp`: Implementação dos métodos e atributos da classe Window. É aqui que são invocados os shaders, renderização dos objetos e atribuições de regras de negócio que atendem todo a aplicação;
- `gamedata.hpp`: Abstração responsável por guardar estados da aplicação;
- `dices.hpp`: Classe com métodos e atributos relacionados aos dados;
- `dices.cpp`: Implementação dos métodos e atributos da classe Dices;
- `assets\dice.frag`: _Fragment shader_, é o objeto que possui as propriedades do dado e programa como os pixels são exibidos na tela.
- `assets\dice.vert`: _Vertex shader_, é o objeto que possui as propriedades de vértices do dado.
- `assets\dice.mtl`: contêm uma ou mais definições de material, cada uma das quais inclui a cor, a textura e o mapa de reflexão de materiais individuais dos dados.
- `assets\dice.obj`: é a representação 3D de um dado.

## Desenvolvimento e Code Review

Abaixo podemos encontrar mais detalhes sobre o desenvolvimento da aplicação

### main.cpp

Não há grandes novidades aqui. O código segue o mesmo padrão do que foi desenvolvido ao longo do curso.

### gamedata.hpp

Aqui ocorre apenas a gestão do input de rolar o dado.

```
enum class Input { Roll };

struct GameData {
    std::bitset<1> m_input;  // [ roll ]
};
```

### window.hpp

O código inclui três bibliotecas: `vector` para utilizar vetores, `random` para geração de números aleatórios, `abcgOpenGL.hpp` que contém funcionalidades relacionadas à biblioteca gráfica OpenGL, e `dices.hpp` que contém a definição da classe Dices.

```#include <vector>
#include <random>
#include "abcgOpenGL.hpp"
#include "dices.hpp"
```

`Window` herda da classe abcg::OpenGLWindow. Isso implica que essa classe possui funcionalidades específicas para a criação de uma janela OpenGL.
Além disso, ela contém funções de membros protegidas, nas quais são chamadas em diferentes momentos do ciclo de vida do OpenGL. Abaixo, segue um resumo:

- onEvent(): Chamada para fazer a gestão de eventos relacionados a teclado e mouse.
- onCreate(): Chamada quando a janela é criada.
- onPaint(): Chamada para renderizar o conteúdo da janela.
- onPaintUI(): Chamada para renderizar a interface do usuário.
- onResize(): Chamada quando a janela é redimensionada.
- onDestroy(): Chamada quando a janela é destruída.

E também possui declaração de funções e membros privados, com as seguintes responsabilidades:

- loadModelFromFile(): Carrega os dados do modelo a partir do arquivo no formato OBJ.
- toDefault(): Realiza centralização do modelo na origem e aplica escala.
- m_program: Identificador do programa OpenGL.
- m_vertices: Vetor de vértices lido de um arquivo OBJ, que será enviado ao Vertex Buffer Object (VBO).
- m_indices: Vetor de índices lido de um arquivo OBJ, que será enviado ao Element Buffer Object (EBO).
- m_verticesToDraw: Quantidade de vértices do VBO que será processada pela função de renderização glDrawElements.
- m_dices: Objeto da classe Dices.
- quantity: Quantidade de dados membros específicos da classe.
- m_viewportSize: Tamanho da viewport.
- m_gameData: Indentificador da abstração de estados da aplicação.

### window.cpp

O código inclui várias bibliotecas necessárias para a execução do programa, como `fmt` para formatação de strings, `imgui` para a criação de interfaces gráficas, `tiny_obj_loader` para carregar modelos OBJ, `glm` para operações matemáticas, e `cppitertools` para iteração.

```
#include "window.hpp"
#include <fmt/core.h>
#include <imgui.h>
#include <tiny_obj_loader.h>
#include <cppitertools/itertools.hpp>
#include <glm/gtx/fast_trigonometry.hpp>
#include <glm/gtx/hash.hpp>
#include <unordered_map>
```

Abaixo segue a especialização explícita da função hash do tipo Vertex. Essa especialização permite que objetos do tipo Vertex sejam usados como chaves em contêineres da STL que requerem hash.

```
template <> struct std::hash<Vertex> {
  size_t operator()(Vertex const &vertex) const noexcept {
    auto const h1{std::hash<glm::vec3>()(vertex.position)};
    return h1;
  }
};
```

#### Função Window::onEvent():

Essa função é chamada quando há interação do usuário com teclado e mouse. Ela verifica o tipo de evento recebido e define o estado correlacionado.

- Evento de Click na Tecla "SPACE":

```
if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
  m_gameData.m_input.set(static_cast<size_t>(Input::Roll));
}
```

- Evento de Click no Botão Esquerdo do Mouse:

```
if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
    m_gameData.m_input.set(static_cast<size_t>(Input::Roll));
}
```

- Evento de Release na Tecla "SPACE":

```
if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_SPACE) {
  m_gameData.m_input.reset(static_cast<size_t>(Input::Roll));
}
```

- Evento de Release no Botão Esquerdo do Mouse:

```
if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
    m_gameData.m_input.reset(static_cast<size_t>(Input::Roll));
}
```

#### Função Window::onCreate():

Essa função é chamada quando a janela é criada. Ela configura o programa OpenGL, carrega um modelo 3D a partir de um arquivo `.obj`, padroniza o modelo e cria instâncias da classe `Dices`. A especialização explícita de `std::hash` é utilizada para manipular objetos do tipo Vertex como chaves em um contêiner.

- Criação do Programa OpenGL:
  Cria um programa OpenGL composto pelos shaders vertex e fragment fornecidos nos arquivos "dice.vert" e "dice.frag", respectivamente.

```
m_program =
    abcg::createOpenGLProgram(
      {
        {
          .source = assetsPath + "dice.vert",
          .stage = abcg::ShaderStage::Vertex
        },
        {
          .source = assetsPath + "dice.frag",
          .stage = abcg::ShaderStage::Fragment
        }
      }
    );
```

- Carregamento do Modelo:
  Chama a função loadModelFromFile para carregar o modelo do arquivo "dice.obj" e preencher os vetores m_vertices e m_indices.

```
loadModelFromFile(assetsPath + "dice.obj");
```

- Padronização do Modelo:
  Chama a função toDefault para centralizar e normalizar as coordenadas do modelo.

```
toDefault();
```

- Configuração de Tamanho do Ponto (Opcional):
  Habilita o tamanho variável de pontos se o código não estiver sendo compilado para Emscripten

```
#if !defined(__EMSCRIPTEN__)
  abcg::glEnable(GL_PROGRAM_POINT_SIZE);
#endif
```

- Criação de Dices:
  Configura a quantidade de vértices a serem desenhados e chama a função create do objeto m_dices para criar as instâncias necessárias para renderizar os dados.

```
m_verticesToDraw = m_indices.size();
m_dices.create(m_program, quantity, m_vertices, m_indices, m_verticesToDraw);
```

#### Função Window::loadModelFromFile():

- Leitura do Arquivo OBJ:

```
tinyobj::ObjReader reader;

if (!reader.ParseFromFile(path.data())) {
  // Tratamento de erro caso falhe ao ler o arquivo OBJ
}
```

- Iteração sobre Vértices e Triângulos:
  Itera sobre os shapes e os índices do modelo OBJ

```
for (const auto& shape : shapes) {
  for (const auto offset : iter::range(shape.mesh.indices.size())) {
    const tinyobj::index_t index{shape.mesh.indices.at(offset)};
    // ...
  }
}
```

- Acesso às Propriedades do Vértice:
  Calcula o índice inicial do vértice e extrai as coordenadas x, y e z do vértice

```
const int startIndex{3 * index.vertex_index};
const float vx{attrib.vertices.at(startIndex + 0)};
const float vy{attrib.vertices.at(startIndex + 1)};
const float vz{attrib.vertices.at(startIndex + 2)};
```

- Criação do Vértice:

```
Vertex vertex{};
vertex.position = {vx, vy, vz};
vertex.color = {(float)material_id, (float)material_id, (float)material_id};
```

- Hashing para Remover Vértices Duplicados:

```
if (hash.count(vertex) == 0) {
  hash[vertex] = m_vertices.size();
  m_vertices.push_back(vertex);
}
```

- Adição de Índices:
  Adiciona os índices ao vetor m_indices.

```
m_indices.push_back(hash[vertex]);
```

#### Função Window::toDefault():

- Encontrar Mínimos e Máximos:

```
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
```

- Cálculo do Centro e Fator de Escala:
  Calcula o centro da caixa delimitada pelos mínimos e máximos e o fator de escala necessário para ajustar o modelo ao volume de visão do pipeline gráfico

```
const auto center{(min + max) / 2.0f};
const auto scaling{2.0f / glm::length(max - min)};
```

- Aplicação da Centralização e Escala a Todos os Vértices:
  Itera sobre todos os vértices no vetor m_vertices e centraliza cada vértice na origem, aplicando a escala.

```
for (auto& vertex : m_vertices) {
  vertex.position = (vertex.position - center) * scaling;
}
```

#### Função Window::onPaint():

- Limpeza do Buffer de Cor e Profundidade:
  Limpa o buffer de cor e profundidade antes de renderizar a cena.

```
abcg::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
```

- Definição da Viewport:
  Define a viewport para a janela com base no tamanho da janela.

```
abcg::glViewport(0, 0, m_viewportSize.x, m_viewportSize.y);
```

- Chamada da Função m_dices.paint():
  Chama a função paint do objeto m_dices para renderizar os dados na tela.

```
m_dices.paint(m_viewportSize, getDeltaTime());
```

#### Função Window::onPaintUI():

- Configuração de Janela de Opções com ImGui:
  Configuração de uma janela de opções usando ImGui, posicionando-a e definindo seu tamanho.

```
ImGui::SetNextWindowPos(ImVec2(m_viewportSize.x - 138, m_viewportSize.y - 40));
ImGui::SetNextWindowSize(ImVec2(128, 35));
ImGui::Begin("Button window", nullptr, ImGuiWindowFlags_NoDecoration);
```

- Evento para Jogar Dado:
  Gestão de evento na interface gráfica que chama a função jogarDado para cada dado no objeto m_dices.

```
if(m_gameData.m_input[static_cast<size_t>(Input::Roll)]){
  for(auto &dice : m_dices.m_dices){
    m_dices.jogarDado(dice);
  }
}
```

- Menu Suspenso para Seleção de Quantidade de Dados:
  Criação de um menu suspenso usando ImGui para selecionar a quantidade de dados a serem jogados. Atualiza a quantidade e chama onCreate quando o valor é alterado.

```
const std::vector<std::string> comboItems{"1", "2", "3", "4", "5"};
ImGui::PushItemWidth(70);
if (ImGui::BeginCombo("Dados", comboItems.at(currentIndex).c_str())) {
  // ...
}
ImGui::PopItemWidth();
if(quantity != (int)currentIndex + 1){
  quantity = currentIndex + 1;
  onCreate();
}
```

#### Função Window::onResize():

- Atualização do Tamanho da Viewport:
  Atualiza o tamanho da viewport quando a janela é redimensionada.

```
m_viewportSize = size;
```

#### Função Window::onDestroy():

- Limpeza de Recursos ao Fechar a Janela:
  Deleta o programa OpenGL e chama a função destroy do objeto m_dices quando a janela é fechada.

```
abcg::glDeleteProgram(m_program);
m_dices.destroy();
```

### dices.hpp

O código inclui duas bibliotecas: `abcgOpenGL.hpp`, que contém funcionalidades relacionadas à biblioteca gráfica OpenGL, e `vector` para utilizar vetores.

```
#include <vector>
#include <random>
#include "abcgOpenGL.hpp"
#include "dices.hpp"
```

`Dices` possui membros e métodos que são essenciais para manipular e renderizar dados em um ambiente OpenGL.

- create(): Inicializa a classe Dices, configurando o programa OpenGL, vértices, índices e a quantidade de dados. Cria os buffers OpenGL necessários.
- paint(): Renderiza os dados, atualizando suas posições e rotações com base no deltaTime fornecido e no tamanho atual da viewport.
- destroy(): Limpa os recursos OpenGL quando o objeto Dices não é mais necessário.

E também possui declaração de funções membros privados, com as seguintes responsabilidades:

- struct Vertex: Define uma estrutura representando um vértice com uma posição 3D e cor. Inclui um operador de igualdade para verificar se dois vértices são iguais com base em suas posições.
- struct Dice: Representa dados individuais com atributos como ângulos de rotação, translação e velocidades.
- m_dices: Lista de instâncias de dados gerenciadas pela classe Dices.
- angulosRetos: Lista de ângulos representando rotações onde uma face do dado está virada para a tela.
- m_randomEngine: Motor de geração de números pseudoaleatórios usado para várias randomizações.
- inicializarDado(): Inicializa um novo objeto Dice com valores padrão.
- jogarDado(): Simula o lançamento de um dado, atualizando seus parâmetros de rotação, translação e tempo.
- pararDado(): Lida com a lógica de um dado pousando, atualizando seu estado quando ele para.
- alterarSpin(): Gera uma velocidade angular e direcional aleatória para um dado.
- checkCollisions(): Verifica colisões entre dados e atualiza seus estados conforme necessário.

### dices.cpp

O código em dices.cpp define as funções responsáveis pela manipulação e renderização de dados tridimensionais. Vamos explorar cada função de maneira mais detalhada, destacando pontos-chave.

#### Função Dices::create:

```
void Dices::create(GLuint program, int quantity, std::vector<Vertex> vertices, std::vector<GLuint> indices, int verticesToDraw) {
    destroy();  // Limpa dados existentes

    // Inicializa gerador de números pseudoaleatórios
    auto seed{std::chrono::steady_clock::now().time_since_epoch().count()};
    m_randomEngine.seed(seed);

    m_program = program;
    m_vertices = vertices;
    m_indices = indices;
    m_verticesToDraw = verticesToDraw;

    m_dices.clear();
    m_dices.resize(quantity);

    // Itera sobre os dados e os inicializa
    for (auto &dice : m_dices) {
        dice = inicializarDado();
    }
}
```

A função começa destruindo dados existentes por meio da chamada à função destroy. Em seguida, inicializa o gerador de números pseudoaleatórios com base no tempo atual e configura os membros da classe Dices com os valores fornecidos. Por fim, itera sobre os dados (Dice) e chama a função inicializarDado para cada um.

#### Função Dices::paint:

```
void Dices::paint(glm::ivec2 size, float deltaTime) {
    // ...

    for (auto &dice : m_dices) {
        // ...

        if (dice.dadoGirando) {
            // Lógica de rotação do dado

            // Ir para a direita/esquerda/cima/baixo
            // ...

            // Finalizar o giro do dado
            if (dice.quadros > dice.maxQuadros) {
                pararDado(dice);
            }
        }

        // Atualizar variáveis do shader
        // ...

        // Desenhar triângulos
        abcg::glDrawElements(GL_TRIANGLES, m_verticesToDraw, GL_UNSIGNED_INT, nullptr);

        abcg::glBindVertexArray(0);
    }

    // ...
}
```

A função itera sobre os dados e realiza a renderização de cada um. Então, verifica se um dado está girando e simula o movimento até que ele pouse em uma face. Ao final do processo, atualiza as variáveis uniformes nos shaders com base nas propriedades dos dados.

#### Função Dices::inicializarDado:

```
Dices::Dice Dices::inicializarDado() {
    Dice dice;

    // ...

    // Gerar VBO
    // ...

    // Gerar EBO
    // ...

    // Criar VAO
    // ...

    // Estado inicial
    // ...

    // Geração de posição aleatória
    // ...

    pararDado(dice);  // Começar em um número aleatório

    return dice;
}
```

A função cria e configura buffers OpenGL e define o estado inicial das variáveis do dado. Ao longo da execução, gera uma posição inicial aleatória e finaliza chamando a função pararDado para começar o dado em um número aleatório.

#### Função Dices::jogarDado:

```
void Dices::jogarDado(Dice &dice) {
    const float FPS = ImGui::GetIO().Framerate;
    std::uniform_int_distribution<int> idist((int)FPS * 2, (int)FPS * 5);
    dice.maxQuadros = idist(m_randomEngine);

    alterarSpin(dice);
    dice.dadoGirando = true;
}
```

Configura um dado a ser lançado. Ela define o número máximo de quadros para girar o dado, chama alterarSpin para definir a rotação do dado e marca o dado como girando.

#### Função Dices::pararDado:

```
void Dices::pararDado(Dice &dice) {
    // ...

    // Lógica para determinar o número final do dado
    // ...

    dice.m_angle.x = glm::radians(angulosRetos[numeroDoDado].x);
    dice.m_angle.y = glm::radians(angulosRetos[numeroDoDado].y);
}
```

Lógica para pousar o dado, incluindo a reinicialização de variáveis. Ao final, determina aleatoriamente o número final do dado e configura as rotações correspondentes.

#### Função Dices::alterarSpin:

```
dice.m_rotation = {0, 0, 0};
std::uniform_int_distribution<int> idist(0, 2);
dice.m_rotation[idist(m_randomEngine)] = 1;
const float FPS = ImGui::GetIO().Framerate;
std::uniform_real_distribution<float> frameDist(FPS * 4, FPS * 8);
dice.velocidadeAngular = {
    glm::radians(frameDist(m_randomEngine)),
    glm::radians(frameDist(m_randomEngine)),
    glm::radians(frameDist(m_randomEngine))
  };

std::uniform_real_distribution<float> fdist(m_deltaTime / 200.0f, m_deltaTime / 100.0f);
dice.velocidadeDirecional.x = fdist(m_randomEngine) * m_viewportSize.x;
dice.velocidadeDirecional.y = fdist(m_randomEngine) * m_viewportSize.y;

```

A função `alterarSpin` é responsável por configurar a rotação do dado (`dice`) quando ele está girando. Ela redefine as rotações em torno dos eixos x, y ou z, escolhido aleatoriamente. Além disso, define as velocidades angulares para cada eixo e as velocidades direcionais para controlar o movimento do dado.

#### Função Dices::checkCollisions:

```
for (auto &dice : m_dices) {
  if (&dice != &current_dice) {
      const auto distance{glm::distance(current_dice.translation, dice.translation)};

      if (distance < 1.2f) {
          if (!current_dice.dadoColidindo) {
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
```

A função `checkCollisions` verifica se há colisões entre o dado atual (current_dice) e outros dados (m_dices). Se houver uma colisão, ela inverte os movimentos do dado para simular um rebote.

#### Função Dices::destroy:

```
for (auto dice : m_dices) {
    abcg::glDeleteBuffers(1, &dice.m_EBO);
    abcg::glDeleteBuffers(1, &dice.m_VBO);
    abcg::glDeleteVertexArrays(1, &dice.m_VAO);
}
```

A função `destroy` libera os recursos associados aos dados existentes. Para cada dado, ela deleta os buffers e arrays de vértices.

### dice.frag

Esse sombreador esta relacionado ao sombreamento das superfícies, ajustando a intensidade da cor com base na coordenada de profundidade para criar uma aparência de sombreamento.

- `#version 300 es`: Indica a versão da linguagem do sombreador. Neste caso, é a versão 3.00 do GLSL destinada ao OpenGL ES (OpenGL for Embedded Systems).
- `precision mediump float;`: Define a precisão média para números de ponto flutuante. mediump é uma precisão intermediária.
- `in vec4 fragColor;`: Declara uma variável de entrada chamada fragColor do tipo vec4 (vetor de quatro componentes de ponto flutuante). Essa variável provavelmente é alimentada pelo sombreador de vértices.
- `out vec4 outColor;`: Declara uma variável de saída chamada outColor do tipo vec4. O valor atribuído a outColor será a cor final do fragmento.
- `float i = 1.3 - gl_FragCoord.z;`: Calcula um valor i baseado na coordenada de profundidade z do fragmento. 1.3 - gl_FragCoord.z significa que quanto mais próximo z for de 1.3, menor será i. Isso cria uma espécie de sombreamento, onde fragmentos mais próximos ao observador são mais claros e os mais distantes são mais escuros.
- `if (gl_FrontFacing) {...} else {...}`: Testa se a face do fragmento é a face da frente. Se for, aplica uma cor calculada multiplicando cada componente de fragColor por i, dando um tom entre branco e cinza. Se não for, atribui uma cor fixa de cinza escuro.
- `outColor = ...`: Define a cor final do fragmento que será enviada ao buffer de cor. Se a face for a frente, a cor é ajustada dinamicamente com base na intensidade i. Se for a parte de trás, a cor fixa é aplicada.

### dice.vert

- `#version 300 es`: Indica a versão da linguagem do sombreador. Neste caso, é a versão 3.00 do GLSL destinada ao OpenGL ES (OpenGL for Embedded Systems).
- `layout(location = 0) in vec3 inPosition;`: Declara a variável de entrada inPosition como um vetor de 3 componentes (vec3), representando a posição (x, y, z) do vértice. O layout(location = 0) indica que esta variável está associada ao índice de localização 0.
- `layout(location = 1) in vec3 inColor;`: Similar à inPosition, mas para a cor do vértice. O layout(location = 1) indica que esta variável está associada ao índice de localização 1.
- `uniform vec3 translation;`: Declaração de uma variável uniforme chamada translation do tipo vec3. Variáveis uniformes são constantes ao longo de um primitivo de renderização e são fornecidas pela aplicação.
- `uniform float rotationX;, uniform float rotationY;, uniform float rotationZ;`: Variáveis uniformes para as rotações em torno dos eixos x, y e z, respectivamente.
- `out vec4 fragColor;`: Declaração da variável de saída fragColor do tipo vec4 que será passada para o sombreador de fragmentos.
- `vec3 newPosition = inPosition;`: Cria uma nova variável chamada newPosition e a inicializa com a posição do vértice de entrada inPosition.

- Blocos condicionais para rotações em torno dos eixos X, Y e Z:

`if (rotationX > 0.0f) {...}`: Se rotationX for maior que 0, aplica uma rotação em torno do eixo X usando as fórmulas de rotação.
`if (rotationY > 0.0f) {...}`: Se rotationY for maior que 0, aplica uma rotação em torno do eixo Y.
`if (rotationZ > 0.0f) {...}`: Se rotationZ for maior que 0, aplica uma rotação em torno do eixo Z.

- `gl_Position = vec4(newPosition + translation, 2.0f);`: Define a posição final do vértice no espaço homogêneo. A posição é calculada adicionando a newPosition (após as rotações) ao vetor de translação translation. O último valor (2.0f) indica que o vértice deve ser deslocado um pouco para trás, tornando-o menor.
- `fragColor = vec4(inColor, 1.0f);`: Atribui à fragColor a cor do vértice de entrada inColor, com um componente alfa de 1.0f (opacidade total).

## Dificuldades Encontradas e Pontos de Melhoria

Ao selecionar mais de um dado a ser jogado, existe a possibilidade de haver sobreposição de objetos. Esse problema se estende até mesmo após o evento de rolar os dados ser acionado, pois em certos casos ocorre uma amarração das entidades, e a checagem de colisões, que deveria separa-los, nao tem o efeito desejado.

## Referências

https://free3d.com/3d-model/dice-34662.html

# About ABCg

![build workflow](https://github.com/hbatagelo/abcg/actions/workflows/build.yml/badge.svg)
[![GitHub release (latest by date)](https://img.shields.io/github/v/release/hbatagelo/abcg)](https://github.com/hbatagelo/abcg/releases/latest)

Development framework accompanying the course [MCTA008-17 Computer Graphics](http://professor.ufabc.edu.br/~harlen.batagelo/cg/) at [UFABC](https://www.ufabc.edu.br/).

[Documentation](https://hbatagelo.github.io/abcg/abcg/doc/html/) | [Release notes](CHANGELOG.md)

ABCg is a lightweight C++ framework that simplifies the development of 3D graphics applications based on [OpenGL](https://www.opengl.org), [OpenGL ES](https://www.khronos.org), [WebGL](https://www.khronos.org/webgl/), and [Vulkan](https://www.vulkan.org). It is designed for the tutorials and assignments of the course "MCTA008-17 Computer Graphics" taught at Federal University of ABC (UFABC).

---

## Main features

- Supported platforms: Linux, macOS, Windows, WebAssembly.
- Supported backends: OpenGL 3.3+, OpenGL ES 3.0+, WebGL 2.0 (via Emscripten), Vulkan 1.3.
- Applications that use the common subset of functions between OpenGL 3.3 and OpenGL ES 3.0 can be built for WebGL 2.0 using the same source code.
- OpenGL functions can be qualified with the `abcg::` namespace to enable throwing exceptions with descriptive GL error messages that include the source code location.
- Includes helper classes and functions for loading textures (using [SDL_image](https://www.libsdl.org/projects/SDL_image/)), loading OBJ 3D models (using [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader)), and compiling GLSL shaders to SPIR-V with [glslang](https://github.com/KhronosGroup/glslang).

---

## Requirements

The following minimum requirements are shared among all platforms:

- [CMake](https://cmake.org/) 3.21.
- A C++ compiler with at least partial support for C++20 (tested with GCC 12, Clang 16, MSVC 17, and emcc 3.1.42).
- A system with support for OpenGL 3.3 (OpenGL backend) or Vulkan 1.3 (Vulkan backend). Conformant software rasterizers such as Mesa's [Gallium llvmpipe](https://docs.mesa3d.org/drivers/llvmpipe.html) and lavapipe (post Jun 2022) are supported. Mesa's [D3D12](https://devblogs.microsoft.com/directx/directx-heart-linux/) backend on [WSL 2.0](https://docs.microsoft.com/en-us/windows/wsl/install) is supported as well.

For WebAssembly:

- [Emscripten](https://emscripten.org/).
- A browser with support for WebGL 2.0.

For building desktop applications:

- [SDL](https://www.libsdl.org/) 2.0.
- [SDL_image](https://www.libsdl.org/projects/SDL_image/) 2.0.
- [GLEW](http://glew.sourceforge.net/) 2.2.0 (required for OpenGL-based applications).
- [Vulkan](https://www.lunarg.com/vulkan-sdk/) 1.3 (required for Vulkan-based applications).

Desktop dependencies can be resolved automatically with [Conan](https://conan.io/), but it is disabled by default. To use Conan, install Conan 1.47 or a later 1.\* version (ABCg is not compatible with Conan 2.0!) and then configure CMake with `-DENABLE_CONAN=ON`.

The default renderer backend is OpenGL (CMake option `GRAPHICS_API=OpenGL`). To use the Vulkan backend, configure CMake with `-DGRAPHICS_API=Vulkan`.

---

## Installation and usage

Start by cloning the repository:

    # Get abcg repo
    git clone https://github.com/hbatagelo/abcg.git

    # Enter the directory
    cd abcg

Follow the instructions below to build the "Hello, World!" sample located in `abcg/examples/helloworld`.

### Windows

- Run `build-vs.bat` for building with the Visual Studio 2022 toolchain.
- Run `build.bat` for building with GCC (MinGW-w64).

`build-vs.bat` and `build.bat` accept two optional arguments: (1) the build type, which is `Release` by default, and (2) an extra CMake option. For example, for a `Debug` build with `-DENABLE_CONAN=ON` using VS 2022, run

    build-vs.bat Debug -DENABLE_CONAN=ON

### Linux and macOS

Run `./build.sh`.

The script accepts two optional arguments: (1) the build type, which is `Release` by default, and (2) an extra CMake option. For example, for a `Debug` build with `-DENABLE_CONAN=ON`, run

    ./build.sh Debug -DENABLE_CONAN=ON

### WebAssembly

1.  Run `build-wasm.bat` (Windows) or `./build-wasm.sh` (Linux/macOS).
2.  Run `runweb.bat` (Windows) or `./runweb.sh` (Linux/macOS) for setting up a local web server.
3.  Open <http://localhost:8080/helloworld.html>.

---

## Docker setup

ABCg can be built in a [Docker](https://www.docker.com/) container. The Dockerfile provided is based on Ubuntu 22.04 and includes Emscripten.

1.  Create the Docker image (`abcg`):

        sudo docker build -t abcg .

2.  Create the container (`abcg_container`):

        sudo docker create -it \
          -p 8080:8080 \
          -v /tmp/.X11-unix:/tmp/.X11-unix:rw \
          -e DISPLAY \
          --name abcg_container abcg

3.  Start the container:

        sudo docker start -ai abcg_container

    On NVIDIA GPUs, install the [NVIDIA Container Toolkit](https://github.com/NVIDIA/nvidia-docker) to allow the container to use the host's NVIDIA driver and X server. Expose the X server with `sudo xhost +local:root` before starting the container.

---

## License

ABCg is licensed under the MIT License. See [LICENSE](https://github.com/hbatagelo/abcg/blob/main/LICENSE) for more information.
