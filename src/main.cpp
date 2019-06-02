#define PI 3.14159265359f


#include <cmath>
#include <cstdio>
#include <cstdlib>

// Headers abaixo são específicos de C++
#include <map>
#include <stack>
#include <string>
#include <vector>
#include <limits>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

// Headers das bibliotecas OpenGL
#include <glad/glad.h>   // Criação de contexto OpenGL 3.3
#include <GLFW/glfw3.h>  // Criação de janelas do sistema operacional

// Headers da biblioteca GLM: criação de matrizes e vetores.
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

// Headers da biblioteca para carregar modelos obj
#include <tiny_obj_loader.h>

// Headers locais, definidos na pasta "include/"
#include "objUtils.h"

//{ callbacks

bool teste = false;

void TesteInterseccao(){
    glm::vec4 bbox_min_world = currentScene[0].model * currentScene[0].bbox_min;
    glm::vec4 bbox_max_world = currentScene[0].model * currentScene[0].bbox_max;


    glm::vec4 centroEsfera = (bbox_min_world + bbox_max_world) / 2.0f;

    glm::vec4 maxEsfera_world = currentScene[0].model * glm::vec4(currentScene[0].bbox_max.x, 0.0f, 0.0f, 1.0f);

    float raioEsfera = norm(maxEsfera_world - centroEsfera);
    if(norm(centroEsfera - GetCameraPosition()) < 5.0f){
        if(IntersectionRaySphere(GetCameraPosition(), GetViewVector(), centroEsfera, raioEsfera)){
            if(teste){
                glDeleteTextures(1, &currentScene[0].texture_id);
                currentScene[0].texture_id = CreateNewTexture("../../data/Liberty-GreenBronze-1.bmp", WrapMode::MIRRORED_REPEAT);
                teste = false;
            }
            else{
                teste = true;
                glDeleteTextures(1, &currentScene[0].texture_id);
                currentScene[0].texture_id = CreateNewTexture("../../data/Liberty-Pavimentazione-1.bmp", WrapMode::MIRRORED_REPEAT);
            }
        }
    }
}


float mouse_sensitivity = 0.01f;

void SetMousePosToMiddle(GLFWwindow* window){
    int windowWidth, windowHeight;

    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    int midPointX = windowWidth/2;
    int midPointY = windowHeight/2;

    glfwSetCursorPos(window, midPointX, midPointY);
}

// Função callback chamada sempre que o usuário movimentar o cursor do mouse em
// cima da janela OpenGL.
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    int windowWidth, windowHeight;

    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    int midPointX = windowWidth/2;
    int midPointY = windowHeight/2;

    // Deslocamento do cursor do mouse em x e y de coordenadas de tela!
    float dx = (xpos - midPointX)*mouse_sensitivity;
    float dy = (ypos - midPointY)*mouse_sensitivity;

    RotateCamera(dx, dy);

    SetMousePosToMiddle(window);
}


float g_ScreenRatio = 1.0f;
// Definição da função que será chamada sempre que a janela do sistema
// operacional for redimensionada, por consequência alterando o tamanho do
// "framebuffer" (região de memória onde são armazenados os pixels da imagem).
void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    // Indicamos que queremos renderizar em toda região do framebuffer. A
    // função "glViewport" define o mapeamento das "normalized device
    // coordinates" (NDC) para "pixel coordinates".  Essa é a operação de
    // "Screen Mapping" ou "Viewport Mapping" vista em aula (slides 33-44 do documento "Aula_07_Transformacoes_Geometricas_3D.pdf").
    glViewport(0, 0, width, height);

    // Atualizamos também a razão que define a proporção da janela (largura /
    // altura), a qual será utilizada na definição das matrizes de projeção,
    // tal que não ocorra distorções durante o processo de "Screen Mapping"
    // acima, quando NDC é mapeado para coordenadas de pixels. Veja slide 227 do documento "Aula_09_Projecoes.pdf".
    //
    // O cast para float é necessário pois números inteiros são arredondados ao
    // serem divididos!
    g_ScreenRatio = (float)width / height;
}

// Função callback chamada sempre que o usuário aperta algum dos botões do mouse
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        //glm::vec4 moveX = glm::vec4(0.1f, 0.0f, 0.0f, 0.0f);
        //MoveObject(moveX, &currentScene[0]);

        TesteInterseccao();
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        glm::vec4 moveX = glm::vec4(-0.1f, 0.0f, 0.0f, 0.0f);
        MoveObject(moveX, &currentScene[0]);

        //SaveScene(currentScene, "aaaaaaa.json");
    }
}


bool isPressingW = false, isPressingS = false, isPressingA = false, isPressingD = false;

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mod){
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS){
        currentScene.push_back(ObjectLoad("../../data/sphere.obj")[0]);
        currentScene[1].texture_id = CreateNewTexture("../../data/Liberty-Pavimentazione-1.bmp", WrapMode::MIRRORED_REPEAT);
        currentScene[1].gpuProgramId = CreateGPUProgram("../../src/shader_vertex.glsl", "../../src/shader_fragment.glsl");
        currentScene[1].textureFilename = "../../data/Liberty-Pavimentazione-1.bmp";
        currentScene[1].fragmentShaderFilename = "../../src/shader_fragment.glsl";
        currentScene[1].vertexShaderFilename = "../../src/shader_vertex.glsl";
        currentScene[0].textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    }

    if(key == GLFW_KEY_W && action == GLFW_PRESS){

        isPressingW = true;
    }

    if(key == GLFW_KEY_W && action == GLFW_RELEASE){

        isPressingW = false;
    }

    if(key == GLFW_KEY_S && action == GLFW_PRESS){

        isPressingS = true;
    }

    if(key == GLFW_KEY_S && action == GLFW_RELEASE){

        isPressingS = false;
    }

    if(key == GLFW_KEY_A && action == GLFW_PRESS){

        isPressingA = true;
    }

    if(key == GLFW_KEY_A && action == GLFW_RELEASE){

        isPressingA = false;
    }

    if(key == GLFW_KEY_D && action == GLFW_PRESS){

        isPressingD = true;
    }

    if(key == GLFW_KEY_D && action == GLFW_RELEASE){

        isPressingD = false;
    }

    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

}
//}

GLFWwindow* initGL(){
    //coisas de inicialização, criação da janela, etc etc etc
    //retorna o ponteiro para a janela

    //inicializa a glfw
    //deve ser chamada antes de tudo
    glfwInit();

    // Pedimos para utilizar OpenGL versão 3.3 (ou superior)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //criação da janela
    GLFWwindow* window = glfwCreateWindow(500, 500, "Janela 1", NULL, NULL);

    //faz com que a janela recém criada seja o contexto atual
    glfwMakeContextCurrent(window);

    // Carregamento de todas funções definidas por OpenGL 3.3, utilizando a
    // biblioteca GLAD.
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, CursorPosCallback);
    //FramebufferSizeCallback(window, 800, 600); // Forçamos a chamada do callback acima, para definir g_ScreenRatio.

    // Habilitamos o Z-buffer. Veja slide 108 do documento "Aula_09_Projecoes.pdf".
    glEnable(GL_DEPTH_TEST);

    // Habilitamos o Backface Culling. Veja slides 22-34 do documento "Aula_13_Clipping_and_Culling.pdf".
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    SetMousePosToMiddle(window);

    ShowCursor(false);

    return window;
}


int main(){
    GLFWwindow* window = initGL();

    //CRIAÇÃO DE UM NOVO OBJETO
    currentScene = ObjectLoad("../../data/sphere.obj");
    currentScene[0].texture_id = CreateNewTexture("../../data/Liberty-GreenBronze-1.bmp", WrapMode::MIRRORED_REPEAT);
    currentScene[0].textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene[0].textureFilename = "../../data/Liberty-GreenBronze-1.bmp";
    currentScene[0].gpuProgramId = CreateGPUProgram("../../src/shader_vertex.glsl", "../../src/shader_fragment.glsl");
    currentScene[0].fragmentShaderFilename = "../../src/shader_fragment.glsl";
    currentScene[0].vertexShaderFilename = "../../src/shader_vertex.glsl";

    //CARREGAMENTO DE UMA CENA DO DISCO
    //currentScene = LoadScene("aaaaaaa.json");


    while (!glfwWindowShouldClose(window)){
        //tudo aqui acontece relativo à janela que é o contexto atual
        //(isso foi definido com a função glfwMakeContextCurrent(window);

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); //define a "clear color" como branco
        // "clear" a janela inteira
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if(isPressingW) MoveCameraForward();
        if(isPressingS) MoveCameraBack();
        if(isPressingA) MoveCameraLeft();
        if(isPressingD) MoveCameraRight();

       for(unsigned int i = 0; i<currentScene.size(); i++){
            DrawVirtualObject(currentScene[i]);
       }



        //troca os buffers para atualizar a janela
        glfwSwapBuffers(window);

        //checa se aconteceram eventos nesse frame e trata eles
        glfwPollEvents();

    }

}
