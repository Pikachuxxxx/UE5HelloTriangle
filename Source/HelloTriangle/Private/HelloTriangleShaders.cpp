#include "HelloTriangleShaders.h"

TGlobalResource<FTriangleVertexBuffer> GTriangleVertexBuf;
TGlobalResource<FTrianlgeVertexBufferElementDesc> GTriangleVertexBufElementDesc;
TGlobalResource<FTriangleIndexBuffer> GTriangleIndexBuf;

void FTriangleVertexBuffer::InitRHI(FRHICommandListBase& RHICmdList)
{
    TResourceArray<FHelloVertex, VERTEXBUFFER_ALIGNMENT> Vertices;
    Vertices.SetNumUninitialized(3);
    Vertices[0].Position = FVector2f(0.0f, 0.75f);
    Vertices[0].Color = FVector4f(1, 0, 0, 1);
    Vertices[1].Position = FVector2f(0.75, -0.75);
    Vertices[1].Color = FVector4f(0, 1, 0, 1);
    Vertices[2].Position = FVector2f(-0.75, -0.75);
    Vertices[2].Color = FVector4f(0, 0, 1, 1);
    FRHIResourceCreateInfo CreateInfo(TEXT("FScreenRectangleVertexBuffer"), &Vertices);
    VertexBufferRHI = RHICmdList.CreateVertexBuffer(Vertices.GetResourceDataSize(), BUF_Static, CreateInfo);
}

void FTriangleIndexBuffer::InitRHI(FRHICommandListBase& RHICmdList)
{
    const uint16 Indices[] = { 0, 1, 2 };
    TResourceArray<uint16, INDEXBUFFER_ALIGNMENT> IndexBuffer;
    uint32 NumIndices = UE_ARRAY_COUNT(Indices);
    IndexBuffer.AddUninitialized(NumIndices);
    FMemory::Memcpy(IndexBuffer.GetData(), Indices, NumIndices * sizeof(uint16));
    FRHIResourceCreateInfo CreateInfo(TEXT("FTriangleIndexBuffer"), &IndexBuffer);
    IndexBufferRHI = RHICmdList.CreateIndexBuffer(sizeof(uint16), IndexBuffer.GetResourceDataSize(), BUF_Static, CreateInfo);
}

void FTrianlgeVertexBufferElementDesc::InitRHI(FRHICommandListBase& RHICmdList)
{
    FVertexDeclarationElementList Elements;
    uint16 Stride = sizeof(FHelloVertex);
    Elements.Add(FVertexElement(0, STRUCT_OFFSET(FHelloVertex, Position), VET_Float2, 0, Stride));
    Elements.Add(FVertexElement(0, STRUCT_OFFSET(FHelloVertex, Color), VET_Float4, 1, Stride));
    VertexDeclarationRHI = PipelineStateCache::GetOrCreateVertexDeclaration(Elements);
}

void FTrianlgeVertexBufferElementDesc::ReleaseRHI()
{
    VertexDeclarationRHI.SafeRelease();
}
