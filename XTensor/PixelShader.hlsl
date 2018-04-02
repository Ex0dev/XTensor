struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
};

float4 main( VS_OUTPUT input ) : SV_TARGET
{
	return input.Color;
}