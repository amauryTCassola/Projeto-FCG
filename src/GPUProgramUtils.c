#include "GPUProgramUtils.h"

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
