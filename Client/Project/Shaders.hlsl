cbuffer cbCameraInfo : register(b1) {
	matrix view : packoffset(c0);
	matrix projection : packoffset(c4);
	float3 cameraPosition : packoffset(c8);
};

cbuffer cbGameObjectInfo : register(b2) {
	matrix worldTransform : packoffset(c0);
};



#include "Light.hlsl"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

#define WATER_HEIGHT 150.0f

#define MATERIAL_ALBEDO_MAP		0x01

// 텍스처
Texture2D albedoMap : register(t5);

// 샘플러
SamplerState gssWrap : register(s0);
SamplerState gssClamp : register(s1);

struct VS_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};



VS_OUTPUT DefaultVertexShader(VS_INPUT input)
{
	VS_OUTPUT output;

	output.normal = mul(input.normal, (float3x3)worldTransform);
	output.normal = normalize(output.normal);

	// 조명 계산을 위해 월드좌표내에서의 포지션값을 계산해 따로 저장
	output.positionW = (float3)mul(float4(input.position, 1.0f), worldTransform);

    output.position = mul(mul(float4(output.positionW, 1.0f), view), projection);
    output.uv = input.uv;
	return output;
}

[earlydepthstencil]
float4 DefaultPixelShader(VS_OUTPUT input) : SV_TARGET {
    float4 cColor;
    if (drawMask & MATERIAL_ALBEDO_MAP) {
        cColor = albedoMap.Sample(gssWrap, input.uv);
    }
    else {
        cColor = diffuse;
    }
   float4 color = CalculateLight(cColor, input.positionW, input.normal);
    //color.g = input.uv.r;
   return color;
}

///////////// Default (Instancing)

struct VS_INPUT_INSTANCE {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float4x4 worldTransform : TRANSFORM;
};

struct PS_INPUT_INSTANCE {
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};


PS_INPUT_INSTANCE DefaultVertexShaderInstance(VS_INPUT_INSTANCE input) {
    PS_INPUT_INSTANCE output;

    output.normal = mul(input.normal, (float3x3) input.worldTransform);
    output.normal = normalize(output.normal);

    // 조명 계산을 위해 월드좌표내에서의 포지션값을 계산해 따로 저장
    output.positionW = (float3) mul(float4(input.position, 1.0f), input.worldTransform);

    output.position = mul(mul(float4(output.positionW, 1.0f), view), projection);
    output.uv = input.uv;
    return output;
}


float4 DefaultPixelShaderInstance(PS_INPUT_INSTANCE input) : SV_TARGET {
    float4 cColor;
    if (drawMask & MATERIAL_ALBEDO_MAP) {
        cColor = albedoMap.Sample(gssWrap, input.uv);
    }
    else {
        cColor = diffuse;
    }
    float4 color = CalculateLight(cColor, input.positionW, input.normal);
    //color.g = input.uv.r;
    return color;
}

struct VS_HITBOX_INPUT {
    float3 position : POSITION;

};

struct VS_HITBOX_OUTPUT {
    float4 position : SV_POSITION;
};


VS_HITBOX_OUTPUT HitboxVertexShader(VS_HITBOX_INPUT input) {
    VS_HITBOX_OUTPUT output;
    output.position = mul(mul(mul(float4(input.position, 1.0f), worldTransform), view), projection);
    return output;
}

float4 HitboxPixelShader(VS_HITBOX_OUTPUT input) : SV_TARGET {
    float4 color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    return color;
}

////////////// Terrain

Texture2D terrainBaseTexture : register(t6);
Texture2D terrainDetailTexture : register(t7);

struct VS_TERRAIN_INPUT {
    float3 position : POSITION;
    float4 color : COLOR;
    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
    
};

struct VS_TERRAIN_OUTPUT {
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 uv0 : TEXCOORD0;
    float2 uv1 : TEXCOORD1;
    float height : HEIGHT;
};


VS_TERRAIN_OUTPUT TerrainVertexShader(VS_TERRAIN_INPUT input) {
    VS_TERRAIN_OUTPUT output;
    output.position = mul(mul(mul(float4(input.position, 1.0f), worldTransform), view), projection);
    output.height = input.position.y;
    output.color = input.color;
    output.uv0 = input.uv0;
    output.uv1 = input.uv1;
    
    return output;
}

[earlydepthstencil]
float4 TerrainPixelShader(VS_TERRAIN_OUTPUT input) : SV_TARGET {
    
    float4 albedoColor = input.color;
    float4 cBaseTexColor = terrainBaseTexture.Sample(gssWrap, input.uv0);
    float gapHeight = max(WATER_HEIGHT + 20 - input.height, 0);
    albedoColor = albedoColor * min(pow(1.1, gapHeight), 3);
    
	 //detail텍스처가 u,v축으로 100번 적용될것임 ( 0~100 사이가 되므로 )
    float4 cDetailTexColor = terrainDetailTexture.Sample(gssWrap, input.uv0 * 100.0f);
	// scale에 따라 매핑. 현재 스케일값이 8배므로 그거에 맞게 됨
	//float4 cDetailTexColor = gtxtTerrainDetailTexture.Sample(gSamplerState, input.uv1);
	 //디퓨즈 * 베이스텍스처 + 디테일을 섞어서
    float4 cColor = albedoColor * saturate((cBaseTexColor * 0.5f) + (cDetailTexColor * 0.5f));

    return (cColor);
}


///////////////

struct VS_B_IN {
    float3 position : POSITION;
    float2 size : SIZE;
};

struct VS_B_OUT {
    float3 center : POSITION;
    float2 size : SIZE;
};

struct GS_B_OUT {
    float4 posH : SV_POSITION;
    float3 posW : POSITION;
    float3 normalW : NORMAL;
    float2 uv : TEXCOORD;
    
};



VS_B_OUT BillboardVertexShader(VS_B_IN input) {
    VS_B_OUT output;

	// 조명 계산을 위해 월드좌표내에서의 포지션값을 계산해 따로 저장
    output.center = input.position + worldTransform._41_42_43;
    output.size = input.size;
    return output;
}

[maxvertexcount(4)]
void BillboardGeometryShader(point VS_B_OUT input[1], inout TriangleStream<GS_B_OUT> outStream) {
    
    float2 sUV = worldTransform._12_13;
    float2 eUV = worldTransform._11_22;
    float2 sizeCoff = worldTransform._33_44;
    
    float3 vUp = float3(0.0f, 1.0f, 0.0f);
    float3 vLook = cameraPosition - input[0].center;
    vLook = normalize(vLook);
    float3 vRight = cross(vUp, vLook);
    float3 fHalfW = input[0].size.x * sizeCoff.x * 0.5f;
    float3 fHalfH = input[0].size.y * sizeCoff.y * 0.5f;
    
    float4 vertex[4];
    vertex[0] = float4(input[0].center + (vRight * fHalfW) - (vUp * fHalfH), 1.0f);
    vertex[1] = float4(input[0].center + (vRight * fHalfW) + (vUp * fHalfH), 1.0f);
    vertex[2] = float4(input[0].center - (vRight * fHalfW) - (vUp * fHalfH), 1.0f);
    vertex[3] = float4(input[0].center - (vRight * fHalfW) + (vUp * fHalfH), 1.0f);
    

    float2 uv[4] = { float2(sUV.x, eUV.y), float2(sUV.x, sUV.y), float2(eUV.x, eUV.y), float2(eUV.x, sUV.y) };
   
    GS_B_OUT output;
    [unroll]
    for (int i = 0; i < 4; ++i) {
        output.posW = vertex[i].xyz;
        output.posH = mul(mul(vertex[i], view), projection);
        output.normalW = vLook;
        output.uv = uv[i];
        outStream.Append(output);
    }   
}

//[earlydepthstencil]
// 알파값을 통한 discard를 하기 위해서는 픽셀쉐이더 처리 후 depth test를 해야함

float4 BillboardPixelShader(GS_B_OUT input) : SV_TARGET {

    float4 cColor = albedoMap.Sample(gssWrap, input.uv);
    if (cColor.a < 0.1f)
        discard;
    return cColor;
}

///////////// SkyBox /////////////

struct VS_S_IN {
    float3 position : POSITION;
    float2 uv : TEXCOORD;
};

struct VS_S_OUT {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

struct GS_S_OUT {
    
};

VS_S_OUT SkyBoxVertexShader(VS_S_IN input) {
    
    float3 pos = input.position + cameraPosition;
    VS_S_OUT output;
    
    output.position = mul(mul(float4(pos, 1.0f), view), projection).xyww;
    output.uv = input.uv;
    return output;
}

float4 SkyBoxPixelShader(VS_S_OUT input) : SV_TARGET {
    float2 uv = input.uv;
    uv.y = 1 - uv.y;
    float4 cColor = albedoMap.Sample(gssClamp, uv);
    return cColor;
}

///////////// Water /////////////

struct VS_W_IN {
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct VS_W_OUT {
    float4 position : SV_POSITION;
    float3 positionW : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

VS_W_OUT VertexWaterShader(VS_W_IN input) {
    VS_W_OUT output;
   
    output.normal = input.normal;

	// 조명 계산을 위해 월드좌표내에서의 포지션값을 계산해 따로 저장.
    // 물의 월드행렬은 항상 단위. world의 pos값들은 텍스처 움직임에 사용
    output.positionW = input.position;
    output.position = mul(mul(float4(output.positionW, 1.0f), view), projection);
    output.uv = input.uv;
    return output;
}

[earlydepthstencil]
float4 PixelWaterShader(VS_W_OUT input) : SV_TARGET {
    float2 uv = input.uv;
    uv.x += worldTransform._41;
    uv.y += worldTransform._42;
    float4 cColor = albedoMap.Sample(gssWrap, uv);

    float4 color = CalculateLight(cColor, input.positionW, input.normal);
    // 물의 투명도
    color.a = 0.5;
    return color;
}

////////////////// 2D //////////////////

struct VS_2D_IN {
    float2 position : POSITION;
};

struct VS_2D_OUT {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

VS_2D_OUT Vertex2DShader(VS_2D_IN input) {
    VS_2D_OUT output;
    
    float2 startpos = float2(worldTransform._11, worldTransform._22);
    float2 startuv = float2(worldTransform._41, worldTransform._43);

    //startpos = float2(0,0);
    
    // 2DUI는 항상 z = 0 (맨 앞)에 그려지고, depth값도 쓴다.
    output.position = float4(input.position.x + startpos.x, input.position.y + startpos.y, 0, 1);
    
    float x, y;
    if (input.position.x == 0)
        output.uv.x = startuv.x;
    else
        output.uv.x = startuv.x + 1;
    
    if (input.position.y == 0)
        output.uv.y = startuv.y;
    else
        output.uv.y = startuv.y - 1;

    return output;
}

float4 Pixel2DShader(VS_2D_OUT input) : SV_TARGET {
    float2 startuv = float2(worldTransform._41, worldTransform._43);
    float2 sizeuv = float2(worldTransform._42, worldTransform._44);
    if(sizeuv.x + startuv.x < input.uv.x) 
        discard;
    if (sizeuv.y - startuv.y < input.uv.y) 
        discard;
    float4 color = albedoMap.Sample(gssWrap, input.uv);
    if (color.a < 0.01f)
        discard;
    return color;
}