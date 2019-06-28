#ifndef MATERIAL_H
#define MATERIAL_H

#include "ShaderProgram.h"
#include "Mesh.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/// Basic camera model
class Material{
public:
  void setAlbedo(std::shared_ptr<ShaderProgram> sPtr, glm::vec3 alb)
  {
    albedo = alb;
    sPtr->set("material.albedo", albedo);
  }

  void setKs(std::shared_ptr<ShaderProgram> sPtr, float ss)
  {
    ks = ss;
    sPtr->set("material.ks", ks);
  }

  void setS(std::shared_ptr<ShaderProgram> sPtr, float ss)
  {
    s = ss;
    sPtr->set("material.s", s);
  }

  void setAlpha(std::shared_ptr<ShaderProgram> sPtr, float alp)
  {
    alpha = alp;
    sPtr->set("material.alpha", alpha);
  }

  void setF(std::shared_ptr<ShaderProgram> sPtr, float f)
  {
    f0 = f;
    sPtr->set("material.f0", f0);
  }

  GLuint loadTextureFromFileToGPU(const std::string & filename){
    int width, height, numComponents;

    // Loading the image in CPU memory using stbd_image
    unsigned char * data = stbi_load (filename.c_str (), &width, &height, &numComponents, 0);
                              // 1 for a 8 bit greyscale image, 3 for 24bits RGB image

    // Create a texture in GPU memory
    GLuint texID;
    glGenTextures (1, &texID);
    glBindTexture (GL_TEXTURE_2D, texID);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Uploading the image data to GPU memory
    glTexImage2D (GL_TEXTURE_2D, 0,(numComponents == 1 ? GL_RED : GL_RGB),
                  width,height,0,(numComponents == 1 ? GL_RED : GL_RGB),GL_UNSIGNED_BYTE,data);
                  // We assume only greyscale or RGB pixels

    // Generating mipmaps for filtered texture fetch
    glGenerateMipmap(GL_TEXTURE_2D);

    // Freeing the now useless CPU memory
    stbi_image_free(data);
    glBindTexture (GL_TEXTURE_2D, 0);
    return texID;
  }

private:
  glm::vec3 albedo;
  float ks;
  float s;
  float alpha;
  float f0;
};

#endif // MATERIAL_H
