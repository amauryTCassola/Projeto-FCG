#version 330 core

// O valor de saída ("out") de um Fragment Shader é a cor final do fragmento.
out vec3 color;


void main()
{
    color = vec3(120,120,120);
    //color = texture(tex, Texcoord);
}
