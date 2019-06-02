#ifndef OBJUTILS
#define OBJUTILS

#include <cmath>
#include <cstdio>
#include <cstdlib>

// Headers abaixo s�o espec�ficos de C++
#include <map>
#include <stack>
#include <string>
#include <vector>
#include <limits>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <sys/stat.h>

// Headers das bibliotecas OpenGL
#include <glad/glad.h>   // Cria��o de contexto OpenGL 3.3
#include <GLFW/glfw3.h>  // Cria��o de janelas do sistema operacional

// Headers da biblioteca GLM: cria��o de matrizes e vetores.
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>


// Headers da biblioteca para carregar modelos obj
#include <tiny_obj_loader.h>

// biblioteca para json
#include <nlohmann/json.hpp>

// for convenience
using json = nlohmann::json;

#include "matrices.h"
#include "stb_imageUtils.h"
#include "cameraUtils.h"
#include "shaderUtil.h"

// Estrutura que representa um modelo geom�trico carregado a partir de um
// arquivo ".obj". Veja https://en.wikipedia.org/wiki/Wavefront_.obj_file .
struct ObjModel
{
    tinyobj::attrib_t                 attrib; //coeficientes de posi��es, normais e textcoord
    std::vector<tinyobj::shape_t>     shapes; //cada shape � um objeto nomeado, com os �ndices dos seus coeficientes (que est�o em attrib)
    std::vector<tinyobj::material_t>  materials; //sei la vei materiais

    // Este construtor l� o modelo de um arquivo utilizando a biblioteca tinyobjloader.
    // Veja: https://github.com/syoyo/tinyobjloader
    ObjModel(const char* filename, const char* basepath = NULL, bool triangulate = true)
    {
        printf("Carregando modelo \"%s\"... ", filename);

        std::string err;
        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename, basepath, triangulate);

        if (!err.empty())
            fprintf(stderr, "\n%s\n", err.c_str());

        if (!ret)
            throw std::runtime_error("Erro ao carregar modelo.");

        printf("OK.\n");
    }
};

enum class WrapMode{REPEAT, MIRRORED_REPEAT, CLAMP_TO_EDGE, CLAMP_TO_BORDER};

enum class CollisionType{ELASTIC, INELASTIC, WALL};

enum class ColliderType{CUBE, SPHERE, CYLINDER, NONE};

enum class MouseCollisionType{MOUSE_OVER, CLICK};

const std::vector<float> black = {0.0f, 0.0f, 0.0f, 1.0f};

// Definimos uma estrutura que armazenar� dados necess�rios para renderizar
// cada objeto da cena virtual.
struct SceneObject
{
    const char*     name;                      // Nome do objeto
    void*           first_index;               // �ndice do primeiro v�rtice dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    int             num_indices;               // N�mero de �ndices do objeto dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    GLenum          rendering_mode;            // Modo de rasteriza��o (GL_TRIANGLES, GL_TRIANGLE_STRIP, etc.)
    GLuint          vertex_array_object_id;    // ID do VAO onde est�o armazenados os atributos do modelo
    glm::mat4       model;                     //matriz model deste objeto
    GLuint          texture_id;                //ID da textura vinculada a este objeto
    glm::vec4       bbox_min;                  // Axis-Aligned Bounding Box do objeto
    glm::vec4       bbox_max;
    const char*     objFilename;               //o endere�o relativo do .obj correspondente a este objeto
    const char*     textureFilename;           //o endere�o relativo da textura correspondente a este objeto
    GLuint          gpuProgramId;              //id do programa de GPU usado para desenhar este objeto
    const char*     fragmentShaderFilename;
    const char*     vertexShaderFilename;
    int             textureWrapMode;

    //NOVOS

    glm::vec4               velocity;                       //vetor velocidade do objeto, representa seu movimento
    glm::vec3               blockMovement;                  //vetor representando em quais eixos o objeto pode se mover. Por exemplo, se essa vari�vel tiver o valor (1, 0, 1) significa que o objeto pode se mover nos eixos X e Z, mas n�o no Y
    float                   decelerationRate;               //velocidade de desacelera��o do objeto, simula atrito, resist�ncia do ar, etc (precisa ser um valor entre 0 e 1)
    const char*             onMouseOverName;                //nome da fun��o, para ser guardado no disco, � utilizado na fun��o de mapeamento de fun��es
    const char*             onClickName;
    int                     thisCollisionType;              //o tipo de colis�o que deve ser implementada quando uma colis�o for detectada (pode ser el�stica, inel�stica ou im�vel (WALL))
    int                     thisColliderType;               //tipo (forma) do colisor deste objeto
    bool                    active;                         //se este objeto deve ser considerado na cena, etc

    std::function<void(std::vector<bool>&, std::vector<SceneObject>&, int callerIndex)>   onMouseOver;   //fun��o chamada quando o usu�rio olhar para o objeto
    std::function<void(std::vector<bool>&, std::vector<SceneObject>&, int callerIndex)>   onClick;       //fun��o chamada quando o usu�rio clicar no objeto
};
//===========================================================================================================================================

std::vector<SceneObject> ObjectLoad(const char* filename);
void ComputeNormals(ObjModel* model);
void DrawScene();
void MoveObject(glm::vec4 movementVector, SceneObject* objToBeMoved);
GLuint CreateGPUProgram(const char* vertex_shader_filename, const char* fragment_shader_filename);
GLuint CreateNewTexture(const char* textureFileName, WrapMode wrapMode, std::vector<GLfloat> borderColor = black);

//void WriteSceneToFile(std::vector<SceneObjectOnDisc> objsList, const char* newSceneFilename);
//std::vector<SceneObjectOnDisc> ReadSceneFromFile(const char* sceneFilename);

void SaveScene(const char* filename);
void OpenScene(const char* filename);

void TestMouseCollision(MouseCollisionType colType);

void Debug_NewObjectSphere();

#include "IntersectionFunctions.h"

#endif// OBJUTILS
