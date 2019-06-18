#define PI 3.14159265359f

#define ISDEBUG

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

// Headers locais, definidos na pasta "include/"
#include "objUtils.h"
#include "TextRenderingUtils.h"
#include "SFXUtils.h"

//{ callbacks

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
    float dx = xpos - midPointX;
    float dy = ypos - midPointY;

    if(dx != 0 || dy != 0){

        SetMousePosToMiddle(window);
        RotateCameraX(dx);
        RotateCameraY(dy);
    }
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
    UpdateFramebufferSize(height, width);
    // Atualizamos também a razão que define a proporção da janela (largura /
    // altura), a qual será utilizada na definição das matrizes de projeção,
    // tal que não ocorra distorções durante o processo de "Screen Mapping"
    // acima, quando NDC é mapeado para coordenadas de pixels. Veja slide 227 do documento "Aula_09_Projecoes.pdf".
    //
    // O cast para float é necessário pois números inteiros são arredondados ao
    // serem divididos!
    UpdateScreenRatio((float)width / height);
}

// Função callback chamada sempre que o usuário aperta algum dos botões do mouse
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        TestOnClick();
    }

    #ifdef ISDEBUG

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        SaveCurrentScene("aaaaaaa.json");
    }
    #endif // ISDEBUG
}


bool isPressingW = false, isPressingS = false, isPressingA = false, isPressingD = false;

bool f = false;

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mod){
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

    if(key == GLFW_KEY_R && action == GLFW_PRESS)
        ReloadScene("../../Scenes/Dummy.json");

    if(key == GLFW_KEY_F1 && action == GLFW_PRESS)
        SaveCurrentScene("../../Scenes/Dummy.json");

    if(key == GLFW_KEY_F && action == GLFW_PRESS){
        f = !f;
        if(f){
            SetLightMode(LightMode::FLASHLIGHT);
        } else {
            SetLightMode(LightMode::DARK);
        }
    }

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
    GLFWwindow* window = glfwCreateWindow(800, 600, "Janela 1", NULL, NULL);

    //faz com que a janela recém criada seja o contexto atual
    glfwMakeContextCurrent(window);

    glfwSetWindowPos(window, 10, 10);

    // Carregamento de todas funções definidas por OpenGL 3.3, utilizando a
    // biblioteca GLAD.
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, CursorPosCallback);

    // Habilitamos o Z-buffer. Veja slide 108 do documento "Aula_09_Projecoes.pdf".
    glEnable(GL_DEPTH_TEST);

    // Habilitamos o Backface Culling. Veja slides 22-34 do documento "Aula_13_Clipping_and_Culling.pdf".
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    SetMousePosToMiddle(window);

    ShowCursor(false);

    FramebufferSizeCallback(window, 800, 600); // Forçamos a chamada do callback acima, para definir g_ScreenRatio.

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    return window;
}


int main(){
    GLFWwindow* window = initGL();

    LoadToCurrentScene("../../Scenes/Dummy.json");
    Debug_CreateNewObjectSphere();

    //CARREGAMENTO DE UMA CENA DO DISCO
    //currentScene = LoadScene("aaaaaaa.json");

    DrawText("", TextPosition::CENTER);

    SetLightMode(LightMode::DARK);

    //SetLightColor(glm::vec4(0.293f, 0.0f, 1.0f, 1.0f));
    //SetLightColor(glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));

    PlaySound("../../sfx/Rain-sound-loop.mp3", true, 0.3f);

    while (!glfwWindowShouldClose(window)){

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); //define a "clear color" como branco
        // "clear" a janela inteira
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //checa se aconteceram eventos nesse frame e trata eles
        glfwPollEvents();


        MoveCamera(isPressingW, isPressingA, isPressingS, isPressingD);

        MoveCurrentSceneObjects();

        TestPhysicalCollisions();

        CallUpdateFuntions();

        DrawCurrentScene();

        TestOnMouseOver();

        FinishFrame();

        //troca os buffers para atualizar a janela
        glfwSwapBuffers(window);

    }

    glfwTerminate();

}
