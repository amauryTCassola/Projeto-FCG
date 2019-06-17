#include "LightUtils.h"
#include "shaderUtil.h"

GLuint FBO, depthMap, shadowGPUProgram;
bool isLightInit = false;
const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;

GLuint createGPUProgram_Light(){

    GLuint fragmentS = GetFragmentShaderId("../../src/fragment_shader_shadowmap.glsl");
    GLuint vertexS = GetVertexShaderId("../../src/vertex_shader_shadowmap.glsl");
    GLuint program = GetGPUProgramId(vertexS, fragmentS);

    return program;
}

void UnbindFrameBufferLight(){
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, GetWidth(), GetHeight());

}

void BindFrameBufferLight(GLuint frameBuffer, int width, int height){

    glBindTexture(GL_TEXTURE_2D, 0);//To make sure the texture isn't bound
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glViewport(0, 0, width, height);
}

GLuint createDepthBufferAttachment_Light(int width, int height, GLuint framebuffer) {
        GLuint texture;
        glGenTextures(1, &texture);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
            glBindTexture(GL_TEXTURE_2D, texture);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture, 0);

                glDrawBuffer(GL_NONE);
                glReadBuffer(GL_NONE);

            glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return texture;
    }

GLuint createFrameBuffer_Light() {
        GLuint frameBuffer;
        glGenFramebuffers(1, &frameBuffer);
        return frameBuffer;
}

void InitShadows(){

    FBO = createFrameBuffer_Light();
    depthMap = createDepthBufferAttachment_Light(SHADOW_WIDTH, SHADOW_HEIGHT, FBO);
    shadowGPUProgram = createGPUProgram_Light();

    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        printf("ERRO: FBO INICIALIZADO INCORRETAMENTE");
        exit(-1);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    isLightInit = true;

}

void DrawObjectToShadowMap(SceneObject objToDraw){


    GLuint program_id = shadowGPUProgram;

    glUseProgram(program_id);

    SetCameraToDraw(program_id, (float)GetWidth() / GetHeight());

    GLint model_matrix_index = glGetUniformLocation(program_id, "model");
    glUniformMatrix4fv(model_matrix_index, 1 , GL_FALSE , glm::value_ptr(objToDraw.model));


    // "Ligamos" o VAO.
    glBindVertexArray(objToDraw.vertex_array_object_id);


    glDrawElements(
        objToDraw.rendering_mode,
        objToDraw.num_indices,
        GL_UNSIGNED_INT,
        (void*)objToDraw.first_index
    );

    // "Desligamos" o VAO
    glBindVertexArray(0);
}

void DrawShadows(glm::vec4 lightPosition, glm::vec4 lightDirection, std::vector<SceneObject>& currentScene){
    if(!isLightInit)
        InitShadows();

    glm::vec4 originalCameraPosition = GetCameraPosition();
    glm::vec4 originalCameraViewVector = GetCameraViewVector();
    CameraMode originalCameraMode = GetCameraMode();

    SetCameraOrtho();
    SetCameraPosition(lightPosition);
    SetCameraMode(CameraMode::FREE);
    SetCameraViewVector(lightDirection);

    BindFrameBufferLight(FBO, SHADOW_WIDTH, SHADOW_HEIGHT);
        glClearColor(0.0f, 0.0f, 0.0f, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        for(unsigned int i = 0; i<currentScene.size(); i++)
            if(currentScene[i].active)DrawObjectToShadowMap(currentScene[i]);
    UnbindFrameBufferLight();


    SetCameraPosition(originalCameraPosition);
    SetCameraViewVector(originalCameraViewVector);
    SetCameraMode(originalCameraMode);
    SetCameraPerspective();

}

glm::mat4 GetLightMatrix(glm::vec4 lightPosition, glm::vec4 lightDirection){

    glm::mat4 lightView = Matrix_Camera_View(lightPosition, lightDirection, glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));
    float nearplane = -0.1f;  // Posição do "near plane"
    float farplane  = -20.0f; // Posição do "far plane"
    //float field_of_view = PI / 3.0f;
    //glm::mat4 lightProjection = Matrix_Perspective(field_of_view, (float)GetWidth()/GetHeight(), nearplane, farplane);

    float sRatio = (float)GetWidth()/GetHeight();
    float t = 3.0f*3.5f/2.5f;
    float b = -t;
    float r = t*sRatio;
    float l = -r;
    glm::mat4 lightProjection = Matrix_Orthographic(l, r, b, t, nearplane, farplane);

    return lightProjection * lightView;

}

GLuint GetShadowMap(){
    return depthMap;
}
