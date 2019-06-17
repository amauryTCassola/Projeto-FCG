#version 330 core

// Atributos de vértice recebidos como entrada ("in") pelo Vertex Shader.
// Veja a função BuildTriangle() em "main.cpp".
layout (location = 0) in vec4 model_coefficients;
layout (location = 1) in vec4 normal_coefficients;
layout (location = 2) in vec2 texcoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Atributos de vértice que serão gerados como saída ("out") pelo Vertex Shader.
// ** Estes serão interpolados pelo rasterizador! ** gerando, assim, valores
// para cada fragmento, os quais serão recebidos como entrada pelo Fragment
// Shader. Veja o arquivo "shader_fragment.glsl".
//out vec4 cor_interpolada_pelo_rasterizador;
out vec2 Texcoord;
out vec4 position_world;
out vec4 position_model;
out vec4 normal;

void main()
{

    gl_Position = projection * view * model * model_coefficients;
    //aplicamos as transformações definidas na matriz model,
    //passamos para o espaço da câmera e, então,
    //projetamos
    //o atual vértice


    // Posição do vértice atual no sistema de coordenadas global (World).
    position_world = model * model_coefficients;

    // Posição do vértice atual no sistema de coordenadas local do modelo.
    position_model = model_coefficients;

    // Normal do vértice atual no sistema de coordenadas global (World).
    // Veja slide 107 do documento "Aula_07_Transformacoes_Geometricas_3D.pdf".
    normal = (inverse(transpose(model)) * normal_coefficients);
    normal.w = 0.0;

    Texcoord = texcoord;
}
