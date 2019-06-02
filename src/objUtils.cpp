#include "objUtils.h"

namespace ns{
    struct SceneObjectOnDisc{
        std::string objFilename;        //o endereço relativo do .obj correspondente a este objeto
        std::string textureFilename;    //o endereço relativo da textura correspondente a este objeto
        std::vector<float>    model;             //a matriz model inicial deste objeto (em forma de vetor)
        std::string fragmentShaderFilename;
        std::string vertexShaderFilename;
        int textureWrapMode;
        std::vector<float>  velocity;                   //vetor velocidade do objeto, representa seu movimento
        std::vector<float>  blockMovement;              //vetor representando em quais eixos o objeto pode se mover. Por exemplo, se essa variável tiver o valor (1, 0, 1) significa que o objeto pode se mover nos eixos X e Z, mas não no Y
        float               decelerationRate;           //velocidade de desaceleração do objeto, simula atrito, resistência do ar, etc (precisa ser um valor entre 0 e 1)
        std::string         onMouseOverName;            //nome da função, para ser guardado no disco, é utilizado na função de mapeamento de funções
        std::string         onClickName;
        int                 thisCollisionType;          //o tipo de colisão que deve ser implementada quando uma colisão for detectada (pode ser elástica, inelástica ou imóvel (WALL))
        int                 thisColliderType;           //tipo (forma) do colisor deste objeto
        bool                active;                     //se este objeto deve ser considerado na cena, etc
    };

    void to_json(json& j, const SceneObjectOnDisc& obj) {
        j = json{{"objFilename", obj.objFilename},
                {"textureFilename", obj.textureFilename},
                {"model", obj.model},
                {"fragmentShaderFilename", obj.fragmentShaderFilename},
                {"vertexShaderFilename", obj.vertexShaderFilename},
                {"textureWrapMode", obj.textureWrapMode},
                {"velocity", obj.velocity},
                {"blockMovement", obj.blockMovement},
                {"decelerationRate", obj.decelerationRate},
                {"onMouseOverName", obj.onMouseOverName},
                {"onClickName", obj.onClickName},
                {"thisCollisionType", obj.thisCollisionType},
                {"thisColliderType", obj.thisColliderType},
                {"active", obj.active},};
    }

    void from_json(const json& j, SceneObjectOnDisc& obj) {
        obj.objFilename = j.at("objFilename");
        obj.textureFilename = j.at("textureFilename");
        obj.model = j.at("model").get<std::vector<float>>();
        obj.fragmentShaderFilename = j.at("fragmentShaderFilename");
        obj.vertexShaderFilename = j.at("vertexShaderFilename");
        obj.textureWrapMode = j.at("textureWrapMode").get<int>();

    }
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



std::vector<SceneObject> ObjectLoad(const char* filename){

    std::vector<SceneObject> newObjList;

    ObjModel newModel(filename);
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

    for(size_t currentShape = 0; currentShape < newModel.shapes.size(); currentShape++){ //itera sobre cada shape que está nesse .obj
        size_t first_index = conteudo_vbo_indices.size();
        size_t num_triangles = newModel.shapes[currentShape].mesh.num_face_vertices.size();

        const float minval = std::numeric_limits<float>::min();
        const float maxval = std::numeric_limits<float>::max();

        glm::vec4 bbox_min = glm::vec4(maxval,maxval,maxval, 1.0f);
        glm::vec4 bbox_max = glm::vec4(minval,minval,minval, 1.0f);

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
           // assert(newModel.shapes[shape].mesh.num_face_vertices[triangle] == 3);

            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = newModel.shapes[currentShape].mesh.indices[3*triangle + vertex];

                conteudo_vbo_indices.push_back(first_index + 3*triangle + vertex);

                const float vx = newModel.attrib.vertices[3*idx.vertex_index + 0];
                const float vy = newModel.attrib.vertices[3*idx.vertex_index + 1];
                const float vz = newModel.attrib.vertices[3*idx.vertex_index + 2];
                //printf("tri %d vert %d = (%.2f, %.2f, %.2f)\n", (int)triangle, (int)vertex, vx, vy, vz);
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

        SceneObject theobject;
        theobject.name           = newModel.shapes[currentShape].name.c_str();
        theobject.first_index    = (void*)first_index; // Primeiro índice
        theobject.num_indices    = last_index - first_index + 1; // Número de indices
        theobject.rendering_mode = GL_TRIANGLES;       // Índices correspondem ao tipo de rasterização GL_TRIANGLES.
        theobject.vertex_array_object_id = new_vao_id;
        theobject.model = Matrix_Identity();
        theobject.bbox_min = bbox_min;
        theobject.bbox_max = bbox_max;
        theobject.bbox_max.w = 1.0f;
        theobject.bbox_min.w = 1.0f;


        theobject.objFilename = filename;

        newObjList.push_back(theobject);
    }


    //AQUI COMEÇAMOS A CRIAR E POPULAR OS VBOs DE FATO

    glGenBuffers(1, &id_vbo_positions);
    glBindBuffer(GL_ARRAY_BUFFER, id_vbo_positions);
        glBufferData(GL_ARRAY_BUFFER, conteudo_vbo_positions.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, conteudo_vbo_positions.size() * sizeof(float), conteudo_vbo_positions.data());
        GLuint location = 0; // "(location = 0)" em "shader_vertex.glsl"
        GLint  number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

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
    }

    glGenBuffers(1, &id_vbo_indices);

    // "Ligamos" o buffer. Note que o tipo agora é GL_ELEMENT_ARRAY_BUFFER.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_vbo_indices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, conteudo_vbo_indices.size() * sizeof(GLuint), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, conteudo_vbo_indices.size() * sizeof(GLuint), conteudo_vbo_indices.data());
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // XXX Errado!
    //

    // "Desligamos" o VAO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);

    return newObjList;
}


void DrawVirtualObject(SceneObject objToDraw)
{
    GLuint program_id = objToDraw.gpuProgramId;

    glUseProgram(program_id);

    UpdateCamera(program_id, 1.0f);

    GLint model_matrix_index = glGetUniformLocation(program_id, "model");
    glUniformMatrix4fv(model_matrix_index, 1 , GL_FALSE , glm::value_ptr(objToDraw.model));
    //a primeira coisa é passar a matriz de modelagem do objeto pra GPU

    GLint bbox_min_uniform = glGetUniformLocation(program_id, "bbox_min");
    GLint bbox_max_uniform = glGetUniformLocation(program_id, "bbox_max");
    glUniform4f(bbox_min_uniform, objToDraw.bbox_min.x, objToDraw.bbox_min.y, objToDraw.bbox_min.z, 1.0f);
    glUniform4f(bbox_max_uniform, objToDraw.bbox_max.x, objToDraw.bbox_max.y, objToDraw.bbox_max.z, 1.0f);


    // "Ligamos" o VAO.
    glBindVertexArray(objToDraw.vertex_array_object_id);

    // "Ligamos" a textura vinculada a este objeto, se tiver
    //if(objToDraw.texture_id != -1)
        glBindTexture(GL_TEXTURE_2D, objToDraw.texture_id);

    glDrawElements(
        objToDraw.rendering_mode,
        objToDraw.num_indices,
        GL_UNSIGNED_INT,
        (void*)objToDraw.first_index
    );

    // "Desligamos" o VAO
    glBindVertexArray(0);

    //"Desligamos" a textura
    glBindTexture(GL_TEXTURE_2D, 0);
}

void MoveObject(glm::vec4 movementVector, SceneObject* objToBeMoved){
    objToBeMoved->model *= Matrix_Translate(movementVector.x, movementVector.y, movementVector.z);
}

GLuint CreateGPUProgram(const char* vertex_shader_filename, const char* fragment_shader_filename){
    GLuint vertex_shader_id = LoadShader_Vertex(vertex_shader_filename); //carrega o vertex shader de um arquivo e já cria o objeto
    GLuint fragment_shader_id = LoadShader_Fragment(fragment_shader_filename);//mesma coisa

    GLuint shaderprogram_id = glCreateProgram(); //cria um novo programa de GPU

    /* Attach our shaders to our program */
    glAttachShader(shaderprogram_id, vertex_shader_id);
    glAttachShader(shaderprogram_id, fragment_shader_id);

    //gera o binário do programa de GPU e sobe ele para a memória
    glLinkProgram(shaderprogram_id);

    return shaderprogram_id;
}

GLuint CreateNewTexture(const char* textureFileName, WrapMode wrapMode, std::vector<GLfloat> borderColor){

    Image newTexture_img = LoadImageFromDisc(textureFileName);

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

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, newTexture_img.width, newTexture_img.height, 0, GL_RGB, GL_UNSIGNED_BYTE, newTexture_img.data);

    //geramos os mipmaps
    glGenerateMipmap(GL_TEXTURE_2D);

    DestroyImg(newTexture_img);

    return newTexture_id;
}


std::function FunctionMapping(const char* functionName){
    return NULL;
}





void WriteSceneToFile(std::vector<ns::SceneObjectOnDisc> objsList, const char* newSceneFilename){

    json codificado;
    for(unsigned int i = 0; i < objsList.size(); i++){
        codificado["Scene"][i] = objsList[i];
    }

    codificado["Variables"] = currentSceneBoolVariables;

    std::ofstream o(newSceneFilename);
    o << std::setw(4) << codificado;

}

void SaveScene(std::vector<SceneObject> scene, const char* filename){
    std::vector<ns::SceneObjectOnDisc> objsListToWrite;

    for(unsigned int i = 0; i<scene.size(); i++){
        ns::SceneObjectOnDisc curObj;

        for(unsigned int j = 0; j < 16; j++){
            curObj.model.push_back(((float*)glm::value_ptr(scene[i].model))[j]);
        }

        std::vector<float> blockMovementAux {scene[i].blockMovement.x, scene[i].blockMovement.y, scene[i].blockMovement.z};
        std::vector<float> velocityAux {scene[i].velocity.x, scene[i].velocity.y, scene[i].velocity.z, scene[i].velocity.w};

        curObj.blockMovement =          blockMovementAux;
        curObj.velocity =               velocityAux;

        curObj.active =                 scene[i].active;
        curObj.decelerationRate =       scene[i].decelerationRate;
        curObj.textureWrapMode =        scene[i].textureWrapMode;
        curObj.thisColliderType =       scene[i].thisColliderType;
        curObj.thisCollisionType =      scene[i].thisCollisionType;

        curObj.objFilename =            std::string(scene[i].objFilename);
        curObj.textureFilename =        std::string(scene[i].textureFilename);
        curObj.fragmentShaderFilename = std::string(scene[i].fragmentShaderFilename);
        curObj.vertexShaderFilename =   std::string(scene[i].vertexShaderFilename);
        curObj.onClickName =            std::string(scene[i].onClickName);
        curObj.onMouseOverName =        std::string(scene[i].onMouseOverName);

        printf("salvou %s com textura %s\n", curObj.objFilename.c_str(), curObj.textureFilename.c_str());

        objsListToWrite.push_back(curObj);
    }

    WriteSceneToFile(objsListToWrite, filename);

}

std::vector<ns::SceneObjectOnDisc> ReadSceneFromFile(const char* sceneFilename){

    // read a JSON file
    std::ifstream i(sceneFilename);
    json j;
    i >> j;

    return j["Scene"].get<std::vector<ns::SceneObjectOnDisc>>();

}

std::vector<bool> ReadSceneVariablesFromFile(const char* sceneFilename){

    // read a JSON file
    std::ifstream i(sceneFilename);
    json j;
    i >> j;

    return j["Variables"].get<std::vector<bool>>();

}


std::vector<SceneObject> LoadScene(const char* sceneFilename){
    std::vector<SceneObject> loadedScene;
    std::vector<SceneObject> aux;

    std::vector<ns::SceneObjectOnDisc> objsList = ReadSceneFromFile(sceneFilename);

    for(unsigned int i = 0; i<objsList.size(); i++){

        aux = ObjectLoad(objsList[i].objFilename.c_str()); //gera o objeto e popula com as informações do .obj

        for(unsigned int j = 0; j<aux.size(); j++){//se for mais de um objeto, popula todos com as mesmas informações
            aux[j].model = glm::mat4(objsList[i].model[0], objsList[i].model[1], objsList[i].model[2], objsList[i].model[3],
                                              objsList[i].model[4], objsList[i].model[5], objsList[i].model[6], objsList[i].model[7],
                                              objsList[i].model[8], objsList[i].model[9], objsList[i].model[10], objsList[i].model[11],
                                              objsList[i].model[12], objsList[i].model[13], objsList[i].model[14], objsList[i].model[15]);

            aux[j].textureWrapMode = objsList[i].textureWrapMode;
            aux[j].textureFilename = objsList[i].textureFilename.c_str();
            aux[j].texture_id = CreateNewTexture(aux[j].textureFilename, (WrapMode)aux[j].textureWrapMode);
            aux[j].fragmentShaderFilename = objsList[i].fragmentShaderFilename.c_str();
            aux[j].vertexShaderFilename = objsList[i].vertexShaderFilename.c_str();
            aux[j].gpuProgramId = CreateGPUProgram(aux[j].vertexShaderFilename, aux[j].fragmentShaderFilename);

            aux[j].velocity = glm::vec4(objsList[i].velocity[0], objsList[i].velocity[1], objsList[i].velocity[2], objsList[i].velocity[3]);
            aux[j].blockMovement = glm::vec3(objsList[i].blockMovement[0], objsList[i].blockMovement[1], objsList[i].blockMovement[2]);
            aux[j].decelerationRate = objsList[i].decelerationRate;

            aux[j].onMouseOverName  = objsList[i].onMouseOverName.c_str();
            aux[j].onMouseOver      = FunctionMapping(aux[j].onMouseOverName);

            aux[j].onClickName      =  objsList[i].onClickName.c_str();
            aux[j].onClick          =  FunctionMapping(aux[j].onClickName);

            aux[j].thisColliderType = objsList[i].thisColliderType;
            aux[j].thisCollisionType = objsList[i].thisCollisionType;

            aux[j].active = objsList[i].active;
        }

        loadedScene.insert(loadedScene.end(), aux.begin(), aux.end());

    }

    return loadedScene;
}

void OpenScene(const char* filename){
    currentScene = LoadScene(filename);
    currentSceneBoolVariables = ReadSceneVariablesFromFile(filename);
}
