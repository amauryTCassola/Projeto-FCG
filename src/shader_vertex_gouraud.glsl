#version 330 core
#define M_PI   3.14159265358979323846
#define M_PI_2 1.57079632679489661923
// Atributos de v�rtice recebidos como entrada ("in") pelo Vertex Shader.
// Veja a fun��o BuildTriangle() em "main.cpp".
layout (location = 0) in vec4 model_coefficients;
layout (location = 1) in vec4 normal_coefficients;
layout (location = 2) in vec2 texcoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

// Par�metros da axis-aligned bounding box (AABB) do modelo
uniform vec4 bbox_min;
uniform vec4 bbox_max;

uniform sampler2D tex;
uniform sampler2D secondaryTex;

uniform vec4 lightPos;
uniform vec4 lightDir;
uniform vec4 lightColor;

out vec2 Texcoord;
out vec4 position_world;
out vec4 position_model;
out vec4 normal;
out vec4 cor_interpolada_pelo_rasterizador;

uniform bool isFlashlight;

const float cos_inner_cone_angle = 1.0; // 5 graus
const float cos_outer_cone_angle = 0.94; // 20 graus

void main()
{

    gl_Position = projection * view * model * model_coefficients;
    //aplicamos as transforma��es definidas na matriz model,
    //passamos para o espa�o da c�mera e, ent�o,
    //projetamos
    //o atual v�rtice


    // Posi��o do v�rtice atual no sistema de coordenadas global (World).
    position_world = model * model_coefficients;

    // Posi��o do v�rtice atual no sistema de coordenadas local do modelo.
    position_model = model_coefficients;

    // Normal do v�rtice atual no sistema de coordenadas global (World).
    // Veja slide 107 do documento "Aula_07_Transformacoes_Geometricas_3D.pdf".
    normal = (inverse(transpose(model)) * normal_coefficients);
    normal.w = 0.0;

    Texcoord = texcoord;


    // Obtemos a posi��o da c�mera utilizando a inversa da matriz que define o
    // sistema de coordenadas da c�mera.
    vec4 origin = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 camera_position = inverse(view) * origin;

    vec4 light_position = lightPos;
    vec3 light_specter = lightColor.rgb; //RGB
    vec3 ambient_light_specter = lightColor.rgb;

    // O fragmento atual � coberto por um ponto que pertence � superf�cie de um
    // dos objetos virtuais da cena. Este ponto, p, possui uma posi��o no
    // sistema de coordenadas global (World coordinates). Esta posi��o � obtida
    // atrav�s da interpola��o, feita pelo rasterizador, da posi��o de cada
    // v�rtice.
    vec4 vertex_position = model * model_coefficients;

    // Normal do fragmento atual, interpolada pelo rasterizador a partir das
    // normais de cada v�rtice.

    vec4 vertex_normal = (inverse(transpose(model)) * normal_coefficients);
    vertex_normal.w = 0.0;
    vertex_normal = normalize(vertex_normal);




    // Vetor que define o sentido da fonte de luz em rela��o ao ponto atual.
    vec4 vertex_to_light = -lightDir; //normalize(fragment_position_world - light_position);

    // Vetor que define o sentido da c�mera em rela��o ao ponto atual.
    vec4 vertex_to_camera = normalize(camera_position - vertex_position);


    //coordenadas de textura
    float U = 0.0;
    float V = 0.0;

    //=========================================
    vec4 bbox_center = (bbox_min + bbox_max) / 2.0;
    vec4 c = bbox_center;
    float ro = 1;

    vec4 plinha = c + ro*(model_coefficients - c)/length(model_coefficients-c);
    vec4 pvetor = plinha - c;

    float theta = atan(pvetor.x, pvetor.z);
    float phi = asin(pvetor.y);


    U = (theta + M_PI)/(2*M_PI);
    V = (phi + M_PI/2)/M_PI;
    //==========================================

    vec3 Kd0 = texture(tex, vec2(U,V)).rgb; //reflet�ncia difusa lida da textura
	vec3 Kd1 = texture(secondaryTex, vec2(U,V)).rgb; //reflet�ncia difusa lida da textura
    vec3 Ka = Kd0/2;

    float lambert = max(0, dot(vertex_normal, vertex_to_light));
    vec3 ambient_term = Ka*ambient_light_specter;
	
	float opening = radians(20); //radians
	
	vec3 corRGB;
	
	if(!isFlashlight){
		corRGB = Kd0*light_specter*lambert + ambient_term;
	} else{
		ambient_term = ambient_term*0.1;
		float cosBeta = dot( normalize(vertex_position-light_position), normalize(lightDir) );
		float cosAlpha = cos(opening);
		if(cosBeta >= cosAlpha){
			float cos_inner_minus_outer_angle = cos_inner_cone_angle - cos_outer_cone_angle;
			float spot = 0.0;
			spot = clamp((cosBeta - cos_outer_cone_angle)/cos_inner_minus_outer_angle, 0.0, 1.0);
			corRGB = ((Kd0*(1 -lambert) + Kd1*(lambert))*light_specter)*spot + ambient_term*(1 - spot);
		}
		else
			corRGB = ambient_term;
	
	}
	
	cor_interpolada_pelo_rasterizador = vec4(corRGB, texture(tex, vec2(U,V)).a);
}
