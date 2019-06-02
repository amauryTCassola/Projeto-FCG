#include "stb_imageUtils.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Image LoadImageFromDisc(const char* filename){

    stbi_set_flip_vertically_on_load(true);
    int width;
    int height;
    int channels;
    unsigned char *data = stbi_load(filename, &width, &height, &channels, 3);

    if ( data == NULL )
    {
        std::exit(EXIT_FAILURE);
    }

    Image newImage;
    newImage.data = data;
    newImage.height = height;
    newImage.width = width;
    newImage.channels = channels;

    return newImage;
}

void DestroyImg(Image img){
    stbi_image_free(img.data);
}
