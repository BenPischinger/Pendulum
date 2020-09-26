
matrix View;
matrix World;


//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------



cbuffer cbChangeOnResize
{
    matrix Projection;
};


cbuffer cbNeverChanges
{
	float3 PositionOfLight;
};



struct VS_INPUT
{
    float4 Pos : POSITION;
    float3 Normal : NORMAL;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
	float3 WorldPosition : TEXCOORD0;
    float3 Normal : NORMAL;
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
	output.Pos = mul( input.Pos, World );
	output.WorldPosition = output.Pos.xyz;
    output.Pos = mul( output.Pos, View );
    output.Pos = mul( output.Pos, Projection );
	output.Normal = mul( input.Normal, World).xyz;
	
    
    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{
	float4 baseColor = float4(1.0, 0.0, 0.0, 1.0); 
	float3 lightDirection = normalize(PositionOfLight - input.WorldPosition);
	float lightIntensity = saturate(dot(lightDirection, normalize(input.Normal)));
	lightIntensity = 0.2f + 0.8f * lightIntensity;

    return baseColor * lightIntensity; 
}



RasterizerState rsMine { CullMode = None; };

//--------------------------------------------------------------------------------------
technique10 Render
{
    pass P0
    {
		SetRasterizerState(rsMine);
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS() ) );
    }
}


