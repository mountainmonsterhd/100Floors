//*********************************
//定义常量缓存
//*********************
//坐标变换矩阵的常量缓存
cbuffer MatrixBuffer
{
	matrix World;		//世界变换矩阵，matrix表示4x4矩阵
	matrix View;		//观察变换矩阵
	matrix Projection;	//投影变换矩阵
	float4 EyePosition; //视点位置
};

//材质信息的常量缓存
cbuffer MaterialBuffer
{
	float4 MatAmbient;		//材质对环境光的反射率
	float4 MatDiffuse;		//材质对漫反射光的反射率
	float4 MatSpecular;		//材质对镜面光的反射率
	float MatPower;			//材质的镜面光反射系数
}

//光源的常量缓存
cbuffer LightBuffer
{
	int type;						//光源类型
	float4 LightPosition;			//光源位置
	float4 LightDirection;			//光源方向
	float4 LightAmbient;			//环境光强度
	float4 LightDiffuse;			//漫反射光强度
	float4 LightSpecular;			//镜面光强度
	float LightAtt0;				//常量衰减因子
	float LightAtt1;				//一次衰减因子
	float LightAtt2;				//二次衰减因子
	float LightAlpha;				//聚光灯内锥角度
	float LightBeta;				//聚光灯外锥角度
	float LightFallOff;				//聚光灯衰减系数
}


Texture2D Texture;		//纹理变量

//定义采样器
SamplerState Sampler
{
	Filter = MIN_MAG_MIP_LINEAR;		//采用线性过滤
	AddressU = MIRROR;					//寻址模式为MIRROR
	AddressV = MIRROR;					//寻址模式为MIRROR
};


//*****************************
//定义输入结构
//**************************
//顶点着色器的输入

struct VS_INPUT
{
	float4 Pos : POSITION;	//顶点坐标
	float3 Norm : NORMAL;		//法向量
	float2 Tex : TEXCOORD0;    //纹理坐标
};
//顶点着色器的输出结构
struct VS_OUTPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
	float3 Norm : TEXCOORD1;
	float4 ViewDirection : TEXCOORD2;
	float4 LightVector : TEXCOORD3;
};
//像素着色器的输入
//struct PS_INPUT
//{
//	float4 Pos : SV_POSITION;
//	float2 Tex : TEXCOORD0;
//	float3 Norm : TEXCOORD1;
//	float4 ViewDirection : TEXCOORD2;
//	float4 LightVector : TEXCOORD3;
//};

//*********************
//定义各类着色器
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

//顶点着色器
VS_OUTPUT VS( VS_INPUT input )
{
	VS_OUTPUT output = (VS_OUTPUT)0;				//声明一个VS_OUTPUT的对象

	output.Pos = mul( input.Pos, World );			//世界变换
	output.Pos = mul( output.Pos, View );			//观察变换
	output.Pos = mul( output.Pos, Projection );		//投影变换

	output.Norm = mul( input.Norm, (float3x3)World );				//获得方向向量
	output.Norm = normalize( output.Norm );							//对法向量进行归一化

	float4 worldPosition = mul( input.Pos, World );					//获取顶点的世界坐标
	output.ViewDirection = EyePosition - worldPosition;				//获取视点方向
	output.ViewDirection = normalize( output.ViewDirection );		//视点方向归一化

	output.LightVector = LightPosition - worldPosition;				//获取光照方向
	output.LightVector = normalize( output.LightVector );			//光照方向归一化
	output.LightVector.w = length( LightPosition - worldPosition);	//光照距离
	
	output.Tex = input.Tex;

	return output;
}

float4 PS(VS_OUTPUT input) : SV_Target
{
	float4 texColor = Texture.Sample(Sampler, input.Tex);
	float4 finalColor;
	finalColor = texColor;
	finalColor.a = texColor.a * MatDiffuse.a; //获取alpha值，即透明度
	return finalColor;		//返回纹理
}

//平行光源像素着色器
float4 PSDirectionalLight( VS_OUTPUT input) : SV_Target
{
	float4 finalColor;					//声明最终颜色
	//声明环境光、漫反射光、镜面光颜色
	float4 ambientColor, diffuseColor , specularColor;
	//光照方向和光线照射方向相反。
	float3 lightVector = -LightDirection.xyz;
	lightVector = normalize( lightVector );
	//用材质环境光反射率和环境光强度相乘得到环境光颜色
	ambientColor = MatAmbient * LightAmbient;
	//将顶点法向量和光照方向进行点乘得到漫反射光因子
	float diffuseFactor = dot(lightVector, input.Norm);
	if(diffuseFactor > 0.0f)
	{
		//用材质的满泛着光的反射率和漫反射光的光照强度以及漫反射光因子相乘得到漫反射光颜色
		diffuseColor = MatDiffuse * LightDiffuse * diffuseFactor;
		//根据光照方向和顶点法向量计算反射方向
		float3 reflection = reflect(-lightVector, input.Norm);
		//根据反射方向，视点方向以及材质的镜面光反射系数来计算镜面反射因子
		float specularFactor = pow(max(dot(reflection,input.ViewDirection.xyz),0.0f),MatPower);
		//材质的镜面反射率，镜面光强度以及镜面反射因子相乘得到镜面光颜色
		specularColor = MatSpecular * LightSpecular * specularFactor;
	}
	float4 texColor = Texture.Sample(Sampler, input.Tex);

	finalColor = saturate(ambientColor + diffuseColor + specularColor);
	finalColor = finalColor * texColor;
	finalColor.a = texColor.a * MatDiffuse.a; //获取alpha值，即透明度
	return finalColor;
}

//点光源像素着色器
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
	finalColor.a = texColor.a * MatDiffuse.a; //获取alpha值，即透明度
	return finalColor; 
}

//聚光灯像素着色器
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
	finalColor.a = texColor.a * MatDiffuse.a; //获取alpha值，即透明度
	return finalColor; 
}

//方向光Technique
technique11 T_DirLight
{
	pass P0
	{
		//设置顶点着色器
		SetVertexShader(CompileShader(vs_5_0,VS()));
		SetPixelShader(CompileShader(ps_5_0,PSDirectionalLight()));
	}
}

//点光源Technique
technique11 T_PointLight
{
	pass P0
	{
		//设置顶点着色器
		SetVertexShader(CompileShader(vs_5_0,VS()));
		SetPixelShader(CompileShader(ps_5_0,PSPointLight()));
	}
}

//聚光灯光源Technique
technique11 T_SpotLight
{
	pass P0
	{
		//设置顶点着色器
		SetVertexShader(CompileShader(vs_5_0,VS()));
		SetPixelShader(CompileShader(ps_5_0,PSSpotLight()));
	}
	pass P1
	{
		//设置顶点着色器
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