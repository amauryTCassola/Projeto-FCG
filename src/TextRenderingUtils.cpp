#include "TextRenderingUtils.h"
#include "ft2build.h"
#include FT_FREETYPE_H
#include "shaderUtil.h"


bool isInit = false;
struct Character {
    GLuint     textureID;  // ID handle of the glyph texture
    glm::ivec2 Size;       // Size of glyph
    glm::ivec2 Bearing;    // Offset from baseline to left/top of glyph
    GLuint     Advance;    // Offset to advance to next glyph
};

std::map<GLchar, Character> Characters;

GLuint VAO, VBO, gpuprogram;
glm::mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);

void Init(){

    FT_Library ft;
    FT_Face face;

    if(FT_Init_FreeType(&ft))
        printf("ERROR::FREETYPE: Could not init FreeType Library");

    if (FT_New_Face(ft, "../../Fonts/courbd.ttf", 0, &face))
        printf("ERROR::FREETYPE: Failed to load font");

    FT_Set_Pixel_Sizes(face, 0, 48);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

    for (GLubyte c = 0; c < 128; c++)
    {
        // Load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            printf("ERROR::FREETYTPE: Failed to load Glyph");
            continue;
        }

        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            (GLuint)face->glyph->advance.x
        };
        Characters.insert(std::pair<GLchar, Character>(c, character));
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);


    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    isInit = true;

    GLuint vertexShader = LoadShader_Vertex("../../src/font_vertex_shader.glsl");
    GLuint fragmentShader = LoadShader_Fragment("../../src/font_fragment_shader.glsl");

    gpuprogram = glCreateProgram(); //cria um novo programa de GPU
    glAttachShader(gpuprogram, vertexShader);
    glAttachShader(gpuprogram, fragmentShader);
    glLinkProgram(gpuprogram);

}

void RenderText(std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{

    // Activate corresponding render state
    glUseProgram(gpuprogram);
    glUniform3f(glGetUniformLocation(gpuprogram, "textColor"), color.x, color.y, color.z);
    glUniformMatrix4fv(glGetUniformLocation(gpuprogram, "projection")       , 1 , GL_FALSE , glm::value_ptr(projection));


    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;
        // Update VBO for each character
        GLfloat vertices[6][4] = {
            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos,     ypos,       0.0, 1.0 },
            { xpos + w, ypos,       1.0, 1.0 },

            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos + w, ypos,       1.0, 1.0 },
            { xpos + w, ypos + h,   1.0, 0.0 }
        };
        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.textureID);
        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

void DrawText(std::string text, TextPosition position){
    if(!isInit)
        Init();

    GLint data[4];

    glGetIntegerv(GL_VIEWPORT, data);

    int width = data[2];
    int height = data[3];
    projection = glm::ortho(0.0f, (float)width, 0.0f, (float)height);

    GLfloat x = 0.0f, y = 0.0f;

    if(position == TextPosition::CENTER){
        x = width/2;
        y = height/2;
    }
    else if(position == TextPosition::SUBTITLE){
        x = width/4;
        y = height/4;
    }

    float fontSize = 0.5f;

    RenderText(text.c_str(), x, y, fontSize, glm::vec3(1.0f, 1.0f, 1.0f));

}
