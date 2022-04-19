#include "Camera.h"

Camera::Camera(float near, float far, float movementSpeed, float mouseSensitivity, glm::vec3 position, glm::vec3 up, float yaw, float pitch, float zoom)
    : _frontVector(glm::vec3(0.0f, 0.0f, 0.0f)), _movementSpeed(movementSpeed), _mouseSensitivity(mouseSensitivity), _zoom(zoom)
{
    _position = position;
    _worldUpVector = up;
    _yaw = yaw;
    _pitch = pitch;
    _nearPlane = near;
    _farPlane = far;
    UpdateCameraVectors();
}

void Camera::SetMovementSpeed(float speed) {
    _movementSpeed = speed;
}

void Camera::SetAspect(float aspect) {
    _aspect = aspect;
}

void Camera::SetNear(float near) {
    _nearPlane = near;
}

void Camera::SetFar(float far) {
    _farPlane = far;
}

float Camera::GetAspect() const {
    return _aspect;
}

float Camera::GetNear() const {
    return _nearPlane;
}

float Camera::GetFar() const {
    return _farPlane;
}

float Camera::GetZoom() const {
    return _zoom;
}

float Camera::GetMovementSpeed() const {
    return _movementSpeed;
}

glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(_position, _position + _frontVector, _upVector);
}

glm::mat4 Camera::GetProjectionMatrix() const {
    return glm::perspective(glm::radians(_zoom), _aspect, _nearPlane, _farPlane);
}

glm::vec3 Camera::GetPosition() const {
    return _position;
}

glm::vec3 Camera::GetRightVector() const {
    return _rightVector;
}

glm::vec3 Camera::GetFrontVector() const {
    return _frontVector;
}

glm::vec3 Camera::GetUpVector() const {
    return _upVector;
}

glm::vec3 Camera::GetWorldUpVector() const {
    return _worldUpVector;
}

// processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
void Camera::ProcessKeyboard(CameraVector direction, float deltaTime) {
    float velocity = _movementSpeed * deltaTime;
    if (direction == CameraVector::FORWARD)
        _position += _frontVector * velocity;
    if (direction == CameraVector::BACKWARD)
        _position -= _frontVector * velocity;
    if (direction == CameraVector::LEFT)
        _position -= _rightVector * velocity;
    if (direction == CameraVector::RIGHT)
        _position += _rightVector * velocity;
    if (direction == CameraVector::UP)
        _position += _upVector * velocity;
    if (direction == CameraVector::WORLD_UP)
        _position += _worldUpVector * velocity;
    if (direction == CameraVector::WORLD_DOWN)
        _position += -_worldUpVector * velocity;
}

// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= _mouseSensitivity;
    yoffset *= _mouseSensitivity;

    _yaw   += xoffset;
    _pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch)
    {
        if (_pitch > 89.0f)
            _pitch = 89.0f;
        if (_pitch < -89.0f)
            _pitch = -89.0f;
    }

    // update _frontVector, _rightVector and _upVector Vectors using the updated Euler angles
    UpdateCameraVectors();
}

// processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
void Camera::ProcessMouseScroll(float yoffset) {
    _zoom = glm::clamp(_zoom - yoffset, 1.0f, 45.0f);
}

void Camera::UpdateCameraVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
    front.y = sin(glm::radians(_pitch));
    front.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
    _frontVector = glm::normalize(front);
    _rightVector = glm::normalize(glm::cross(_frontVector, _worldUpVector));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    _upVector    = glm::normalize(glm::cross(_rightVector, _frontVector));
}
