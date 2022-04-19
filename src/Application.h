#ifndef SOLARSYSTEM_APPLICATION_H
#define SOLARSYSTEM_APPLICATION_H
#include "Auxiliary_Modules/AuxiliaryModules.h"
#include "Solar_System/SolarSystem.h"
#include "SystemModules.h"

namespace {
    Camera camera(0.001f, 20000.f, 7.5f, 0.1f, glm::vec3(-134.0f, 0.0f, 0.0f));
    float lastX, lastY;
    float starExposure = 8.0f, starGamma = 0.4545454f, starTemperatureInKelvin = 5778.0f;
    double deltaTime = 0.0, lastFrame = 0.0;
    bool isFirstMouse = true, isTimeRun = true, isRenderHints = true, isRenderPlanetStarDistances = true, isRenderSatelliteDistances = true, isVertSyncEnabled = true;
}

struct RenderableAtmosphere {
    std::unique_ptr<Atmosphere> atmosphere;
    float hScaleFactor, parentEarthSizeCoefficient;
    bool isUseToneMapping = false;
};

struct RenderableSceneComponent {
    glm::mat4 lightSpaceMatrix;
    std::shared_ptr<Planet> planet;
    std::vector<std::shared_ptr<Satellite>> satellites;
    std::vector<RenderableAtmosphere> atmospheres;
    std::unique_ptr<Clouds> clouds;
    std::unique_ptr<PlanetaryRing> planetaryRing;
};

class Application {
public:
    Application();
    ~Application();
    void Exec();

private:
    GLFWwindow* _mainWindow = nullptr;
    uint16_t _displayWidth = 0, _displayHeight = 0;
    ssize_t _nearestPlanetIndex = 0;
    FPS_Handler _fpsHandler;
    FT_Library _ft = nullptr;
    bool _isBackgroundMusicPlay = false, _isSearchNearestPlanet = false;
    ISoundEngine* _soundEngine = nullptr;
    std::string _currentMusicTrack;
    glm::mat4 _cameraProjection = glm::mat4(), _cameraView = glm::mat4();
    std::unique_ptr<std::thread> _backgroundMusicThread, _searchNearestPlanetThread;
    std::unique_ptr<TextRenderer> _textRenderer;
    std::unique_ptr<ShadowMapFBO> _shadowMapFBO;
    std::unique_ptr<HDR> _hdr;
    std::unique_ptr<SkyBox> _skyBox;
    std::unique_ptr<Shader> _shadowMapShader;
    std::unique_ptr<Shader> _mainSkyBoxShader, _mainTextShader, _mainStarShader, _mainCoronaStarShader, _mainPlanetShader, _mainAtmosphereShader, _mainCloudsShader,
        _mainRingShader;
    std::unique_ptr<LensFlare> _lensFlare;
    std::shared_ptr<Star> _sun;
    std::vector<RenderableSceneComponent> _renderableSceneComponents;
    std::vector<std::string_view> _backgroundSongs;

    void InitSystems();
    void InitScene();
    void InitStarSystem();
    void InitMercury(const MeshHolder& sphereModel);
    void InitVenus(const MeshHolder& sphereModel);
    void InitEarthSystem(const MeshHolder& sphereModel);
    void InitMarsSystem(const MeshHolder& sphereModel);
    void InitJupiterSystem(const MeshHolder& sphereModel);
    void InitSaturnSystem(const MeshHolder& sphereModel);
    void InitUranusSystem(const MeshHolder& sphereModel);
    void InitNeptuneSystem(const MeshHolder& sphereModel);
    void InitPlutoSystem(const MeshHolder& sphereModel);
    void InitSongList();
    void Dispose();
    void StartSearchNearestPlanet();
    void StartPlayBackgroundMusic();
    void StopSearchNearestPlanet();
    void StopPlayBackgroundMusic();
    void LoadWindowIcon() const;
    void DisplaySystemInformation() const;
    void ProcessSceneComponentsRendering();
    void ShadowMapPass(const RenderableSceneComponent& component);
    void RenderPass(const RenderableSceneComponent& component);
    void ProcessStarRendering();
    void RenderStarCorona() const;
    void RenderStar() const;
    void RenderStarEffects() const;
    void RenderAtmospheres(const std::vector<RenderableAtmosphere>& renderableAtmospheres, const glm::mat4& lightSpaceMatrix, const PlanetaryRing* ring) const;
    void RenderClouds(Clouds* renderableClouds, const glm::mat4& lightSpaceMatrix) const;
    void RenderPlanetaryRing(const Shader& shader, PlanetaryRing* planetaryRing, const glm::mat4& lightSpaceMatrix) const;
    void RenderPlanetSatelliteStarDistances() const;
    void RenderSpaceObjectDistance(const SpaceObject* spaceObject) const;
    void RenderHints() const;
    void ConfigureMainShaders();
    void ConfigureMainPlanetShader(const RenderableSceneComponent& renderableComponent);
    void UpdateOcclusionQuery();
    void ProcessInput(GLFWwindow* window);
    float CalculateSpaceObjectDistance(const SpaceObject* spaceObject) const;
    glm::vec3 CurrentFpsColor() const;
    static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void MouseCallback(GLFWwindow* window, double xPos, double yPos);
    static void ScrollCallback(GLFWwindow* window, double xoffset, double yOffset);
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void VertSync(bool enable);
    static bool WGLExtensionSupported(const char* extensionName);
};

#endif //SOLARSYSTEM_APPLICATION_H
