#version 330 core

// Constantes
#define M_PI   3.14159265358979323846
#define M_PI_2 1.57079632679489661923

// Atributos de fragmentos recebidos como entrada ("in") pelo Fragment Shader.
// Neste exemplo, este atributo foi gerado pelo rasterizador como a
// interpola��o da cor de cada v�rtice, definidas em "shader_vertex.glsl" e
// "main.cpp" (array color_coefficients).
in vec4 cor_interpolada_pelo_rasterizador;
in vec2 Texcoord;
in vec4 position_world;
in vec4 position_model;
in vec4 normal;
in vec4 FragPosLightSpace;

//Textura computada no c�digo C++
uniform sampler2D tex;
uniform sampler2D secondaryTex;

// Matrizes computadas no c�digo C++ e enviadas para a GPU
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Par�metros da axis-aligned bounding box (AABB) do modelo
uniform vec4 bbox_min;
uniform vec4 bbox_max;

uniform vec4 lightPos;
uniform vec4 lightDir;
uniform vec4 lightColor;

// O valor de sa�da ("out") de um Fragment Shader � a cor final do fragmento.
layout(location = 0) out vec4 color;

void main()
{

    color = cor_interpolada_pelo_rasterizador;
}
