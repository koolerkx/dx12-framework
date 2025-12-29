#pragma once
#include <DirectXMath.h>

struct alignas(256) FrameCB {
    DirectX::XMFLOAT4X4 view;
    DirectX::XMFLOAT4X4 proj;
    DirectX::XMFLOAT4X4 viewProj;
    DirectX::XMFLOAT3 cameraPos;
    float time;
    DirectX::XMFLOAT2 screenSize;
    DirectX::XMFLOAT2 _padding;
};

struct alignas(256) ObjectCB {
    DirectX::XMFLOAT4X4 world;
    DirectX::XMFLOAT4X4 worldViewProj;
    DirectX::XMFLOAT4 color;
};

struct alignas(256) LightingCB {
    DirectX::XMFLOAT3 lightDirection;
    float lightIntensity;
    DirectX::XMFLOAT3 lightColor;
    float _padding;
};
