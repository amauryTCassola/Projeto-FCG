#ifndef OBJUTILS
#define OBJUTILS

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
#include <sys/stat.h>

// Headers das bibliotecas OpenGL
#include <glad/glad.h>   // Criação de contexto OpenGL 3.3
#include <GLFW/glfw3.h>  // Criação de janelas do sistema operacional

// Headers da biblioteca GLM: criação de matrizes e vetores.
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

// Estrutura que representa um modelo geométrico carregado a partir de um
// arquivo ".obj". Veja https://en.wikipedia.org/wiki/Wavefront_.obj_file .
struct ObjModel
{
    tinyobj::attrib_t                 attrib; //coeficientes de posições, normais e textcoord
    std::vector<tinyobj::shape_t>     shapes; //cada shape é um objeto nomeado, com os índices dos seus coeficientes (que estão em attrib)
    std::vector<tinyobj::material_t>  materials; //sei la vei materiais

    // Este construtor lê o modelo de um arquivo utilizando a biblioteca tinyobjloader.
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

// Definimos uma estrutura que armazenará dados necessários para renderizar
// cada objeto da cena virtual.
struct SceneObject
{
    const char*     name;                      // Nome do objeto
    void*           first_index;               // Índice do primeiro vértice dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    int             num_indices;               // Número de índices do objeto dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    GLenum          rendering_mode;            // Modo de rasterização (GL_TRIANGLES, GL_TRIANGLE_STRIP, etc.)
    GLuint          vertex_array_object_id;    // ID do VAO onde estão armazenados os atributos do modelo
    glm::mat4       model;                     //matriz model deste objeto
    GLuint          texture_id;                //ID da textura vinculada a este objeto
    glm::vec4       bbox_min;                  // Axis-Aligned Bounding Box do objeto
    glm::vec4       bbox_max;
    const char*     objFilename;               //o endereço relativo do .obj correspondente a este objeto
    const char*     textureFilename;           //o endereço relativo da textura correspondente a este objeto
    GLuint          gpuProgramId;              //id do programa de GPU usado para desenhar este objeto
    const char*     fragmentShaderFilename;
    const char*     vertexShaderFilename;
    int             textureWrapMode;

    //NOVOS

    glm::vec4               velocity;                       //vetor velocidade do objeto, representa seu movimento
    glm::vec3               blockMovement;                  //vetor representando em quais eixos o objeto pode se mover. Por exemplo, se essa variável tiver o valor (1, 0, 1) significa que o objeto pode se mover nos eixos X e Z, mas não no Y
    float                   decelerationRate;               //velocidade de desaceleração do objeto, simula atrito, resistência do ar, etc (precisa ser um valor entre 0 e 1)
    const char*             onMouseOverName;                //nome da função, para ser guardado no disco, é utilizado na função de mapeamento de funções
    const char*             onClickName;
    int                     thisCollisionType;              //o tipo de colisão que deve ser implementada quando uma colisão for detectada (pode ser elástica, inelástica ou imóvel (WALL))
    int                     thisColliderType;               //tipo (forma) do colisor deste objeto
    bool                    active;                         //se este objeto deve ser considerado na cena, etc

    std::function<void(std::vector<bool>&, std::vector<SceneObject>&, int callerIndex)>   onMouseOver;   //função chamada quando o usuário olhar para o objeto
    std::function<void(std::vector<bool>&, std::vector<SceneObject>&, int callerIndex)>   onClick;       //função chamada quando o usuário clicar no objeto
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
