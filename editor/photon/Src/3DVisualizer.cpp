/*=============================================================================
	3DVisualizer.cpp: Helper class to visualize simple 3D geometry
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "stdafx.h"

namespace Lightmass
{

/**
 * Helper wrapper to encapsulate D3D-specific variables.
 */
struct FD3DWrapper
{
	/** Constructor */
	FD3DWrapper()
// 	:	D3D(NULL)
// 	,	D3DDevice(NULL)
	{
	}

// 	/** DirectX 9.0 interface */
// 	IDirect3D9*			D3D;
// 	/** DirectX 9.0 device */
// 	IDirect3DDevice9*	D3DDevice;
};

/**
 * D3D FVF vertex, stored in the .x file.
 */
struct D3DXVertex
{
	/** Vertex positionm, world space. */
	FLOAT		Pos[3];
	/** Unit normal. */
	FLOAT		Normal[3];
	/** Diffuse vertex color. */
// 	D3DCOLOR	Color1;
// 	/** Emissive vertex color. */
// 	D3DCOLOR	Color2;
};

extern const char* ShaderFxFile;

/** Constructor. Initializes a D3D Device. */
F3DVisualizer::F3DVisualizer()
{
// 	D3D = new FD3DWrapper;
// 	HRESULT Result;
// 	D3D->D3D = Direct3DCreate9(D3D_SDK_VERSION);
// 	D3D->D3DDevice = NULL;
// 	D3DPRESENT_PARAMETERS D3Dpp;
// 	ZeroMemory( &D3Dpp, sizeof(D3Dpp) );
// 	D3Dpp.Windowed   = TRUE;
// 	D3Dpp.SwapEffect = D3DSWAPEFFECT_COPY;
// 	Result = D3D->D3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, GetDesktopWindow(), D3DCREATE_HARDWARE_VERTEXPROCESSING|D3DCREATE_NOWINDOWCHANGES, &D3Dpp, &D3D->D3DDevice );
}

/** Destructor. Destroys the D3D Device. */
F3DVisualizer::~F3DVisualizer()
{
// 	D3D->D3DDevice->Release();
// 	D3D->D3D->Release();
// 	delete D3D;
}

/** Returns the current number of triangles. */
INT F3DVisualizer::NumTriangles() const
{
	return Triangles.Num();
}

/** Returns the current number of lines. */
INT F3DVisualizer::NumLines() const
{
	return Lines.Num();
}

/** Removes all geometry. */
void F3DVisualizer::Clear()
{
	Triangles.Empty();
	Lines.Empty();
}

/**
 * Adds a triangle to be visualized.
 * @param P1	First vertex of the triangle
 * @param P2	Second vertex of the triangle
 * @param P3	Third vertex of the triangle
 * @param Color	Color of the triangle
 */
void F3DVisualizer::AddTriangle( const FVector4& P1, const FVector4& P2, const FVector4& P3, FColor Color )
{
	Triangles.AddItem( FTriangle(P1, P2, P3, Color) );
}

/**
 * Adds a triangle to be visualized.
 * @param P1	First vertex of the line
 * @param P2	Second vertex of the line
 * @param Color	Color of the line
 */
void F3DVisualizer::AddLine( const FVector4& P1, const FVector4& P2, FColor Color )
{
	Lines.AddItem( FLine(P1, P2, Color) );
}

/**
 * Exports all geometry into a D3D .x file into the current working folder.
 * @param Filename	Desired filename (may include path)
 * @param bShow		Whether the D3D .x file viewer should be invoked. If shown, we'll block until it has been closed.
 */
void F3DVisualizer::Export( const TCHAR* Filename, UBOOL bShow/*=FALSE*/ )
{
// 	ID3DXMesh* Mesh;
// 	Mesh = NULL;
// 	INT NumPrimitives = NumTriangles() + NumLines()*2;
// 	INT NumVertices = NumTriangles()*3 + NumLines()*4;
// 	HRESULT Result = D3DXCreateMeshFVF( NumPrimitives, NumVertices, D3DXMESH_32BIT, D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_SPECULAR, D3D->D3DDevice, &Mesh );
// 	void* VertexBuffer = NULL;
// 	void* IndexBuffer = NULL;
// 	Result = Mesh->LockVertexBuffer(D3DLOCK_DISCARD, &VertexBuffer);
// 	Result = Mesh->LockIndexBuffer(D3DLOCK_DISCARD, &IndexBuffer);
// 
// 	D3DXVertex* Vertices = (D3DXVertex*)VertexBuffer;
// 	DWORD* Indices = (DWORD*) IndexBuffer;
// 	INT NumVerticesStored = 0;
// 	INT NumIndicesStored = 0;
// 
// 	// Add the triangles to the vertexbuffer and indexbuffer.
// 	for ( INT TriangleIndex=0; TriangleIndex < NumTriangles(); ++TriangleIndex )
// 	{
// 		const FVector4& P1 = Triangles(TriangleIndex).Vertices[0];
// 		const FVector4& P2 = Triangles(TriangleIndex).Vertices[1];
// 		const FVector4& P3 = Triangles(TriangleIndex).Vertices[2];
// 		const FColor& Color = Triangles(TriangleIndex).Color;
// 		Vertices[NumVerticesStored+0].Pos[0] = P1[0];
// 		Vertices[NumVerticesStored+0].Pos[1] = P1[1];
// 		Vertices[NumVerticesStored+0].Pos[2] = P1[2];
// 		Vertices[NumVerticesStored+0].Color1 = Color.DWColor();
// 		Vertices[NumVerticesStored+0].Color2 = 0;
// 		Vertices[NumVerticesStored+1].Pos[0] = P2[0];
// 		Vertices[NumVerticesStored+1].Pos[1] = P2[1];
// 		Vertices[NumVerticesStored+1].Pos[2] = P2[2];
// 		Vertices[NumVerticesStored+1].Color1 = Color.DWColor();
// 		Vertices[NumVerticesStored+1].Color2 = 0;
// 		Vertices[NumVerticesStored+2].Pos[0] = P3[0];
// 		Vertices[NumVerticesStored+2].Pos[1] = P3[1];
// 		Vertices[NumVerticesStored+2].Pos[2] = P3[2];
// 		Vertices[NumVerticesStored+2].Color1 = Color.DWColor();
// 		Vertices[NumVerticesStored+2].Color2 = 0;
// 
// 		// Reverse triangle winding order for the .x file.
// 		Indices[NumIndicesStored+0] = NumVerticesStored + 0;
// 		Indices[NumIndicesStored+1] = NumVerticesStored + 2;
// 		Indices[NumIndicesStored+2] = NumVerticesStored + 1;
// 
// 		NumVerticesStored += 3;
// 		NumIndicesStored += 3;
// 	}
// 
// 	// Add the lines to the vertexbuffer and indexbuffer.
// 	for ( INT LineIndex=0; LineIndex < NumLines(); ++LineIndex )
// 	{
// 		const FVector4& P1 = Lines(LineIndex).Vertices[0];
// 		const FVector4& P2 = Lines(LineIndex).Vertices[1];
// 		const FColor& Color = Lines(LineIndex).Color;
// 		Vertices[NumVerticesStored+0].Pos[0] = P1[0];
// 		Vertices[NumVerticesStored+0].Pos[1] = P1[1] - 5.0f;
// 		Vertices[NumVerticesStored+0].Pos[2] = P1[2];
// 		Vertices[NumVerticesStored+0].Color1 = 0;
// 		Vertices[NumVerticesStored+0].Color2 = Color.DWColor();
// 		Vertices[NumVerticesStored+1].Pos[0] = P1[0];
// 		Vertices[NumVerticesStored+1].Pos[1] = P1[1] + 5.0f;
// 		Vertices[NumVerticesStored+1].Pos[2] = P1[2];
// 		Vertices[NumVerticesStored+1].Color1 = 0;
// 		Vertices[NumVerticesStored+1].Color2 = Color.DWColor();
// 		Vertices[NumVerticesStored+2].Pos[0] = P2[0];
// 		Vertices[NumVerticesStored+2].Pos[1] = P2[1] - 5.0f;
// 		Vertices[NumVerticesStored+2].Pos[2] = P2[2];
// 		Vertices[NumVerticesStored+2].Color1 = 0;
// 		Vertices[NumVerticesStored+2].Color2 = Color.DWColor();
// 		Vertices[NumVerticesStored+3].Pos[0] = P2[0];
// 		Vertices[NumVerticesStored+3].Pos[1] = P2[1] + 5.0f;
// 		Vertices[NumVerticesStored+3].Pos[2] = P2[2];
// 		Vertices[NumVerticesStored+3].Color1 = 0;
// 		Vertices[NumVerticesStored+3].Color2 = Color.DWColor();
// 
// 		Indices[NumIndicesStored+0] = NumVerticesStored+0;
// 		Indices[NumIndicesStored+1] = NumVerticesStored+2;
// 		Indices[NumIndicesStored+2] = NumVerticesStored+1;
// 		Indices[NumIndicesStored+3] = NumVerticesStored+2;
// 		Indices[NumIndicesStored+4] = NumVerticesStored+3;
// 		Indices[NumIndicesStored+5] = NumVerticesStored+1;
// 
// 		NumVerticesStored += 4;
// 		NumIndicesStored += 6;
// 	}
// 
// 	Mesh->UnlockVertexBuffer();
// 	Mesh->UnlockIndexBuffer();
// 	Result = D3DXComputeNormals( Mesh, NULL );
// 	D3DXMATERIAL MeshMaterial;
// 	MeshMaterial.MatD3D.Ambient.r = 0.1f;
// 	MeshMaterial.MatD3D.Ambient.g = 0.1f;
// 	MeshMaterial.MatD3D.Ambient.b = 0.1f;
// 	MeshMaterial.MatD3D.Ambient.a = 0.0f;
// 	MeshMaterial.MatD3D.Diffuse.r = 1.0f;
// 	MeshMaterial.MatD3D.Diffuse.g = 1.0f;
// 	MeshMaterial.MatD3D.Diffuse.b = 1.0f;
// 	MeshMaterial.MatD3D.Diffuse.a = 1.0f;
// 	MeshMaterial.MatD3D.Emissive.r = 1.0f;
// 	MeshMaterial.MatD3D.Emissive.g = 1.0f;
// 	MeshMaterial.MatD3D.Emissive.b = 1.0f;
// 	MeshMaterial.MatD3D.Emissive.a = 1.0f;
// 	MeshMaterial.MatD3D.Specular.r = 1.0f;
// 	MeshMaterial.MatD3D.Specular.g = 1.0f;
// 	MeshMaterial.MatD3D.Specular.b = 1.0f;
// 	MeshMaterial.MatD3D.Specular.a = 1.0f;
// 	MeshMaterial.MatD3D.Power = 16.0f;
// 	MeshMaterial.pTextureFilename = NULL;
// 
// 	D3DXEFFECTINSTANCE EffectInstance;
// 	EffectInstance.pEffectFilename = "D3DExport.fx";
// 	EffectInstance.NumDefaults = 0;
// 	EffectInstance.pDefaults = NULL;
// 
// 	// Write out the .x file.
// 	D3DXSaveMeshToX( Filename, Mesh, NULL, &MeshMaterial, &EffectInstance, 1, D3DXF_FILEFORMAT_BINARY );
// 	Mesh->Release();
// 
// 	// Write out the .fx file, if it doesn't always exist.
// 	HANDLE ShaderFile = CreateFile( TEXT("D3DExport.fx"), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
// 	if (ShaderFile != INVALID_HANDLE_VALUE)
// 	{
// 		::DWORD BytesWritten;
// 		WriteFile(ShaderFile, ShaderFxFile, (DWORD) strlen(ShaderFxFile), &BytesWritten, NULL);
// 		CloseHandle( ShaderFile );
// 	}
// 
// 	// If specified, run the default viewer for .x files and block until it's closed.
// 	if ( bShow )
// 	{
// 		system( TCHAR_TO_ANSI(Filename) );
// 	}
}

const char* ShaderFxFile =
	"int sas : SasGlobal\n"
	"<\n"
	"	bool SasUiVisible = false;\n"
	"	int3 SasVersion= {1,1,0};\n"
	">;\n"
	"\n"
	"float4x4 g_mWorld : WORLD\n"
	"<\n"
	"	bool SasUiVisible = false;\n"
	"	string SasBindAddress= \"Sas.Skeleton.MeshToJointToWorld[0]\";\n"
	">;        \n"
	"\n"
	"float4x4 g_mView : VIEW\n"
	"<\n"
	"	bool SasUiVisible = false;\n"
	"	string SasBindAddress= \"Sas.Camera.WorldToView\";\n"
	">;   \n"
	"\n"
	"float4x4 g_mProj : PROJECTION\n"
	"<\n"
	"	bool SasUiVisible = false;\n"
	"	string SasBindAddress= \"Sas.Camera.Projection\";\n"
	">;\n"
	"\n"
	"float4 g_vLightColor\n"
	"<\n"
	"	bool SasUiVisible = false;\n"
	"	string SasBindAddress= \"Sas.PointLight[0].Color\";\n"
	"> = {1.0f, 1.0f, 1.0f, 1.0f}; // Light value\n"
	"\n"
	"float3 g_vLight\n"
	"<  \n"
	"	bool SasUiVisible = false;\n"
	"	string SasBindAddress= \"Sas.PointLight[0].Position\";\n"
	"> = {0.0f, 3.0f, -100.0f};\n"
	"\n"
	"// Object material attributes\n"
	"float4 Diffuse\n"
	"<\n"
	"	string SasUiControl = \"ColorPicker\";\n"
	"> = { 0.5f, 0.5f, 0.5, 1.0f};      // Diffuse color of the material\n"
	"\n"
	"float4 Specular\n"
	"<\n"
	"	string SasUiControl = \"ColorPicker\";\n"
	"> = {1.0f, 1.0f, 1.0f, 1.0f};  // Specular color of the material\n"
	"\n"
	"float  Power\n"
	"<\n"
	"    string SasUiLabel = \"Specular Power\";\n"
	"    string SasUiControl = \"Slider\"; \n"
	"    float SasUiMin = 1.0f; \n"
	"    float SasUiMax = 32.0f; \n"
	"    int SasUiSteps = 31;\n"
	"\n"
	"> = 8.0f;\n"
	"\n"
	"//--------------------------------------------------------------------------------------\n"
	"// Vertex shader output structure\n"
	"//--------------------------------------------------------------------------------------\n"
	"struct VS_OUTPUT\n"
	"{\n"
	"    float4 Position   : POSITION;   // vertex position \n"
	"    float4 Diffuse    : COLOR0;     // vertex diffuse color (note that COLOR0 is clamped from 0..1)\n"
	"//    float2 TextureUV  : TEXCOORD0;  // vertex texture coords \n"
	"};\n"
	"\n"
	"\n"
	"//--------------------------------------------------------------------------------------\n"
	"// This shader computes standard transform and lighting\n"
	"//--------------------------------------------------------------------------------------\n"
	"VS_OUTPUT RenderSceneVS( float4 vPos : POSITION, \n"
	"                         float3 vNormal : NORMAL,\n"
	"						 float4 vColor1 : COLOR0,\n"
	"						 float4 vColor2 : COLOR1 )\n"
	"//                         float2 vTexCoord0 : TEXCOORD0 )\n"
	"{\n"
	"	VS_OUTPUT Output;\n"
	"	\n"
	"    float4x4 g_mWorldView= mul(g_mWorld, g_mView);\n"
	"    float4x4 g_mWorldViewProjection= mul(g_mWorldView, g_mProj);\n"
	" 	float4 lightInView= mul( g_vLight, g_mView);\n"
	"\n"
	"    // Transform the position from object space to homogeneous projection space\n"
	"    Output.Position = mul( vPos, g_mWorldViewProjection );\n"
	"\n"
	"    // Compute the view-space position\n"
	"    float4 ViewPos = mul( vPos, g_mWorldView );\n"
	"\n"
	"    // Compute view-space normal\n"
	"    float3 ViewNormal = normalize( mul( vNormal, (float3x3)g_mWorldView ) );\n"
	"\n"
	"    // Compute diffuse lighting\n"
	"    Output.Diffuse = dot( ViewNormal, normalize( lightInView - ViewPos ) ) * Diffuse * vColor1 + vColor2;\n"
	"   \n"
	"    return Output;    \n"
	"}\n"
	"\n"
	"\n"
	"//--------------------------------------------------------------------------------------\n"
	"// Pixel shader output structure\n"
	"//--------------------------------------------------------------------------------------\n"
	"struct PS_OUTPUT\n"
	"{\n"
	"    float4 RGBColor : COLOR0;  // Pixel color    \n"
	"};\n"
	"\n"
	"\n"
	"//--------------------------------------------------------------------------------------\n"
	"// This shader outputs the pixel's color by modulating the texture's\n"
	"// color with diffuse material color\n"
	"//--------------------------------------------------------------------------------------\n"
	"PS_OUTPUT RenderScenePS( VS_OUTPUT In ) \n"
	"{ \n"
	"    PS_OUTPUT Output;\n"
	"\n"
	"    // Lookup mesh texture and modulate it with diffuse\n"
	"  //  Output.RGBColor = tex2D(MeshTextureSampler, In.TextureUV) * In.Diffuse;\n"
	"	Output.RGBColor = In.Diffuse;\n"
	"\n"
	"    return Output;\n"
	"}\n"
	"\n"
	"\n"
	"//--------------------------------------------------------------------------------------\n"
	"// Renders scene \n"
	"//--------------------------------------------------------------------------------------\n"
	"technique RenderScene\n"
	"{\n"
	"    pass P0\n"
	"    {          \n"
	"        VertexShader = compile vs_2_0 RenderSceneVS();\n"
	"        PixelShader  = compile ps_2_0 RenderScenePS(); \n"
	"		CullMode = None;\n"
	"    }\n"
	"}\n";
}
