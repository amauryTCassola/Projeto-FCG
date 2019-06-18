#version 330 core

// Constantes
#define M_PI   3.14159265358979323846
#define M_PI_2 1.57079632679489661923

// Atributos de fragmentos recebidos como entrada ("in") pelo Fragment Shader.
// Neste exemplo, este atributo foi gerado pelo rasterizador como a
// interpola��o da cor de cada v�rtice, definidas em "shader_vertex.glsl" e
// "main.cpp" (array color_coefficients).
//in vec4 cor_interpolada_pelo_rasterizador;
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

uniform bool isFlashlight;

const float cos_inner_cone_angle = 1.0; // 5 graus
const float cos_outer_cone_angle = 0.94; // 20 graus

// O valor de sa�da ("out") de um Fragment Shader � a cor final do fragmento.
layout(location = 0) out vec4 color;

void main()
{
    // Obtemos a posi��o da c�mera utilizando a inversa da matriz que define o
    // sistema de coordenadas da c�mera.
    vec4 origin = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 camera_position = inverse(view) * origin;

    vec4 light_position = lightPos;
    vec4 light_specter = lightColor; //RGB
    vec4 ambient_light_specter = lightColor;

    // O fragmento atual � coberto por um ponto que pertence � superf�cie de um
    // dos objetos virtuais da cena. Este ponto, p, possui uma posi��o no
    // sistema de coordenadas global (World coordinates). Esta posi��o � obtida
    // atrav�s da interpola��o, feita pelo rasterizador, da posi��o de cada
    // v�rtice.
    vec4 fragment_position = position_world;

    // Normal do fragmento atual, interpolada pelo rasterizador a partir das
    // normais de cada v�rtice.
    vec4 fragment_normal = normalize(normal);

    // Vetor que define o sentido da fonte de luz em rela��o ao ponto atual.
    vec4 fragment_to_light = -lightDir; //normalize(fragment_position_world - light_position);

    // Vetor que define o sentido da c�mera em rela��o ao ponto atual.
    vec4 fragment_to_camera = normalize(camera_position - fragment_position);


    //coordenadas de textura
    float U = 0.0;
    float V = 0.0;

    //=========================================
    vec4 bbox_center = (bbox_min + bbox_max) / 2.0;
    vec4 c = bbox_center;
    float ro = 1;

    vec4 plinha = c + ro*(position_model - c)/length(position_model-c);
    vec4 pvetor = plinha - c;

    float theta = atan(pvetor.x, pvetor.z);
    float phi = asin(pvetor.y);


    U = (theta + M_PI)/(2*M_PI);
    V = (phi + M_PI/2)/M_PI;
    //==========================================

    vec3 Kd0 = texture(tex, vec2(U,V)).rgb; //reflet�ncia difusa lida da textura
	vec3 Kd1 = texture(secondaryTex, vec2(U,V)).rgb; //reflet�ncia difusa lida da textura
    vec3 Ka = Kd0/2;

    float lambert = max(0, dot(fragment_normal, fragment_to_light));
    vec3 ambient_term = Ka*ambient_light_specter;
	
	float opening = radians(20); //radians
	
	vec3 corRGB;
	
	if(!isFlashlight){
		corRGB = Kd0*light_specter*lambert + ambient_term;
	} else{
		ambient_term = ambient_term*0.1;
		float cosBeta = dot( normalize(fragment_position-light_position), normalize(lightDir) );
		float cosAlpha = cos(opening);
		if(cosBeta >= cosAlpha){
			float cos_inner_minus_outer_angle = cos_inner_cone_angle - cos_outer_cone_angle;
			float spot = 0.0;
			spot = clamp((cosBeta - cos_outer_cone_angle)/cos_inner_minus_outer_angle, 0.0, 1.0);
			corRGB = ((Kd0*(1-lambert)*light_specter + Kd1*lambert*light_specter))*spot + (1-spot)*ambient_term;
		}
		else
			corRGB = ambient_term;
	}
	color = vec4(corRGB, texture(tex, vec2(U,V)).a);
}
