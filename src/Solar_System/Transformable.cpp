#include "Transformable.h"

Transformable::Transformable(const Shader& shader) : _shader(shader)
{
}

void Transformable::Translate(const glm::vec3& position) {
    _position = position;
    _matrixModel = glm::translate(_matrixModel, position);
}

void Transformable::Scale(const glm::vec3& scale) {
    _matrixModel = glm::scale(_matrixModel, scale);
}

void Transformable::Rotate(float angle, const glm::vec3& axisRotation) {
    _matrixModel = glm::rotate(_matrixModel, glm::radians(angle), axisRotation);
    _lastRotationAngle = angle;
    UpdateRotationMatrix();
}

void Transformable::LoadIdentityModelMatrix() {
    _rotationMatrix = glm::mat4(1.0f);
    _matrixModel = glm::mat4(1.0f);
}

void Transformable::UpdateModelMatrix() {
    _shader.SetMat4("model", _matrixModel);
}

void Transformable::SetShader(const Shader& shader) {
    _shader = shader;
}

Shader Transformable::GetShader() const {
    return _shader;
}

glm::mat4 Transformable::GetRotationMatrix() const {
    return _rotationMatrix;
}

glm::vec3 Transformable::GetPosition() const {
    return _position;
}

float Transformable::GetLastRotationAngle() const {
    return _lastRotationAngle;
}

void Transformable::UpdateRotationMatrix() {
    // https://stackoverflow.com/questions/45091505/opengl-transforming-objects-with-multiple-rotations-of-different-axis
    // Extract a rotation only matrix from the model matrix
    glm::vec3 x = glm::normalize(glm::vec3(_matrixModel[0][0], _matrixModel[0][1], _matrixModel[0][2]));
    glm::vec3 y = glm::normalize(glm::vec3(_matrixModel[1][0], _matrixModel[1][1], _matrixModel[1][2]));
    glm::vec3 z = glm::normalize(glm::vec3(_matrixModel[2][0], _matrixModel[2][1], _matrixModel[2][2]));

    glm::mat4 r;
    r[0][0] = x[0]; r[0][1] = x[1]; r[0][2] = x[2]; r[0][3] = 0.0f;
    r[1][0] = y[0]; r[1][1] = y[1]; r[1][2] = y[2]; r[0][3] = 0.0f;
    r[2][0] = z[0]; r[2][1] = z[1]; r[2][2] = z[2]; r[0][3] = 0.0f;
    r[3][0] = 0.0f; r[3][1] = 0.0f; r[3][2] = 0.0f; r[0][3] = 1.0f;

    _rotationMatrix = r;
}
