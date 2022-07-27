#include "d3dUtility.h"
#include "Light.h"
#include "Camera.h"
#include "ObjLoader.h"
#include "modle.h"
#include <ctime>

HDC hdc,mdc,bufdc;		//二位面板的dc
HBITMAP Loadingpanel,startingpanel,failurepanel,successpanel,bar;		//二维图片句柄
HBITMAP bmp;		//二维面板图片句柄
int lenth = 1;		//进度条长度
std::wstring finalstr;  //死亡时的文字

//声明全局的指针
ID3D11Device* device = NULL;//D3D11设备接口
IDXGISwapChain* swapChain = NULL;//交换链接口
ID3D11DeviceContext* immediateContext = NULL;
ID3D11RenderTargetView* renderTargetView = NULL;//渲染目标视图  

//Effect相关全局指针
ID3D11InputLayout* vertexLayout;
ID3DX11Effect* effect;
ID3DX11EffectTechnique* technique;

//声明三个坐标系矩阵
XMMATRIX world;         //用于世界变换的矩阵
XMMATRIX view;          //用于观察变换的矩阵
XMMATRIX projection;    //用于投影变换的矩阵

//声明材质和光照的全局对象
Material		boxMaterial;      //箱子材质
Material		floorMaterial;    //地板材质
Material		mirrorMaterial;    //水面材质

Light			light[4];      //光源数组
int             lightType =0;  //光源类型

ID3D11DepthStencilView* depthStencilView;  //深度模板视图
ID3D11Texture2D* depthStencilBuffer;       //深度缓存

ID3D11BlendState *blendStateAlpha;        //混合状态
ID3D11BlendState* noColorWriteBS;         //禁止颜色写入

ID3D11RasterizerState* NoCullRS;             //背面消隐状态
ID3D11RasterizerState* counterClockFrontRS;  //改为拟时针为正面

ID3D11DepthStencilState* markMirrorDSS;      //填充镜子区域
ID3D11DepthStencilState* drawReflectionDSS;  //写入镜像

Camera* camera = new Camera();    //摄像机对象
XMVECTOR Eye;                     //视点位置

Modle item;      //物体模型

int mousex0,mousex1;			//鼠标移动前
int mousey0,mousey1;			//鼠标移动后
RECT WINR;						//窗口的矩形
HWND m_handle;					//该窗口句柄

int goal;						//得分

bool loadingcomplete = 0;	//判断是否加载完成
static bool success = 0;    //游戏成功标志
static bool gamefailed = 0; //游戏失败标志
static bool gamestarted = 0;//是否开始游戏标志
static bool loading = 0;	//是否加载标志
static bool viewworld = 0;	//是否查看世界

int windowswidth = 1280;		//窗口长
int windowsheight = 720;		//窗口宽
//碰撞判定函数
//x,y,z代表物体空间的坐标信息，Modle是对所有物体进行判断，最后返回值是bool，碰撞了返回1，未碰撞返回0
//并在里面进行游戏条件判断，撞到天花板底部则游戏失败，物体踩到安全区则游戏成功改变变量gamestarted和success和gamefailed的值
bool collisionDetermination(float x,float y,float z,Modle object); 
///设置模型信息（给Modle类里的参数赋值，且生成随机数，以便于之后绘制随机的方块）
//第一个参数是我定义的模型类，第二个参数是判断是否需要随机数
m_modle modleinfo(m_modle m,bool randneed);
//返回一个(a,b)区间的随机的浮点数
float random(float a,float b);		
//光照函数
void SetLightEffect(Light light);
//初始化函数
void startup();
//播放音乐
void musicFun()
{
	if(!gamestarted&&!gamefailed&&!success)
	{
		PlaySound(L"music/start.wav",NULL,SND_ASYNC|SND_LOOP);
	}
	else if(!gamestarted&&!gamefailed&&success)
	{
		PlaySound(L"music/win.wav",NULL,SND_ASYNC);
	}
	else if(!gamestarted&&gamefailed&&!success)
	{
		PlaySound(L"music/failed.wav",NULL,SND_ASYNC);
	}
	else if(gamestarted)
	{
		PlaySound(L"music/gamingtime.wav",NULL,SND_ASYNC|SND_LOOP);
	}
}

//多线程载入模型
DWORD WINAPI Fun(LPVOID lpParamter)
{

	//载入模型
	lenth = 3;
	item.wall.loadModle(device, "mod/", "wall.obj","wall");
	lenth = 30;
	item.floor.loadModle(device, "mod/", "floor.obj","floor");
	lenth = 80;
	item.celling.loadModle(device, "mod/", "celling.obj","celling");
	lenth = 200;
	item.safeplace.loadModle(device, "mod/", "safeplace.obj","safeplace");
	lenth = 280;
	item.man[0].loadModle(device, "mod/", "man0.obj","man");
	lenth = 350;
	item.man[1].loadModle(device, "mod/", "man1.obj","man");
	lenth = 400;
	item.man[2].loadModle(device, "mod/", "man2.obj","man");
	lenth = 510;
	item.man[3].loadModle(device, "mod/", "man3.obj","man");
	lenth = 600;
	item.celling.modle->setMaterialAlpha(0.7f);
	lenth = 690;
	startup();
	lenth = 700;
	//设置模型信息
	item.celling.setinfo(modleinfo(item.celling,false),false);
	lenth = 720;
	item.safeplace.setinfo(modleinfo(item.safeplace,false),false);
	lenth = 830;
	loadingcomplete = 1;
	return 0L;
}
//绘制二维函数
void MyPaint(HDC hdc);
//定义一个顶点结构，这个顶点包含坐标和法向量和纹理坐标
struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT2 Tex;
	XMFLOAT3 Normal;
};


//**************以下为框架函数******************
bool Setup()
{

	//预处理块
	m_handle = FindWindow(NULL,L"是男人就下100层");
	GetWindowRect(m_handle, &WINR);
	mousex0 = WINR.left + windowswidth/2;
	mousey0 = WINR.top + windowsheight/2;
	SetCursorPos(mousex0,mousey0);
	ShowCursor(false);

	hdc = GetDC(m_handle);
	bmp = CreateCompatibleBitmap(hdc,1280,720);
	mdc = CreateCompatibleDC(hdc);
	bufdc = CreateCompatibleDC(hdc);
	SelectObject(mdc,bmp);
	Loadingpanel = (HBITMAP)LoadImage(NULL,L"pic/loadingbg.bmp",IMAGE_BITMAP,1280,720,LR_LOADFROMFILE);
	bar = (HBITMAP)LoadImage(NULL,L"pic/bar.bmp",IMAGE_BITMAP,524,18,LR_LOADFROMFILE);
	startingpanel = (HBITMAP)LoadImage(NULL,L"pic/gamestart.bmp",IMAGE_BITMAP,1280,720,LR_LOADFROMFILE);
	successpanel = (HBITMAP)LoadImage(NULL,L"pic/gamewin.bmp",IMAGE_BITMAP,1280,720,LR_LOADFROMFILE);
	failurepanel = (HBITMAP)LoadImage(NULL,L"pic/gameover.bmp",IMAGE_BITMAP,1280,720,LR_LOADFROMFILE);

	//这里主要包含5个主要步骤
	//第一步载入外部文件（包括fx文件及图像文件）
	//第二步创建各种渲染状态
	//第三步创建输入布局
	//第四步设置材质和光照
	//*************第一步载入外部文件（包括fx文件及图像文件）****************************
	HRESULT hr = S_OK;              //声明HRESULT的对象用于记录函数调用是否成功
	ID3DBlob* pTechBlob = NULL;     //声明ID3DBlob的对象用于存放从文件读取的信息
	//从我们之前建立的.fx文件读取着色器相关信息
	hr = D3DX11CompileFromFile( L"Shader.fx", NULL, NULL, NULL, "fx_5_0", 
		D3DCOMPILE_ENABLE_STRICTNESS, 0, NULL, &pTechBlob, NULL, NULL );
	if( FAILED(hr) )
	{
		::MessageBox( NULL, L"fx文件载入失败", L"Error", MB_OK ); //如果读取失败，弹出错误信息
		return hr;
	}
	//调用D3DX11CreateEffectFromMemory创建ID3DEffect对象
	hr = D3DX11CreateEffectFromMemory(pTechBlob->GetBufferPointer(), 
		pTechBlob->GetBufferSize(), 0, device, &effect); 

	if( FAILED(hr) )
	{
		::MessageBox( NULL, L"创建Effect失败", L"Error", MB_OK );  //创建失败，弹出错误信息
		return hr;
	}

	//*************第二步创建各种渲染状态************************************************
	//半透明效果
	D3D11_BLEND_DESC blendDesc;                  
	ZeroMemory(&blendDesc, sizeof(blendDesc));   //清零操作
	blendDesc.AlphaToCoverageEnable = false;     //关闭AlphaToCoverage多重采样技术
	blendDesc.IndependentBlendEnable = false;    //不针对多个RenderTarget使用不同的混合状态
	//只针对RenderTarget[0]设置绘制混合状态，忽略1-7
	blendDesc.RenderTarget[0].BlendEnable = true;                   //开启混合
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;     //设置源因子
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;//设置目标因子
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;         //混合操作
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;      //源混合百分比因子
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;    //目标混合百分比因子
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;    //混合百分比的操作
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;  //写掩码
	//创建ID3D11BlendState接口
	device->CreateBlendState(&blendDesc, &blendStateAlpha);

	//关闭背面消隐
	D3D11_RASTERIZER_DESC ncDesc;        //光栅器描述
	ZeroMemory(&ncDesc,sizeof(ncDesc));  //清零操作
	ncDesc.CullMode = D3D11_CULL_NONE;   //剔除特定朝向的三角形，这里不剔除，即全部绘制
	ncDesc.FillMode = D3D11_FILL_SOLID;  //填充模式，这里为利用三角形填充
	ncDesc.FrontCounterClockwise = false;//是否设置逆时针绕续的三角形为正面
	ncDesc.DepthClipEnable = true;       //开启深度裁剪
	//创建一个关闭背面消隐的状态，在需要用的时候才设置给设备上下文
	if(FAILED(device->CreateRasterizerState(&ncDesc,&NoCullRS)))
	{
		MessageBox(NULL,L"Create 'NoCull' rasterizer state failed!",L"Error",MB_OK);
		return false;
	}


	//*************第三步创建输入布局****************************************************
	//用GetTechniqueByName获取ID3DX11EffectTechnique的对象
	//先设置默认的technique到Effect
	technique = effect->GetTechniqueByName("TexTech");                //默认Technique

	//D3DX11_PASS_DESC结构用于描述一个Effect Pass
	D3DX11_PASS_DESC PassDesc;
	//利用GetPassByIndex获取Effect Pass
	//再利用GetDesc获取Effect Pass的描述，并存如PassDesc对象中
	technique->GetPassByIndex(0)->GetDesc(&PassDesc);

	//创建并设置输入布局
	//这里我们定义一个D3D11_INPUT_ELEMENT_DESC数组，
	//由于我们定义的顶点结构包括位置坐标和法向量，所以这个数组里有两个元素
	D3D11_INPUT_ELEMENT_DESC layout [] = 
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	//layout元素个数
	UINT numElements = ARRAYSIZE(layout);
	//调用CreateInputLayout创建输入布局
	hr = device->CreateInputLayout(layout, numElements, PassDesc.pIAInputSignature,
		PassDesc.IAInputSignatureSize, &vertexLayout ); 
	//设置生成的输入布局到设备上下文中
	immediateContext->IASetInputLayout( vertexLayout );
	if( FAILED( hr ) )
	{
		::MessageBox( NULL, L"创建Input Layout失败", L"Error", MB_OK );
		return hr;
	}
	//*************第三步创建输入布局****************************************************

	//*************第四步设置材质和光照**************************************************


	// 设置光源
	Light dirLight, pointLight, spotLight;
	// 方向光只需要设置：方向、3种光照强度
	dirLight.type = 0;
	dirLight.direction = XMFLOAT4(0.0f, -1.0f, 1.0f, 1.0f); 
	dirLight.ambient = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);   //前三位分别表示红绿蓝光的强度
	dirLight.diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);   //同上
	dirLight.specular = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);  //同上

	// 点光源需要设置：位置、3中光照强度、3个衰减因子
	pointLight.type = 1;
	pointLight.position = XMFLOAT4(-4.0f, 3.0f, 0.0f, 1.0f); //光源位置
	pointLight.ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);   //前三位分别表示红绿蓝光的强度
	pointLight.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);   //同上
	pointLight.specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);  //同上
	pointLight.attenuation0 = 0;      //常量衰减因子
	pointLight.attenuation1 = 0.1f;   //一次衰减因子
	pointLight.attenuation2 = 0;      //二次衰减因子

	// 聚光灯需要设置Light结构中所有的成员
	spotLight.type = 2;
	spotLight.position = XMFLOAT4(0.0f, 3.0f, 0.0f, 1.0f); //光源位置
	spotLight.direction = XMFLOAT4(0.0f, -1.0f, 0.0f, 1.0f); //光照方向
	spotLight.ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);   //前三位分别表示红绿蓝光的强度
	spotLight.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);   //同上
	spotLight.specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);  //同上
	spotLight.attenuation0 = 0;    //常量衰减因子
	spotLight.attenuation1 = 0.1f; //一次衰减因子
	spotLight.attenuation2 = 0;    //二次衰减因子
	spotLight.alpha = XM_PI / 6;   //内锥角度
	spotLight.beta = XM_PI / 3;    //外锥角度
	spotLight.fallOff = 1.0f;      //衰减系数，一般为1.0

	Light nolight;
	nolight.type = 3;

	light[0] = dirLight;
	light[1] = pointLight;
	light[2] = spotLight;
	light[3] = nolight;
	//*************第四步设置材质和光照**************************************************

	//*************第五步设置虚拟摄像机视点**********************************************
	Eye =  XMVectorSet( 0.0f, 6.5f, -7.0f, 0.0f );//相机位置
	XMVECTOR AT = XMVectorSet(0.0f,0.0f,0.0f,0.0f);
	camera->SetEye(Eye);   //设置视点位置
	camera->Pitch(XM_PI/4);
	camera->Yaw(XM_PI/4);
	//*************第五步设置虚拟摄像机视点**********************************************
	//*************第六步创建obj对象并载入图像文件***************************************
	//初始化

	item.wall.modle = new ObjLoader(effect);
	item.floor.modle = new ObjLoader(effect);
	item.celling.modle = new ObjLoader(effect);
	item.safeplace.modle = new ObjLoader(effect);
	for(int i = 0;i<4;i++)
		item.man[i].modle = new ObjLoader(effect);
	//创建多线程进行加载
	HANDLE hThread = CreateThread(NULL, 0, Fun, NULL, 0, NULL);
	CloseHandle(hThread);
	musicFun();
	//*************第六步创建obj对象并载入图像文件***************************************
	return true;
}


void startup()
{
	success = 0;    //游戏成功标志
	gamefailed = 0; //游戏失败标志
	viewworld = 0;	//是否查看世界
	goal = 0;		//分数设置为0
	camera = new Camera();
	Eye =  XMVectorSet( 0.0f, 6.5f, -7.0f, 0.0f );//相机位置
	XMVECTOR AT = XMVectorSet(0.0f,0.0f,0.0f,0.0f);
	camera->SetEye(Eye);   //设置视点位置
	camera->Pitch(XM_PI/4);
	camera->Apply();
	item.floor.setinfo(modleinfo(item.floor,true),true);
	item.celling.setinfo(modleinfo(item.celling,false),false);
	/*for(int i = 0;i<4;i++)
	item.man[i].setinfo(modleinfo(item.man[i],false),false);*/
}

void Cleanup()
{
	//释放全局指针
	if(renderTargetView) renderTargetView->Release();
	if(immediateContext) immediateContext->Release();
	if(swapChain) swapChain->Release();
	if(device) device->Release();

	if(vertexLayout) vertexLayout->Release();
	if(effect) effect->Release();

	if(depthStencilView) depthStencilView->Release();  

	if(blendStateAlpha) blendStateAlpha->Release(); 
	if(NoCullRS) NoCullRS->Release(); 
	/*if(item.celling.modle) item.celling.modle->Release();
	if(item.floor.modle) item.floor.modle->Release();
	if(item.wall.modle) item.wall.modle->Release();
	if(item.safeplace.modle) item.safeplace.modle->Release();
	for(int i = 0;i<4;i++)
	if(item.man[i].modle) item.man[i].modle->Release();*/


}

bool Display(float timeDelta)
{
	if( device ) 
	{
		static float cellingheight = 6.0f;
		static float Vcelling = 0.0025f;
		static float transX = 0.0f;
		static float transZ = 0.0f;
		static float transY = 5.0f;
		static float Vx = 0.0f;
		static float Vy = 0.0f;
		static float Vz = 0.0f;
		static float celinga = 0.00001f;
		static float ax = 0.0005f;
		static float ay = 0.00098f;
		static float az = 0.0005f;
		static bool initalpha = 1;
		static float flooralpah[100];
		static int mannum = 0;
		static bool showpanel = 1;
		static float angle = 0.0f;
		showpanel = !viewworld&&!gamestarted||!loadingcomplete;
		if(loadingcomplete&&gamestarted||viewworld)
		{
			//声明一个数组存放颜色信息，4个元素分别表示红，绿，蓝以及alpha
			float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; 
			immediateContext->ClearRenderTargetView( renderTargetView, ClearColor );
			//清除当前绑定的深度模板视图
			immediateContext->ClearDepthStencilView(depthStencilView,D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,1.f,0);
			//先指定混合因子，一般不用它，除非在上面混合因子指定为使用blend factor
			float BlendFactor[] = {0,0,0,0};
			//初始化定义信息


			if(initalpha)
			{
				for(int i = 0;i<100;i++)
				{
					flooralpah[i] = 1.0f;
				}
				initalpha = 0;
			}

			if(viewworld&&loadingcomplete)//游戏未开始时,且进入观察模式，可以移动相机
			{
				//通过键盘改变虚拟摄像头方向.
				if(::GetAsyncKeyState(VK_LEFT) & 0x8000f)  //向左移动
				{
					camera->MoveRightBy(-timeDelta * 12.0f);
				}

				if(::GetAsyncKeyState(VK_RIGHT) & 0x8000f)  //向右移动
				{
					camera->MoveRightBy(timeDelta* 12.0f);
				}

				if(::GetAsyncKeyState(VK_UP) & 0x8000f)  //向前移动
				{
					camera->MoveForwardBy(timeDelta* 12.0f);
				}

				if(::GetAsyncKeyState(VK_DOWN) & 0x8000f)  //向后移动
				{
					camera->MoveForwardBy(-timeDelta* 12.0f);
				}

				if(::GetAsyncKeyState(VK_NUMPAD8) & 0x8000f)  //向上移动
				{
					camera->MoveUpBy(timeDelta* 12.0f);
				}

				if(::GetAsyncKeyState(VK_NUMPAD2) & 0x8000f) //向下移动
				{
					camera->MoveUpBy(-timeDelta* 12.0f);
				}
				//重新生成观察矩阵
				camera->Apply();
				//*****************************开始游戏的接口放在这里


			}
			//初始化世界矩阵
			world = XMMatrixIdentity();   
			//设置投影矩阵
			projection = XMMatrixPerspectiveFovLH( XM_PIDIV2, (float)windowswidth / (float)windowsheight, 0.01f, 100.0f );
			//将坐标变换矩阵的常量缓存中的矩阵和坐标设置到Effect框架中---------------------

			effect->GetVariableByName("World")->AsMatrix()->SetMatrix((float*)&world);  //设置世界坐标系
			XMMATRIX ViewMATRIX =  XMLoadFloat4x4(&camera->GetView());
			effect->GetVariableByName("View")->AsMatrix()->SetMatrix((float*)&ViewMATRIX);    //设置观察坐标系
			effect->GetVariableByName("EyePosition")->AsMatrix()->SetMatrix((float*)&Eye); //设置视点
			effect->GetVariableByName("Projection")->AsMatrix()->SetMatrix((float*)&projection); //设置投影坐标系
			//光源的常量缓存中的光源信息设置到Effect框架中
			SetLightEffect(light[lightType]);
			if((gamestarted||viewworld)&&loadingcomplete)
			{
				if(viewworld)
				{
					if(::GetAsyncKeyState('Q') & 0x8000f) //退出上帝模式
					{
						viewworld = 0;
					}
				}
				if(gamestarted)
				{
					if(::GetAsyncKeyState('Q') & 0x8000f) //直接退出游戏
					{
						gamestarted = 0;
					}
					//增加游戏难度
					if(Vcelling<0.02f&&transY>-100.0f)
					{
						Vcelling += celinga;
					}
					else if(Vcelling<0.024f&&transY>-200.0f)
					{
						Vcelling += celinga;
					}
					else if(Vcelling<0.028f&&transY>-300.0f)
					{
						Vcelling += celinga;
					}
					else if(Vcelling<0.032f&&transY>-400.0f)
					{
						Vcelling += celinga;
					}
					else if(Vcelling<0.036f&&transY>-500.0f)
					{
						Vcelling += celinga;
					}

					cellingheight -= Vcelling;
					item.celling.setbottom(item.celling.getbottom()-Vcelling);
				}
				//**************************************绘制游戏内容************************//

				world = XMMatrixIdentity(); 
				effect->GetVariableByName("World")->AsMatrix()->SetMatrix((float*)&world);
				//*****************************************************绘制墙壁
				item.wall.renderModle(immediateContext, technique);
				//*******************************************************绘制角色
				//操作函数

				if (!collisionDetermination(transX, transY, transZ, item))
				{
					Vy -= ay;
					static float airtime = 0.0f;
					airtime += 10*timeDelta;
					if((int)airtime==1)
					{
						goal++;
						airtime = 0.0f;
					}
				}
				else
				{
					if(Vy<-0.18f&&gamestarted)
					{
						gamestarted = 0;
						gamefailed = 1;
						musicFun();
						finalstr = L"你掉得太快摔死了！得分：" + std::to_wstring(goal);
					}
					Vy = 0;
				}
				if (::GetAsyncKeyState('A') & 0x8000f) //按下左方向键
				{			
					if(Vz<0)
						angle += 2*timeDelta;
					if(Vz>=0)
						angle -= 2*timeDelta;
					if(angle <= XM_PI/2+5*timeDelta&&angle >= XM_PI/2-5*timeDelta)
						angle = XM_PI/2;
					Vx -= ax;//速度按x轴负方向增加，即Vx减小
					if (Vx <= -0.04f) 
						Vx = -0.04f;
				}					
				//if (::GetAsyncKeyState(VK_LEFT) & 0x0f)//松开左方向键
				else
				{
					if (Vx <= 0)//松开后速度慢慢下降最后为0
					{
						Vx += ax;
						if (Vx >= 0) Vx = 0;
					}

				}
				if (::GetAsyncKeyState('D') & 0x8000f) //按下右方向键
				{
					if(Vz>0)
						angle += 2*timeDelta;
					if(Vz<=0)
						angle -= 2*timeDelta;
					if(angle <= 3*XM_PI/2+5*timeDelta&&angle >= 3*XM_PI/2-5*timeDelta)
						angle = 3*XM_PI/2;
					Vx += ax;//速度按x轴正方向增加,及Vx增大
					if (Vx >=0.04f) Vx = 0.04f;
				}
				//if (::GetAsyncKeyState(VK_RIGHT) & 0x0f) //松开右方向键
				else
				{
					if (Vx >= 0)//速度不等于0时，速度降到0
					{
						Vx -= ax;
						if (Vx <= 0) Vx = 0;
					}

				}	
				if (::GetAsyncKeyState('W') & 0x8000f) //按下上方向键
				{
					if(Vx>0)
						angle -= 2*timeDelta;
					if(Vx<=0)
						angle += 2*timeDelta;
					if(angle <= 5*timeDelta&&angle >= -5*timeDelta)
						angle = 0;
					Vz += az;
					if (Vz >= 0.04f) Vz = 0.04f;
				}
				//if (::GetAsyncKeyState(VK_UP) & 0x0f) //松开上方向键
				else
				{ 
					if (Vz >= 0)
					{
						Vz -= az;
						if (Vz <= 0) Vz = 0;
					}
				}
				if (::GetAsyncKeyState('S') & 0x8000f) //按下下方向键
				{
					if(Vx>0)
						angle += 2*timeDelta;
					if(Vx<=0)
						angle -= 2*timeDelta;
					if(angle >= XM_PI-5*timeDelta&&angle <= XM_PI+5*timeDelta)
						angle = XM_PI;
					Vz -= az;
					if(Vz >= 0.04f) Vz = 0.04f;
				}						
				//if (::GetAsyncKeyState(VK_DOWN) & 0x0f) //松开下方向键
				else
				{
					if (Vz <= 0)
					{
						Vz += az;
						if (Vz >= 0) Vz = 0;
					}
				}  
				static bool jumping = 0;
				if((::GetAsyncKeyState(VK_SPACE) & 0x8000f)&&!jumping)
				{
					Vy = 0.03f;
					jumping = 1;
				}
				else
				{
					jumping = 0;
				}
				if(Vx!=0.0f||Vz!=0.0f)
				{
					mannum++;
					if(mannum>3)
						mannum = 0;
				}
				transX += Vx;
				transY += Vy;
				transZ += Vz;
				if(transX >= 5.0f) transX = 5.0f;       //不让人物移出墙右边界
				if(transX <= -5.0f) transX = -5.0f;     //不让人物移出墙左边界
				if(transZ >= 4.0f) transZ = 4.0f;       //不让人物移出墙前边界
				if(transZ <= -4.0f) transZ = -4.0f;     //不让人物移出墙后边界
				if(!viewworld)
				{
					camera->SetEye(XMVectorSet( 0.0f, transY+6.5f, -7.0f, 0.0f ));
					camera->Apply();
				}

				world = XMMatrixIdentity();
				world =   XMMatrixRotationY(angle)*XMMatrixTranslation(transX, transY, transZ);
				effect->GetVariableByName("World")->AsMatrix()->SetMatrix((float*)&world);  //设置世界坐标系

				item.man[mannum].renderModle(immediateContext, technique);
				//***************************************************绘制安全区
				world = XMMatrixIdentity(); 
				world = XMMatrixTranslation(0.0f,-600.0f,0.0f);
				effect->GetVariableByName("World")->AsMatrix()->SetMatrix((float*)&world);
				item.safeplace.renderModle(immediateContext, technique);
				//*************************************************************这里要用透明来绘制天花板和踩踏板
				immediateContext->OMSetBlendState(blendStateAlpha,BlendFactor,0xffffffff);
				immediateContext->RSSetState(NoCullRS);


				for(int i = 0;i<100;i++)
				{
					if(i == 0)
					{
						world = XMMatrixIdentity();
						world = XMMatrixTranslation(0.0f, 0.0f, 0.0f);//第一个方块，不采用随机数
						effect->GetVariableByName("World")->AsMatrix()->SetMatrix((float*)&world);  //设置世界坐标系
						item.floor.renderModle(immediateContext, technique);
					}
					else
					{
						world = XMMatrixIdentity();
						world = XMMatrixTranslation(item.floor.getrandx(i), item.floor.getrandy(i)-6.0f*(float)(i+1), item.floor.getrandz(i));
						effect->GetVariableByName("World")->AsMatrix()->SetMatrix((float*)&world);  //设置世界坐标系
						item.floor.renderModle(immediateContext, technique);
					}
					flooralpah[i] = 0.3f;
					if(transY<item.floor.getfloorbottom(i)-1.0f)
					{
						item.floor.setfloorleft(i,100.f);//如果落下超过了地板的底部，将其信息移开	
						world = world * XMMatrixTranslation(100.0f,0.0f,0.0f);
						effect->GetVariableByName("World")->AsMatrix()->SetMatrix((float*)&world);
					}
					item.floor.modle->setMaterialAlpha(flooralpah[i]);
				}
				//********************************天花板*****************
				world = XMMatrixIdentity();
				world = XMMatrixTranslation(0.0f, cellingheight, 0.0f);//设置天花板位置
				effect->GetVariableByName("World")->AsMatrix()->SetMatrix((float*)&world);  //设置世界坐标系
				//绘制天花板
				item.celling.renderModle(immediateContext, technique);


				immediateContext->OMSetBlendState(0,0,0xffffffff);
				immediateContext->RSSetState(0);

			}

			swapChain->Present( 0, 0 );
		}

		else if(showpanel)
		{
			MyPaint(hdc);
		}
		if(loading&&loadingcomplete)
		{
			//这里填写加载项，相机位置初始化等
			transX = 0.0f;
			transY = 0.0f;
			transZ = 0.0f;
			Vcelling = 0.015f;
			cellingheight = 6.0f;
			startup();
			gamestarted = 1; //加载完毕，开始游戏
			loading = 0;//加载完后，停止加载
			musicFun();
		}
	}
	return true;
}
//**************框架函数******************


//
// 回调函数
//
LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch( msg )
	{
	case WM_DESTROY:
		::PostQuitMessage(0);
		break;

	case WM_MOUSEMOVE:
		if(!gamestarted)
		{
			POINT MP;
			GetCursorPos(&MP);
			mousex1 = MP.x;
			mousey1 = MP.y;
			camera->Yaw((float)(mousex1-mousex0)/100);
			camera->Pitch((float)(mousey1-mousey0)/100);
			SetCursorPos(mousex0,mousey0);
		}
		break;
	case WM_KEYDOWN:
		if( wParam == VK_ESCAPE )
			::DestroyWindow(hwnd);
		if(wParam == VK_F1)  //按F1键将光源类型改为方向光
			lightType = 0;
		if(wParam == VK_F2)  //按F2键将光源类型改为点光源
			lightType = 1;
		if(wParam == VK_F3)  //按F3键将光源类型改为聚光灯光源
			lightType = 2;
		if(wParam == VK_F4)  //按F4键将光源类型改为无光照
			lightType = 3;
		if(wParam == VK_RETURN&&loadingcomplete&&!gamestarted)
		{
			loading = 1;
		}
		if(wParam == 'V'&&!gamestarted)
			viewworld = 1;
		break;
	}
	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

//
// 主函数WinMain
//
int WINAPI WinMain(HINSTANCE hinstance,
				   HINSTANCE prevInstance, 
				   PSTR cmdLine,
				   int showCmd)
{

	//初始化
	//**注意**:最上面声明的IDirect3DDevice9指针，在这里作为参数传给InitD3D函数
	if(!d3d::InitD3D(hinstance,
		1280,
		720,
		&renderTargetView,
		&immediateContext,
		&swapChain,     
		&device,
		&depthStencilBuffer,
		&depthStencilView))// [out]The created device.
	{
		::MessageBox(0, L"InitD3D() - FAILED", 0, 0);
		return 0;
	}

	if(!Setup())
	{
		::MessageBox(0, L"Setup() - FAILED", 0, 0);
		return 0;
	}

	d3d::EnterMsgLoop( Display );

	Cleanup();

	return 0;
}

//光源的常量缓存设置到Effect框架中
//由于光照设置比较复杂，所以以一个函数来进行设置
void SetLightEffect(Light light)
{
	//首先将光照类型，环境光强度，漫射光强度，镜面光强度设置到Effect中
	effect->GetVariableByName("type")->AsScalar()->SetInt(light.type);
	effect->GetVariableByName("LightAmbient")->AsVector()->SetFloatVector((float*)&(light.ambient));
	effect->GetVariableByName("LightDiffuse")->AsVector()->SetFloatVector((float*)&(light.diffuse));
	effect->GetVariableByName("LightSpecular")->AsVector()->SetFloatVector((float*)&(light.specular));

	//下面根据光照类型的不同设置不同的属性
	if(light.type == 0)  //方向光
	{
		//方向光只需要“方向”这个属性即可
		effect->GetVariableByName("LightDirection")->AsVector()->SetFloatVector((float*)&(light.direction));
		//将方向光的Tectnique设置到Effect
		technique = effect->GetTechniqueByName("T_DirLight");
	}
	else if(light.type == 1)  //点光源
	{
		//点光源需要“位置”，“常量衰变因子”，“一次衰变因子”，“二次衰变因子”
		effect->GetVariableByName("LightPosition")->AsVector()->SetFloatVector((float*)&(light.position));
		effect->GetVariableByName("LightAtt0")->AsScalar()->SetFloat(light.attenuation0);
		effect->GetVariableByName("LightAtt1")->AsScalar()->SetFloat(light.attenuation1);
		effect->GetVariableByName("LightAtt2")->AsScalar()->SetFloat(light.attenuation2);

		//将点光源的Tectnique设置到Effect
		technique = effect->GetTechniqueByName("T_PointLight");
	}
	else if(light.type == 2) //聚光灯光源
	{
		//点光源需要“方向”，“方向”，“常量衰变因子”，“一次衰变因子”，“二次衰变因子”
		//“内锥角度”，“外锥角度”，“聚光灯衰减系数”
		effect->GetVariableByName("LightPosition")->AsVector()->SetFloatVector((float*)&(light.position));
		effect->GetVariableByName("LightDirection")->AsVector()->SetFloatVector((float*)&(light.direction));

		effect->GetVariableByName("LightAtt0")->AsScalar()->SetFloat(light.attenuation0);
		effect->GetVariableByName("LightAtt1")->AsScalar()->SetFloat(light.attenuation1);
		effect->GetVariableByName("LightAtt2")->AsScalar()->SetFloat(light.attenuation2);

		effect->GetVariableByName("LightAlpha")->AsScalar()->SetFloat(light.alpha);
		effect->GetVariableByName("LightBeta")->AsScalar()->SetFloat(light.beta);
		effect->GetVariableByName("LightFallOff")->AsScalar()->SetFloat(light.fallOff);

		//将聚光灯光源的Tectnique设置到Effect
		technique = effect->GetTechniqueByName("T_SpotLight");
	}
	else if(light.type == 3)
	{
		technique = effect->GetTechniqueByName("TexTech");
	}
}

//给模型赋值
m_modle modleinfo(m_modle m,bool randneed)
{
	std::vector<ObjVertex>	Verticesinfo = m.modle->getVertexInfo();
	int vcount = Verticesinfo.size();
	m.setleft(Verticesinfo[0].vPosition.x);
	m.setright(Verticesinfo[0].vPosition.x);
	m.setforward(Verticesinfo[0].vPosition.z);
	m.setback(Verticesinfo[0].vPosition.z);
	m.settop(Verticesinfo[0].vPosition.y);
	m.setbottom(Verticesinfo[0].vPosition.y);
	//如果需要随机，则储存
	if(randneed)
	{
		srand((int)time(0));
		for(int j = 0;j<100;j++)
		{

			m.setrandx(j, random(-5.0f,5.0f));
			m.setrandy(j, random(-2.0f,2.0f));
			m.setrandz(j, random(-5.0f,5.0f));
		}
	}
	for(int i=0;i<vcount;i++)
	{
		Verticesinfo[i].vPosition;//顶点信息例如：（0,1,0）为y正方向，即top，储存top为所有点中y值最大的那个
		//接下来，遍历所有点的信息，分别对m里面物体的left,right,forward,back,top,bottom进行赋值
		//我写一个例子：
		if(m.getleft()>Verticesinfo[i].vPosition.x)
		{
			m.setleft(Verticesinfo[i].vPosition.x);  //如果获取的点的x坐标比之前的m中left坐标更小（即更偏左）则设置
		}
		//*********************************此处继续编写*************************************
		if (m.getright() < Verticesinfo[i].vPosition.x)
		{
			m.setright(Verticesinfo[i].vPosition.x);  //如果获取的点的x坐标比之前的m中right坐标更大（即更偏右）则设置
		}
		if (m.getforward() < Verticesinfo[i].vPosition.z)
		{
			m.setforward(Verticesinfo[i].vPosition.z);  //如果获取的点的z坐标比之前的m中forward坐标更小（即更偏前）则设置
		}
		if (m.getback() > Verticesinfo[i].vPosition.z)
		{
			m.setback(Verticesinfo[i].vPosition.z);  //如果获取的点的z坐标比之前的m中back坐标更大（即更偏后）则设置
		}
		if (m.gettop() < Verticesinfo[i].vPosition.y)
		{
			m.settop(Verticesinfo[i].vPosition.y);  //如果获取的点的y坐标比之前的m中top坐标更大（即更偏上）则设置
		}
		if (m.getbottom() > Verticesinfo[i].vPosition.y)
		{
			m.setbottom(Verticesinfo[i].vPosition.y);  //如果获取的点的y坐标比之前的m中bottom坐标更小（即更偏下）则设置
		}
	}
	if(m.getname() == "safeplace")
	{
		m.settop(m.gettop()-600.0f);
		m.setbottom(m.getbottom()-600.0f);
	}
	if(m.getname() == "celling")
	{
		m.settop(m.gettop()+6.0f);
		m.setbottom(m.getbottom()+6.0f);
	}

	if(m.getname() == "floor")//如果需要随机，则要在相应的地方加上随机数
	{
		m.setfloorleft(0,m.getleft());
		m.setfloorright(0,m.getright());
		m.setfloorforward(0,m.getforward());
		m.setfloorback(0,m.getback());
		m.setfloortop(0,m.gettop());
		m.setfloorbottom(0,m.getbottom());
		for(int i = 1;i<100;i++)
		{
			m.setfloorleft(i,m.getleft() + m.getrandx(i));
			m.setfloorright(i,m.getright() + m.getrandx(i));
			m.setfloorforward(i,m.getforward() + m.getrandz(i));
			m.setfloorback(i,m.getback() + m.getrandz(i));
			m.setfloortop(i,m.gettop()-6.0f*(i+1) + m.getrandy(i));
			m.setfloorbottom(i,m.getbottom()-6.0f*(i+1) + m.getrandy(i));
		}
	}
	return m;
}

//产生范围内随机浮点数
float random(float a,float b)
{
	float fnum = (float)rand()/(float)(RAND_MAX);
	int num = rand()%(int)(b-a+1)+(int)a;
	return (float)num + fnum;
}

//碰撞判定函数
//x,y,z代表物体空间的坐标信息，Modle是对所有物体进行判断，最后返回值是bool，碰撞了返回1，未碰撞返回0
//并在里面进行游戏条件判断，撞到天花板底部则游戏失败，物体踩到安全区则游戏成功改变变量gamestarted和success和gamefailed的值
bool collisionDetermination(float x,float y,float z,Modle object)
{
	//****************************交给你们编写了，不懂的qq上问我

	if (y + 1.0f >= object.celling.getbottom())
	{
		gamestarted = 0;
		gamefailed = 1;
		musicFun();
		finalstr = L"你被天花板扎死了！得分：" + std::to_wstring(goal);
		return true;
	}		
	for (int i = 0; i < 100; i++)
	{		
		if ((y + 0.5f <= object.floor.getfloortop(i))&&(x+0.95f>=object.floor.getfloorleft(i)) &&( x-0.95f<=object.floor.getfloorright(i)) && (z+0.95f >= object.floor.getfloorback(i)) && (z-0.95f <= object.floor.getfloorforward(i)))
			return true;		
	}		
	if (y - 0.5 <= object.safeplace.gettop())		
	{			
		gamestarted = 0;			
		success = 1;
		musicFun();
		finalstr = L"你成功啦！得分：" + std::to_wstring(goal);
		return true;		
	}
	return false;
}


void MyPaint(HDC hdc)
{
	if(!loadingcomplete)
	{
		SelectObject(bufdc,Loadingpanel);
		BitBlt(mdc,0,0,1280,720,bufdc,0,0,SRCCOPY);
		SelectObject(bufdc,bar);
		BitBlt(mdc,300,350,lenth,18,bufdc,0,0,SRCCOPY);
		TextOut(mdc,500,400,L"加载中。。。。",7);
	}
	else if(!gamestarted&&!gamefailed&&!success)
	{
		SelectObject(bufdc,startingpanel);
		BitBlt(mdc,0,0,1280,720,bufdc,0,0,SRCCOPY);
	}
	else if(!gamestarted&&gamefailed&&!success)
	{
		SelectObject(bufdc,failurepanel);
		BitBlt(mdc,0,0,1280,720,bufdc,0,0,SRCCOPY);
		TextOut(mdc,530,300,finalstr.c_str(),finalstr.length());
	}
	else if(!gamestarted&&!gamefailed&&success)
	{
		SelectObject(bufdc,successpanel);
		BitBlt(mdc,0,0,1280,720,bufdc,0,0,SRCCOPY);
		TextOut(mdc,530,300,finalstr.c_str(),finalstr.length());
	}

	BitBlt(hdc,0,0,1280,720,mdc,0,0,SRCCOPY);
}
