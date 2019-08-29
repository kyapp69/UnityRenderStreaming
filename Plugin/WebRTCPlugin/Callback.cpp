#include "pch.h"
#include "Context.h"
#include "IUnityGraphics.h"
#include "IUnityGraphicsD3D11.h"
#include "IUnityGraphicsD3D12.h"

namespace WebRTC
{
    IUnityInterfaces* s_UnityInterfaces = nullptr;
    IUnityGraphics* s_Graphics = nullptr;
    //d3d11 context
    ID3D11DeviceContext* context;
    //d3d11 device
    ID3D11Device* g_D3D11Device = nullptr;
    //d3d12 device
    ID3D12Device* g_D3D12Device = nullptr;

    //natively created ID3D11Texture2D ptrs
    UnityFrameBuffer* renderTextures[bufferedFrameNum];

    Context* s_context;
}
using namespace WebRTC;
//get d3d11 device
static void UNITY_INTERFACE_API OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType)
{
    switch (eventType)
    {
    case kUnityGfxDeviceEventInitialize:
    {
        auto* graphics = s_UnityInterfaces->Get<IUnityGraphics>();
        switch (graphics->GetRenderer())
        {
        case kUnityGfxRendererD3D11:
        {
            g_D3D11Device = s_UnityInterfaces->Get<IUnityGraphicsD3D11>()->GetDevice();
            g_D3D11Device->GetImmediateContext(&context);
            break;
        }
        case kUnityGfxRendererD3D12:
        {
            if (auto ifs = s_UnityInterfaces->Get<IUnityGraphicsD3D12v5>()) {
                g_D3D12Device = ifs->GetDevice();
            }
            else if (auto ifs = s_UnityInterfaces->Get<IUnityGraphicsD3D12v4>()) {
                g_D3D12Device = ifs->GetDevice();
            }
            else if (auto ifs = s_UnityInterfaces->Get<IUnityGraphicsD3D12v3>()) {
                g_D3D12Device = ifs->GetDevice();
            }
            else if (auto ifs = s_UnityInterfaces->Get<IUnityGraphicsD3D12v2>()) {
                g_D3D12Device = ifs->GetDevice();
            }
            else if (auto ifs = s_UnityInterfaces->Get<IUnityGraphicsD3D12>()) {
                g_D3D12Device = ifs->GetDevice();
            }
            else
            {
                // unknown IUnityGraphicsD3D12 version
                LogPrint("Unknown IUnityGraphicsD3D12 version\n");
                return;
            }
            break;
        }
        };
    }
    case kUnityGfxDeviceEventShutdown:
    {
        for (auto rt : renderTextures)
        {
            if (rt)
            {
                rt->Release();
                rt = nullptr;
            }
        }
        //UnityPluginUnload not called normally
        s_Graphics->UnregisterDeviceEventCallback(OnGraphicsDeviceEvent);
        break;
    }
    case kUnityGfxDeviceEventBeforeReset:
    {
        break;
    }
    case kUnityGfxDeviceEventAfterReset:
    {
        break;
    }
    };
}
// Unity plugin load event
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* unityInterfaces)
{
    s_UnityInterfaces = unityInterfaces;
    s_Graphics = unityInterfaces->Get<IUnityGraphics>();
    s_Graphics->RegisterDeviceEventCallback(OnGraphicsDeviceEvent);
    OnGraphicsDeviceEvent(kUnityGfxDeviceEventInitialize);
}
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload()
{
    s_Graphics->UnregisterDeviceEventCallback(OnGraphicsDeviceEvent);
}

static void UNITY_INTERFACE_API OnRenderEvent(int eventID)
{
    if (s_context != nullptr)
    {
        s_context->EncodeFrame();
    }
}

extern "C" UnityRenderingEvent UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API GetRenderEventFunc(Context* context)
{
    s_context = context;
    return OnRenderEvent;
}
