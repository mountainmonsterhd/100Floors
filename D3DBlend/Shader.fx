//*********************************
//���峣������
//*********************
//����任����ĳ�������
cbuffer MatrixBuffer
{
	matrix World;		//����任����matrix��ʾ4x4����
	matrix View;		//�۲�任����
	matrix Projection;	//ͶӰ�任����
	float4 EyePosition; //�ӵ�λ��
};

//������Ϣ�ĳ�������
cbuffer MaterialBuffer
{
	float4 MatAmbient;		//���ʶԻ�����ķ�����
	float4 MatDiffuse;		//���ʶ��������ķ�����
	float4 MatSpecular;		//���ʶԾ����ķ�����
	float MatPower;			//���ʵľ���ⷴ��ϵ��
}

//��Դ�ĳ�������
cbuffer LightBuffer
{
	int type;						//��Դ����
	float4 LightPosition;			//��Դλ��
	float4 LightDirection;			//��Դ����
	float4 LightAmbient;			//������ǿ��
	float4 LightDiffuse;			//�������ǿ��
	float4 LightSpecular;			//�����ǿ��
	float LightAtt0;				//����˥������
	float LightAtt1;				//һ��˥������
	float LightAtt2;				//����˥������
	float LightAlpha;				//�۹����׶�Ƕ�
	float LightBeta;				//�۹����׶�Ƕ�
	float LightFallOff;				//�۹��˥��ϵ��
}


Texture2D Texture;		//��������

//���������
SamplerState Sampler
{
	Filter = MIN_MAG_MIP_LINEAR;		//�������Թ���
	AddressU = MIRROR;					//ѰַģʽΪMIRROR
	AddressV = MIRROR;					//ѰַģʽΪMIRROR
};


//*****************************
//��������ṹ
//**************************
//������ɫ��������

struct VS_INPUT
{
	float4 Pos : POSITION;	//��������
	float3 Norm : NORMAL;		//������
	float2 Tex : TEXCOORD0;    //��������
};
//������ɫ��������ṹ
struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
	float3 Norm : TEXCOORD1;
	float4 ViewDirection : TEXCOORD2;
	float4 LightVector : TEXCOORD3;
};
//������ɫ��������
//struct PS_INPUT
//{
//	float4 Pos : SV_POSITION;
//	float2 Tex : TEXCOORD0;
//	float3 Norm : TEXCOORD1;
//	float4 ViewDirection : TEXCOORD2;
//	float4 LightVector : TEXCOORD3;
//};

//*********************
//���������ɫ��
//*************************

//PS_INPUT VS(VS_INPUT input)
//{
//	PS_INPUT output = (PS_INPUT)0;
//	output.Pos = mul( input.Pos, World );
//	output.Pos = mul( output.Pos, View );
//	output.Pos = mul( output.Pos, Projection );
//
//	output.Tex = input.Tex;
//	return output;
//}

//������ɫ��
VS_OUTPUT VS( VS_INPUT input )
{
	VS_OUTPUT output = (VS_OUTPUT)0;				//����һ��VS_OUTPUT�Ķ���

	output.Pos = mul( input.Pos, World );			//����任
	output.Pos = mul( output.Pos, View );			//�۲�任
	output.Pos = mul( output.Pos, Projection );		//ͶӰ�任

	output.Norm = mul( input.Norm, (float3x3)World );				//��÷�������
	output.Norm = normalize( output.Norm );							//�Է��������й�һ��

	float4 worldPosition = mul( input.Pos, World );					//��ȡ�������������
	output.ViewDirection = EyePosition - worldPosition;				//��ȡ�ӵ㷽��
	output.ViewDirection = normalize( output.ViewDirection );		//�ӵ㷽���һ��

	output.LightVector = LightPosition - worldPosition;				//��ȡ���շ���
	output.LightVector = normalize( output.LightVector );			//���շ����һ��
	output.LightVector.w = length( LightPosition - worldPosition);	//���վ���
	
	output.Tex = input.Tex;

	return output;
}

float4 PS(VS_OUTPUT input) : SV_Target
{
	float4 texColor = Texture.Sample(Sampler, input.Tex);
	float4 finalColor;
	finalColor = texColor;
	finalColor.a = texColor.a * MatDiffuse.a; //��ȡalphaֵ����͸����
	return finalColor;		//��������
}

//ƽ�й�Դ������ɫ��
float4 PSDirectionalLight( VS_OUTPUT input) : SV_Target
{
	float4 finalColor;					//����������ɫ
	//���������⡢������⡢�������ɫ
	float4 ambientColor, diffuseColor , specularColor;
	//���շ���͹������䷽���෴��
	float3 lightVector = -LightDirection.xyz;
	lightVector = normalize( lightVector );
	//�ò��ʻ����ⷴ���ʺͻ�����ǿ����˵õ���������ɫ
	ambientColor = MatAmbient * LightAmbient;
	//�����㷨�����͹��շ�����е�˵õ������������
	float diffuseFactor = dot(lightVector, input.Norm);
	if(diffuseFactor > 0.0f)
	{
		//�ò��ʵ������Ź�ķ����ʺ��������Ĺ���ǿ���Լ��������������˵õ����������ɫ
		diffuseColor = MatDiffuse * LightDiffuse * diffuseFactor;
		//���ݹ��շ���Ͷ��㷨�������㷴�䷽��
		float3 reflection = reflect(-lightVector, input.Norm);
		//���ݷ��䷽���ӵ㷽���Լ����ʵľ���ⷴ��ϵ�������㾵�淴������
		float specularFactor = pow(max(dot(reflection,input.ViewDirection.xyz),0.0f),MatPower);
		//���ʵľ��淴���ʣ������ǿ���Լ����淴��������˵õ��������ɫ
		specularColor = MatSpecular * LightSpecular * specularFactor;
	}
	float4 texColor = Texture.Sample(Sampler, input.Tex);

	finalColor = saturate(ambientColor + diffuseColor + specularColor);
	finalColor = finalColor * texColor;
	finalColor.a = texColor.a * MatDiffuse.a; //��ȡalphaֵ����͸����
	return finalColor;
}

//���Դ������ɫ��
float4 PSPointLight( VS_OUTPUT input) : SV_Target
{
	float4 finalColor;

	float4 ambientColor, diffuseColor , specularColor;

	float3 lightVector = input.LightVector.xyz;
	lightVector = normalize( lightVector );

	ambientColor = MatAmbient * LightAmbient;

	float diffuseFactor = dot(lightVector, input.Norm);
	if(diffuseFactor > 0.0f)
	{
		diffuseColor = MatDiffuse * LightDiffuse * diffuseFactor;

		float3 reflection = reflect(-lightVector, input.Norm);

		float specularFactor = pow(max(dot(reflection,input.ViewDirection.xyz),0.0f),MatPower);

		specularColor = MatSpecular * LightSpecular * specularFactor;
	}

	float d = input.LightVector.w;

	float att = LightAtt0 + LightAtt1*d + LightAtt2*d*d;

	float4 texColor = Texture.Sample(Sampler, input.Tex);

	finalColor = saturate(ambientColor + diffuseColor + specularColor);
	finalColor = finalColor * texColor;
	finalColor.a = texColor.a * MatDiffuse.a; //��ȡalphaֵ����͸����
	return finalColor; 
}

//�۹��������ɫ��
float4 PSSpotLight( VS_OUTPUT input) : SV_Target
{
	float4 finalColor;

	float4 ambientColor, diffuseColor , specularColor;

	float3 lightVector = input.LightVector.xyz;
	lightVector = normalize( lightVector );
	float d = input.LightVector.w;
	float3 lightDirection = LightDirection.xyz;
	lightDirection = normalize(lightDirection);

	float cosTheta = dot(-lightVector,lightDirection);
	
	if(cosTheta < cos(LightBeta/2))
		return float4(0.0f, 0.0f, 0.0f, 1.0f);
	ambientColor = MatAmbient * LightAmbient;

	float diffuseFactor = dot(lightVector, input.Norm);
	if(diffuseFactor > 0.0f)
	{
		diffuseColor = MatDiffuse * LightDiffuse * diffuseFactor;

		float3 reflection = reflect(-lightVector, input.Norm);

		float specularFactor = pow(max(dot(reflection,input.ViewDirection.xyz),0.0f),MatPower);

		specularColor = MatSpecular * LightSpecular * specularFactor;
	}

	float att = LightAtt0 + LightAtt1*d + LightAtt2*d*d;
	if(cosTheta > cos(LightAlpha / 2))
		finalColor = saturate(ambientColor + diffuseColor + specularColor)/att * Texture.Sample(Sampler,input.Tex);
	else if((cosTheta >= cos(LightBeta / 2)) && (cosTheta <= cos(LightAlpha/2)))
	{
		float spotFactor = pow((cosTheta - cos(LightBeta/2))/
			(cos(LightAlpha/2) - cos(LightBeta/2)),1);
		finalColor = spotFactor * saturate(ambientColor + diffuseColor + specularColor)/att * Texture.Sample(Sampler,input.Tex);
	}
	float4 texColor = Texture.Sample(Sampler, input.Tex);

	finalColor = finalColor * texColor;
	finalColor.a = texColor.a * MatDiffuse.a; //��ȡalphaֵ����͸����
	return finalColor; 
}

//�����Technique
technique11 T_DirLight
{
	pass P0
	{
		//���ö�����ɫ��
		SetVertexShader(CompileShader(vs_5_0,VS()));
		SetPixelShader(CompileShader(ps_5_0,PSDirectionalLight()));
	}
}

//���ԴTechnique
technique11 T_PointLight
{
	pass P0
	{
		//���ö�����ɫ��
		SetVertexShader(CompileShader(vs_5_0,VS()));
		SetPixelShader(CompileShader(ps_5_0,PSPointLight()));
	}
}

//�۹�ƹ�ԴTechnique
technique11 T_SpotLight
{
	pass P0
	{
		//���ö�����ɫ��
		SetVertexShader(CompileShader(vs_5_0,VS()));
		SetPixelShader(CompileShader(ps_5_0,PSSpotLight()));
	}
	pass P1
	{
		//���ö�����ɫ��
		SetVertexShader(CompileShader(vs_5_0,VS()));
		SetPixelShader(CompileShader(ps_5_0,PSSpotLight()));
	}
}

technique11 TexTech
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader( CompileShader( ps_5_0, PS()));
	}
}