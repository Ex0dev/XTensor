struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
};

cbuffer CBPerObject
{
	float4x4 WVP;
};

VS_OUTPUT main( float4 pos : POSITION, float4 color : COLOR )
{
	VS_OUTPUT output;

	output.Position = mul(pos, WVP);
	output.Color = color;

	return output;
}