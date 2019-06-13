#ifndef SCENEUTILS
#define SCENEUTILS

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

#define PI 3.14159265359f

// Headers das bibliotecas OpenGL
#include <glad/glad.h>   // Cria��o de contexto OpenGL 3.3
#include <GLFW/glfw3.h>  // Cria��o de janelas do sistema operacional

// Headers da biblioteca GLM: cria��o de matrizes e vetores.
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

//estrutura que guarda as informa��es de algum modelo 3D em mem�ria
struct ModelInformation{
    void*           first_index;               // �ndice do primeiro v�rtice dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    int             num_indices;               // N�mero de �ndices do objeto dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    GLenum          rendering_mode;            // Modo de rasteriza��o (GL_TRIANGLES, GL_TRIANGLE_STRIP, etc.)
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

// Definimos uma estrutura que armazenar� dados necess�rios para renderizar
// cada objeto da cena virtual.
struct SceneObject
{
    std::string     name;                      // Nome do objeto
    void*           first_index;               // �ndice do primeiro v�rtice dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    int             num_indices;               // N�mero de �ndices do objeto dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    GLenum          rendering_mode;            // Modo de rasteriza��o (GL_TRIANGLES, GL_TRIANGLE_STRIP, etc.)
    GLuint          vertex_array_object_id;    // ID do VAO onde est�o armazenados os atributos do modelo
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

    std::string     objFilename;               //o endere�o relativo do .obj correspondente a este objeto
    GLuint          gpuProgramId;              //id do programa de GPU usado para desenhar este objeto
    std::string     fragmentShaderFilename;
    std::string     vertexShaderFilename;
    int             textureWrapMode;
    glm::vec4       velocity;                       //vetor velocidade do objeto, representa seu movimento
    glm::vec4       blockMovement;                  //vetor representando em quais eixos o objeto pode se mover. Por exemplo, se essa vari�vel tiver o valor (1, 0, 1) significa que o objeto pode se mover nos eixos X e Z, mas n�o no Y
    float           decelerationRate;               //velocidade de desacelera��o do objeto, simula atrito, resist�ncia do ar, etc (precisa ser um valor entre 0 e 1)
    std::string     onMouseOverName;                //nome da fun��o, para ser guardado no disco, � utilizado na fun��o de mapeamento de fun��es
    std::string     onClickName;
    std::string     onCollisionName;
    std::string     onMoveName;
    std::string     updateName;
    int             thisCollisionType;              //o tipo de colis�o que deve ser implementada quando uma colis�o for detectada (pode ser el�stica, inel�stica ou im�vel (WALL))
    int             thisColliderType;               //tipo (forma) do colisor deste objeto
    bool            active;                         //se este objeto deve ser considerado na cena, etc
    GLuint          vertexShaderId;
    GLuint          fragmentShaderId;
    std::vector<GLuint> buffers;

    std::vector<std::string> textureFilenames;
    std::vector<GLuint> textureIds;
    int activeTexture = 0;

    std::vector<std::string>  collisionsList;             //lista de objetos colidindo com este no frame atual

    std::function<void(std::vector<SceneObject>&, int)>   onMouseOver;  //fun��o chamada quando o usu�rio olhar para o objeto
    std::function<void(std::vector<SceneObject>&, int)>   onClick;      //fun��o chamada quando o usu�rio clicar no objeto
    std::function<void(std::vector<SceneObject>&, int)>   onMove;       //fun��o chamada quando o objeto for movido, pode ser usada para fazer anima��es
    std::function<void(std::vector<SceneObject>&, int)>   update;       //fun��o chamada todos os frames no fim do frame (�til para implementar anima��es baseadas em curvas de B�zier, por exemplo)
    std::function<void(std::vector<SceneObject>&, int, int)>   onCollision;   //fun��o chamada quando � detectada uma colis�o


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
