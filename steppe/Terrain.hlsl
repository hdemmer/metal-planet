
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
	
	float h = sin(input.position.x) + cos(input.position.y);

    output.position.xz = input.position + float2(-5.0,-5.0);
    output.position.y = 0.0;

	output.normal = float3(0,1,0);
	output.diffuse=output.position*0.1;

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
