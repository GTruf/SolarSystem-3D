#include "Shader.h"

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath) {
    std::string vertexCode;
    std::string fragmentCode;
    std::string geometryCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    std::ifstream gShaderFile;

    // Убеждаемся, что объекты ifstream могут выбросить исключение
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        // Открываем файлы
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::ostringstream vShaderStream, fShaderStream;

        // Считываем содержимое файловых буферов в потоки
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();

        // Закрываем файлы
        vShaderFile.close();
        fShaderFile.close();

        // Конвертируем данные из потока в строковые переменные
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();

        if(!geometryPath.empty()) {
            gShaderFile.open(geometryPath);
            std::stringstream gShaderStream;
            gShaderStream << gShaderFile.rdbuf();
            gShaderFile.close();
            geometryCode = gShaderStream.str();
        }
    }

    catch (const std::ifstream::failure& e) {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    // Этап №2: Компилируем шейдеры
    size_t vertex, fragment;

    // Вершинный шейдер
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, nullptr);
    glCompileShader(vertex);
    CheckCompileErrors(vertex, ShaderType::VertexShader, vertexPath);

    // Фрагментный шейдер
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, nullptr);
    glCompileShader(fragment);
    CheckCompileErrors(fragment, ShaderType::FragmentShader, fragmentPath);

    // Геометрический шейдер (если есть)
    size_t geometry;
    if(!geometryPath.empty()) {
        const char* gShaderCode = geometryCode.c_str();
        geometry = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometry, 1, &gShaderCode, nullptr);
        glCompileShader(geometry);
        CheckCompileErrors(geometry, ShaderType::GeometryShader, geometryPath);
    }

    // Шейдерная программа
    _shaderProgramID = glCreateProgram();
    glAttachShader(_shaderProgramID, vertex); // Прикрепление вершинного шейдера
    glAttachShader(_shaderProgramID, fragment); // Прикрепление фрагментного шейдера
    if (!geometryPath.empty())
        glAttachShader(_shaderProgramID, geometry); // Прикрепление геометрического шейдера
    glLinkProgram(_shaderProgramID); // Сборка шейдерной программы из прикреплённых шейдеров
    CheckCompileErrors(_shaderProgramID, ShaderType::ShaderProgram, fragmentPath);

    // Удаление шейдеров
    glDetachShader(_shaderProgramID, vertex);
    glDeleteShader(vertex);
    glDetachShader(_shaderProgramID, fragment);
    glDeleteShader(fragment);
    if (!geometryPath.empty()) {
        glDetachShader(_shaderProgramID, geometry);
        glDeleteShader(geometry);
    }
}

void Shader::Use() const {
    glUseProgram(_shaderProgramID);
}

void Shader::SetBool(const std::string& name, bool value) const {
    glUniform1i(glGetUniformLocation(_shaderProgramID, name.c_str()), static_cast<int>(value));
}

void Shader::SetInt(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(_shaderProgramID, name.c_str()), value);
}

void Shader::SetFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(_shaderProgramID, name.c_str()), value);
}

void Shader::SetDouble(const std::string &name, double value) const {
    glUniform1d(glGetUniformLocation(_shaderProgramID, name.c_str()), value);
}

void Shader::SetVec2(const std::string& name, const glm::vec2& value) const {
    glUniform2fv(glGetUniformLocation(_shaderProgramID, name.c_str()), 1, &value[0]);
}

void Shader::SetVec2(const std::string& name, float x, float y) const {
    glUniform2f(glGetUniformLocation(_shaderProgramID, name.c_str()), x, y);
}

void Shader::SetVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(glGetUniformLocation(_shaderProgramID, name.c_str()), 1, &value[0]);
}

void Shader::SetVec3(const std::string& name, float x, float y, float z) const {
    glUniform3f(glGetUniformLocation(_shaderProgramID, name.c_str()), x, y, z);
}

void Shader::SetVec4(const std::string& name, const glm::vec4& value) const {
    glUniform4fv(glGetUniformLocation(_shaderProgramID, name.c_str()), 1, &value[0]);
}

void Shader::SetVec4(const std::string& name, float x, float y, float z, float w) const {
    glUniform4f(glGetUniformLocation(_shaderProgramID, name.c_str()), x, y, z, w);
}

void Shader::SetMat2(const std::string& name, const glm::mat2& mat) const {
    glUniformMatrix2fv(glGetUniformLocation(_shaderProgramID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::SetMat3(const std::string& name, const glm::mat3& mat) const {
    glUniformMatrix3fv(glGetUniformLocation(_shaderProgramID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::SetMat4(const std::string& name, const glm::mat4& mat) const {
    glUniformMatrix4fv(glGetUniformLocation(_shaderProgramID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::SetVec2Double(const std::string& name, const glm::dvec2& value) const {
    glUniform2dv(glGetUniformLocation(_shaderProgramID, name.c_str()), 1, &value[0]);
}

void Shader::SetVec2Double(const std::string& name, double x, double y) const {
    glUniform2d(glGetUniformLocation(_shaderProgramID, name.c_str()), x, y);
}

void Shader::SetVec3Double(const std::string& name, const glm::dvec3& value) const {
    glUniform3dv(glGetUniformLocation(_shaderProgramID, name.c_str()), 1, &value[0]);
}

void Shader::SetVec3Double(const std::string& name, double x, double y, double z) const {
    glUniform3d(glGetUniformLocation(_shaderProgramID, name.c_str()), x, y, z);
}

void Shader::SetVec4Double(const std::string& name, const glm::dvec4& value) const {
    glUniform4dv(glGetUniformLocation(_shaderProgramID, name.c_str()), 1, &value[0]);
}

void Shader::SetVec4Double(const std::string& name, double x, double y, double z, double w) const {
    glUniform4d(glGetUniformLocation(_shaderProgramID, name.c_str()), x, y, z, w);
}

void Shader::SetMat2Double(const std::string& name, const glm::dmat2& mat) const {
    glUniformMatrix2dv(glGetUniformLocation(_shaderProgramID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::SetMat3Double(const std::string& name, const glm::dmat3& mat) const {
    glUniformMatrix3dv(glGetUniformLocation(_shaderProgramID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::SetMat4Double(const std::string& name, const glm::dmat4& mat) const {
    glUniformMatrix4dv(glGetUniformLocation(_shaderProgramID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

size_t Shader::GetProgramId() const {
    return _shaderProgramID;
}

void Shader::CheckCompileErrors(size_t shader, ShaderType type, const std::string& path) {
    int success;
    char infoLog[1024];
    if (type != ShaderType::ShaderProgram) {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            throw std::runtime_error("ERROR::SHADER_COMPILATION_ERROR of type: " + ShaderTypeToString(type) + " " + path + "\n" + std::string(infoLog) + "\n -- --------------------------------------------------- -- ");
        }
    }
    else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
            throw std::runtime_error("ERROR::PROGRAM_LINKING_ERROR of type: " + ShaderTypeToString(type) + "\n" + std::string(infoLog) + path + "\n -- --------------------------------------------------- --");
        }
    }
}

std::string Shader::ShaderTypeToString(ShaderType type) {
    switch(type) {
        case ShaderType::VertexShader: return "Vertex Shader";
        case ShaderType::FragmentShader: return "Fragment Shader";
        case ShaderType::GeometryShader: return "Geometry Shader";
        case ShaderType::ShaderProgram: return "Shader Program";
        default: return "";
    }
}
