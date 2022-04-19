#ifndef SOLARSYSTEM_SHADER_H
#define SOLARSYSTEM_SHADER_H
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader {
public:
    explicit Shader(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath = "");
    void Use() const;
    void SetBool(const std::string& name, bool value) const;
    void SetInt(const std::string& name, int value) const;
    void SetFloat(const std::string& name, float value) const;
    void SetDouble(const std::string& name, double value) const;
    void SetVec2(const std::string& name, const glm::vec2& value) const;
    void SetVec2(const std::string& name, float x, float y) const;
    void SetVec3(const std::string& name, const glm::vec3& value) const;
    void SetVec3(const std::string& name, float x, float y, float z) const;
    void SetVec4(const std::string& name, const glm::vec4& value) const;
    void SetVec4(const std::string& name, float x, float y, float z, float w) const;
    void SetMat2(const std::string& name, const glm::mat2& mat) const;
    void SetMat3(const std::string& name, const glm::mat3& mat) const;
    void SetMat4(const std::string& name, const glm::mat4& mat) const;
    void SetVec2Double(const std::string& name, const glm::dvec2& value) const;
    void SetVec2Double(const std::string& name, double x, double y) const;
    void SetVec3Double(const std::string& name, const glm::dvec3& value) const;
    void SetVec3Double(const std::string& name, double x, double y, double z) const;
    void SetVec4Double(const std::string& name, const glm::dvec4& value) const;
    void SetVec4Double(const std::string& name, double x, double y, double z, double w) const;
    void SetMat2Double(const std::string& name, const glm::dmat2& mat) const;
    void SetMat3Double(const std::string& name, const glm::dmat3& mat) const;
    void SetMat4Double(const std::string& name, const glm::dmat4& mat) const;
    size_t GetProgramId() const;

private:
    enum class ShaderType {
        VertexShader,
        FragmentShader,
        GeometryShader,
        ShaderProgram
    };

    size_t _shaderProgramID = 0;

    static void CheckCompileErrors(size_t shader, ShaderType type, const std::string& path = "");
    static std::string ShaderTypeToString(ShaderType type);
};

#endif //SOLARSYSTEM_SHADER_H
