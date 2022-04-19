#ifndef SOLARSYSTEM_TRANSFORMABLE_H
#define SOLARSYSTEM_TRANSFORMABLE_H
#include "../Auxiliary_Modules/Shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Transformable {
public:
    explicit Transformable(const Shader& shader);
    void Translate(const glm::vec3& position);
    void Scale(const glm::vec3& scale);
    void Rotate(float angle, const glm::vec3& axisRotation);
    void LoadIdentityModelMatrix();
    void UpdateModelMatrix();
    void SetShader(const Shader& shader);
    Shader GetShader() const;
    glm::mat4 GetRotationMatrix() const;
    glm::vec3 GetPosition() const;
    float GetLastRotationAngle() const;

protected:
    Shader _shader;

private:
    glm::mat4 _matrixModel = glm::mat4(1.0f), _rotationMatrix = glm::mat4(1.0f); // rotationMatrix is separately needed for the correct ring normal for ray casting
    glm::vec3 _position = glm::vec3();
    float _lastRotationAngle = 0.0;

    void UpdateRotationMatrix();
};

#endif //SOLARSYSTEM_TRANSFORMABLE_H
