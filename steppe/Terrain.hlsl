/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer
{
    matrix modelViewProjectionMatrix;
};

//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float2 position : POSITION;
};

struct DeferredVertexInputType
{
    float3 position : POSITION;
	float3 normal : NORMAL;
	float3 diffuse : DIFFUSE;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
DeferredVertexInputType TerrainVertexShader(VertexInputType input)
{
    DeferredVertexInputType output;
    
    // Calculate the position of the vertex against the world, view, and projection matrices.
	
    output.position.xz = 0.3 * input.position;
    output.position.y = -0.1;

	output.normal = float3(0,1,0);
	output.diffuse=output.position;

    return output;
}

////////////////////////////////////////////////////////////////////////////////
// Geometry Shader
////////////////////////////////////////////////////////////////////////////////

[maxvertexcount(3)]
void DummyGeometryShader( triangle DeferredVertexInputType input[3], inout TriangleStream<DeferredVertexInputType> TriangleOutputStream )
{
    TriangleOutputStream.Append( input[0] );
    TriangleOutputStream.Append( input[1] );
    TriangleOutputStream.Append( input[2] );
    TriangleOutputStream.RestartStrip();
}
