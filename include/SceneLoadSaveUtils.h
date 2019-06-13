#ifndef SCENEUTILS
#define SCENEUTILS

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

#define PI 3.14159265359f

// Headers das bibliotecas OpenGL
#include <glad/glad.h>   // Criação de contexto OpenGL 3.3
#include <GLFW/glfw3.h>  // Criação de janelas do sistema operacional

// Headers da biblioteca GLM: criação de matrizes e vetores.
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

// biblioteca para json
#include <nlohmann/json.hpp>

// Headers da biblioteca para carregar modelos obj
#include <tiny_obj_loader.h>

#include "matrices.h"
#include "shaderUtil.h"

// for convenience
using json = nlohmann::json;

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
        //printf("Carregando modelo \"%s\"... ", filename);

        std::string err;
        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename, basepath, triangulate);

        if (!err.empty())
            fprintf(stderr, "\n%s\n", err.c_str());

        if (!ret)
            throw std::runtime_error("Erro ao carregar modelo.");

       // printf("OK.\n");
    }
};

enum class WrapMode{REPEAT, MIRRORED_REPEAT, CLAMP_TO_EDGE, CLAMP_TO_BORDER};

enum class CollisionType{ELASTIC, INELASTIC, WALL};

enum class ColliderType{OBB, SPHERE, NONE};

enum class MouseCollisionType{MOUSE_OVER, CLICK};

const std::vector<float> black = {0.0f, 0.0f, 0.0f, 1.0f};

//estrutura que guarda as informações de algum modelo 3D em memória
struct ModelInformation{
    void*           first_index;               // Índice do primeiro vértice dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    int             num_indices;               // Número de índices do objeto dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    GLenum          rendering_mode;            // Modo de rasterização (GL_TRIANGLES, GL_TRIANGLE_STRIP, etc.)
    GLuint          vertex_array_object_id;
    glm::vec4       bbox_min_min_min;                  // Axis-Aligned Bounding Box do objeto
    glm::vec4       bbox_max_min_min;
    glm::vec4       bbox_max_min_max;
    glm::vec4       bbox_min_min_max;
    glm::vec4       bbox_max_max_max;
    glm::vec4       bbox_min_max_max;
    glm::vec4       bbox_min_max_min;
    glm::vec4       bbox_max_max_min;
    std::vector<GLuint> buffers;
};

// Definimos uma estrutura que armazenará dados necessários para renderizar
// cada objeto da cena virtual.
struct SceneObject
{
    std::string     name;                      // Nome do objeto
    void*           first_index;               // Índice do primeiro vértice dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    int             num_indices;               // Número de índices do objeto dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    GLenum          rendering_mode;            // Modo de rasterização (GL_TRIANGLES, GL_TRIANGLE_STRIP, etc.)
    GLuint          vertex_array_object_id;    // ID do VAO onde estão armazenados os atributos do modelo
    glm::mat4       model;                     //matriz model deste objeto
    glm::mat4       rotationMatrix;
    glm::mat4       translationMatrix;
    glm::mat4       scaleMatrix;

    glm::vec4       bbox_min_min_min;                  // Axis-Aligned Bounding Box do objeto
    glm::vec4       bbox_max_min_min;
    glm::vec4       bbox_max_min_max;
    glm::vec4       bbox_min_min_max;
    glm::vec4       bbox_max_max_max;
    glm::vec4       bbox_min_max_max;
    glm::vec4       bbox_min_max_min;
    glm::vec4       bbox_max_max_min;

    std::string     objFilename;               //o endereço relativo do .obj correspondente a este objeto
    GLuint          gpuProgramId;              //id do programa de GPU usado para desenhar este objeto
    std::string     fragmentShaderFilename;
    std::string     vertexShaderFilename;
    int             textureWrapMode;
    glm::vec4       velocity;                       //vetor velocidade do objeto, representa seu movimento
    glm::vec4       blockMovement;                  //vetor representando em quais eixos o objeto pode se mover. Por exemplo, se essa variável tiver o valor (1, 0, 1) significa que o objeto pode se mover nos eixos X e Z, mas não no Y
    float           decelerationRate;               //velocidade de desaceleração do objeto, simula atrito, resistência do ar, etc (precisa ser um valor entre 0 e 1)
    std::string     onMouseOverName;                //nome da função, para ser guardado no disco, é utilizado na função de mapeamento de funções
    std::string     onClickName;
    std::string     onCollisionName;
    std::string     onMoveName;
    std::string     updateName;
    int             thisCollisionType;              //o tipo de colisão que deve ser implementada quando uma colisão for detectada (pode ser elástica, inelástica ou imóvel (WALL))
    int             thisColliderType;               //tipo (forma) do colisor deste objeto
    bool            active;                         //se este objeto deve ser considerado na cena, etc
    GLuint          vertexShaderId;
    GLuint          fragmentShaderId;
    std::vector<GLuint> buffers;

    std::vector<std::string> textureFilenames;
    std::vector<GLuint> textureIds;
    int activeTexture = 0;

    std::vector<std::string>  collisionsList;             //lista de objetos colidindo com este no frame atual

    std::function<void(std::vector<SceneObject>&, int)>   onMouseOver;  //função chamada quando o usuário olhar para o objeto
    std::function<void(std::vector<SceneObject>&, int)>   onClick;      //função chamada quando o usuário clicar no objeto
    std::function<void(std::vector<SceneObject>&, int)>   onMove;       //função chamada quando o objeto for movido, pode ser usada para fazer animações
    std::function<void(std::vector<SceneObject>&, int)>   update;       //função chamada todos os frames no fim do frame (útil para implementar animações baseadas em curvas de Bézier, por exemplo)
    std::function<void(std::vector<SceneObject>&, int, int)>   onCollision;   //função chamada quando é detectada uma colisão


    glm::mat4   parentMatrix = Matrix_Identity();
    std::vector<std::string> childrenNames;
    std::vector<int> childrenIndices;
    int parentIndex = -1;

};


void OpenScene(std::string filename, std::vector<SceneObject>& currentScene);
void SaveScene(std::string filename, std::vector<SceneObject> currentScene);
void UnloadScene(std::vector<SceneObject>& currentScene);
void Debug_NewObjectSphere(std::vector<SceneObject>& currentScene);
void OpenSceneAdditive(std::string filename, std::vector<SceneObject>& currentScene);


std::vector<SceneObject> ObjectLoad(std::string filename);
void ComputeNormals(ObjModel* model);

GLuint GetFragmentShaderId(std::string fragment_shader_filename);
GLuint GetGPUProgramId(GLuint vertex_shader_id, GLuint fragment_shader_id);
GLuint GetVertexShaderId(std::string vertex_shader_filename);
GLuint GetTextureId(std::string textureFileName, WrapMode wrapMode, std::vector<GLfloat> borderColor = black);
#endif // SCENEUTILS
