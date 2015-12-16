struct VS_OUT
{
	float4 Pos : POSITION;
	float4 Color : COLOR;
	float3 normal : NORMAL;

	float3 otherPosition : POSITION2;
};

Texture2D txDiffuse : register(t0);
SamplerState sampAni;

float4 PS_main(VS_OUT input) : SV_Target
{
	float4 s = input.Color;

	float3 norNormal = normalize(input.normal);
	float3 norLightPos = normalize(input.otherPosition - float3(0.0, 0.0, -3.0));

	float angle = max(dot(norLightPos, norNormal), 0.0);

	if(s.w < 0.2)
		discard;

	return s * angle + s * 0.01;
	//return float4(1.0, 0.0, 0.0, 1.0);
};