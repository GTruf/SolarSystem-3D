#include "Application.h"
#include <SDL_image.h>
#include <random>
#include <iomanip>

using namespace std;

Application::Application() : _fpsHandler(240) {
    InitSystems();
    InitScene();
}

void Application::Exec() {
    while (!glfwWindowShouldClose(_mainWindow)) {
        _fpsHandler.RunFrameTimer();

        const double currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ProcessInput(_mainWindow);
        ConfigureMainShaders();
        _skyBox->Render(*_mainSkyBoxShader); // If rendered at the end, it overlaps atmospheres with clouds
        RenderStarCorona();
        ProcessSceneComponentsRendering();
        RenderStarEffects();

        if (isRenderPlanetStarDistances || isRenderSatelliteDistances)
            RenderPlanetSatelliteStarDistances();
        if (isRenderHints)
            RenderHints();

        glfwSwapBuffers(_mainWindow);
        glfwPollEvents();

        _fpsHandler.WaitForFrameTimer();
    }
}

void Application::ProcessSceneComponentsRendering() {
    // The idea is to render a scene component (planet, its satellites, rings, etc.) into a shadow map,
    // then immediately into a regular buffer, and then clear the shadow map (depth buffer) so that the planet closest
    // to the sun does not cover all the others.
    // Cleaning is necessary because only one shadow map is used, and without cleaning, the depth will be overlapped by objects from all over the scene
    // Thus, by changing the lightSpaceMatrix, it can be created the impression that an omnidirectional light source is used in the scene.
    // If desired, you can use the technique with a cube depth map, but due to the lack of precision of z-buffer, shadows are killed

    for (const auto& renderableSceneComponent : _renderableSceneComponents) {
        ShadowMapPass(renderableSceneComponent);
        RenderPass(renderableSceneComponent);
    }
}

void Application::ShadowMapPass(const RenderableSceneComponent& component) {
    glBindFramebuffer(GL_FRAMEBUFFER, _shadowMapFBO->GetFBO());
    glClear(GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, _shadowMapFBO->GetShadowMapWidth(), _shadowMapFBO->GetShadowMapHeight());

    _shadowMapShader->Use();
    _shadowMapShader->SetMat4("lightSpaceMatrix", component.lightSpaceMatrix);

    component.planet->SetShader(*_shadowMapShader);
    component.planet->AdjustToParent(isTimeRun);
    component.planet->Render();

    for (const auto& satellite : component.satellites) {
        satellite->SetShader(*_shadowMapShader);
        satellite->AdjustToParent(isTimeRun);
        satellite->Render();
    }

    RenderPlanetaryRing(*_shadowMapShader, component.planetaryRing.get(), component.lightSpaceMatrix);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Application::RenderPass(const RenderableSceneComponent& component) {
    glViewport(0, 0, _displayWidth, _displayHeight);
    _mainPlanetShader->Use();

    ConfigureMainPlanetShader(component);

    component.planet->SetShader(*_mainPlanetShader);
    component.planet->AdjustToParent(isTimeRun);
    component.planet->Render();

    for (const auto& satellite : component.satellites) {
        satellite->SetShader(*_mainPlanetShader);
        satellite->AdjustToParent(isTimeRun);
        satellite->Render();
    }

    // Occlusion request will be carried out for the nearest planet, so if planet has a rings, they do not cover the sun.
    // This is a bit of a strange technique, but necessary due to the fact that the ring itself is a solid
    // 3D model, and not procedurally generated particles.
    if (component.planet == _renderableSceneComponents[_nearestPlanetIndex].planet) // Render star once and update occlusion query for nearest planet
        ProcessStarRendering();

    RenderAtmospheres(component.atmospheres, component.lightSpaceMatrix, component.planetaryRing.get());
    RenderClouds(component.clouds.get(), component.lightSpaceMatrix);
    RenderPlanetaryRing(*_mainRingShader, component.planetaryRing.get(), component.lightSpaceMatrix);
}

void Application::RenderAtmospheres(const std::vector<RenderableAtmosphere>& renderableAtmospheres, const glm::mat4& lightSpaceMatrix, const PlanetaryRing* ring) const {
    if (!renderableAtmospheres.empty()) {
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        _mainAtmosphereShader->Use();
        _mainAtmosphereShader->SetMat4("lightSpaceMatrix", lightSpaceMatrix);

        for (const auto& renderableAtmosphere : renderableAtmospheres) {
            _mainAtmosphereShader->SetVec3("camPosition", camera.GetPosition() - renderableAtmosphere.atmosphere->GetPosition());
            _mainAtmosphereShader->SetVec3("lightPos", _sun->GetPosition() - renderableAtmosphere.atmosphere->GetPosition());
            _mainAtmosphereShader->SetVec3("mieTint", renderableAtmosphere.atmosphere->GetMieTint());
            _mainAtmosphereShader->SetFloat("SCALE_H_FACTOR", renderableAtmosphere.hScaleFactor);
            _mainAtmosphereShader->SetFloat("earthSizeCoefficient", renderableAtmosphere.parentEarthSizeCoefficient);
            _mainAtmosphereShader->SetBool("isUseToneMapping", renderableAtmosphere.isUseToneMapping);
            _mainAtmosphereShader->SetBool("isNearbyPlanetaryRing", ring != nullptr);

            if (ring) {
                _mainAtmosphereShader->SetVec3("ringParentPlanetCenter", ring->GetParent()->GetPosition());
                _mainAtmosphereShader->SetFloat("ringParentPlanetRadiusSquared", ring->GetParent()->GetRadius() * ring->GetParent()->GetRadius());
                _mainAtmosphereShader->SetBool("isUseSphereIntersect", ring->GetParent() != renderableAtmosphere.atmosphere->GetParent());

                _mainAtmosphereShader->SetVec3("ringCenter", ring->GetPosition());
                _mainAtmosphereShader->SetVec3("ringNormal", ring->GetRingNormal());
                _mainAtmosphereShader->SetVec2("ringInnerOuterRadiuses", glm::vec2(ring->GetInnerRadius(), ring->GetOuterRadius()));
                _mainAtmosphereShader->SetInt("ringDiffuse", 9);
                glBindTextureUnit(9, ring->GetRingTexture());
            }

            // Inside the atmosphere
            if (CalculateSpaceObjectDistance(renderableAtmosphere.atmosphere.get()) <= renderableAtmosphere.atmosphere->GetAtmosphereOuterBoundary())
                glFrontFace(GL_CW);

            renderableAtmosphere.atmosphere->AdjustToParent();
            renderableAtmosphere.atmosphere->Render();

            glFrontFace(GL_CCW);
        }

        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
    }
}

void Application::RenderClouds(Clouds* renderableClouds, const glm::mat4& lightSpaceMatrix) const {
    if (renderableClouds) {
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_COLOR);
        glDisable(GL_CULL_FACE);

        _mainCloudsShader->Use();
        _mainCloudsShader->SetMat4("lightSpaceMatrix", lightSpaceMatrix);
        renderableClouds->AdjustToParent(isTimeRun);
        renderableClouds->Render();

        glEnable(GL_CULL_FACE);
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
    }
}

void Application::RenderPlanetaryRing(const Shader& shader, PlanetaryRing* planetaryRing, const glm::mat4& lightSpaceMatrix) const {
    if (planetaryRing) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        shader.Use();
        shader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);
        planetaryRing->SetShader(shader);
        planetaryRing->AdjustToParent();
        planetaryRing->Render();

        glDisable(GL_BLEND);
    }
}

void Application::ProcessStarRendering() {
    glDepthMask(GL_FALSE);
    glBeginQuery(GL_SAMPLES_PASSED, _sun->GetStarOcclusionValue(0));
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    RenderStar();
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEndQuery(GL_SAMPLES_PASSED);

    glBeginQuery(GL_SAMPLES_PASSED, _sun->GetStarOcclusionValue(1));
    RenderStar();
    glEndQuery(GL_SAMPLES_PASSED);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);

    UpdateOcclusionQuery();
}

void Application::RenderStarCorona() const {
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    _mainCoronaStarShader->Use();
    _sun->SetShader(*_mainCoronaStarShader);
    _sun->TakeStarSystemCenter();
    _sun->Render();
    _sun->SetShader(*_mainStarShader);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void Application::RenderStar() const {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // To allow make the alpha channel of the star at 0 when it is not visible due to some object

    _mainStarShader->Use();
    _sun->TakeStarSystemCenter();
    _sun->Render();

    glDisable(GL_BLEND);
}

void Application::RenderStarEffects() const {
    const PlanetaryRing* nearestPlanetaryRing = nullptr;
    if (!_renderableSceneComponents.empty())
        nearestPlanetaryRing = _renderableSceneComponents[_nearestPlanetIndex].planetaryRing.get();

    optional<RingCameraInfo> ringCameraInfo;
    if (nearestPlanetaryRing) {
        ringCameraInfo = {camera.GetPosition(), nearestPlanetaryRing->GetPosition(), nearestPlanetaryRing->GetRingNormal(),
                          glm::vec2(nearestPlanetaryRing->GetInnerRadius(), nearestPlanetaryRing->GetOuterRadius()),
                          nearestPlanetaryRing->GetRingTexture()};
    }

    glBindFramebuffer(GL_FRAMEBUFFER, _hdr->GetHdrFBO());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    _sun->RenderGlow(_cameraProjection, _cameraView, camera.GetFrontVector() - camera.GetRightVector(), camera.GetAspect(),
                     CalculateSpaceObjectDistance(_sun.get()), ringCameraInfo, starTemperatureInKelvin);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    _hdr->Render(starExposure, starGamma);
    float intensity = glm::min(_sun->GetCurrentGlowSize() * _sun->GetVisibility(), 1.0f);
    _lensFlare->Render(_cameraProjection, _cameraView, _sun->GetPosition(), glm::vec3(1.0), camera.GetAspect(), 0.1, intensity, ringCameraInfo);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
}

void Application::RenderPlanetSatelliteStarDistances() const {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (isRenderPlanetStarDistances)
        RenderSpaceObjectDistance(_sun.get());

    for(const auto& renderableComponentPS : _renderableSceneComponents) {
        if (isRenderPlanetStarDistances) {
            RenderSpaceObjectDistance(renderableComponentPS.planet.get());
        }

        if (isRenderSatelliteDistances) {
            for(const auto& satellite : renderableComponentPS.satellites) {
                RenderSpaceObjectDistance(satellite.get());
            }
        }
    }

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void Application::RenderSpaceObjectDistance(const SpaceObject* spaceObject) const {
    deque<wchar_t> distanceInfo(spaceObject->GetEngName().begin(), spaceObject->GetEngName().end());

    if (!spaceObject->GetOtherLangName().empty()) {
        distanceInfo.push_back(L'[');
        distanceInfo.insert(distanceInfo.end(), spaceObject->GetOtherLangName().begin(), spaceObject->GetOtherLangName().end());
        distanceInfo.push_back(L']');
        distanceInfo.push_back(L' ');
    }

    wstring distance(to_wstring(static_cast<uint16_t>(CalculateSpaceObjectDistance(spaceObject))));
    distanceInfo.insert(distanceInfo.end(), make_move_iterator(distance.begin()), make_move_iterator(distance.end()));

    _mainTextShader->Use();
    _mainTextShader->SetVec3("particleCenterWorldSpace", spaceObject->GetPosition());
    _mainTextShader->SetBool("is3D", true);
    _textRenderer->Render(*_mainTextShader, distanceInfo, 0.0, 0.0, 0.075, glm::vec3(0.98431, 0.80784, 0.69412)); // RGB: 251 206 177
}

void Application::RenderHints() const {
    static const glm::mat4 textProjection = glm::ortho(0.0f, static_cast<float>(_displayWidth), 0.0f, static_cast<float>(_displayHeight));
    static const string gpuHintString = string(reinterpret_cast<const char *>(glGetString(GL_RENDERER)));
    static constexpr glm::vec3 textColor = glm::vec3(0.98431, 0.80784, 0.69412);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    _mainTextShader->Use();
    _mainTextShader->SetMat4("projection", textProjection);
    _mainTextShader->SetBool("is3D", false);

    deque<wstring> fpsHint;
    fpsHint.emplace_back(L"FPS: ");
    fpsHint.emplace_back(to_wstring(_fpsHandler.GetCurrentFps()));

    deque<wstring> gpuHint;
    gpuHint.emplace_back(wstring(gpuHintString.begin(), gpuHintString.end()));

    deque<wstring> soundVolumeHint;
    stringstream soundVolumeStream;
    soundVolumeStream << fixed << setprecision(0) << _soundEngine->getSoundVolume() * 100.0;
    string soundVolume = soundVolumeStream.str();
    soundVolumeHint.emplace_back(wstring(L"Sound volume(PgUp/PgDown): ").append(soundVolume.begin(), soundVolume.end()).append(L" %"));

    deque<wstring> currentMusicTrackHint;
    currentMusicTrackHint.emplace_back(wstring(_currentMusicTrack.begin(), _currentMusicTrack.end()));

    deque<wstring> timeRunHint;
    timeRunHint.emplace_back(L"Time running(F): ");
    timeRunHint.emplace_back((isTimeRun) ? L"On" : L"Off");

    deque<wstring> planetStarHint;
    planetStarHint.emplace_back(L"Planet/Star distances(Z): ");
    planetStarHint.emplace_back((isRenderPlanetStarDistances) ? L"On" : L"Off");

    deque<wstring> satelliteHint;
    satelliteHint.emplace_back(L"Satellite distances(X): ");
    satelliteHint.emplace_back((isRenderSatelliteDistances) ? L"On" : L"Off");

    deque<wstring> cameraSpeedHint;
    cameraSpeedHint.emplace_back(L"Camera speed(1/2): ");
    cameraSpeedHint.emplace_back(to_wstring(camera.GetMovementSpeed()));

    deque<wstring> smoothCameraHint;
    smoothCameraHint.emplace_back(L"Smooth camera(Arrows)");

    deque<wstring> smoothZoomHint;
    smoothZoomHint.emplace_back(L"Smooth zoom(V/B)");

    deque<wstring> movementHint;
    movementHint.emplace_back(L"Move up/down(SPACE/C)");

    deque<wstring> speedBostHint;
    speedBostHint.emplace_back(L"Speed boost(SHIFT)");

    deque<wstring> starExposureHint;
    starExposureHint.emplace_back(L"Star Exposure(3/4): ");
    starExposureHint.emplace_back(to_wstring(starExposure));

    deque<wstring> starGammaHint;
    starGammaHint.emplace_back(L"Star Gamma(5/6): ");
    starGammaHint.emplace_back(to_wstring(starGamma));

    deque<wstring> starTemperatureHint;
    stringstream  starTemperatureStream;
    starTemperatureStream << fixed << setprecision(0) << starTemperatureInKelvin;
    string starTemperatureStr = starTemperatureStream.str();
    starTemperatureHint.emplace_back(wstring(L"Star Temperature(7/8): ").append(make_move_iterator(starTemperatureStr.begin()),
                                                                                make_move_iterator(starTemperatureStr.end())));
    deque<wstring> vertSyncHint;
    vertSyncHint.emplace_back(L"Vert Sync(F1): ");
    vertSyncHint.emplace_back((isVertSyncEnabled) ? L"On" : L"Off");

    deque<wstring> textHints;
    textHints.emplace_back(L"Text hints(TAB)");

    _textRenderer->ReverseRender(*_mainTextShader, currentMusicTrackHint, 0.99 * _displayWidth, 0.95 * _displayHeight, 0.35, textColor);
    _textRenderer->Render(*_mainTextShader, fpsHint, 0.01 * _displayWidth, 0.95 * _displayHeight, 0.35, CurrentFpsColor());
    _textRenderer->Render(*_mainTextShader, gpuHint, 0.01 * _displayWidth, 0.925 * _displayHeight, 0.35, textColor);
    _textRenderer->Render(*_mainTextShader, soundVolumeHint, 0.01 * _displayWidth, 0.9 * _displayHeight, 0.35, textColor);
    _textRenderer->Render(*_mainTextShader, timeRunHint, 0.01 * _displayWidth, 0.875 * _displayHeight, 0.35, textColor);
    _textRenderer->Render(*_mainTextShader, planetStarHint, 0.01 * _displayWidth, 0.85 * _displayHeight, 0.35, textColor);
    _textRenderer->Render(*_mainTextShader, satelliteHint, 0.01 * _displayWidth, 0.825 * _displayHeight, 0.35, textColor);
    _textRenderer->Render(*_mainTextShader, cameraSpeedHint, 0.01 * _displayWidth, 0.8 * _displayHeight, 0.35, textColor);
    _textRenderer->Render(*_mainTextShader, smoothCameraHint, 0.01 * _displayWidth, 0.775 * _displayHeight, 0.35, textColor);
    _textRenderer->Render(*_mainTextShader, smoothZoomHint, 0.01 * _displayWidth, 0.75 * _displayHeight, 0.35, textColor);
    _textRenderer->Render(*_mainTextShader, movementHint, 0.01 * _displayWidth, 0.725 * _displayHeight, 0.35, textColor);
    _textRenderer->Render(*_mainTextShader, speedBostHint, 0.01 * _displayWidth, 0.7 * _displayHeight, 0.35, textColor);
    _textRenderer->Render(*_mainTextShader, starExposureHint, 0.01 * _displayWidth, 0.675 * _displayHeight, 0.35, textColor);
    _textRenderer->Render(*_mainTextShader, starGammaHint, 0.01 * _displayWidth, 0.65 * _displayHeight, 0.35, textColor);
    _textRenderer->Render(*_mainTextShader, starTemperatureHint, 0.01 * _displayWidth, 0.625 * _displayHeight, 0.35, textColor);
    _textRenderer->Render(*_mainTextShader, vertSyncHint, 0.01 * _displayWidth, 0.6 * _displayHeight, 0.35, textColor);
    _textRenderer->Render(*_mainTextShader, textHints, 0.01 * _displayWidth, 0.575 * _displayHeight, 0.35, textColor);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void Application::ConfigureMainShaders() {
    static const double zCoef = 2.0 / glm::log2(camera.GetFar() + 1.0); // For log z-buffer [для логарифмического z-буфера]

    // So that zoom does not work with skybox
    static const glm::mat4 skyBoxProjection = glm::perspective(glm::radians(45.0f), camera.GetAspect(), camera.GetNear(), camera.GetFar());

    _cameraProjection = camera.GetProjectionMatrix();
    _cameraView = camera.GetViewMatrix();

    _mainSkyBoxShader->Use();
    _mainSkyBoxShader->SetMat4("view", glm::mat4(glm::mat3(_cameraView)));
    _mainSkyBoxShader->SetMat4("projection", skyBoxProjection);

    _mainTextShader->Use();
    _mainTextShader->SetMat4("projection", _cameraProjection);
    _mainTextShader->SetMat4("view", _cameraView);
    _mainTextShader->SetInt("text", 0);

    _mainStarShader->Use();
    _mainStarShader->SetMat4("projection", _cameraProjection);
    _mainStarShader->SetMat4("view", _cameraView);
    _mainStarShader->SetVec3("centerDir", glm::normalize(camera.GetPosition() - _sun->GetPosition()));
    _mainStarShader->SetVec3("shiftStarColor", _sun->GetShiftColor());
    _mainStarShader->SetVec3("colorMult", glm::vec3(0.96862745, 0.58039215, 0.235294117) * _sun->GetShiftColor()); // 247, 148, 60
    _mainStarShader->SetFloat("sunTemperatureInKelvin", _sun->GetStarTemperatureInKelvin());
    _mainStarShader->SetFloat("starRadiusInKilometers", _sun->GetStarRadius());
    _mainStarShader->SetFloat("zCoef", zCoef);
    _mainStarShader->SetFloat("uColorMap", _sun->GetTemperatureColorUCoordinate());
    _mainStarShader->SetBool("isVisible", _sun->GetVisibility() == 1.0);
    _mainStarShader->SetInt("colorMap", 0);
    glBindTextureUnit(0, _sun->GetStarSpectrumTexture());

    _mainCoronaStarShader->Use();
    _mainCoronaStarShader->SetMat4("projection", _cameraProjection);
    _mainCoronaStarShader->SetMat4("view", _cameraView);
    _mainCoronaStarShader->SetVec3("center", _sun->GetPosition());
    _mainCoronaStarShader->SetVec3("cameraRight", camera.GetRightVector());
    _mainCoronaStarShader->SetVec3("cameraUp", camera.GetUpVector());
    _mainCoronaStarShader->SetVec3("starShiftColor", _sun->GetShiftColor());
    _mainCoronaStarShader->SetFloat("zCoef", zCoef);
    _mainCoronaStarShader->SetFloat("maxSize", 7.1);
    _mainCoronaStarShader->SetFloat("starRadius", _sun->GetStarRadius());
    _mainCoronaStarShader->SetFloat("deltaTime", glfwGetTime() * 0.002);

    _mainPlanetShader->Use();
    _mainPlanetShader->SetMat4("projection", _cameraProjection);
    _mainPlanetShader->SetMat4("view", _cameraView);
    _mainPlanetShader->SetVec3("viewPos", camera.GetPosition());
    _mainPlanetShader->SetVec3("lightPos", _sun->GetPosition());
    _mainPlanetShader->SetVec3("starGlowTint", _sun->GetGlowTintMult());
    _mainPlanetShader->SetFloat("farPlane", camera.GetFar());
    _mainPlanetShader->SetFloat("zCoef", zCoef);
    _mainPlanetShader->SetFloat("bias", 0.0005);
    _mainPlanetShader->SetInt("shadowMap", 6);
    glBindTextureUnit(6, _shadowMapFBO->GetShadowMap());

    _mainAtmosphereShader->Use();
    _mainAtmosphereShader->SetMat4("projection", _cameraProjection);
    _mainAtmosphereShader->SetMat4("view", _cameraView);
    _mainAtmosphereShader->SetFloat("farPlane", camera.GetFar());
    _mainAtmosphereShader->SetFloat("zCoef", zCoef);
    _mainAtmosphereShader->SetFloat("bias", 0.001);
    _mainAtmosphereShader->SetInt("shadowMap", 11);
    glBindTextureUnit(11, _shadowMapFBO->GetShadowMap());

    _mainCloudsShader->Use();
    _mainCloudsShader->SetMat4("projection", _cameraProjection);
    _mainCloudsShader->SetMat4("view", _cameraView);
    _mainCloudsShader->SetVec3("viewPos", camera.GetPosition());
    _mainCloudsShader->SetVec3("lightPos", _sun->GetPosition());
    _mainCloudsShader->SetFloat("farPlane", camera.GetFar());
    _mainCloudsShader->SetFloat("zCoef", zCoef);
    _mainCloudsShader->SetFloat("bias", 0.001);
    _mainCloudsShader->SetInt("shadowMap", 8);
    glBindTextureUnit(8, _shadowMapFBO->GetShadowMap());

    _mainRingShader->Use();
    _mainRingShader->SetMat4("projection", _cameraProjection);
    _mainRingShader->SetMat4("view", _cameraView);
    _mainRingShader->SetVec3("lightPos", _sun->GetPosition());
    _mainRingShader->SetVec3("camPos", camera.GetPosition());
    _mainRingShader->SetVec3("starGlowTint", _sun->GetGlowTintMult());
    _mainRingShader->SetFloat("zCoef", zCoef);
    _mainRingShader->SetFloat("bias", 0.001);
    _mainRingShader->SetInt("shadowMap", 5);
    glBindTextureUnit(5, _shadowMapFBO->GetShadowMap());
}

void Application::ConfigureMainPlanetShader(const RenderableSceneComponent& renderableComponent) {
    _mainPlanetShader->SetMat4("lightSpaceMatrix", renderableComponent.lightSpaceMatrix);
    _mainPlanetShader->SetBool("isNearbyPlanetaryRing", renderableComponent.planetaryRing != nullptr);

    if (renderableComponent.clouds)
        _mainPlanetShader->SetFloat("yRotation", renderableComponent.clouds->GetLastRotationAngle() - renderableComponent.planet->GetLastRotationAngle());

    if (renderableComponent.planetaryRing) {
        _mainPlanetShader->SetVec3("parentPlanetCenter", renderableComponent.planet->GetPosition());
        _mainPlanetShader->SetFloat("parentPlanetRadiusSquared", renderableComponent.planet->GetRadius() * renderableComponent.planet->GetRadius());

        _mainPlanetShader->SetVec3("ringCenter", renderableComponent.planetaryRing->GetPosition());
        _mainPlanetShader->SetVec3("ringNormal", renderableComponent.planetaryRing->GetRingNormal());
        _mainPlanetShader->SetVec2("ringInnerOuterRadiuses", glm::vec2(renderableComponent.planetaryRing->GetInnerRadius(),
                                                                       renderableComponent.planetaryRing->GetOuterRadius()));
        _mainPlanetShader->SetInt("ringDiffuse", 12);
        glBindTextureUnit(12, renderableComponent.planetaryRing->GetRingTexture());
    }
}

void Application::UpdateOcclusionQuery() {
    // Idea: a single query will tell us how many pixels passed, but we also need to know how many pixels are visible when
    // the object isn't occluded so that we can determine what fraction of the pixels passed the test.
    // For this reason, we will actually do 2 occlusion tests. One will turn off depth testing so that it always passes, and
    // the other will enable depth testing. We can determine what fraction of the star is visible by taking passedSamples / totalSamples and
    // use it to shrink or increase the lens flare and glow as the star gets occluded by an object.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // [Идея: один запрос скажет нам, сколько пикселей прошло, но нам также нужно знать, сколько пикселей видно, когда объект не закрыт,
    // чтобы мы могли определить, какая часть пикселей прошла тест. По этой причине мы фактически проводим 2 теста на окклюзию.
    // Один отключит проверку глубины, чтобы она всегда проходила, а другой включит проверку глубины. Мы можем определить, какая часть звезды видна,
    // взяв passedSamples / totalSamples и используя их для уменьшения или увеличения бликов и свечения линз, когда звезда закрывается объектом.]

    if (_sun->GetStarOcclusionValue(0) == 0) {
        glGenQueries(2, _sun->GetStarOcclusion().data());
    }
    else {
        GLint totalSamples = 0;
        GLint passedSamples = 0;

        glGetQueryObjectiv(_sun->GetStarOcclusionValue(0), GL_QUERY_RESULT, &totalSamples);
        glGetQueryObjectiv(_sun->GetStarOcclusionValue(1), GL_QUERY_RESULT, &passedSamples);

        if (passedSamples == 0) {
            _sun->SetVisibility(0.0f);
        }
        else {
            _sun->SetVisibility(static_cast<float>(passedSamples) / static_cast<float>(totalSamples));
        }
    }
}

float Application::CalculateSpaceObjectDistance(const SpaceObject* spaceObject) const {
    return glm::length(spaceObject->GetPosition() - camera.GetPosition());
}

glm::vec3 Application::CurrentFpsColor() const {
    const auto fpsCount = _fpsHandler.GetCurrentFps();

    if (fpsCount >= 59)
        return {0, 0.694117, 0.270588};
    else if (fpsCount >= 45 && fpsCount < 59)
        return {1, 1, 0};
    else if ((fpsCount >= 30 && fpsCount < 45))
        return {0.996078, 0.760784, 0};
    else
        return {0.96470588, 0, 0};
}

void Application::InitSystems() {
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4 /*32*/); // Scenes with individual planets (e.g. only Earth, only Saturn, etc.) use msaa x32
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    _displayWidth = glfwGetVideoMode(glfwGetPrimaryMonitor())->width;
    _displayHeight = glfwGetVideoMode(glfwGetPrimaryMonitor())->height;

    _mainWindow = glfwCreateWindow(_displayWidth, _displayHeight, "SolarSystem", nullptr, nullptr);

    if (_mainWindow == nullptr) {
        glfwTerminate();
        throw runtime_error("Failed to create GLFW window");
    }

    glfwSetInputMode(_mainWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwMakeContextCurrent(_mainWindow);
    glfwSetFramebufferSizeCallback(_mainWindow, FramebufferSizeCallback);
    glfwSetCursorPosCallback(_mainWindow, MouseCallback);
    glfwSetScrollCallback(_mainWindow, ScrollCallback);
    glfwSetKeyCallback(_mainWindow, KeyCallback);

    glewExperimental = true;
    glewInit();

    FT_Init_FreeType(&_ft);

    _soundEngine = createIrrKlangDevice(ESOD_AUTO_DETECT, ESEO_MULTI_THREADED | ESEO_LOAD_PLUGINS);
    if (!_soundEngine) {
        glfwTerminate();
        throw runtime_error("Failed to init sound engine");
    }
    _soundEngine->setSoundVolume(0.3); // 30% by default

    if (SDL_Init(SDL_INIT_EVERYTHING)) {
        Dispose();
        throw runtime_error("Failed to init SDL");
    }

    if (!IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG)) {
        Dispose();
        throw runtime_error("Failed to init SDL_Image");
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
    glEnable(GL_POLYGON_SMOOTH);
    glCullFace(GL_BACK);

    LoadWindowIcon();
    DisplaySystemInformation();
}

void Application::InitScene() {
    camera.SetAspect(static_cast<float>(_displayWidth) / static_cast<float>(_displayHeight));
    _shadowMapFBO = make_unique<ShadowMapFBO>(3000, 3000); // Planets one by one use 6000x6000
    _hdr = make_unique<HDR>(Shader("../resource/shaders/passThrough.vs", "../resource/shaders/hdr.fs"), _displayWidth, _displayHeight);

    const vector<string> skyBoxFaces = {
            "../resource/textures/Main SkyBox/PositiveX.dds",
            "../resource/textures/Main SkyBox/NegativeX.dds",
            "../resource/textures/Main SkyBox/PositiveY.dds",
            "../resource/textures/Main SkyBox/NegativeY.dds",
            "../resource/textures/Main SkyBox/PositiveZ.dds",
            "../resource/textures/Main SkyBox/NegativeZ.dds"
    };

    _skyBox = make_unique<SkyBox>(skyBoxFaces);
    _mainTextShader = make_unique<Shader>("../resource/shaders/text.vs", "../resource/shaders/text.fs");
    _textRenderer = make_unique<TextRenderer>(_ft, "../resource/fonts/Arial.ttf");
    FT_Done_FreeType(_ft);
    _shadowMapShader = make_unique<Shader>("../resource/shaders/shadowMap.vs", "../resource/shaders/shadowMap.fs");
    _mainSkyBoxShader = make_unique<Shader>("../resource/shaders/skyBox.vs", "../resource/shaders/skyBox.fs");
    _mainStarShader = make_unique<Shader>("../resource/shaders/star.vs", "../resource/shaders/star.fs");
    _mainCoronaStarShader = make_unique<Shader>("../resource/shaders/starCorona.vs", "../resource/shaders/starCorona.fs");
    _mainPlanetShader = make_unique<Shader>("../resource/shaders/planetLighting.vs", "../resource/shaders/planetLighting.fs");
    _mainAtmosphereShader = make_unique<Shader>("../resource/shaders/atmosphere.vs", "../resource/shaders/atmosphere.fs");
    _mainCloudsShader = make_unique<Shader>("../resource/shaders/planetLighting.vs", "../resource/shaders/cloudsLighting.fs");
    _mainRingShader = make_unique<Shader>("../resource/shaders/planetaryRingLighting.vs", "../resource/shaders/planetaryRingLighting.fs");
    _lensFlare = make_unique<LensFlare>(Shader("../resource/shaders/lensFlare.vs", "../resource/shaders/lensFlare.fs"), TextureImage2D("../resource/textures/flares_bright.dds"),
            FlaresInfo {4,
            {
                FlareSprite{false, 1.0, 7.0, 0},
                FlareSprite{false, 1.35, 0.3, 1},
                FlareSprite{false, 1.5, 0.4, 4},
                FlareSprite{false, 1.7, 0.6, 5},
                FlareSprite{false, 1.9, 1.2, 6},
                FlareSprite{false, 2.1, 0.4, 2},
                FlareSprite{false, 2.25, 0.2, 3},
                FlareSprite{false, 2.75, 2.0, 7}
            }});

    InitSongList();
    InitStarSystem();

    glfwShowWindow(_mainWindow);
    glfwSetWindowMonitor(_mainWindow, glfwGetPrimaryMonitor(), 0, 0, _displayWidth, _displayHeight, GLFW_DONT_CARE);

    StartSearchNearestPlanet();
    StartPlayBackgroundMusic();
}

void Application::InitSongList() {
    _backgroundSongs = vector<string_view> {
            "../resource/sounds/Stellardrone - Galaxies.mp3",
            "../resource/sounds/Stellardrone - Mars.mp3",
            "../resource/sounds/Stellardrone - Billions And Billions.mp3",
            "../resource/sounds/Stellardrone - Gravitation (Remix).mp3",
            "../resource/sounds/Stellardrone - The Edge of Forever.mp3"
    };

    // So that the song sequence is different for each program start
    //shuffle(_backgroundSongs.begin(), _backgroundSongs.end(), random_device());

    // For old gcc
    // https://stackoverflow.com/questions/34680805/why-is-random-library-producing-the-same-results-every-time-when-using-stdun
     default_random_engine randEngine(static_cast<uint32_t>(chrono::high_resolution_clock::now().time_since_epoch().count()));
     shuffle(_backgroundSongs.begin(), _backgroundSongs.end(), randEngine); // So that the song sequence is different for each program start
}

void Application::InitStarSystem() {
    MeshHolder sphereModel("../resource/models/sphere.obj");

    StarInfo sunInfo(sphereModel, *_mainStarShader, Shader("../resource/shaders/starGlow.vs", "../resource/shaders/starGlow.fs"), TextureImage2D("../resource/textures/Star_Spectrum.dds"),
                     starTemperatureInKelvin, 696342.0, glm::vec3(0.99607843, 0.890196078, 0.725490196), L"Sun", L"Солнце"); // rgb(254, 227, 185)
    _sun = make_shared<Sun>(sunInfo);

    // Initialization in such an order that there is enough virtual memory to initialize all textures (to avoid bad_alloc)
    InitNeptuneSystem(sphereModel);
    InitMercury(sphereModel);
    InitVenus(sphereModel);
    InitMarsSystem(sphereModel);
    InitEarthSystem(sphereModel);
    InitJupiterSystem(sphereModel);
    InitUranusSystem(sphereModel);
    InitSaturnSystem(sphereModel);
    InitPlutoSystem(sphereModel);
}

void Application::InitMercury(const MeshHolder& sphereModel) {
    PlanetInfo mercuryInfo(sphereModel, 0.38, *_mainPlanetShader,
            {
                TextureImage2D("../resource/textures/Mercury_Diffuse.dds"),
            }, TextureImage2D("../resource/textures/Mercury_Normal.dds"), L"Mercury", L"Меркурий", TextureImage2D("../resource/textures/Mercury_Specular.dds"));
    shared_ptr<Planet> mercury = make_shared<Mercury>(mercuryInfo, _sun);

    const glm::mat4 lightProjection = glm::ortho(-mercury->GetRadius() * 3.0f, mercury->GetRadius() * 3.0f, -mercury->GetRadius() * 3.0f, mercury->GetRadius() * 3.0f, camera.GetNear(), camera.GetFar());
    const glm::mat4 lightView = glm::lookAt(_sun->GetPosition(), mercury->GetPosition() - _sun->GetPosition(), glm::vec3(0.0, 1.0, 0.0));
    const glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    RenderableSceneComponent mercurySystemComponent;
    mercurySystemComponent.lightSpaceMatrix = lightSpaceMatrix;
    mercurySystemComponent.planet = move(mercury);
    _renderableSceneComponents.push_back(move(mercurySystemComponent));
}

void Application::InitVenus(const MeshHolder& sphereModel) {
    PlanetInfo venusInfo(sphereModel, 0.95, *_mainPlanetShader,
            {
                TextureImage2D("../resource/textures/Venus_Diffuse.dds"),
            }, TextureImage2D("../resource/textures/Venus_Normal.dds"), L"Venus", L"Венера");
    shared_ptr<Planet> venus = make_shared<Venus>(venusInfo, _sun);

    AtmosphereInfo venusAtmosphereInfo(sphereModel, *_mainAtmosphereShader, 1.1, glm::vec3(203/255.f, 158/255.f, 69/255.), venus->GetRadius() - 0.00007, 1.995);
    unique_ptr<Atmosphere> venusAtmosphere = make_unique<Atmosphere>(venusAtmosphereInfo, venus);

    const glm::mat4 lightProjection = glm::ortho(-venus->GetRadius() * 3.0f, venus->GetRadius() * 3.0f, -venus->GetRadius() * 3.0f, venus->GetRadius() * 3.0f, camera.GetNear(), camera.GetFar());
    const glm::mat4 lightView = glm::lookAt(_sun->GetPosition(), venus->GetPosition() - _sun->GetPosition(), glm::vec3(0.0, 1.0, 0.0));
    const glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    RenderableAtmosphere renderableVenusAtmosphere;
    renderableVenusAtmosphere.atmosphere = move(venusAtmosphere);
    renderableVenusAtmosphere.hScaleFactor = 6.0;
    renderableVenusAtmosphere.parentEarthSizeCoefficient = venus->GetEarthSizeCoefficient();

    RenderableSceneComponent venusSystemComponent;
    venusSystemComponent.lightSpaceMatrix = lightSpaceMatrix;
    venusSystemComponent.planet = move(venus);
    venusSystemComponent.atmospheres.push_back(move(renderableVenusAtmosphere));
    _renderableSceneComponents.push_back(move(venusSystemComponent));
}

void Application::InitEarthSystem(const MeshHolder& sphereModel) {
    PlanetInfo earthInfo(sphereModel, 1.0, *_mainPlanetShader,
            {
                TextureImage2D("../resource/textures/Earth_Day_Diffuse.dds"),
                TextureImage2D("../resource/textures/Earth_Clouds_Diffuse.dds"),
                TextureImage2D("../resource/textures/Earth_Night_Diffuse.dds"),
            }, TextureImage2D("../resource/textures/Earth_Normal.dds"), L"Earth", L"Земля", TextureImage2D("../resource/textures/Earth_Specular.dds"));
    shared_ptr<Planet> earth = make_shared<Earth>(earthInfo, _sun);

    SatelliteInfo moonInfo(sphereModel, 0.2724, *_mainPlanetShader, {TextureImage2D("../resource/textures/Moon_Diffuse.dds")}, TextureImage2D("../resource/textures/Moon_Normal.dds"),
                           L"Moon", L"Луна");
    shared_ptr<Satellite> moon = make_shared<Moon>(moonInfo, earth);

    AtmosphereInfo earthAtmosphereInfo(sphereModel, *_mainAtmosphereShader, 1.1, glm::vec3(0.3, 0.7, 1.0), earth->GetRadius() - 0.00007, 2.1);
    unique_ptr<Atmosphere> earthAtmosphere = make_unique<Atmosphere>(earthAtmosphereInfo, earth);

    CloudsInfo earthCloudsInfo(sphereModel, *_mainCloudsShader, 1.0055, TextureImage2D("../resource/textures/Earth_Clouds_Diffuse.dds"),
                               TextureImage2D("../resource/textures/Earth_Clouds_Normal.dds"));
    unique_ptr<Clouds> earthClouds = make_unique<EarthClouds>(earthCloudsInfo, earth);

    const glm::mat4 lightProjection = glm::ortho(-earth->GetRadius() * 3.0f, earth->GetRadius() * 3.0f, -earth->GetRadius() * 3.0f, earth->GetRadius() * 3.0f, camera.GetNear(), camera.GetFar());
    const glm::mat4 lightView = glm::lookAt(_sun->GetPosition(), earth->GetPosition() - _sun->GetPosition(), glm::vec3(0.0, 1.0, 0.0));
    const glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    RenderableAtmosphere renderableEarthAtmosphere;
    renderableEarthAtmosphere.atmosphere = move(earthAtmosphere);
    renderableEarthAtmosphere.hScaleFactor = 6.0;
    renderableEarthAtmosphere.parentEarthSizeCoefficient = earth->GetEarthSizeCoefficient();

    RenderableSceneComponent earthSystemComponent;
    earthSystemComponent.lightSpaceMatrix = lightSpaceMatrix;
    earthSystemComponent.planet = move(earth);
    earthSystemComponent.satellites = vector<shared_ptr<Satellite>>{move(moon)};
    earthSystemComponent.atmospheres.push_back(move(renderableEarthAtmosphere));
    earthSystemComponent.clouds = move(earthClouds);
    _renderableSceneComponents.push_back(move(earthSystemComponent));
}

void Application::InitMarsSystem(const MeshHolder& sphereModel) {
    MeshHolder phobosModel("../resource/models/phobos.obj"), deimosModel("../resource/models/deimos.obj");

    PlanetInfo marsInfo(sphereModel, 0.53, *_mainPlanetShader,
            {
                TextureImage2D("../resource/textures/Mars_Diffuse.dds"),
            }, TextureImage2D("../resource/textures/Mars_Normal.dds"), L"Mars", L"Марс");
    shared_ptr<Planet> mars = make_shared<Mars>(marsInfo, _sun);

    SatelliteInfo phobosInfo(phobosModel, 0.001768, *_mainPlanetShader, {TextureImage2D("../resource/textures/Phobos_Diffuse.dds")}, TextureImage2D("../resource/textures/Phobos_Normal.dds"),
                             L"Phobos", L"Фобос");
    SatelliteInfo deimosInfo(deimosModel, 0.00097316, *_mainPlanetShader, {TextureImage2D("../resource/textures/Deimos_Diffuse.dds")}, TextureImage2D("../resource/textures/Deimos_Normal.dds"),
                             L"Deimos", L"Деймос");
    shared_ptr<Satellite> phobos = make_shared<Phobos>(phobosInfo, mars);
    shared_ptr<Satellite> deimos = make_shared<Deimos>(deimosInfo, mars);

    AtmosphereInfo marsAtmosphereInfo(sphereModel, *_mainAtmosphereShader, 0.583, glm::vec3(0.976, 0.302, 0.208), mars->GetRadius() - 0.00007, 1.113);
    unique_ptr<Atmosphere> marsAtmosphere = make_unique<Atmosphere>(marsAtmosphereInfo, mars);

    const glm::mat4 lightProjection = glm::ortho(-mars->GetRadius() * 3.0f, mars->GetRadius() * 3.0f, -mars->GetRadius() * 3.0f, mars->GetRadius() * 3.0f, camera.GetNear(),
                                                 glm::length(_sun->GetPosition() - mars->GetPosition()) + 50.f);
    const glm::mat4 lightView = glm::lookAt(_sun->GetPosition(), mars->GetPosition() - _sun->GetPosition(), glm::vec3(0.0, 1.0, 0.0));
    const glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    RenderableAtmosphere renderableMarsAtmosphere;
    renderableMarsAtmosphere.atmosphere = move(marsAtmosphere);
    renderableMarsAtmosphere.hScaleFactor = 6.0;
    renderableMarsAtmosphere.parentEarthSizeCoefficient = mars->GetEarthSizeCoefficient();

    RenderableSceneComponent marsSystemComponent;
    marsSystemComponent.lightSpaceMatrix = lightSpaceMatrix;
    marsSystemComponent.planet = move(mars);
    marsSystemComponent.satellites = vector<shared_ptr<Satellite>>{move(phobos), move(deimos)};
    marsSystemComponent.atmospheres.push_back(move(renderableMarsAtmosphere));
    _renderableSceneComponents.push_back(move(marsSystemComponent));
}

void Application::InitJupiterSystem(const MeshHolder& sphereModel) {
    PlanetInfo jupiterInfo(sphereModel, 11.2, *_mainPlanetShader,
            {
                TextureImage2D("../resource/textures/Jupiter_Diffuse.dds"),
            }, TextureImage2D("../resource/textures/Jupiter_Normal.dds"), L"Jupiter", L"Юпитер");
    shared_ptr<Planet> jupiter = make_shared<Jupiter>(jupiterInfo, _sun);

    SatelliteInfo ioInfo(sphereModel, 0.28592, *_mainPlanetShader, {TextureImage2D("../resource/textures/Io_Diffuse.dds")}, TextureImage2D("../resource/textures/Io_Normal.dds"),
                         L"Io", L"Ио");
    SatelliteInfo europaInfo(sphereModel, 0.244985, *_mainPlanetShader, {TextureImage2D("../resource/textures/Europa_Diffuse.dds")}, TextureImage2D("../resource/textures/Europa_Normal.dds"),
                             L"Europa", L"Европа");
    SatelliteInfo ganymedeInfo(sphereModel, 0.41345, *_mainPlanetShader, {TextureImage2D("../resource/textures/Ganymede_Diffuse.dds")}, TextureImage2D("../resource/textures/Ganymede_Normal.dds"),
                               L"Ganymede", L"Ганимед");
    SatelliteInfo callistoInfo(sphereModel, 0.3783236, *_mainPlanetShader, {TextureImage2D("../resource/textures/Callisto_Diffuse.dds")}, TextureImage2D("../resource/textures/Callisto_Normal.dds"),
                               L"Callisto", L"Каллисто");
    shared_ptr<Satellite> io = make_shared<Io>(ioInfo, jupiter);
    shared_ptr<Satellite> europa = make_shared<Europa>(europaInfo, jupiter);
    shared_ptr<Satellite> ganymede = make_shared<Ganymede>(ganymedeInfo, jupiter);
    shared_ptr<Satellite> callisto = make_shared<Callisto>(callistoInfo, jupiter);

    AtmosphereInfo jupiterAtmosphereInfo(sphereModel, *_mainAtmosphereShader, 11.4, glm::vec3(153.f/255, 139.f/255, 120.f/255), jupiter->GetRadius() - 0.00007, 23.35);
    unique_ptr<Atmosphere> jupiterAtmosphere = make_unique<Atmosphere>(jupiterAtmosphereInfo, jupiter);

    const glm::mat4 lightProjection = glm::ortho(-jupiter->GetRadius() * 3.0f, jupiter->GetRadius() * 3.0f, -jupiter->GetRadius() * 3.0f, jupiter->GetRadius() * 3.0f, camera.GetNear(), camera.GetFar());
    const glm::mat4 lightView = glm::lookAt(_sun->GetPosition(), jupiter->GetPosition() - _sun->GetPosition(), glm::vec3(0.0, 1.0, 0.0));
    const glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    RenderableAtmosphere renderableJupiterAtmosphere;
    renderableJupiterAtmosphere.atmosphere = move(jupiterAtmosphere);
    renderableJupiterAtmosphere.hScaleFactor = 26.0;
    renderableJupiterAtmosphere.parentEarthSizeCoefficient = jupiter->GetEarthSizeCoefficient();
    renderableJupiterAtmosphere.isUseToneMapping = true;

    RenderableSceneComponent jupiterSystemComponent;
    jupiterSystemComponent.lightSpaceMatrix = lightSpaceMatrix;
    jupiterSystemComponent.planet = move(jupiter);
    jupiterSystemComponent.satellites = vector<shared_ptr<Satellite>>{move(io), move(europa), move(ganymede), move(callisto)};
    jupiterSystemComponent.atmospheres.push_back(move(renderableJupiterAtmosphere));
    _renderableSceneComponents.push_back(move(jupiterSystemComponent));
}

void Application::InitSaturnSystem(const MeshHolder& sphereModel) {
    MeshHolder saturnRingModel("../resource/models/saturn_ring.obj");

    PlanetInfo saturnInfo(sphereModel, 9.14, *_mainPlanetShader,
            {
                TextureImage2D("../resource/textures/Saturn_Diffuse.dds"),
            }, TextureImage2D("../resource/textures/Saturn_Normal.dds"), L"Saturn", L"Сатурн");
    shared_ptr<Planet> saturn = make_shared<Saturn>(saturnInfo, _sun);

    PlanetaryRingInfo saturnRingInfo(saturnRingModel, 22.0, 43.7, *_mainPlanetShader, TextureImage2D("../resource/textures/Saturn_Rings.dds")); // Radiuses from 3D model
    unique_ptr<PlanetaryRing> saturnRing = make_unique<SaturnRing>(saturnRingInfo, saturn);

    SatelliteInfo mimasInfo(sphereModel, 0.03111, *_mainPlanetShader, {TextureImage2D("../resource/textures/Mimas_Diffuse.dds")}, TextureImage2D("../resource/textures/Mimas_Normal.dds"),
                            L"Mimas", L"Мимас");
    SatelliteInfo enceladusInfo(sphereModel, 0.03957, *_mainPlanetShader, {TextureImage2D("../resource/textures/Enceladus_Diffuse.dds")}, TextureImage2D("../resource/textures/Enceladus_Normal.dds"),
                            L"Enceladus", L"Энцелад");
    SatelliteInfo tethysInfo(sphereModel, 0.083346, *_mainPlanetShader, {TextureImage2D("../resource/textures/Tethys_Diffuse.dds")}, TextureImage2D("../resource/textures/Tethys_Normal.dds"),
                            L"Tethys", L"Тефия");
    SatelliteInfo dioneInfo(sphereModel, 0.08812, *_mainPlanetShader, {TextureImage2D("../resource/textures/Dione_Diffuse.dds")}, TextureImage2D("../resource/textures/Dione_Normal.dds"),
                            L"Dione", L"Диона");
    SatelliteInfo rheaInfo(sphereModel, 0.119886, *_mainPlanetShader, {TextureImage2D("../resource/textures/Rhea_Diffuse.dds")}, TextureImage2D("../resource/textures/Rhea_Normal.dds"),
                            L"Rhea", L"Рея");
    SatelliteInfo titanInfo(sphereModel, 0.404136, *_mainPlanetShader, {TextureImage2D("../resource/textures/Titan_Diffuse.dds")}, TextureImage2D("../resource/textures/Titan_Normal.dds"),
                            L"Titan", L"Титан");
    SatelliteInfo iapetusInfo(sphereModel, 0.115288, *_mainPlanetShader, {TextureImage2D("../resource/textures/Iapetus_Diffuse.dds")}, TextureImage2D("../resource/textures/Iapetus_Normal.dds"),
                            L"Iapetus", L"Япет");
    shared_ptr<Satellite> mimas = make_shared<Mimas>(mimasInfo, saturn);
    shared_ptr<Satellite> enceladus = make_shared<Enceladus>(enceladusInfo, saturn);
    shared_ptr<Satellite> tethys = make_shared<Tethys>(tethysInfo, saturn);
    shared_ptr<Satellite> dione = make_shared<Dione>(dioneInfo, saturn);
    shared_ptr<Satellite> rhea = make_shared<Rhea>(rheaInfo, saturn);
    shared_ptr<Satellite> titan = make_shared<Titan>(titanInfo, saturn);
    shared_ptr<Satellite> iapetus = make_shared<Iapetus>(iapetusInfo, saturn);

    AtmosphereInfo saturnAtmosphereInfo(sphereModel, *_mainAtmosphereShader, 9.34, glm::vec3(84.f/255, 132.f/255, 176.f/255), saturn->GetRadius() - 0.00007, 18.6);
    unique_ptr<Atmosphere> saturnAtmosphere = make_unique<Atmosphere>(saturnAtmosphereInfo, saturn);

    AtmosphereInfo titanAtmosphereInfo(sphereModel, *_mainAtmosphereShader, 0.504136, glm::vec3(40.f/255, 33.f/255, 72.f/255), titan->GetRadius() - 0.00007, 0.8429210,
                                       glm::vec3(0.36862745, 0.0666667, 0.0196078)); // Mie tint rgb(94, 17, 5));
    unique_ptr<Atmosphere> titanAtmosphere = make_unique<Atmosphere>(titanAtmosphereInfo, titan);

    const glm::mat4 lightProjection = glm::ortho(-saturn->GetRadius() * 3.0f, saturn->GetRadius() * 3.0f, -saturn->GetRadius() * 3.0f, saturn->GetRadius() * 3.0f, camera.GetNear(), camera.GetFar());
    const glm::mat4 lightView = glm::lookAt(_sun->GetPosition(), saturn->GetPosition() - _sun->GetPosition(), glm::vec3(0.0, 1.0, 0.0));
    const glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    RenderableAtmosphere renderableSaturnAtmosphere;
    renderableSaturnAtmosphere.atmosphere = move(saturnAtmosphere);
    renderableSaturnAtmosphere.hScaleFactor = 27.0;
    renderableSaturnAtmosphere.parentEarthSizeCoefficient = saturn->GetEarthSizeCoefficient();
    renderableSaturnAtmosphere.isUseToneMapping = true;

    RenderableAtmosphere renderableTitanAtmosphere;
    renderableTitanAtmosphere.atmosphere = move(titanAtmosphere);
    renderableTitanAtmosphere.hScaleFactor = 4.8;
    renderableTitanAtmosphere.parentEarthSizeCoefficient = titan->GetEarthSizeCoefficient();

    RenderableSceneComponent saturnSystemComponent;
    saturnSystemComponent.lightSpaceMatrix = lightSpaceMatrix;
    saturnSystemComponent.planet = move(saturn);
    saturnSystemComponent.satellites = vector<shared_ptr<Satellite>>{move(mimas), move(enceladus), move(tethys), move(dione), move(rhea), move(titan), move(iapetus)};
    saturnSystemComponent.atmospheres.push_back(move(renderableSaturnAtmosphere));
    saturnSystemComponent.atmospheres.push_back(move(renderableTitanAtmosphere));
    saturnSystemComponent.planetaryRing = move(saturnRing);
    _renderableSceneComponents.push_back(move(saturnSystemComponent));
}

void Application::InitUranusSystem(const MeshHolder& sphereModel) {
    MeshHolder uranusRingModel("../resource/models/uranus_ring.obj");

    PlanetInfo uranusInfo(sphereModel, 3.98085, *_mainPlanetShader,
            {
                TextureImage2D("../resource/textures/Uranus_Diffuse.dds"),
                TextureImage2D("../resource/textures/Uranus_Clouds_Diffuse.dds")
            }, TextureImage2D("../resource/textures/Uranus_Normal.dds"), L"Uranus", L"Уран");
    shared_ptr<Planet> uranus = make_shared<Uranus>(uranusInfo, _sun);
    PlanetaryRingInfo uranusRingInfo(uranusRingModel, 12.6, 16.0, *_mainPlanetShader, TextureImage2D("../resource/textures/Uranus_Rings.dds")); // Radiuses from 3D model
    unique_ptr<PlanetaryRing> uranusRing = make_unique<UranusRing>(uranusRingInfo, uranus);

    SatelliteInfo mirandaInfo(sphereModel, 0.0368858, *_mainPlanetShader, {TextureImage2D("../resource/textures/Miranda_Diffuse.dds")}, TextureImage2D("../resource/textures/Miranda_Normal.dds"),
                            L"Miranda", L"Миранда");
    SatelliteInfo arielInfo(sphereModel, 0.090865, *_mainPlanetShader, {TextureImage2D("../resource/textures/Ariel_Diffuse.dds")}, TextureImage2D("../resource/textures/Ariel_Normal.dds"),
                            L"Ariel", L"Ариэль");
    SatelliteInfo umbrielInfo(sphereModel, 0.091775, *_mainPlanetShader, {TextureImage2D("../resource/textures/Umbriel_Diffuse.dds")}, TextureImage2D("../resource/textures/Umbriel_Normal.dds"),
                            L"Umbriel", L"Умбриэль");
    SatelliteInfo titaniaInfo(sphereModel, 0.123748, *_mainPlanetShader, {TextureImage2D("../resource/textures/Titania_Diffuse.dds")}, TextureImage2D("../resource/textures/Titania_Normal.dds"),
                            L"Titania", L"Титания");
    SatelliteInfo oberonInfo(sphereModel, 0.11951, *_mainPlanetShader, {TextureImage2D("../resource/textures/Oberon_Diffuse.dds")}, TextureImage2D("../resource/textures/Oberon_Normal.dds"),
                            L"Oberon", L"Оберон");
    shared_ptr<Satellite> miranda = make_shared<Miranda>(mirandaInfo, uranus);
    shared_ptr<Satellite> ariel = make_shared<Ariel>(arielInfo, uranus);
    shared_ptr<Satellite> umbriel = make_shared<Umbriel>(umbrielInfo, uranus);
    shared_ptr<Satellite> titania = make_shared<Titania>(titaniaInfo, uranus);
    shared_ptr<Satellite> oberon = make_shared<Oberon>(oberonInfo, uranus);

    CloudsInfo uranusCloudsInfo(sphereModel, *_mainCloudsShader, 3.98635, TextureImage2D("../resource/textures/Uranus_Clouds_Diffuse.dds"),
                            TextureImage2D("../resource/textures/Uranus_Clouds_Normal.dds"));
    unique_ptr<Clouds> uranusClouds = make_unique<UranusClouds>(uranusCloudsInfo, uranus);

    AtmosphereInfo uranusAtmosphereInfo(sphereModel, *_mainAtmosphereShader, 4.0, glm::vec3(45.f/255, 101.f/255, 114.f/255), uranus->GetRadius() - 0.00007, 8.1);
    unique_ptr<Atmosphere> uranusAtmosphere = make_unique<Atmosphere>(uranusAtmosphereInfo, uranus);

    const glm::mat4 lightProjection = glm::ortho(-uranus->GetRadius() * 3.0f, uranus->GetRadius() * 3.0f, -uranus->GetRadius() * 3.0f, uranus->GetRadius() * 3.0f, camera.GetNear(), camera.GetFar());
    const glm::mat4 lightView = glm::lookAt(_sun->GetPosition(), uranus->GetPosition() - _sun->GetPosition(), glm::vec3(0.0, 1.0, 0.0));
    const glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    RenderableAtmosphere renderableUranusAtmosphere;
    renderableUranusAtmosphere.atmosphere = move(uranusAtmosphere);
    renderableUranusAtmosphere.hScaleFactor = 24.0;
    renderableUranusAtmosphere.parentEarthSizeCoefficient = uranus->GetEarthSizeCoefficient();
    renderableUranusAtmosphere.isUseToneMapping = true;

    RenderableSceneComponent uranusSystemComponent;
    uranusSystemComponent.lightSpaceMatrix = lightSpaceMatrix;
    uranusSystemComponent.planet = move(uranus);
    uranusSystemComponent.satellites = vector<shared_ptr<Satellite>>{move(miranda), move(ariel), move(umbriel), move(titania), move(oberon)};
    uranusSystemComponent.atmospheres.push_back(move(renderableUranusAtmosphere));
    uranusSystemComponent.clouds = move(uranusClouds);
    uranusSystemComponent.planetaryRing = move(uranusRing);
    _renderableSceneComponents.push_back(move(uranusSystemComponent));
}

void Application::InitNeptuneSystem(const MeshHolder& sphereModel) {
    PlanetInfo neptuneInfo(sphereModel, 3.8647, *_mainPlanetShader,
            {
                TextureImage2D("../resource/textures/Neptune_Diffuse.dds"),
                TextureImage2D("../resource/textures/Neptune_Clouds_Diffuse.dds")
            }, TextureImage2D("../resource/textures/Neptune_Normal.dds"), L"Neptune", L"Нептун");
    shared_ptr<Planet> neptune = make_shared<Neptune>(neptuneInfo, _sun);

    SatelliteInfo tritonInfo(sphereModel, 0.2724, *_mainPlanetShader, {TextureImage2D("../resource/textures/Triton_Diffuse.dds")}, TextureImage2D("../resource/textures/Triton_Normal.dds"),
                             L"Triton", L"Тритон");
    shared_ptr<Satellite> triton = make_shared<Triton>(tritonInfo, neptune);

    CloudsInfo neptuneCloudsInfo(sphereModel, *_mainCloudsShader, 3.87, TextureImage2D("../resource/textures/Neptune_Clouds_Diffuse.dds"),
                                TextureImage2D("../resource/textures/Neptune_Clouds_Normal.dds"));
    unique_ptr<Clouds> neptuneClouds = make_unique<NeptuneClouds>(neptuneCloudsInfo, neptune);

    AtmosphereInfo neptuneAtmosphereInfo(sphereModel, *_mainAtmosphereShader, 3.9, glm::vec3(62.f/255, 92.f/255, 169.f/255), neptune->GetRadius() - 0.00007, 7.9);
    unique_ptr<Atmosphere> neptuneAtmosphere = make_unique<Atmosphere>(neptuneAtmosphereInfo, neptune);

    const glm::mat4 lightProjection = glm::ortho(-neptune->GetRadius() * 3.0f, neptune->GetRadius() * 3.0f, -neptune->GetRadius() * 3.0f, neptune->GetRadius() * 3.0f, camera.GetNear(), camera.GetFar());
    const glm::mat4 lightView = glm::lookAt(_sun->GetPosition(), neptune->GetPosition() - _sun->GetPosition(), glm::vec3(0.0, 1.0, 0.0));
    const glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    RenderableAtmosphere renderableNeptuneAtmosphere;
    renderableNeptuneAtmosphere.atmosphere = move(neptuneAtmosphere);
    renderableNeptuneAtmosphere.hScaleFactor = 23.0;
    renderableNeptuneAtmosphere.parentEarthSizeCoefficient = neptune->GetEarthSizeCoefficient();
    renderableNeptuneAtmosphere.isUseToneMapping = true;

    RenderableSceneComponent neptuneSystemComponent;
    neptuneSystemComponent.lightSpaceMatrix = lightSpaceMatrix;
    neptuneSystemComponent.planet = move(neptune);
    neptuneSystemComponent.satellites = vector<shared_ptr<Satellite>>{move(triton)};
    neptuneSystemComponent.atmospheres.push_back(move(renderableNeptuneAtmosphere));
    neptuneSystemComponent.clouds = move(neptuneClouds);
    _renderableSceneComponents.push_back(move(neptuneSystemComponent));
}

void Application::InitPlutoSystem(const MeshHolder& sphereModel) {
    PlanetInfo plutoInfo(sphereModel, 0.18651, *_mainPlanetShader,
            {
                TextureImage2D("../resource/textures/Pluto_Diffuse.dds"),
            }, TextureImage2D("../resource/textures/Pluto_Normal.dds"), L"Pluto", L"Плутон", TextureImage2D("../resource/textures/Pluto_Specular.dds"));
    shared_ptr<Planet> pluto = make_shared<Pluto>(plutoInfo, _sun);

    SatelliteInfo charonInfo(sphereModel, 0.09512, *_mainPlanetShader, {TextureImage2D("../resource/textures/Charon_Diffuse.dds")}, TextureImage2D("../resource/textures/Charon_Normal.dds"),
                             L"Charon", L"Харон", TextureImage2D("../resource/textures/Charon_Specular.dds"));
    shared_ptr<Satellite> charon  = make_shared<Charon>(charonInfo, pluto);

    AtmosphereInfo plutoAtmosphereInfo(sphereModel, *_mainAtmosphereShader, 0.45, glm::vec3(92.f/255, 120.f/255, 141.f/255), pluto->GetRadius(), 1.0,
                                       glm::vec3(35.f/255, 52.f/255, 220.f/255));
    unique_ptr<Atmosphere> plutoAtmosphere = make_unique<Atmosphere>(plutoAtmosphereInfo, pluto);

    const glm::mat4 lightProjection = glm::ortho(-pluto->GetRadius() * 3.0f, pluto->GetRadius() * 3.0f, -pluto->GetRadius() * 3.0f, pluto->GetRadius() * 3.0f, camera.GetNear(), camera.GetFar());
    const glm::mat4 lightView = glm::lookAt(_sun->GetPosition(), pluto->GetPosition() - _sun->GetPosition(), glm::vec3(0.0, 1.0, 0.0));
    const glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    RenderableAtmosphere renderablePlutoAtmosphere;
    renderablePlutoAtmosphere.atmosphere = move(plutoAtmosphere);
    renderablePlutoAtmosphere.hScaleFactor = 16.0;
    renderablePlutoAtmosphere.parentEarthSizeCoefficient = pluto->GetEarthSizeCoefficient();
    renderablePlutoAtmosphere.isUseToneMapping = true;

    RenderableSceneComponent plutoSystemComponent;
    plutoSystemComponent.lightSpaceMatrix = lightSpaceMatrix;
    plutoSystemComponent.planet = move(pluto);
    plutoSystemComponent.satellites = vector<shared_ptr<Satellite>>{move(charon)};
    plutoSystemComponent.atmospheres.push_back(move(renderablePlutoAtmosphere));
    _renderableSceneComponents.push_back(move(plutoSystemComponent));
}

void Application::StartSearchNearestPlanet() {
    _isSearchNearestPlanet = true;

    auto searchNearestPlanet = [=]() {
        return min_element(_renderableSceneComponents.begin(), _renderableSceneComponents.end(),
                           [=](const RenderableSceneComponent& left, const RenderableSceneComponent& right)
        {
            return CalculateSpaceObjectDistance(left.planet.get()) < CalculateSpaceObjectDistance(right.planet.get());
        });
    };

    _searchNearestPlanetThread = make_unique<thread>([=]() {
        while (_isSearchNearestPlanet) {
            auto nearestPlanetIt = searchNearestPlanet();
            if (nearestPlanetIt != _renderableSceneComponents.end()) {
                _nearestPlanetIndex = distance(_renderableSceneComponents.begin(), nearestPlanetIt);
            }

            this_thread::sleep_for(25ms); // Optimization
        }
    });
}

void Application::StartPlayBackgroundMusic() {
    _isBackgroundMusicPlay = true;

    // Maps the value from one range to another. For example, 10 from [0; 100] to [0; 1] will map to 0.1
    auto mapRange = [](float value, float inMin, float inMax, float outMin, float outMax) {
        return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);
    };

    _backgroundMusicThread = make_unique<thread>([=]() {
        for (ssize_t i = 0; i < _backgroundSongs.size() && _isBackgroundMusicPlay; i++) {
            // At the beginning of the loop so that there is no delay when exiting the program
            this_thread::sleep_for(1s); // A second of waiting before a new song

            auto song = _soundEngine->play2D(_backgroundSongs[i].data(), false, true, true);
            song->setVolume(0);
            song->setIsPaused(false);
            _currentMusicTrack = _backgroundSongs[i].substr(19); // Remove "../resource/sounds/"

            while (!song->isFinished()) {
                // https://www.desmos.com/calculator/kbn9mql7ay
                if (song->getPlayPosition() < 5000) { // The first 5s (5000ms) the song volume increases smoothly to 1.0
                    auto x = mapRange(song->getPlayPosition(), 0.0f, 5000.0f, 0.0f, 0.7f); // About 0.7 the function is already 1.0
                    auto volume = exp(x) - 1; // Smooth increase in volume
                    song->setVolume(clamp(volume, 0.0f, 1.0f));
                }
                else if (song->getPlayPosition() > song->getPlayLength() - 5000) { // The last 5s the song volume decreases smoothly to ~0.0
                    // About 6.0 the function is already ~0.0
                    auto x = mapRange(song->getPlayPosition(), song->getPlayLength() - 5000, song->getPlayLength(), 0.0f, 6.0f);
                    auto volume = exp(-x); // Smooth decrease in volume
                    // auto x = mapRange(song->getPlayPosition(), song->getPlayLength() - 5000.0f, song->getPlayLength(), 0.0f, 0.7f);
                    // auto volume = exp(0.7f - x) - 1; // Smooth decrease in volume https://www.desmos.com/calculator/1vw3chep9a
                    song->setVolume(clamp(volume, 0.0f, 1.0f));
                }

                this_thread::sleep_for(25ms); // Optimization
            }

            if (i == _backgroundSongs.size() - 1) // Repeat songs from the beginning
                i = -1; // Will increment in the next iteration

            song->drop();
        }
    });
}

void Application::LoadWindowIcon() const {
    constexpr auto execIconPath = "../resource/icons/solarsystem-logo.png";
    SDL_Surface* windowIcon = IMG_Load(execIconPath);

    if (windowIcon == nullptr)
        throw runtime_error(string("Cannot load exe icon ") + execIconPath);

    GLFWimage image;
    image.pixels = static_cast<unsigned char*>(windowIcon->pixels);
    image.width = windowIcon->w;
    image.height = windowIcon->h;
    glfwSetWindowIcon(_mainWindow, 1, &image);
    SDL_FreeSurface(windowIcon);
}

void Application::DisplaySystemInformation() const {
    cout << "GPU Supplier: " << glGetString(GL_VENDOR) << endl;
    cout << "GPU: " << glGetString(GL_RENDERER) << endl;

    GLint majorVersion, minorVersion;
    glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
    glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
    cout << "OpenGL version: " << majorVersion << '.' << minorVersion << endl;

    GLint totalMemoryKb;
    glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &totalMemoryKb);

    GLint currentMemoryKb;
    glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &currentMemoryKb);
    cout << "Total GPU Memory: " << totalMemoryKb << " kb\nFree GPU Memory: " << currentMemoryKb << " kb" << endl;

    GLint maxTextureSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    cout << "Max texture size in the system: "<< maxTextureSize << "x" << maxTextureSize << endl;
    cout << "Driver: " << glGetString(GL_VERSION) << endl;
}

void Application::ProcessInput(GLFWwindow* window) {
    static float movementSpeed = camera.GetMovementSpeed();

    if (isFirstMouse) {
        lastX = 0;
        lastY = 0;
        isFirstMouse = false;
    }

    float xPos = lastX, yPos = lastY;
    float shiftIncrease = 1.0f, yScroll = 0;

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        shiftIncrease = 4 * camera.GetMovementSpeed();
    }
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.ProcessKeyboard(CameraVector::FORWARD, deltaTime * shiftIncrease);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.ProcessKeyboard(CameraVector::BACKWARD, deltaTime * shiftIncrease);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.ProcessKeyboard(CameraVector::LEFT, deltaTime * shiftIncrease);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.ProcessKeyboard(CameraVector::RIGHT, deltaTime * shiftIncrease);
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        camera.ProcessKeyboard(CameraVector::WORLD_UP, deltaTime * shiftIncrease);
    }
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        camera.ProcessKeyboard(CameraVector::WORLD_DOWN, deltaTime * shiftIncrease);
    }
    if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS) {
        _soundEngine->setSoundVolume(clamp(_soundEngine->getSoundVolume() + 0.01, 0.0, 1.0));
    }
    if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) {
        _soundEngine->setSoundVolume(clamp(_soundEngine->getSoundVolume() - 0.01, 0.0, 1.0));
    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        movementSpeed = glm::clamp(movementSpeed + 0.01f, 0.0f, 150.f);
        camera.SetMovementSpeed(movementSpeed);
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        movementSpeed = glm::clamp(movementSpeed - 0.01f, 0.0f, 150.f);
        camera.SetMovementSpeed(movementSpeed);
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
        starExposure = glm::clamp(starExposure + 0.1f, 0.0f, 20.f);
    }
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
        starExposure = glm::clamp(starExposure - 0.1f, 0.0f, 20.f);
    }
    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) {
        starGamma = glm::clamp(starGamma + 0.01f, 0.0f, 2.f);
    }
    if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) {
        starGamma = glm::clamp(starGamma - 0.01f, 0.0f, 2.f);
    }
    if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) {
        starTemperatureInKelvin = glm::clamp(starTemperatureInKelvin + 15.0f, 800.f, 30000.f);
    }
    if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) {
        starTemperatureInKelvin = glm::clamp(starTemperatureInKelvin - 15.0f, 800.f, 30000.f);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        xPos -= 1;
        float xOffset = xPos - lastX;
        lastX = xPos;
        camera.ProcessMouseMovement(xOffset, 0);
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        xPos += 1;
        float xOffset = xPos - lastX;
        lastX = xPos;
        camera.ProcessMouseMovement(xOffset, 0);
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        yPos -= 1;
        float yOffset = lastY - yPos; // reversed since y-coordinates go from bottom to top
        lastY = yPos;
        camera.ProcessMouseMovement(0, yOffset);
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        yPos += 1;
        float yOffset = lastY - yPos; // reversed since y-coordinates go from bottom to top
        lastY = yPos;
        camera.ProcessMouseMovement(0, yOffset);
    }
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
        yScroll += 0.16f;
        camera.ProcessMouseScroll(yScroll);
    }
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
        yScroll -= 0.16f;
        camera.ProcessMouseScroll(yScroll);
    }

    glfwSetCursorPos(window, lastX, lastY);
}

// GLFW: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void Application::FramebufferSizeCallback(GLFWwindow*, int width, int height) {
    // Make sure the viewport matches the new window dimensions; note that width and
    // Height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// GLFW: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void Application::MouseCallback(GLFWwindow*, double xPos, double yPos) {
    if (isFirstMouse) {
        lastX = xPos;
        lastY = yPos;
        isFirstMouse = false;
    }

    float xOffset = xPos - lastX;
    float yOffset = lastY - yPos; // Reversed since y-coordinates go from bottom to top

    lastX = xPos;
    lastY = yPos;

    camera.ProcessMouseMovement(xOffset, yOffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void Application::ScrollCallback(GLFWwindow*, double, double yOffset) {
    camera.ProcessMouseScroll(yOffset);
}

void Application::KeyCallback(GLFWwindow*, int key, int, int action, int) {
    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        isTimeRun = !isTimeRun;
    }
    if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
        isRenderPlanetStarDistances = !isRenderPlanetStarDistances;
    }
    if (key == GLFW_KEY_X && action == GLFW_PRESS) {
        isRenderSatelliteDistances = !isRenderSatelliteDistances;
    }
    if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
        isRenderHints = !isRenderHints;
    }
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        isVertSyncEnabled = !isVertSyncEnabled;
        VertSync(isVertSyncEnabled);
    }
}

bool Application::WGLExtensionSupported(const char* extensionName) {
    // This is pointer to function which returns pointer to string with list of all wgl extensions
    PFNWGLGETEXTENSIONSSTRINGEXTPROC wglGetExtensionsStringEXT = nullptr;

    // Determine pointer to wglGetExtensionsStringEXT function
    wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)wglGetProcAddress("wglGetExtensionsStringEXT");

    return strstr(wglGetExtensionsStringEXT(), extensionName) != nullptr;
}

void Application::VertSync(bool enable) {
    PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = nullptr;
    PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT = nullptr;

    if (WGLExtensionSupported("WGL_EXT_swap_control")) {
        // Extension is supported, init pointers.
        wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");

        // This is another function from WGL_EXT_swap_control extension
        wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)wglGetProcAddress("wglGetSwapIntervalEXT");
    }

    wglSwapIntervalEXT(enable);
}

void Application::StopSearchNearestPlanet() {
    _isSearchNearestPlanet = false;
    if (_searchNearestPlanetThread)
        _searchNearestPlanetThread->join();
}

void Application::StopPlayBackgroundMusic() {
    _isBackgroundMusicPlay = false;
    _soundEngine->stopAllSounds();
    if (_backgroundMusicThread)
        _backgroundMusicThread->join();
}

void Application::Dispose() {
    glfwTerminate();
    SDL_Quit();
    IMG_Quit();
    StopSearchNearestPlanet();
    StopPlayBackgroundMusic();
    _soundEngine->drop();
}

Application::~Application() {
    Dispose();
}
