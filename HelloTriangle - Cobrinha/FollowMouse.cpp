/* Segue o Mouse - código adaptado de https://learnopengl.com/Getting-started/Hello-Triangle
 *
 * Adaptado por Rossana Baptista Queiroz
 * para a disciplina de Fundamentos de Computação Gráfica - Unisinos
 * Versão inicial: 05/10/2024 (ver gravação da aula)
 * Última atualização em 17/10/2024
 *
 * Este programa desenha um triângulo que segue o cursor do mouse
 * usando OpenGL e GLFW.
 * A posição e a rotação do triângulo são calculadas com base no movimento do mouse.
 */

#include <iostream>
#include <string>
#include <assert.h>

// Bibliotecas GLAD para carregar funções OpenGL
#include <glad/glad.h>

// Biblioteca GLFW para criar janelas e gerenciar entrada de teclado/mouse
#include <GLFW/glfw3.h>

// GLM para operações matemáticas (vetores, matrizes)
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <vector>

using namespace std;
using namespace glm;

// Constantes
const float Pi = 3.14159265;
const GLuint WIDTH = 800, HEIGHT = 600; // Dimensões da janela

// Estrutura para armazenar informações sobre as geometrias da cena
struct Geometry {
    GLuint VAO;        // Vertex Array Geometry
    vec3 position;     // Posição do objeto
    float angle;       // Ângulo de rotação
    vec3 dimensions;   // Escala do objeto (largura, altura)
    vec3 color;        // Cor do objeto
    int nVertices;     // Número de vértices a desenhar
};

// Variáveis globais
bool keys[1024];   // Estados das teclas (pressionadas/soltas)
vec2 mousePos;     // Posição do cursor do mouse
vec3 dir = vec3(0.0, -1.0, 0.0); // Vetor direção (do objeto para o mouse)

//vector que armazena todos os segmentos da cobrina ( inclindo cabeça)
vector<Geometry> cobrinha;

//objeto geometry que representa os olhos da cabeça
Geometry eyes;


// Protótipos das funções
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
int setupShader();           // Função para configurar os shaders
int setupGeometry();         // Função para configurar a geometria (triângulo)
void drawGeometry(GLuint shaderID, GLuint VAO, int nVertices, vec3 position, vec3 dimensions, float angle, vec3 color, GLuint drawingMode = GL_TRIANGLE_FAN, int offset = 0, vec3 axis = vec3(0.0, 0.0, 1.0));


int createCircle(int nPoints, float radius, float xc = 0, float yc = 0) {
    // Vetor para armazenar os vértices do círculo
    vector<GLfloat> vertices;
    // Ângulo inicial e incremento para cada ponto do círculo
    float angle = 0.0;
    float slice = 2 * Pi / static_cast<float>(nPoints);
    // Adiciona o ponto central do círculo (0.0, 0.0, 0.0)
    vertices.push_back(xc); // Xc
    vertices.push_back(yc); // Yc
    vertices.push_back(0.0f); // Zc
    // Calcula as coordenadas de cada ponto do círculo e adiciona ao vetor de vértices
    for (int i = 0; i < nPoints + 1; i++) {
        float x = xc + radius * cos(angle);
        float y = yc + radius * sin(angle);
        float z = 0.0f;
        vertices.push_back(x); // Coordenada X
        vertices.push_back(y); // Coordenada Y
        vertices.push_back(z); // Coordenada Z
        angle += slice; // Incrementa o ângulo para o próximo ponto
}
    // Identificadores para o VBO (Vertex Buffer Object) e VAO (Vertex Array Object)
    GLuint VBO, VAO;

    // Geração do identificador do VBO e vinculação
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Envia os dados do vetor de vértices para a GPU
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    // Geração do identificador do VAO e vinculação
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Configuração do ponteiro de atributos para os vértices
    // layout (location = 0) no Vertex Shader, 3 componentes por vértice (x, y, z)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Desvincula o VBO e o VAO para evitar modificações acidentais
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Retorna o identificador do VAO, que será utilizado para desenhar o círculo
    return VAO;
}


//cria o segmento da cobrinha retornando um obj geometry com a posição e cor apropriados
// i: indice do segmento ( 0 para a cabeça, indices maiors para o corpo)
// dir:vector direção indicando a direção inicial do segmento
Geometry createSegment(int i, vec3 dir)
{
    const float minDistance = 15.0f; // Distância mínima entre os segmentos da cobrinha
    cout << "Criando segmento " << i << endl;
    // Inicializa um objeto Geometry para armazenar as informações do segmento
    Geometry segment;
    segment.VAO = createCircle(32, 0.5f ); // Cria a geometria do segmento como um círculo
    segment.nVertices = 34; // Número de vértices do círculo
    // Define a posição inicial do segmento
    if (i == 0) { // Cabeça
         segment.position = vec3(400.0, 300.0, 0.0); // Posição inicial no centro da tela
    } 
    else {
        // Ajusta a direção com base na posição dos segmentos anteriores para evitar sobreposição
        if (i >= 2)
        dir = normalize(cobrinha[i - 1].position - cobrinha[i - 2].position);
        // Posiciona o novo segmento com uma distância mínima do segmento anterior
        segment.position = cobrinha[i - 1].position + minDistance * dir;

}

segment.dimensions=vec3(50,50,1.0); //define as dimensões do segmento (tamanho do circulo)
segment.angle=0.0;  //Angulo inicial (sem rotação)

//altera a cor do segmento entre azul e amarelo dependendo do indice
if(i % 2 ==0){
    segment.color=vec3(0,0,1); // azul para segmento PAR
}
else{
    segment.color=vec3(1,1,0); // Amarelo para segmento Impar
}

return segment;
}


int createEyes(int nPoints, float radius){

vector<GLfloat> vertices;

//Ângulo inicial e incremento para cada ponto do circulo
float angle = 0.0;
float slice = 2*Pi/static_cast<float>(nPoints);

//Posições iniciais para os circulos dos olhos
float xi=0.125f ;// posição inicial X das escleras
float yi= 0.3f  ;// posição inicial Y das escleras
radius = 0.225f; //raio das escleras

// Olho esquerdo (esclera)
vertices.push_back(xi); // Xc
vertices.push_back(yi); // Yc
vertices.push_back(0.0f); // Zc

for (int i = 0; i < nPoints + 1; i++) {
float x = xi + radius * cos(angle);
float y = yi + radius * sin(angle);
float z = 0.0f;

vertices.push_back(x); // Coordenada X
vertices.push_back(y); // Coordenada Y
vertices.push_back(z); // Coordenada Z
angle += slice; // Incrementa o ângulo para o próximo ponto
}

// Olho direito (esclera)
angle = 0.0;
vertices.push_back(xi); // Xc
vertices.push_back(-yi); // Yc
vertices.push_back(0.0f); // Zc

for (int i = 0; i < nPoints + 1; i++) {
float x = xi + radius * cos(angle);
float y = -yi + radius * sin(angle);
float z = 0.0f;

vertices.push_back(x); // Coordenada X
vertices.push_back(y); // Coordenada Y
vertices.push_back(z); // Coordenada Z
angle += slice;
}

// Olho esquerdo (pupila)
radius = 0.18f; // Raio das pupilas
xi += 0.09f; // Ajuste de posição para as pupilas
angle = 0.0;
vertices.push_back(xi); // Xc
vertices.push_back(yi); // Yc
vertices.push_back(0.0f); // Zc

for (int i = 0; i < nPoints + 1; i++) {
float x = xi + radius * cos(angle);
float y = yi + radius * sin(angle);
float z = 0.0f;

vertices.push_back(x); // Coordenada X
vertices.push_back(y); // Coordenada Y
vertices.push_back(z); // Coordenada Z
angle += slice;
}

// Olho direito (pupila)
angle = 0.0;
vertices.push_back(xi); // Xc
vertices.push_back(-yi); // Yc
vertices.push_back(0.0f); // Zc

for (int i = 0; i < nPoints + 1; i++) {
float x = xi + radius * cos(angle);
float y = -yi + radius * sin(angle);
float z = 0.0f;

vertices.push_back(x); // Coordenada X
vertices.push_back(y); // Coordenada Y
vertices.push_back(z); // Coordenada Z
angle += slice;
}

// Identificadores para o VBO (Vertex Buffer Object) e VAO (Vertex Array Object)
GLuint VBO, VAO;

// Geração do identificador do VBO e vinculação
glGenBuffers(1, &VBO);
glBindBuffer(GL_ARRAY_BUFFER, VBO);

// Envia os dados do vetor de vértices para a GPU
glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

// Geração do identificador do VAO e vinculação
glGenVertexArrays(1, &VAO);
glBindVertexArray(VAO);

// Configuração do ponteiro de atributos para os vértices
// layout (location = 0) no Vertex Shader, 3 componentes por vértice (x, y, z)
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
glEnableVertexAttribArray(0);

// Desvincula o VBO e o VAO para evitar modificações acidentais
glBindBuffer(GL_ARRAY_BUFFER, 0);
glBindVertexArray(0);

// Retorna o identificador do VAO, que será utilizado para desenhar os olhos (escleras e pupilas)
return VAO;

}

int main() {
    // Inicializa GLFW e configurações de versão do OpenGL
    glfwInit();
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Seguindo o Mouse", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    // Inicializa GLAD para carregar todas as funções OpenGL
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Falha ao inicializar GLAD" << std::endl;
        return -1;
    }

    // Informações sobre o Renderer e a versão OpenGL
    const GLubyte *renderer = glGetString(GL_RENDERER);
    const GLubyte *version = glGetString(GL_VERSION);
    cout << "Renderer: " << renderer << endl;
    cout << "Versão OpenGL suportada: " << version << endl;
    // Constantes do programa
    const float Pi = 3.14159265;     // Valor de Pi, utilizado em cálculos de ângulos e círculos
    const float minDistance = 15.0f; // Distância mínima entre os segmentos da cobrinha
    const float maxDistance = 17.0f; // Distância máxima permitida entre os segmentos da cobrinha
    const float smoothFactor = 0.1f; // Fator de suavização para o movimento dos segmentos
    const GLuint WIDTH = 800;        // Largura da janela da aplicação em pixels
    const GLuint HEIGHT = 600;       // Altura da janela da aplicação em pixels


    // Configurações de viewport
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // Compila o shader e cria a geometria (triângulo)
    GLuint shaderID = setupShader();
    Geometry triangle;
    /* triangle.VAO = setupGeometry();
    triangle.position = vec3(0.0, 0.0, 0.0);
    triangle.color = vec3(1.0, 1.0, 0.0);  // Amarelo
    triangle.dimensions = vec3(50.0, 50.0, 1.0);
    triangle.nVertices = 3;  // Triângulo  */

    //criação da cabeça
    Geometry head = createSegment(0,dir);
    cobrinha.push_back(head);

    //criação da geometria dos olhos
    eyes.VAO = createEyes(32,0.25);
    eyes.nVertices=34;
    eyes.position=vec3(400,300,0);
    eyes.dimensions=vec3(50,50,1.0);
    eyes.angle=0.0;
    eyes.color=vec3(1.0,1.0,1.0);


    // Ativa o teste de profundidade
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS); // Sempre passa no teste de profundidade (desnecessário se não houver profundidade)

    glUseProgram(shaderID);

    // Matriz de projeção ortográfica (usada para desenhar em 2D)
    mat4 projection = ortho(0.0f, 800.0f, 0.0f, 600.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "projection"), 1, GL_FALSE, value_ptr(projection));

    // Loop da aplicação
    while (!glfwWindowShouldClose(window)) {
        // Processa entradas (teclado e mouse)
        glfwPollEvents();

        // Limpa a tela
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   
         vec3 position = cobrinha[0].position;
          float vel = 2.0;

        // Pega a posição do mouse e calcula a direção
        double xPos, yPos;
        glfwGetCursorPos(window, &xPos, &yPos);
        mousePos = vec2(xPos, height - yPos);  // Inverte o eixo Y para se alinhar à tela
        vec3 dir = normalize(vec3(mousePos, 0.0) - position);
        position = position + 0.5f*dir;
        float angle = atan2(dir.y, dir.x);
        float lookangle = atan2(dir.y, dir.x);

        /*// Move o triângulo suavemente na direção do mouse
        if (distance(triangle.position, vec3(mousePos, 0.0)) > 0.01f) {
            triangle.position += 0.5f * dir;  // Aumente ou diminua 0.5f para controlar a velocidade
        }*/

        // Atualiza o ângulo de rotação do triângulo
        //triangle.angle = angle + radians(-90.0f); // Rotaciona para que a ponta aponte para o mouse

        // Atualiza a posição e ângulo da cabeça e dos olhos
        cobrinha[0].position = position;
        cobrinha[0].angle = angle;
        eyes.position = position;
        eyes.angle = angle;

        // Atualiza a posição dos segmentos da cobrinha para seguir a cabeça
        for (int i = 1; i < cobrinha.size(); i++)
        {
        vec3 dir = normalize(cobrinha[i - 1].position - cobrinha[i].position);
        float distance = length(cobrinha[i - 1].position - cobrinha[i].position);
        // Calcula a nova posição do segmento com suavidade, respeitando as distâncias mínima e máxima
        vec3 targetPosition = cobrinha[i].position;
        float dynamicSmoothFactor = smoothFactor * (distance / maxDistance);
        if (distance < minDistance)
        {
            targetPosition = cobrinha[i].position + (distance - minDistance) * dir;
        }
        else if (distance > maxDistance)
        {
            targetPosition = cobrinha[i].position + (distance - maxDistance) * dir;
        }
        // Interpolação suave para a nova posição do segmento
        cobrinha[i].position = mix(cobrinha[i].position, targetPosition, dynamicSmoothFactor);
        }

        bool addNew = false; 
        // Adiciona um novo segmento à cobrinha quando a tecla Espaço é pressionada
        if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        {
            addNew = true;
        }
        // Adiciona novos segmentos à cobrinha quando solicitado
        if (addNew)
        {
            cobrinha.push_back(createSegment(cobrinha.size(), -dir));
            addNew = false;
        }
        // Desenha os segmentos da cobrinha e os olhos
        for (int i = cobrinha.size() - 1; i >= 0; i--)
        {
            drawGeometry(shaderID, cobrinha[i].VAO, cobrinha[i].nVertices, cobrinha[i].position,
            cobrinha[i].dimensions, cobrinha[i].angle, cobrinha[i].color, GL_TRIANGLE_FAN);
        if (i == 0)
        { // cabeça
        // Desenha as escleras dos olhos
            drawGeometry(shaderID, eyes.VAO, eyes.nVertices, eyes.position, eyes.dimensions, eyes.angle,
            eyes.color, GL_TRIANGLE_FAN, 0);
            drawGeometry(shaderID, eyes.VAO, eyes.nVertices, eyes.position, eyes.dimensions, eyes.angle,
            eyes.color, GL_TRIANGLE_FAN, 34);
            // Desenha as pupilas dos olhos
            drawGeometry(shaderID, eyes.VAO, eyes.nVertices, eyes.position, eyes.dimensions, eyes.angle,
            vec3(0.0, 0.0, 0.0), GL_TRIANGLE_FAN, 2 * 34);
            drawGeometry(shaderID, eyes.VAO, eyes.nVertices, eyes.position, eyes.dimensions, eyes.angle,
            vec3(0.0, 0.0, 0.0), GL_TRIANGLE_FAN, 3 * 34);
        }
        }

        // Desenha o triângulo e o cursor
        drawGeometry(shaderID, triangle.VAO, triangle.nVertices, triangle.position, triangle.dimensions, triangle.angle, triangle.color);
        drawGeometry(shaderID, triangle.VAO, 3, vec3(mousePos, 0.0), vec3(10.0, 10.0, 1.0), 0.0f, vec3(1.0, 0.0, 1.0));

        // Troca os buffers da tela
        glfwSwapBuffers(window);
    }

    // Limpa a memória alocada pelos buffers
    glDeleteVertexArrays(1, &triangle.VAO);
    glfwTerminate();
    return 0;
}

// Callback de teclado
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (action == GLFW_PRESS)
        keys[key] = true;
    if (action == GLFW_RELEASE)
        keys[key] = false;
}


// Configura e compila os shaders
int setupShader() {
    // Código do vertex shader
    const GLchar *vertexShaderSource = R"(
    #version 400
    layout (location = 0) in vec3 position;
    uniform mat4 projection;
    uniform mat4 model;
    void main() {
        gl_Position = projection * model * vec4(position, 1.0);
    })";

    // Código do fragment shader
    const GLchar *fragmentShaderSource = R"(
    #version 400
    uniform vec4 inputColor;
    out vec4 color;
    void main() {
        color = inputColor;
    })";

    // Compilação do vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // Verificando erros de compilação do vertex shader
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Compilação do fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // Verificando erros de compilação do fragment shader
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Linkando os shaders no programa
    GLuint shaderID = glCreateProgram();
    glAttachShader(shaderID, vertexShader);
    glAttachShader(shaderID, fragmentShader);
    glLinkProgram(shaderID);
    // Verificando erros de linkagem do programa de shaders
    glGetProgramiv(shaderID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // Limpando os shaders compilados após o link
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderID;
}

// Configura a geometria do triângulo
int setupGeometry() {
    GLfloat vertices[] = {
        -0.5f, -0.5f, 0.0f, // Vértice 1
         0.5f, -0.5f, 0.0f, // Vértice 2
         0.0f,  0.5f, 0.0f  // Vértice 3
    };

    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Desvincula o VAO e o VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}

// Função para desenhar o objeto
void drawGeometry(GLuint shaderID, GLuint VAO, int nVertices, vec3 position, vec3 dimensions, float angle, vec3 color, GLuint drawingMode, int offset, vec3 axis) {
    glBindVertexArray(VAO); // Vincula o VAO

    // Aplica as transformações de translação, rotação e escala
    mat4 model = mat4(1.0f);
    model = translate(model, position);
    model = rotate(model, angle, axis);
    model = scale(model, dimensions);

    // Envia a matriz de modelo ao shader
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));
    
    // Envia a cor do objeto ao shader
    glUniform4f(glGetUniformLocation(shaderID, "inputColor"), color.r, color.g, color.b, 1.0f);

    // Desenha o objeto
    glDrawArrays(drawingMode, offset, nVertices);

    // Desvincula o VAO
    glBindVertexArray(0);
}
