// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "../src/mtlpp.hpp"
#include "window.hpp"

ns::Ref<mtlpp::Device>     g_device;
mtlpp::CommandQueue        g_commandQueue;
mtlpp::Buffer              g_vertexBuffer;
mtlpp::Buffer              g_denormBuffer;
mtlpp::Buffer              g_denormUintBuffer;
mtlpp::RenderPipelineState g_renderPipelineState;

void Render(const Window& win)
{
    mtlpp::CommandBuffer commandBuffer = g_commandQueue.CommandBuffer();
    
    mtlpp::RenderPassDescriptor renderPassDesc = win.GetRenderPassDescriptor();
    if (renderPassDesc)
    {
        mtlpp::RenderCommandEncoder renderCommandEncoder = commandBuffer.RenderCommandEncoder(renderPassDesc);
        renderCommandEncoder.SetRenderPipelineState(g_renderPipelineState);
        renderCommandEncoder.SetVertexBuffer(g_vertexBuffer, 0, 0);
		renderCommandEncoder.SetFragmentBuffer(g_denormBuffer, 0, 0);
		renderCommandEncoder.SetFragmentBuffer(g_denormBuffer, 0, 1);
		renderCommandEncoder.SetFragmentBuffer(g_denormUintBuffer, 0, 1);
        renderCommandEncoder.Draw(mtlpp::PrimitiveType::Triangle, 0, 3);
        renderCommandEncoder.EndEncoding();
        commandBuffer.Present(win.GetDrawable());
    }

    commandBuffer.Commit();
    commandBuffer.WaitUntilCompleted();
}

int main()
{
    const char shadersSrc[] = R"""(
        #include <metal_stdlib>
        using namespace metal;

        vertex float4 vertFunc(
            const device packed_float3* vertexArray [[buffer(0)]],
            unsigned int vID[[vertex_id]])
        {
            return float4(vertexArray[vID], 1.0);
        }

        fragment half4 fragFunc(const device float4* denormFloat [[buffer(0)]], const device uint4* denormUint [[buffer(1)]])
        {
			float4 df = denormFloat[0];
			uint4 du = denormUint[0];
			if (as_type<uint>(df.x) == du.x && as_type<uint>(df.y) == du.y && as_type<uint>(df.z) == du.z && as_type<uint>(df.w) == du.w)
			{
				return half4(0.0, 1.0, 0.0, 1.0);
			}
			else
			{
				return half4(1.0, 0.0, 0.0, 1.0);
			}
        }
    )""";

    const float vertexData[] =
    {
         0.0f,  1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
    };
	
	const unsigned denormData[] =
	{
		1, 0xFF, 0xFFFF, 0x7FFFFF
	};
	
	ns::Array<ns::Ref<mtlpp::Device>> devices = mtlpp::Device::CopyAllDevices();
	// g_device = devices[0];
	
	g_device = mtlpp::Device::CreateSystemDefaultDevice();
    g_commandQueue = g_device->NewCommandQueue();

	ns::Error Error;
    mtlpp::Library library = g_device->NewLibrary(shadersSrc, mtlpp::CompileOptions(), &Error);
	
	if (Error.GetPtr())
	{
		printf("Error: %s", Error.GetLocalizedDescription().GetCStr());
	}
	assert(library.GetPtr());
	
    mtlpp::Function vertFunc = library.NewFunction("vertFunc");
    mtlpp::Function fragFunc = library.NewFunction("fragFunc");

    g_vertexBuffer = g_device->NewBuffer(vertexData, sizeof(vertexData), mtlpp::ResourceOptions::CpuCacheModeDefaultCache);
	
	g_denormBuffer = g_device->NewBuffer(denormData, sizeof(denormData), mtlpp::ResourceOptions::CpuCacheModeDefaultCache);
	
	g_denormUintBuffer = g_device->NewBuffer(denormData, sizeof(denormData), mtlpp::ResourceOptions::CpuCacheModeDefaultCache);

    mtlpp::RenderPipelineDescriptor renderPipelineDesc;
    renderPipelineDesc.SetVertexFunction(vertFunc);
    renderPipelineDesc.SetFragmentFunction(fragFunc);
    renderPipelineDesc.GetColorAttachments()[0].SetPixelFormat(mtlpp::PixelFormat::BGRA8Unorm);
    g_renderPipelineState = g_device->NewRenderPipelineState(renderPipelineDesc, nullptr);

    Window win(g_device, &Render, 320, 240);
	
    Window::Run();

    return 0;
}

