#include "FPS_Handler.h"

FPS_Handler::FPS_Handler(uint16_t maxFps) : _frameTime(1.0 / maxFps)
{
}

void FPS_Handler::RunFrameTimer() {
    TryUpdateFps();
    _frameStartPoint = steady_clock::now();
}

void FPS_Handler::WaitForFrameTimer() {
    _tempFps++;
    while (duration<double>(steady_clock::now() - _frameStartPoint).count() < _frameTime);
}

uint16_t FPS_Handler::GetCurrentFps() const {
    return _currentFps;
}

void FPS_Handler::TryUpdateFps() {
    if (duration<double>((steady_clock::now() - _tempStartPoint)).count() >= 1.0) {
        _currentFps = _tempFps;
        _tempFps = 0;
        _tempStartPoint = steady_clock::now();
    }
}
