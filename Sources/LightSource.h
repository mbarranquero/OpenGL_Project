#ifndef LIGHTSOURCE_H
#define LIGHTSOURCE_H

#include "ShaderProgram.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

/// Basic camera model
class LightSource : Transform{
public:

/// Lumiere 1
  void setPosition(std::shared_ptr<ShaderProgram> sPtr, glm::vec3 pos)
  {
    position = pos;
    sPtr->set("lightSource.position", position);
  }

  void setColor(std::shared_ptr<ShaderProgram> sPtr, glm::vec3 col)
  {
    color = col;
    sPtr->set("lightSource.color", color);
  }

  void setIntensity(std::shared_ptr<ShaderProgram> sPtr, float ss)
  {
    intensity = ss;
    sPtr->set("lightSource.intensity", intensity);
  }

  void setAc(std::shared_ptr<ShaderProgram> sPtr, float conA)
  {
    Ac = conA;
    sPtr->set("lightSource.Ac", Ac);
  }

  void setAl(std::shared_ptr<ShaderProgram> sPtr, float rCoef)
  {
    Al = rCoef;
    sPtr->set("lightSource.Al", Al);
  }

  void setAq(std::shared_ptr<ShaderProgram> sPtr, float dCoef)
  {
    Aq = dCoef;
    sPtr->set("lightSource.Aq", Aq);
  }

/// Lumiere 2
  void setPositionB(std::shared_ptr<ShaderProgram> sPtr, glm::vec3 pos)
  {
    position = pos;
    sPtr->set("lightSourceB.position", position);
  }

  void setColorB(std::shared_ptr<ShaderProgram> sPtr, glm::vec3 col)
  {
    color = col;
    sPtr->set("lightSourceB.color", color);
  }

  void setIntensityB(std::shared_ptr<ShaderProgram> sPtr, float ss)
  {
    intensity = ss;
    sPtr->set("lightSourceB.intensity", intensity);
  }

  void setAcB(std::shared_ptr<ShaderProgram> sPtr, float conA)
  {
    Ac = conA;
    sPtr->set("lightSourceB.Ac", Ac);
  }

  void setAlB(std::shared_ptr<ShaderProgram> sPtr, float rCoef)
  {
    Al = rCoef;
    sPtr->set("lightSourceB.Al", Al);
  }

  void setAqB(std::shared_ptr<ShaderProgram> sPtr, float dCoef)
  {
    Aq = dCoef;
    sPtr->set("lightSourceB.Aq", Aq);
  }

/// Lumiere 3
  void setPositionBi(std::shared_ptr<ShaderProgram> sPtr, glm::vec3 pos)
  {
    position = pos;
    sPtr->set("lightSourceBi.position", position);
  }

  void setColorBi(std::shared_ptr<ShaderProgram> sPtr, glm::vec3 col)
  {
    color = col;
    sPtr->set("lightSourceBi.color", color);
  }

  void setIntensityBi(std::shared_ptr<ShaderProgram> sPtr, float ss)
  {
    intensity = ss;
    sPtr->set("lightSourceBi.intensity", intensity);
  }

  void setAcBi(std::shared_ptr<ShaderProgram> sPtr, float conA)
  {
    Ac = conA;
    sPtr->set("lightSourceBi.Ac", Ac);
  }

  void setAlBi(std::shared_ptr<ShaderProgram> sPtr, float rCoef)
  {
    Al = rCoef;
    sPtr->set("lightSourceBi.Al", Al);
  }

  void setAqBi(std::shared_ptr<ShaderProgram> sPtr, float dCoef)
  {
    Aq = dCoef;
    sPtr->set("lightSourceBi.Aq", Aq);
  }

private:
  glm::vec3 position;
  glm::vec3 color;
  float intensity;
  float Ac;
  float Al;
  float Aq;

};

#endif // LIGHTSOURCE_H
