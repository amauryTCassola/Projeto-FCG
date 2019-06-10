#include "SceneLoadSaveUtils.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace ns{
    struct SceneObjectOnDisc{
        std::string name;
        std::string objFilename;        //o endereço relativo do .obj correspondente a este objeto
        std::vector<std::string> textureFilenames;    //o endereço relativo da textura correspondente a este objeto


        std::vector<float>  translationMatrix;
        std::vector<float>  rotationMatrix;
        std::vector<float>  scaleMatrix;


        std::string fragmentShaderFilename;
        std::string vertexShaderFilename;
        int textureWrapMode;
        std::vector<float>  velocity;                   //vetor velocidade do objeto, representa seu movimento
        std::vector<float>  blockMovement;              //vetor representando em quais eixos o objeto pode se mover. Por exemplo, se essa variável tiver o valor (1, 0, 1) significa que o objeto pode se mover nos eixos X e Z, mas não no Y
        float               decelerationRate;           //velocidade de desaceleração do objeto, simula atrito, resistência do ar, etc (precisa ser um valor entre 0 e 1)
        std::string         onMouseOverName;            //nome da função, para ser guardado no disco, é utilizado na função de mapeamento de funções
        std::string         onClickName;
        std::string         onCollisionName;
        std::string         onMoveName;
        int                 thisCollisionType;          //o tipo de colisão que deve ser implementada quando uma colisão for detectada (pode ser elástica, inelástica ou imóvel (WALL))
        int                 thisColliderType;           //tipo (forma) do colisor deste objeto
        bool                active;                     //se este objeto deve ser considerado na cena, etc
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
                {"onMoveName", obj.onMoveName}};
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
        obj.thisCollisionType = j.at("thisCollisionType").get<int>();
        obj.thisColliderType = j.at("thisColliderType").get<int>();
        obj.active = j.at("active").get<bool>();
        obj.onMoveName = j.at("onMoveName");
        obj.translationMatrix = j.at("translationMatrix").get<std::vector<float>>();
        obj.rotationMatrix = j.at("rotationMatrix").get<std::vector<float>>();
        obj.scaleMatrix = j.at("scaleMatrix").get<std::vector<float>>();
    }
}

std::map<std::string, GLuint> texturesMap;
std::map<std::pair<GLuint, GLuint>, GLuint> GPUProgramsMap;
std::map<std::string, GLuint> fragmentShadersMap;
std::map<std::string, GLuint> vertexShadersMap;
std::map<std::string, ModelInformation> modelsMap;

#include "Scene0Functions.h"
std::function<void(std::vector<SceneObject>&, int callerIndex)> FunctionMapping(std::string functionName){

    if(functionName.compare("SphereOnClick") == 0) return SphereOnClick;
    if(functionName.compare("SphereOnMouseOver") == 0) return SphereOnMouseOver;
    if(functionName.compare("SphereOnMove") == 0) return SphereOnMove;
    if(functionName.compare("SphereChildOnMove") == 0) return SphereChildOnMove;
    if(functionName.compare("RabbitOnClick") == 0) return RabbitOnClick;

    return NULL;
}

std::function<void(std::vector<SceneObject>&, int, int)> CollisionFunctionMapping(std::string functionName){
    return NULL;
}

bool KeyExistsInMap(std::map<std::string, GLuint> thisMap, std::string key){
    if(thisMap.find(key) != thisMap.end()) return true;
    else return false;
}

// Função que computa as normais de um ObjModel, caso elas não tenham sido
// especificadas dentro do arquivo ".obj"
void ComputeNormals(ObjModel* model)
{
    if ( !model->attrib.normals.empty() )
        return;

    // Primeiro computamos as normais para todos os TRIÂNGULOS.
    // Segundo, computamos as normais dos VÉRTICES através do método proposto
    // por Gouraud, onde a normal de cada vértice vai ser a média das normais de
    // todas as faces que compartilham este vértice.

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

            // PREENCHA AQUI o cálculo da normal de um triângulo cujos vértices
            // estão nos pontos "a", "b", e "c", definidos no sentido anti-horário.
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

    newModelInformation.first_index    = (void*)first_index; // Primeiro índice
    newModelInformation.num_indices    = last_index - first_index + 1; // Número de indices
    newModelInformation.rendering_mode = GL_TRIANGLES;       // Índices correspondem ao tipo de rasterização GL_TRIANGLES.
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


    //AQUI COMEÇAMOS A CRIAR E POPULAR OS VBOs DE FATO

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

    // "Ligamos" o buffer. Note que o tipo agora é GL_ELEMENT_ARRAY_BUFFER.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_vbo_indices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, conteudo_vbo_indices.size() * sizeof(GLuint), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, conteudo_vbo_indices.size() * sizeof(GLuint), conteudo_vbo_indices.data());
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // XXX Errado!
    //
    buffers.push_back(id_vbo_indices);
    // "Desligamos" o VAO, evitando assim que operações posteriores venham a
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

    //gera o binário do programa de GPU e sobe ele para a memória
    glLinkProgram(shaderprogram_id);

    return shaderprogram_id;
}

GLuint CreateNewTexture(std::string textureFileName, WrapMode wrapMode, std::vector<GLfloat> borderColor){


    stbi_set_flip_vertically_on_load(true);
    int width;
    int height;
    int channels;
    unsigned char *data = stbi_load(textureFileName.c_str(), &width, &height, &channels, 3);

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

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR); //filtering, estamos usando uma técnica mista de
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_LINEAR); //mipmap, nearest e linear interpolation


    //AQUI NÓS CARREGAMOS A IMAGEM PARA O OBJETO DE TEXTURA
    //void glTexImage2D(	GLenum target,  -> O TIPO DE TEXTURA
 	//GLint level,                          -> NÍVEL DE DETALHE (0 é a imagem base, >0 é para mipmaps)
 	//GLint internalformat,                 -> especifica número/tamanho dos componentes de cor DA TEXTURA(por exemplo, GL_RGBA)
 	//GLsizei width,                        -> largura da imagem
 	//GLsizei height,                       -> altura da imagem
 	//GLint border,                         -> este valor deve ser sempre 0 (????)
 	//GLenum format,                        -> especifica o formato dos pixels DA IMAGEM
 	//GLenum type,                          -> tipo dos dados que estão no array data
 	//const GLvoid * data);                 -> ponteiro para um array de pixels em memória (a imagem)

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    //geramos os mipmaps
    glGenerateMipmap(GL_TEXTURE_2D);



    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);

    return newTexture_id;
}

GLuint GetTextureId(std::string textureFileName, WrapMode wrapMode, std::vector<GLfloat> borderColor = black){
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

        aux = LoadNewObject(objsList[i].objFilename); //gera o objeto e popula com as informações do .obj

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

        aux.onCollisionName = objsList[i].onCollisionName;
        aux.onCollision     = CollisionFunctionMapping(aux.onCollisionName);

        aux.thisColliderType = objsList[i].thisColliderType;
        aux.thisCollisionType = objsList[i].thisCollisionType;

        aux.active = objsList[i].active;

        aux.name = objsList[i].name;

        aux.onMoveName = objsList[i].onMoveName;
        aux.onMove = FunctionMapping(aux.onMoveName);

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
    for(unsigned int i = 0; i<currentScene.size(); i++){
        glDeleteShader(currentScene[i].vertexShaderId);
        glDeleteShader(currentScene[i].fragmentShaderId);
        glDeleteProgram(currentScene[i].gpuProgramId);
        glDeleteBuffers(currentScene[i].buffers.size(), &currentScene[i].buffers[0]);
        glDeleteTextures(currentScene[i].textureIds.size(), &currentScene[i].textureIds[0]);
        glDeleteVertexArrays(1, &currentScene[i].vertex_array_object_id);
    }
    currentScene.clear();
}

void Debug_NewObjectSphere(std::vector<SceneObject>& currentScene){
    //CRIAÇÃO DE UMA NOVA CENA

    currentScene.push_back(LoadNewObject("../../data/sphere.obj"));
    currentScene.back().name = "Esfera1";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/Liberty-GreenBronze-1.bmp", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureIds.push_back(GetTextureId("../../data/Liberty-Pavimentazione-1.bmp", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/Liberty-GreenBronze-1.bmp");
    currentScene.back().textureFilenames.push_back("../../data/Liberty-Pavimentazione-1.bmp");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = std::string("SphereOnClick");
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = std::string("SphereOnMouseOver");
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = std::string("SphereOnMove");
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::SPHERE;
    currentScene.back().thisCollisionType = (int)CollisionType::ELASTIC;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Identity();
    currentScene.back().translationMatrix = Matrix_Translate(0.0f, -0.5f, 0.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(0.5f,0.5f,0.5f);
    //currentScene.back().childrenNames = std::vector<std::string>{"Esfera2"};

    currentScene.push_back(LoadNewObject("../../data/bunny.obj"));
    currentScene.back().name = "Bunny2";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/Liberty-GreenBronze-1.bmp", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureIds.push_back(GetTextureId("../../data/Liberty-Pavimentazione-1.bmp", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/Liberty-GreenBronze-1.bmp");
    currentScene.back().textureFilenames.push_back("../../data/Liberty-Pavimentazione-1.bmp");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
    currentScene.back().decelerationRate = 0.3f;
    currentScene.back().onClickName = "RabbitOnClick";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::OBB;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Identity();
    currentScene.back().translationMatrix = Matrix_Translate(0.0f, 0.0f, 5.0f);
    currentScene.back().scaleMatrix = Matrix_Identity();

    currentScene.push_back(LoadNewObject("../../data/sphere.obj"));
    currentScene.back().name = "Esfera2";
    currentScene.back().textureIds.push_back(GetTextureId("../../data/Liberty-GreenBronze-1.bmp", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureIds.push_back(GetTextureId("../../data/Liberty-Pavimentazione-1.bmp", WrapMode::MIRRORED_REPEAT));
    currentScene.back().textureFilenames.push_back("../../data/Liberty-GreenBronze-1.bmp");
    currentScene.back().textureFilenames.push_back("../../data/Liberty-Pavimentazione-1.bmp");
    currentScene.back().textureWrapMode = (int)WrapMode::MIRRORED_REPEAT;
    currentScene.back().vertexShaderId = GetVertexShaderId("../../src/shader_vertex.glsl");
    currentScene.back().fragmentShaderId = GetFragmentShaderId("../../src/shader_fragment.glsl");
    currentScene.back().gpuProgramId = GetGPUProgramId(currentScene.back().vertexShaderId, currentScene.back().fragmentShaderId);
    currentScene.back().fragmentShaderFilename = std::string("../../src/shader_fragment.glsl");
    currentScene.back().vertexShaderFilename = std::string("../../src/shader_vertex.glsl");
    currentScene.back().active = true;
    currentScene.back().blockMovement = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
    currentScene.back().decelerationRate = 0.0f;
    currentScene.back().onClickName = "";
    currentScene.back().onClick = FunctionMapping(currentScene.back().onClickName);
    currentScene.back().onMouseOverName = "";
    currentScene.back().onMouseOver = FunctionMapping(currentScene.back().onMouseOverName);
    currentScene.back().onMoveName = "SphereChildOnMove";
    currentScene.back().onMove = FunctionMapping(currentScene.back().onMoveName);
    currentScene.back().velocity = glm::vec4(0, 0, 0, 0);
    currentScene.back().thisColliderType = (int)ColliderType::NONE;
    currentScene.back().thisCollisionType = (int)CollisionType::WALL;
    currentScene.back().model = Matrix_Identity();
    currentScene.back().rotationMatrix = Matrix_Identity();
    currentScene.back().translationMatrix = Matrix_Translate(2.0f, 0.0f, 0.0f);
    currentScene.back().scaleMatrix = Matrix_Scale(0.5f, 0.5f, 0.5f);



    for(unsigned int i = 0; i<currentScene.size(); i++){
        if(currentScene[i].childrenNames.size() > 0){
            currentScene[i].childrenIndices = GetChildrenList(currentScene[i].childrenNames, currentScene, i);
        }
    }
}
