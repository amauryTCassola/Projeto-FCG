#include "SceneLoadSaveUtils.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "FunctionMappingUtils.h"

namespace ns{
    struct SceneObjectOnDisc{
        std::string name;
        std::string objFilename;        //o endere�o relativo do .obj correspondente a este objeto
        std::vector<std::string> textureFilenames;    //o endere�o relativo da textura correspondente a este objeto


        std::vector<float>  translationMatrix;
        std::vector<float>  rotationMatrix;
        std::vector<float>  scaleMatrix;


        std::string fragmentShaderFilename;
        std::string vertexShaderFilename;
        int textureWrapMode;
        std::vector<float>  velocity;                   //vetor velocidade do objeto, representa seu movimento
        std::vector<float>  blockMovement;              //vetor representando em quais eixos o objeto pode se mover. Por exemplo, se essa vari�vel tiver o valor (1, 0, 1) significa que o objeto pode se mover nos eixos X e Z, mas n�o no Y
        float               decelerationRate;           //velocidade de desacelera��o do objeto, simula atrito, resist�ncia do ar, etc (precisa ser um valor entre 0 e 1)
        std::string         onMouseOverName;            //nome da fun��o, para ser guardado no disco, � utilizado na fun��o de mapeamento de fun��es
        std::string         onClickName;
        std::string         onCollisionName;
        std::string         onMoveName;
        std::string         updateName;
        int                 thisCollisionType;          //o tipo de colis�o que deve ser implementada quando uma colis�o for detectada (pode ser el�stica, inel�stica ou im�vel (WALL))
        int                 thisColliderType;           //tipo (forma) do colisor deste objeto
        bool                active;                     //se este objeto deve ser considerado na cena, etc

        std::vector<std::string>    childrenList;       //lista dos nomes dos objetos "filhos"
    };

    void to_json(json& j, const SceneObjectOnDisc& obj) {
        j = json{{"name", obj.name},
                {"objFilename", obj.objFilename},
                {"textureFilenames", obj.textureFilenames},
                {"translationMatrix", obj.translationMatrix},
                {"rotationMatrix", obj.rotationMatrix},
                {"scaleMatrix", obj.scaleMatrix},
                {"fragmentShaderFilename", obj.fragmentShaderFilename},
                {"vertexShaderFilename", obj.vertexShaderFilename},
                {"textureWrapMode", obj.textureWrapMode},
                {"velocity", obj.velocity},
                {"blockMovement", obj.blockMovement},
                {"decelerationRate", obj.decelerationRate},
                {"onMouseOverName", obj.onMouseOverName},
                {"onClickName", obj.onClickName},
                {"onCollisionName", obj.onCollisionName},
                {"thisCollisionType", obj.thisCollisionType},
                {"thisColliderType", obj.thisColliderType},
                {"active", obj.active},
                {"onMoveName", obj.onMoveName},
                {"updateName", obj.updateName},
                {"childrenList", obj.childrenList}};
    }

    void from_json(const json& j, SceneObjectOnDisc& obj) {
        obj.name = j.at("name");
        obj.objFilename = j.at("objFilename");
        obj.textureFilenames = j.at("textureFilenames").get<std::vector<std::string>>();
        obj.fragmentShaderFilename = j.at("fragmentShaderFilename");
        obj.vertexShaderFilename = j.at("vertexShaderFilename");
        obj.textureWrapMode = j.at("textureWrapMode").get<int>();
        obj.velocity = j.at("velocity").get<std::vector<float>>();
        obj.blockMovement = j.at("blockMovement").get<std::vector<float>>();
        obj.decelerationRate = j.at("decelerationRate").get<float>();
        obj.onMouseOverName = j.at("onMouseOverName");
        obj.onClickName = j.at("onClickName");
        obj.onCollisionName = j.at("onCollisionName");
        obj.updateName = j.at("updateName");
        obj.thisCollisionType = j.at("thisCollisionType").get<int>();
        obj.thisColliderType = j.at("thisColliderType").get<int>();
        obj.active = j.at("active").get<bool>();
        obj.onMoveName = j.at("onMoveName");
        obj.translationMatrix = j.at("translationMatrix").get<std::vector<float>>();
        obj.rotationMatrix = j.at("rotationMatrix").get<std::vector<float>>();
        obj.scaleMatrix = j.at("scaleMatrix").get<std::vector<float>>();
        obj.childrenList = j.at("childrenList").get<std::vector<std::string>>();



    }
}

std::map<std::string, GLuint> texturesMap;
std::map<std::pair<GLuint, GLuint>, GLuint> GPUProgramsMap;
std::map<std::string, GLuint> fragmentShadersMap;
std::map<std::string, GLuint> vertexShadersMap;
std::map<std::string, ModelInformation> modelsMap;

bool KeyExistsInMap(std::map<std::string, GLuint> thisMap, std::string key){
    if(thisMap.find(key) != thisMap.end()) return true;
    else return false;
}

// Fun��o que computa as normais de um ObjModel, caso elas n�o tenham sido
// especificadas dentro do arquivo ".obj"
void ComputeNormals(ObjModel* model)
{
    if ( !model->attrib.normals.empty() )
        return;

    // Primeiro computamos as normais para todos os TRI�NGULOS.
    // Segundo, computamos as normais dos V�RTICES atrav�s do m�todo proposto
    // por Gouraud, onde a normal de cada v�rtice vai ser a m�dia das normais de
    // todas as faces que compartilham este v�rtice.

    size_t num_vertices = model->attrib.vertices.size() / 3;

    std::vector<int> num_triangles_per_vertex(num_vertices, 0);
    std::vector<glm::vec4> vertex_normals(num_vertices, glm::vec4(0.0f,0.0f,0.0f,0.0f));

    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);

            glm::vec4  vertices[3];
            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                const float vx = model->attrib.vertices[3*idx.vertex_index + 0];
                const float vy = model->attrib.vertices[3*idx.vertex_index + 1];
                const float vz = model->attrib.vertices[3*idx.vertex_index + 2];
                vertices[vertex] = glm::vec4(vx,vy,vz,1.0);
            }

            const glm::vec4  a = vertices[0];
            const glm::vec4  b = vertices[1];
            const glm::vec4  c = vertices[2];

            glm::vec4 vectorAB = b - a;
            glm::vec4 vectorAC = c - a;

            vectorAB = vectorAB/norm(vectorAB);
            vectorAC = vectorAC/norm(vectorAC);

            // PREENCHA AQUI o c�lculo da normal de um tri�ngulo cujos v�rtices
            // est�o nos pontos "a", "b", e "c", definidos no sentido anti-hor�rio.
            const glm::vec4  n = crossproduct(vectorAB, vectorAC);


            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                num_triangles_per_vertex[idx.vertex_index] += 1;
                vertex_normals[idx.vertex_index] += n;
                model->shapes[shape].mesh.indices[3*triangle + vertex].normal_index = idx.vertex_index;
            }
        }
    }

    model->attrib.normals.resize( 3*num_vertices );

    for (size_t i = 0; i < vertex_normals.size(); ++i)
    {
        glm::vec4 n = vertex_normals[i] / (float)num_triangles_per_vertex[i];
        n /= norm(n);
        model->attrib.normals[3*i + 0] = n.x;
        model->attrib.normals[3*i + 1] = n.y;
        model->attrib.normals[3*i + 2] = n.z;
    }
}

ModelInformation Load3DModel(std::string filename){

    ModelInformation newModelInformation;

    ObjModel newModel(filename.c_str());
    ComputeNormals(&newModel);

    GLuint new_vao_id;
    glGenVertexArrays(1, &new_vao_id);
    glBindVertexArray(new_vao_id);

    std::vector<GLuint> conteudo_vbo_indices;
    std::vector<float>  conteudo_vbo_positions;
    std::vector<float>  conteudo_vbo_normals;
    std::vector<float>  conteudo_vbo_textureCoords;

    GLuint id_vbo_indices;
    GLuint id_vbo_positions;
    GLuint id_vbo_normals;
    GLuint id_vbo_textureCoords;
    size_t first_index = conteudo_vbo_indices.size();
    size_t num_triangles = newModel.shapes[0].mesh.num_face_vertices.size();

    const float minval = std::numeric_limits<float>::min();
    const float maxval = std::numeric_limits<float>::max();

    glm::vec4 bbox_min = glm::vec4(maxval,maxval,maxval, 1.0f);
    glm::vec4 bbox_max = glm::vec4(minval,minval,minval, 1.0f);

    for (size_t triangle = 0; triangle < num_triangles; ++triangle)
    {

        for (size_t vertex = 0; vertex < 3; ++vertex)
        {
            tinyobj::index_t idx = newModel.shapes[0].mesh.indices[3*triangle + vertex];

            conteudo_vbo_indices.push_back(first_index + 3*triangle + vertex);

            const float vx = newModel.attrib.vertices[3*idx.vertex_index + 0];
            const float vy = newModel.attrib.vertices[3*idx.vertex_index + 1];
            const float vz = newModel.attrib.vertices[3*idx.vertex_index + 2];
            conteudo_vbo_positions.push_back( vx ); // X
            conteudo_vbo_positions.push_back( vy ); // Y
            conteudo_vbo_positions.push_back( vz ); // Z
            conteudo_vbo_positions.push_back( 1.0f ); // W

            bbox_min.x = std::min(bbox_min.x, vx);
            bbox_min.y = std::min(bbox_min.y, vy);
            bbox_min.z = std::min(bbox_min.z, vz);
            bbox_max.x = std::max(bbox_max.x, vx);
            bbox_max.y = std::max(bbox_max.y, vy);
            bbox_max.z = std::max(bbox_max.z, vz);

            if ( idx.normal_index != -1 )
            {
                const float nx = newModel.attrib.normals[3*idx.normal_index + 0];
                const float ny = newModel.attrib.normals[3*idx.normal_index + 1];
                const float nz = newModel.attrib.normals[3*idx.normal_index + 2];
                conteudo_vbo_normals.push_back( nx ); // X
                conteudo_vbo_normals.push_back( ny ); // Y
                conteudo_vbo_normals.push_back( nz ); // Z
                conteudo_vbo_normals.push_back( 0.0f ); // W
            }

            if ( idx.texcoord_index != -1 )
            {
                const float u = newModel.attrib.texcoords[2*idx.texcoord_index + 0];
                const float v = newModel.attrib.texcoords[2*idx.texcoord_index + 1];
                conteudo_vbo_textureCoords.push_back( u );
                conteudo_vbo_textureCoords.push_back( v );
            }
        }
    }

    size_t last_index = conteudo_vbo_indices.size() - 1;

    newModelInformation.first_index    = (void*)first_index; // Primeiro �ndice
    newModelInformation.num_indices    = last_index - first_index + 1; // N�mero de indices
    newModelInformation.rendering_mode = GL_TRIANGLES;       // �ndices correspondem ao tipo de rasteriza��o GL_TRIANGLES.
    newModelInformation.vertex_array_object_id = new_vao_id;

    newModelInformation.bbox_min_min_min = bbox_min;
    newModelInformation.bbox_max_max_max = bbox_max;
    newModelInformation.bbox_max_max_max.w = 1.0f;
    newModelInformation.bbox_min_min_min.w = 1.0f;

    newModelInformation.bbox_max_max_min = glm::vec4(bbox_max.x, bbox_max.y, bbox_min.z, 1.0f);
    newModelInformation.bbox_max_min_max = glm::vec4(bbox_max.x, bbox_min.y, bbox_max.z, 1.0f);
    newModelInformation.bbox_max_min_min = glm::vec4(bbox_max.x, bbox_min.y, bbox_min.z, 1.0f);

    newModelInformation.bbox_min_max_max = glm::vec4(bbox_min.x, bbox_max.y, bbox_max.z, 1.0f);
    newModelInformation.bbox_min_max_min = glm::vec4(bbox_min.x, bbox_max.y, bbox_min.z, 1.0f);
    newModelInformation.bbox_min_min_max = glm::vec4(bbox_min.x, bbox_min.y, bbox_max.z, 1.0f);


    //AQUI COME�AMOS A CRIAR E POPULAR OS VBOs DE FATO

    std::vector<GLuint> buffers;

    glGenBuffers(1, &id_vbo_positions);
    glBindBuffer(GL_ARRAY_BUFFER, id_vbo_positions);
        glBufferData(GL_ARRAY_BUFFER, conteudo_vbo_positions.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, conteudo_vbo_positions.size() * sizeof(float), conteudo_vbo_positions.data());
        GLuint location = 0; // "(location = 0)" em "shader_vertex.glsl"
        GLint  number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    buffers.push_back(id_vbo_positions);

    if(!conteudo_vbo_normals.empty()){ //se o .obj tinha normais
        glGenBuffers(1, &id_vbo_normals);
        glBindBuffer(GL_ARRAY_BUFFER, id_vbo_normals);
            glBufferData(GL_ARRAY_BUFFER, conteudo_vbo_normals.size() * sizeof(float), NULL, GL_STATIC_DRAW);
            glBufferSubData(GL_ARRAY_BUFFER, 0, conteudo_vbo_normals.size() * sizeof(float), conteudo_vbo_normals.data());
            GLuint location = 1; // "(location = 0)" em "shader_vertex.glsl"
            GLint  number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
            glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        buffers.push_back(id_vbo_normals);
    }

    if(!conteudo_vbo_textureCoords.empty()){ //se o .obj tinha coordenadas de textura
        glGenBuffers(1, &id_vbo_textureCoords);
        glBindBuffer(GL_ARRAY_BUFFER, id_vbo_textureCoords);
            glBufferData(GL_ARRAY_BUFFER, conteudo_vbo_textureCoords.size() * sizeof(float), NULL, GL_STATIC_DRAW);
            glBufferSubData(GL_ARRAY_BUFFER, 0, conteudo_vbo_textureCoords.size() * sizeof(float), conteudo_vbo_textureCoords.data());
            GLuint location = 2; // "(location = 0)" em "shader_vertex.glsl"
            GLint  number_of_dimensions = 2; // vec2 em "shader_vertex.glsl"
            glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        buffers.push_back(id_vbo_textureCoords);
    }

    glGenBuffers(1, &id_vbo_indices);

    // "Ligamos" o buffer. Note que o tipo agora � GL_ELEMENT_ARRAY_BUFFER.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_vbo_indices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, conteudo_vbo_indices.size() * sizeof(GLuint), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, conteudo_vbo_indices.size() * sizeof(GLuint), conteudo_vbo_indices.data());
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // XXX Errado!
    //
    buffers.push_back(id_vbo_indices);
    // "Desligamos" o VAO, evitando assim que opera��es posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);

    newModelInformation.buffers = buffers;

    return newModelInformation;
}

SceneObject LoadNewObject(std::string objFilename){
    SceneObject newObj;
    ModelInformation modelInfo;

    if(modelsMap.find(objFilename) != modelsMap.end()){
        modelInfo = modelsMap.at(objFilename);

    } else {
        modelInfo = Load3DModel(objFilename);
        modelsMap.insert ( std::pair<std::string,ModelInformation>(objFilename,modelInfo));
    }

    newObj.bbox_max_max_max = modelInfo.bbox_max_max_max;
    newObj.bbox_max_max_min = modelInfo.bbox_max_min_max;
    newObj.bbox_max_min_max = modelInfo.bbox_max_min_max;
    newObj.bbox_max_min_min = modelInfo.bbox_max_min_min;
    newObj.bbox_min_max_max = modelInfo.bbox_min_max_max;
    newObj.bbox_min_max_min = modelInfo.bbox_min_max_min;
    newObj.bbox_min_min_max = modelInfo.bbox_min_min_max;
    newObj.bbox_min_min_min = modelInfo.bbox_min_min_min;
    newObj.buffers = modelInfo.buffers;
    newObj.first_index = modelInfo.first_index;
    newObj.num_indices = modelInfo.num_indices;
    newObj.rendering_mode = modelInfo.rendering_mode;
    newObj.vertex_array_object_id = modelInfo.vertex_array_object_id;
    newObj.objFilename = objFilename;

    return newObj;
}

GLuint CreateFragmentShader(std::string fragment_shader_filename){
    return LoadShader_Fragment(fragment_shader_filename.c_str());
}

GLuint CreateVertexShader(std::string vertex_shader_filename){
    return LoadShader_Vertex(vertex_shader_filename.c_str());
}

GLuint CreateGPUProgram(GLuint vertex_shader_id, GLuint fragment_shader_id){

    GLuint shaderprogram_id = glCreateProgram(); //cria um novo programa de GPU

    /* Attach our shaders to our program */
    glAttachShader(shaderprogram_id, vertex_shader_id);
    glAttachShader(shaderprogram_id, fragment_shader_id);

    //gera o bin�rio do programa de GPU e sobe ele para a mem�ria
    glLinkProgram(shaderprogram_id);

    return shaderprogram_id;
}

GLuint CreateNewTexture(std::string textureFileName, WrapMode wrapMode, std::vector<GLfloat> borderColor){


    stbi_set_flip_vertically_on_load(true);
    int width;
    int height;
    int channels;
    unsigned char *data = stbi_load(textureFileName.c_str(), &width, &height, &channels, 4);

    if ( data == NULL )
    {
        std::exit(EXIT_FAILURE);
    }


    GLuint newTexture_id;
    glGenTextures(1, &newTexture_id);

    glBindTexture(GL_TEXTURE_2D, newTexture_id);


    //configuramos o modo de wrap de acordo com o input;
    if(wrapMode == WrapMode::CLAMP_TO_BORDER){
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &borderColor[0]);
    }
    else if(wrapMode == WrapMode::CLAMP_TO_EDGE){
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    else if(wrapMode == WrapMode::MIRRORED_REPEAT){
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    }
    else{
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR); //filtering, estamos usando uma t�cnica mista de
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_LINEAR); //mipmap, nearest e linear interpolation


    //AQUI N�S CARREGAMOS A IMAGEM PARA O OBJETO DE TEXTURA
    //void glTexImage2D(	GLenum target,  -> O TIPO DE TEXTURA
 	//GLint level,                          -> N�VEL DE DETALHE (0 � a imagem base, >0 � para mipmaps)
 	//GLint internalformat,                 -> especifica n�mero/tamanho dos componentes de cor DA TEXTURA(por exemplo, GL_RGBA)
 	//GLsizei width,                        -> largura da imagem
 	//GLsizei height,                       -> altura da imagem
 	//GLint border,                         -> este valor deve ser sempre 0 (????)
 	//GLenum format,                        -> especifica o formato dos pixels DA IMAGEM
 	//GLenum type,                          -> tipo dos dados que est�o no array data
 	//const GLvoid * data);                 -> ponteiro para um array de pixels em mem�ria (a imagem)

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    //geramos os mipmaps
    glGenerateMipmap(GL_TEXTURE_2D);



    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);

    return newTexture_id;
}

GLuint GetTextureId(std::string textureFileName, WrapMode wrapMode, std::vector<GLfloat> borderColor){
    if(KeyExistsInMap(texturesMap, textureFileName)){
        return texturesMap.at(textureFileName);
    }
    else{
        GLuint newTextureId = CreateNewTexture(textureFileName, wrapMode, borderColor);
        texturesMap.insert ( std::pair<std::string,GLuint>(textureFileName,newTextureId));
        return newTextureId;
    }
}

GLuint GetVertexShaderId(std::string vertex_shader_filename){
    if(KeyExistsInMap(vertexShadersMap, vertex_shader_filename)){
        return vertexShadersMap.at(vertex_shader_filename);
    }
    else{
        GLuint newVertexId = CreateVertexShader(vertex_shader_filename);
        vertexShadersMap.insert ( std::pair<std::string,GLuint>(vertex_shader_filename,newVertexId));
        return newVertexId;
    }
}

GLuint GetFragmentShaderId(std::string fragment_shader_filename){
    if(KeyExistsInMap(fragmentShadersMap, fragment_shader_filename)){
        return fragmentShadersMap.at(fragment_shader_filename);
    }
    else{
        GLuint newFragmentId = CreateFragmentShader(fragment_shader_filename);
        fragmentShadersMap.insert ( std::pair<std::string,GLuint>(fragment_shader_filename,newFragmentId));
        return newFragmentId;
    }
}

GLuint GetGPUProgramId(GLuint vertex_shader_id, GLuint fragment_shader_id){
    if(GPUProgramsMap.find(std::pair<GLuint, GLuint>(vertex_shader_id, fragment_shader_id)) != GPUProgramsMap.end()){
        return GPUProgramsMap.at(std::pair<GLuint, GLuint>(vertex_shader_id, fragment_shader_id));
    }
    else{
        GLuint newGPUProgramId = CreateGPUProgram(vertex_shader_id, fragment_shader_id);
        GPUProgramsMap.insert( std::pair<std::pair<GLuint, GLuint>, GLuint> (std::pair<GLuint, GLuint>(vertex_shader_id, fragment_shader_id), newGPUProgramId) );
        return newGPUProgramId;
    }
}

std::vector<int> GetChildrenList(std::vector<std::string> childrenNames, std::vector<SceneObject>& currentScene, int parentIndex){
    std::vector<int> retorno;
    for(unsigned int i = 0; i<childrenNames.size(); i++){
        for(unsigned int j = 0; j<currentScene.size(); j++){
            if(childrenNames[i].compare(currentScene[j].name) == 0){
                retorno.push_back(j);
                currentScene[j].parentIndex = parentIndex;
            }
        }
    }
    return retorno;
}

void WriteSceneToFile(std::vector<ns::SceneObjectOnDisc> objsList, std::string newSceneFilename){

    json codificado;
    for(unsigned int i = 0; i < objsList.size(); i++){
        codificado["Scene"][i] = objsList[i];
    }

    std::ofstream o(newSceneFilename.c_str());
    o << std::setw(4) << codificado;

}

void SaveScene(std::string filename, std::vector<SceneObject> currentScene){
    std::vector<ns::SceneObjectOnDisc> objsListToWrite;

    for(unsigned int i = 0; i<currentScene.size(); i++){
        ns::SceneObjectOnDisc curObj;

        for(unsigned int j = 0; j < 16; j++){
            curObj.translationMatrix.push_back(((float*)glm::value_ptr(currentScene[i].translationMatrix))[j]);
            curObj.rotationMatrix.push_back(((float*)glm::value_ptr(currentScene[i].rotationMatrix))[j]);
            curObj.scaleMatrix.push_back(((float*)glm::value_ptr(currentScene[i].scaleMatrix))[j]);
        }

        std::vector<float> blockMovementAux {currentScene[i].blockMovement.x, currentScene[i].blockMovement.y, currentScene[i].blockMovement.z};
        std::vector<float> velocityAux {currentScene[i].velocity.x, currentScene[i].velocity.y, currentScene[i].velocity.z, currentScene[i].velocity.w};

        curObj.name = currentScene[i].name;

        curObj.blockMovement =          blockMovementAux;
        curObj.velocity =               velocityAux;

        curObj.active =                 currentScene[i].active;
        curObj.decelerationRate =       currentScene[i].decelerationRate;
        curObj.textureWrapMode =        currentScene[i].textureWrapMode;
        curObj.thisColliderType =       currentScene[i].thisColliderType;
        curObj.thisCollisionType =      currentScene[i].thisCollisionType;

        curObj.objFilename =            currentScene[i].objFilename;
        curObj.textureFilenames =        currentScene[i].textureFilenames;
        curObj.fragmentShaderFilename = currentScene[i].fragmentShaderFilename;
        curObj.vertexShaderFilename =   currentScene[i].vertexShaderFilename;
        curObj.onClickName =            currentScene[i].onClickName;
        curObj.onMouseOverName =        currentScene[i].onMouseOverName;
        curObj.onCollisionName =        currentScene[i].onCollisionName;
        curObj.onMoveName =             currentScene[i].onMoveName;
        curObj.updateName =             currentScene[i].updateName;

        curObj.childrenList =           currentScene[i].childrenNames;

        objsListToWrite.push_back(curObj);
    }

    WriteSceneToFile(objsListToWrite, filename);

}

std::vector<ns::SceneObjectOnDisc> ReadSceneFromFile(std::string sceneFilename){

    // read a JSON file
    std::ifstream i(sceneFilename.c_str());
    json j;
    i >> j;

    return j["Scene"].get<std::vector<ns::SceneObjectOnDisc>>();

}

std::vector<SceneObject> LoadScene(std::string sceneFilename){
    std::vector<SceneObject> loadedScene;
    SceneObject aux;

    std::vector<ns::SceneObjectOnDisc> objsList = ReadSceneFromFile(sceneFilename);

    for(unsigned int i = 0; i<objsList.size(); i++){

        aux = LoadNewObject(objsList[i].objFilename); //gera o objeto e popula com as informa��es do .obj

        aux.translationMatrix = glm::mat4(objsList[i].translationMatrix[0], objsList[i].translationMatrix[1], objsList[i].translationMatrix[2], objsList[i].translationMatrix[3],
                                          objsList[i].translationMatrix[4], objsList[i].translationMatrix[5], objsList[i].translationMatrix[6], objsList[i].translationMatrix[7],
                                          objsList[i].translationMatrix[8], objsList[i].translationMatrix[9], objsList[i].translationMatrix[10], objsList[i].translationMatrix[11],
                                          objsList[i].translationMatrix[12], objsList[i].translationMatrix[13], objsList[i].translationMatrix[14], objsList[i].translationMatrix[15]);

        aux.rotationMatrix = glm::mat4(objsList[i].rotationMatrix[0], objsList[i].rotationMatrix[1], objsList[i].rotationMatrix[2], objsList[i].rotationMatrix[3],
                                          objsList[i].rotationMatrix[4], objsList[i].rotationMatrix[5], objsList[i].rotationMatrix[6], objsList[i].rotationMatrix[7],
                                          objsList[i].rotationMatrix[8], objsList[i].rotationMatrix[9], objsList[i].rotationMatrix[10], objsList[i].rotationMatrix[11],
                                          objsList[i].rotationMatrix[12], objsList[i].rotationMatrix[13], objsList[i].rotationMatrix[14], objsList[i].rotationMatrix[15]);

        aux.scaleMatrix = glm::mat4(objsList[i].scaleMatrix[0], objsList[i].scaleMatrix[1], objsList[i].scaleMatrix[2], objsList[i].scaleMatrix[3],
                                          objsList[i].scaleMatrix[4], objsList[i].scaleMatrix[5], objsList[i].scaleMatrix[6], objsList[i].scaleMatrix[7],
                                          objsList[i].scaleMatrix[8], objsList[i].scaleMatrix[9], objsList[i].scaleMatrix[10], objsList[i].scaleMatrix[11],
                                          objsList[i].scaleMatrix[12], objsList[i].scaleMatrix[13], objsList[i].scaleMatrix[14], objsList[i].scaleMatrix[15]);

        aux.textureWrapMode = objsList[i].textureWrapMode;
        aux.textureFilenames = objsList[i].textureFilenames;

        for(unsigned int k = 0; k<aux.textureFilenames.size(); k++){
            aux.textureIds.push_back(GetTextureId(aux.textureFilenames[k], (WrapMode)aux.textureWrapMode));
        }

        aux.fragmentShaderFilename = objsList[i].fragmentShaderFilename;
        aux.vertexShaderFilename = objsList[i].vertexShaderFilename;

        aux.vertexShaderId = GetVertexShaderId(aux.vertexShaderFilename);
        aux.fragmentShaderId = GetFragmentShaderId(aux.fragmentShaderFilename);
        aux.gpuProgramId = GetGPUProgramId(aux.vertexShaderId, aux.fragmentShaderId);

        aux.velocity = glm::vec4(objsList[i].velocity[0], objsList[i].velocity[1], objsList[i].velocity[2], objsList[i].velocity[3]);
        aux.blockMovement = glm::vec4(objsList[i].blockMovement[0], objsList[i].blockMovement[1], objsList[i].blockMovement[2], objsList[i].blockMovement[3]);
        aux.decelerationRate = objsList[i].decelerationRate;

        aux.onMouseOverName  = objsList[i].onMouseOverName;
        aux.onMouseOver      = FunctionMapping(aux.onMouseOverName);

        aux.onClickName      =  objsList[i].onClickName;
        aux.onClick          =  FunctionMapping(aux.onClickName);

        aux.updateName      =  objsList[i].updateName;
        aux.update          =  FunctionMapping(aux.updateName);

        aux.onCollisionName = objsList[i].onCollisionName;
        aux.onCollision     = CollisionFunctionMapping(aux.onCollisionName);

        aux.thisColliderType = objsList[i].thisColliderType;
        aux.thisCollisionType = objsList[i].thisCollisionType;

        aux.active = objsList[i].active;

        aux.name = objsList[i].name;

        aux.onMoveName = objsList[i].onMoveName;
        aux.onMove = FunctionMapping(aux.onMoveName);

        aux.childrenNames = objsList[i].childrenList;

        loadedScene.push_back(aux);

    }

    for(unsigned int i = 0; i<loadedScene.size(); i++){
        if(loadedScene[i].childrenNames.size() > 0){
            loadedScene[i].childrenIndices = GetChildrenList(loadedScene[i].childrenNames, loadedScene, i);
        }
    }

    return loadedScene;
}

void OpenScene(std::string filename, std::vector<SceneObject>& currentScene){
    currentScene = LoadScene(filename);
}

void OpenSceneAdditive(std::string filename, std::vector<SceneObject>& currentScene){

    std::vector<SceneObject> newScene = LoadScene(filename);

    currentScene.insert( currentScene.end(), newScene.begin(), newScene.end() );
}

void UnloadScene(std::vector<SceneObject>& currentScene){
    for (auto const& item : texturesMap){
        glDeleteTextures(1, &item.second);
    }
    for (auto const& item : vertexShadersMap){
        glDeleteShader(item.second);
    }
    for (auto const& item : fragmentShadersMap){
        glDeleteShader(item.second);
    }
    for (auto const& item : GPUProgramsMap){
        glDeleteProgram(item.second);
    }
    for (auto const& item : modelsMap){
        glDeleteBuffers(item.second.buffers.size(), &item.second.buffers[0]);
        glDeleteVertexArrays(1, &item.second.vertex_array_object_id);
    }

    texturesMap.clear();
    GPUProgramsMap.clear();
    fragmentShadersMap.clear();
    vertexShadersMap.clear();
    modelsMap.clear();
    currentScene.clear();
}

void Debug_NewObjectSphere(std::vector<SceneObject>& currentScene){
    //CRIA��O DE UMA NOVA CENA

    currentScene.push_back(LoadNewObject("../../data/crate.obj"));
    currentScene.back().name = "assentoDummy";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/crate_1.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/crate_1.jpg");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/crate_1.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/crate_1.jpg");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient_spherical.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient_spherical.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::NONE;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Identity();
    currentScene.back().translationMatrix = Matrix_Translate(0.0f, -8.0f, 0.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(0.2f, 0.1f, 0.2f);

    currentScene.push_back(LoadNewObject("../../data/crate.obj"));
    currentScene.back().name = "plaquinha1";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/plaquinha.png", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/plaquinha.png");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/plaquinha.png", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/plaquinha.png");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient+blinn-phong_planar(1).glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient+blinn-phong_planar(1).glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "DescricaoDummy";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Rotate_Y(PI)*Matrix_Rotate_X(PI/2);
    currentScene.back().translationMatrix = Matrix_Translate(7.5f, -50.0f, 28.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(0.075f, 0.005f, 0.075f);
    currentScene.back().childrenNames.push_back("pedestal1");

    currentScene.push_back(LoadNewObject("../../data/crate.obj"));
    currentScene.back().name = "pedestal1";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/Liberty-GreenBronze-1.bmp", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/Liberty-GreenBronze-1.bmp");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/Liberty-GreenBronze-1.bmp", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/Liberty-GreenBronze-1.bmp");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient_spherical.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient_spherical.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::NONE;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Identity();
    currentScene.back().translationMatrix = Matrix_Translate(0.0f, 0.0f, 1.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(1.5f, 1.5f, 80.0f);

    currentScene.push_back(LoadNewObject("../../data/crate.obj"));
    currentScene.back().name = "Chao1";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/Liberty-Pavimentazione-1.bmp", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/Liberty-Pavimentazione-1.bmp");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/Liberty-Pavimentazione-1.bmp", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/Liberty-Pavimentazione-1.bmp");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient+blinn-phong_planar.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient+blinn-phong_planar.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("LightningGeneratorUpdate");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::NONE;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Rotate_X(PI/2);
    currentScene.back().translationMatrix = Matrix_Translate(0.0f, -3.0f, 0.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(10.0f, 0.5f, 10.0f);

    currentScene.push_back(LoadNewObject("../../data/crate.obj"));
    currentScene.back().name = "Teto1";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/ceiling.png", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/ceiling.png");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/ceiling.png", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/ceiling.png");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient+blinn-phong_planar.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient+blinn-phong_planar.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::NONE;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Rotate_X(PI/2);
    currentScene.back().translationMatrix = Matrix_Translate(0.0f, 3.0f, 0.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(10.0f, 0.5f, 10.0f);

    currentScene.push_back(LoadNewObject("../../data/sphere.obj"));
    currentScene.back().name = "bola1";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/Liberty-Pavimentazione-1.bmp", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/Liberty-Pavimentazione-1.bmp");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/Liberty-Pavimentazione-1.bmp", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/Liberty-Pavimentazione-1.bmp");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex_gouraud.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_gouraud_lambert+ambient_spherical.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_gouraud_lambert+ambient_spherical.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex_gouraud.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(1.0f, 0.0f, 1.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "SphereOnMouseOver";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "SphereOnMove";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::SPHERE;
    currentScene.back().thisCollisionType = (int)CollisionType::ELASTIC;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Identity();
    currentScene.back().translationMatrix = Matrix_Translate(6.0f, -1.0f, -6.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(0.5f, 0.5f, 0.5f);

    currentScene.push_back(LoadNewObject("../../data/crate.obj"));
    currentScene.back().name = "Parede1";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/wallConcrete.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/wallConcrete.jpg");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/wallConcrete.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/wallConcrete.jpg");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient+blinn-phong_planar.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient+blinn-phong_planar.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Identity();
    currentScene.back().translationMatrix = Matrix_Translate(0.0f, 0.0f, -600.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(10.0f, 1.0f, 0.01f);

    currentScene.push_back(LoadNewObject("../../data/crate.obj"));
    currentScene.back().name = "Parede2";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/wallConcrete.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/wallConcrete.jpg");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/wallConcrete.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/wallConcrete.jpg");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient+blinn-phong_planar.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient+blinn-phong_planar.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Rotate_Y(PI/2);
    currentScene.back().translationMatrix = Matrix_Translate(1000.0f, 0.0f, 0.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(0.01f, 1.0f, 10.0f);

    currentScene.push_back(LoadNewObject("../../data/crate.obj"));
    currentScene.back().name = "Parede3";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/wallConcrete.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/wallConcrete.jpg");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/wallConcrete.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/wallConcrete.jpg");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient+blinn-phong_planar.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient+blinn-phong_planar.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Rotate_Y(PI/2);
    currentScene.back().translationMatrix = Matrix_Translate(-1000.0f, 0.0f, 0.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(0.01f, 1.0f, 10.0f);

    currentScene.push_back(LoadNewObject("../../data/crate.obj"));
    currentScene.back().name = "Parede4";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/wallConcrete.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/wallConcrete.jpg");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/wallConcrete.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/wallConcrete.jpg");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient+blinn-phong_planar.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient+blinn-phong_planar.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Identity();
    currentScene.back().translationMatrix = Matrix_Translate(0.0f, 0.0f, 1000.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(10.0f, 1.0f, 0.01f);

    currentScene.push_back(LoadNewObject("../../data/crate.obj"));
    currentScene.back().name = "Parede5";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/wallConcrete.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/wallConcrete.jpg");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/wallConcrete.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/wallConcrete.jpg");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient+blinn-phong_planar.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient+blinn-phong_planar.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Rotate_Y(PI/2);
    currentScene.back().translationMatrix = Matrix_Translate(-200.0f, 0.0f, 1.5f);
    currentScene.back().scaleMatrix = Matrix_Scale(0.01f, 1.0f, 5.0f);

    currentScene.push_back(LoadNewObject("../../data/crate.obj"));
    currentScene.back().name = "Parede6";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/wallConcrete.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/wallConcrete.jpg");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/wallConcrete.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/wallConcrete.jpg");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient+blinn-phong_planar.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient+blinn-phong_planar.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Rotate_Y(PI/2);
    currentScene.back().translationMatrix = Matrix_Translate(200.0f, 0.0f, 1.5f);
    currentScene.back().scaleMatrix = Matrix_Scale(0.01f, 1.0f, 5.0f);

    currentScene.push_back(LoadNewObject("../../data/crate.obj"));
    currentScene.back().name = "Parede7";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/wallConcrete.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/wallConcrete.jpg");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/wallConcrete.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/wallConcrete.jpg");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient+blinn-phong_planar.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient+blinn-phong_planar.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Identity();
    currentScene.back().translationMatrix = Matrix_Translate(1.39f, 0.0f, 250.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(5.1f, 1.0f, 0.01f);

    currentScene.push_back(LoadNewObject("../../data/crate.obj"));
    currentScene.back().name = "Parede8";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/wallConcrete.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/wallConcrete.jpg");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/wallConcrete.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/wallConcrete.jpg");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient+blinn-phong_planar.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient+blinn-phong_planar.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Identity();
    currentScene.back().translationMatrix = Matrix_Translate(-1.39f, 0.0f, 250.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(5.1f, 1.0f, 0.01f);


    currentScene.push_back(LoadNewObject("../../data/FrameOBJ.obj"));
    currentScene.back().name = "Moldura1";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/pine1.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/pine1.jpg");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/pine1.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/pine1.jpg");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient+blinn-phong_planar_no_secondary.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient+blinn-phong_planar_no_secondary.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Rotate_Y(PI/2);
    currentScene.back().translationMatrix = Matrix_Translate(-8.25f, -2.0f, 20.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(0.25f, 0.25f, 0.25f);
    currentScene.back().childrenNames.push_back("marilyn");

    currentScene.push_back(LoadNewObject("../../data/crate.obj"));
    currentScene.back().name = "marilyn";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/Warhol_Marilyn.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/Warhol_Marilyn.jpg");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/Warhol_MarilynDark.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/Warhol_MarilynDark.jpg");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient+blinn-phong_planar(1).glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient+blinn-phong_planar(1).glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Identity();
    currentScene.back().translationMatrix = Matrix_Translate(0.0f, 1.25f, 0.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(1.9f, 2.0f, 0.5f);

    currentScene.push_back(LoadNewObject("../../data/FrameOBJ.obj"));
    currentScene.back().name = "Moldura2";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/pine1.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/pine1.jpg");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/pine1.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/pine1.jpg");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient+blinn-phong_planar_no_secondary.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient+blinn-phong_planar_no_secondary.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Rotate_Y(PI/2);
    currentScene.back().translationMatrix = Matrix_Translate(-8.25f, -2.0f, 30.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(0.25f, 0.25f, 0.25f);
    currentScene.back().childrenNames.push_back("painting_city");

    currentScene.push_back(LoadNewObject("../../data/crate.obj"));
    currentScene.back().name = "painting_city";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/painting_city.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/painting_city.jpg");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/painting_city.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/painting_city.jpg");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient+blinn-phong_planar_no_secondary.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient+blinn-phong_planar_no_secondary.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Identity();
    currentScene.back().translationMatrix = Matrix_Translate(0.0f, 1.25f, 0.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(1.9f, 2.0f, 0.5f);




    currentScene.push_back(LoadNewObject("../../data/FrameOBJ.obj"));
    currentScene.back().name = "Moldura3";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/pine1.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/pine1.jpg");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/pine1.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/pine1.jpg");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient+blinn-phong_planar_no_secondary.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient+blinn-phong_planar_no_secondary.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Rotate_Y(-PI/2);
    currentScene.back().translationMatrix = Matrix_Translate(8.25f, -2.0f, 20.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(0.25f, 0.25f, 0.25f);
    currentScene.back().childrenNames.push_back("abstract_dancing");

    currentScene.push_back(LoadNewObject("../../data/crate.obj"));
    currentScene.back().name = "abstract_dancing";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/abstract_dancing.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/abstract_dancing.jpg");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/abstract_dancing.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/abstract_dancing.jpg");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient+blinn-phong_planar_no_secondary.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient+blinn-phong_planar_no_secondary.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Identity();
    currentScene.back().translationMatrix = Matrix_Translate(0.0f, 1.25f, 0.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(1.9f, 2.0f, 0.5f);

    currentScene.push_back(LoadNewObject("../../data/FrameOBJ.obj"));
    currentScene.back().name = "Moldura4";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/pine1.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/pine1.jpg");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/pine1.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/pine1.jpg");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient+blinn-phong_planar_no_secondary.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient+blinn-phong_planar_no_secondary.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Rotate_Y(-PI/2);
    currentScene.back().translationMatrix = Matrix_Translate(8.25f, -2.0f, 30.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(0.25f, 0.25f, 0.25f);
    currentScene.back().childrenNames.push_back("abstract2");

    currentScene.push_back(LoadNewObject("../../data/crate.obj"));
    currentScene.back().name = "abstract2";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/abstract2.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/abstract2.jpg");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/abstract2Dark.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/abstract2Dark.jpg");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient+blinn-phong_planar(1).glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient+blinn-phong_planar(1).glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Identity();
    currentScene.back().translationMatrix = Matrix_Translate(0.0f, 1.25f, 0.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(1.9f, 2.0f, 0.5f);

    currentScene.push_back(LoadNewObject("../../data/frame5set.obj"));
    currentScene.back().name = "Abstrata";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/abstract.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/abstract.jpg");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/abstractDark.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/abstractDark.jpg");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient+blinn-phong_planar_with_secondary.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient+blinn-phong_planar_with_secondary.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "AbstrataDummyOnMouseOver";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Identity();
    currentScene.back().translationMatrix = Matrix_Translate(60.0f, -15.0f, 48.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(0.05f, 0.05f, 0.05f);

    currentScene.push_back(LoadNewObject("../../data/desk1.obj"));
    currentScene.back().name = "Balcao";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/white wood.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/white wood.jpg");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/white wood.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/white wood.jpg");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient+blinn-phong_planar_XZ.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient+blinn-phong_planar_XZ.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "GavetaOnClick";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "GavetaOnMouseOver";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::NONE;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Rotate_Y(-PI/2);
    currentScene.back().translationMatrix = Matrix_Translate(-750.0f, -750.0f, 8500.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(0.0015f, 0.0015f, 0.001f);
    currentScene.back().childrenNames.push_back("Gaveta1");

    currentScene.push_back(LoadNewObject("../../data/deskdrawer.obj"));
    currentScene.back().name = "Gaveta1";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/blackboard.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/blackboard.jpg");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/blackboard.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/blackboard.jpg");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient_spherical_no_secondary.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient_spherical_no_secondary.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = "GavetaUpdate";
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Identity();
    currentScene.back().translationMatrix = Matrix_Translate(0.0f, 200.0f, 0.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(1.0f, 0.75f, 1.0f);

    currentScene.push_back(LoadNewObject("../../data/chave1.obj"));
    currentScene.back().name = "chave1";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/metal.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/metal.jpg");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/metal.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/metal.jpg");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient+blinn-phong_planar_no_secondary.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient+blinn-phong_planar_no_secondary.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "ChaveOnMouseOver";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = "";
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Identity();
    currentScene.back().translationMatrix = Matrix_Translate(-23.0f, -8.0f, 175.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(0.05f, 0.05f, 0.05f);

    currentScene.push_back(LoadNewObject("../../data/crate.obj"));
    currentScene.back().name = "porta1";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/door1.png", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/door1.png");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/doorDark1.png", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/doorDark1.png");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient+blinn-phong_planar(1).glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient+blinn-phong_planar(1).glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Identity();
    currentScene.back().translationMatrix = Matrix_Translate(3.0f, -0.5f, 999.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(0.4f, 0.75f, 0.01f);
    currentScene.back().childrenNames.push_back("macaneta1");

    currentScene.push_back(LoadNewObject("../../data/doorknob1.obj"));
    currentScene.back().name = "macaneta1";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/bronze.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/bronze.jpg");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/bronze.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/bronze.jpg");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient_spherical_no_secondary.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient_spherical_no_secondary.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Rotate_Y(-PI/2);
    currentScene.back().translationMatrix = Matrix_Translate(-69.0f, -12.0f, -10.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(0.01f, 0.005f, 0.5f);

    currentScene.push_back(LoadNewObject("../../data/crate.obj"));
    currentScene.back().name = "porta2";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/door1.png", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/door1.png");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/doorDark0.png", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/doorDark0.png");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient+blinn-phong_planar(1).glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient+blinn-phong_planar(1).glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Rotate_Y(PI);
    currentScene.back().translationMatrix = Matrix_Translate(0.0f, -0.5f, -599.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(0.4f, 0.75f, 0.01f);
    currentScene.back().childrenNames.push_back("macaneta2");

    currentScene.push_back(LoadNewObject("../../data/doorknob1.obj"));
    currentScene.back().name = "macaneta2";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/bronze.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/bronze.jpg");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/bronze.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/bronze.jpg");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient_spherical_no_secondary.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient_spherical_no_secondary.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Rotate_Y(-PI/2);
    currentScene.back().translationMatrix = Matrix_Translate(-69.0f, -12.0f, -10.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(0.01f, 0.005f, 0.5f);

    currentScene.push_back(LoadNewObject("../../data/crate.obj"));
    currentScene.back().name = "porta3";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/door1.png", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/door1.png");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/doorDark0.png", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/doorDark0.png");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient+blinn-phong_planar(1).glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient+blinn-phong_planar(1).glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Rotate_Y(-PI/2);
    currentScene.back().translationMatrix = Matrix_Translate(-999.0f, -0.5f, 0.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(0.01f, 0.75f, 0.4f);
    currentScene.back().childrenNames.push_back("macaneta3");

    currentScene.push_back(LoadNewObject("../../data/doorknob1.obj"));
    currentScene.back().name = "macaneta3";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/bronze.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/bronze.jpg");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/bronze.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/bronze.jpg");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient_spherical_no_secondary.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient_spherical_no_secondary.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Rotate_Y(-PI/2);
    currentScene.back().translationMatrix = Matrix_Translate(-69.0f, -12.0f, -10.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(0.01f, 0.005f, 0.5f);

    currentScene.push_back(LoadNewObject("../../data/crate.obj"));
    currentScene.back().name = "porta4";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/door1.png", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/door1.png");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/doorDark0.png", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/doorDark0.png");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient+blinn-phong_planar(1).glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient+blinn-phong_planar(1).glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Rotate_Y(PI/2);
    currentScene.back().translationMatrix = Matrix_Translate(999.0f, -0.5f, 0.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(0.01f, 0.75f, 0.4f);
    currentScene.back().childrenNames.push_back("macaneta4");

    currentScene.push_back(LoadNewObject("../../data/doorknob1.obj"));
    currentScene.back().name = "macaneta4";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/bronze.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/bronze.jpg");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/bronze.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/bronze.jpg");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient_spherical_no_secondary.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient_spherical_no_secondary.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Rotate_Y(-PI/2);
    currentScene.back().translationMatrix = Matrix_Translate(-69.0f, -12.0f, -10.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(0.01f, 0.005f, 0.5f);

    currentScene.push_back(LoadNewObject("../../data/Venus_de_Milo.obj"));
    currentScene.back().name = "venus";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/aesthetic.png", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/aesthetic.png");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/aesthetic.png", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/aesthetic.png");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient_spherical.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient_spherical.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "VidroDummyOnClick";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "VidroDummyOnMouseOver";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Identity();
    currentScene.back().translationMatrix = Matrix_Translate(-6000.0f, -2000.0f, -6000.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(0.0005f,0.0005f,0.0005f);

    currentScene.push_back(LoadNewObject("../../data/crate.obj"));
    currentScene.back().name = "plaquinha2";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/plaquinha.png", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/plaquinha.png");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/plaquinha.png", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/plaquinha.png");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient+blinn-phong_planar(1).glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient+blinn-phong_planar(1).glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "DescricaoVenus";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Rotate_Y(PI)*Matrix_Rotate_X(PI/2);
    currentScene.back().translationMatrix =Matrix_Translate(-50.0f, -50.0f, -25.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(0.075f, 0.005f, 0.075f);
    currentScene.back().childrenNames.push_back("pedestal2");

    currentScene.push_back(LoadNewObject("../../data/crate.obj"));
    currentScene.back().name = "pedestal2";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/Liberty-GreenBronze-1.bmp", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/Liberty-GreenBronze-1.bmp");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/Liberty-GreenBronze-1.bmp", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/Liberty-GreenBronze-1.bmp");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient_spherical.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient_spherical.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::NONE;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Identity();
    currentScene.back().translationMatrix = Matrix_Translate(0.0f, 0.0f, 1.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(1.5f, 1.5f, 80.0f);

    currentScene.push_back(LoadNewObject("../../data/crate.obj"));
    currentScene.back().name = "plaquinha3";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/plaquinha.png", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/plaquinha.png");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/plaquinha.png", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/plaquinha.png");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient+blinn-phong_planar(1).glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient+blinn-phong_planar(1).glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "DescricaoGourard";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Rotate_Y(PI)*Matrix_Rotate_X(PI/2);
    currentScene.back().translationMatrix = Matrix_Translate(50.0f, -50.0f, -30.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(0.075f, 0.005f, 0.075f);
    currentScene.back().childrenNames.push_back("pedestal3");

    currentScene.push_back(LoadNewObject("../../data/crate.obj"));
    currentScene.back().name = "pedestal3";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/Liberty-GreenBronze-1.bmp", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/Liberty-GreenBronze-1.bmp");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/Liberty-GreenBronze-1.bmp", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/Liberty-GreenBronze-1.bmp");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient_spherical.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient_spherical.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::NONE;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Identity();
    currentScene.back().translationMatrix = Matrix_Translate(0.0f, 0.0f, 1.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(1.5f, 1.5f, 80.0f);

    currentScene.push_back(LoadNewObject("../../data/bunny.obj"));
    currentScene.back().name = "bunny";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/blue.png", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/blue.png");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/blue.png", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/blue.png");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient+blinn-phong_planar_XZ.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient+blinn-phong_planar_XZ.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "VidroDummyOnClick";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "VidroDummyOnMouseOver";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Identity();
    currentScene.back().translationMatrix = Matrix_Translate(-6.0f, 0.0f, -4.0f);
    currentScene.back().scaleMatrix = Matrix_Identity();

    currentScene.push_back(LoadNewObject("../../data/crate.obj"));
    currentScene.back().name = "plaquinha4";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/plaquinha.png", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/plaquinha.png");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/plaquinha.png", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/plaquinha.png");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient+blinn-phong_planar(1).glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient+blinn-phong_planar(1).glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "DescricaoBunny";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Rotate_Y(PI)*Matrix_Rotate_X(PI/2);
    currentScene.back().translationMatrix = Matrix_Translate(-75.0f, -50.0f, -25.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(0.075f, 0.005f, 0.075f);
    currentScene.back().childrenNames.push_back("pedestal4");

    currentScene.push_back(LoadNewObject("../../data/crate.obj"));
    currentScene.back().name = "pedestal4";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/Liberty-GreenBronze-1.bmp", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/Liberty-GreenBronze-1.bmp");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/Liberty-GreenBronze-1.bmp", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/Liberty-GreenBronze-1.bmp");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient_spherical.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient_spherical.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::NONE;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Identity();
    currentScene.back().translationMatrix = Matrix_Translate(0.0f, 0.0f, 1.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(1.5f, 1.5f, 80.0f);

    currentScene.push_back(LoadNewObject("../../data/chest.obj"));
    currentScene.back().name = "chest";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/wood.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/wood.jpg");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/wood.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/wood.jpg");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient_spherical_no_secondary.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient_spherical_no_secondary.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "AbreBau";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "DescricaoChest";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = "AnimacaoBau";
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::NONE;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Rotate_Y(PI/2);
    currentScene.back().translationMatrix = Matrix_Translate(30.0f, -4.0f, -25.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(0.2f, 0.2f, 0.2f);
    currentScene.back().childrenNames.push_back("chestLid");

    currentScene.push_back(LoadNewObject("../../data/chestLid.obj"));
    currentScene.back().name = "chestLid";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/wood.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/wood.jpg");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/wood.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/wood.jpg");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient_spherical_no_secondary.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient_spherical_no_secondary.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Identity();
    currentScene.back().translationMatrix = Matrix_Translate(1.1f, 1.0f, 0.0f);
    currentScene.back().scaleMatrix = Matrix_Identity();

    currentScene.push_back(LoadNewObject("../../data/chave2.obj"));
    currentScene.back().name = "chave2";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/metal.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/metal.jpg");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/metal.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/metal.jpg");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient+blinn-phong_planar_no_secondary.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient+blinn-phong_planar_no_secondary.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "ChaveOnMouseOver";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = "";
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Identity();
    currentScene.back().translationMatrix = Matrix_Translate(120.0f, -17.0f, -100.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(0.05f, 0.05f, 0.05f);

    currentScene.push_back(LoadNewObject("../../data/crate.obj"));
    currentScene.back().name = "plaquinha5";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/plaquinha.png", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/plaquinha.png");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/plaquinha.png", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/plaquinha.png");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient+blinn-phong_planar(1).glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient+blinn-phong_planar(1).glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "DescricaoBau";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Rotate_Y(PI)*Matrix_Rotate_X(PI/2);
    currentScene.back().translationMatrix = Matrix_Translate(75.0f, -50.0f, -50.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(0.075f, 0.005f, 0.075f);
    currentScene.back().childrenNames.push_back("pedestal5");

    currentScene.push_back(LoadNewObject("../../data/crate.obj"));
    currentScene.back().name = "pedestal5";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/Liberty-GreenBronze-1.bmp", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/Liberty-GreenBronze-1.bmp");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/Liberty-GreenBronze-1.bmp", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/Liberty-GreenBronze-1.bmp");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient_spherical.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient_spherical.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::NONE;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Identity();
    currentScene.back().translationMatrix = Matrix_Translate(0.0f, 0.0f, 1.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(1.5f, 1.5f, 80.0f);

    currentScene.push_back(LoadNewObject("../../data/crate.obj"));
    currentScene.back().name = "starryNight";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/starry_night.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/starry_night.jpg");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/starry_nightDark.jpg", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/starry_nightDark.jpg");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient+blinn-phong_planar(1).glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient+blinn-phong_planar(1).glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "DescricaoStarryNight";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Rotate_Y(PI);
    currentScene.back().translationMatrix = Matrix_Translate(-4.0f, 0.0f, 245.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(1.41f,0.8f, 0.01f);

    currentScene.push_back(LoadNewObject("../../data/chair.obj"));
    currentScene.back().name = "cadeira";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/chairtexture.png", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/chairtexture.png");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/chairtexture.png", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/chairtexture.png");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient_spherical_no_secondary.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient_spherical_no_secondary.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Rotate_Y(PI/2);
    currentScene.back().translationMatrix = Matrix_Translate(-1.2f, -0.6f, 12.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(0.8f, 0.8f, 0.8f);



    currentScene.push_back(LoadNewObject("../../data/crate.obj"));
    currentScene.back().name = "vidroDummy";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/vidro.png", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/vidro.png");
    currentScene.back().textureIds.push_back(GetTextureId("../../data/vidroDark.png", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/vidroDark.png");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment_lambert+ambient+blinn-phong_planar(1).glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment_lambert+ambient+blinn-phong_planar(1).glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "VidroDummyOnClick";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "VidroDummyOnMouseOver";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().updateName = std::string("");
    currentScene.back().update = FunctionMapping(currentScene.back().updateName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Identity();
    currentScene.back().translationMatrix = Matrix_Identity();
    currentScene.back().scaleMatrix = Matrix_Scale(1.0f,1.0f, 1.0f);

    for(unsigned int i = 0; i<currentScene.size(); i++){
        if(currentScene[i].childrenNames.size() > 0){
            currentScene[i].childrenIndices = GetChildrenList(currentScene[i].childrenNames, currentScene, i);
        }
    }
}
