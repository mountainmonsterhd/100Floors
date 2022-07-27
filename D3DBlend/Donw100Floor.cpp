#include "d3dUtility.h"
#include "Light.h"
#include "Camera.h"
#include "ObjLoader.h"
#include "modle.h"
#include <ctime>

HDC hdc,mdc,bufdc;		//��λ����dc
HBITMAP Loadingpanel,startingpanel,failurepanel,successpanel,bar;		//��άͼƬ���
HBITMAP bmp;		//��ά���ͼƬ���
int lenth = 1;		//����������
std::wstring finalstr;  //����ʱ������

//����ȫ�ֵ�ָ��
ID3D11Device* device = NULL;//D3D11�豸�ӿ�
IDXGISwapChain* swapChain = NULL;//�������ӿ�
ID3D11DeviceContext* immediateContext = NULL;
ID3D11RenderTargetView* renderTargetView = NULL;//��ȾĿ����ͼ  

//Effect���ȫ��ָ��
ID3D11InputLayout* vertexLayout;
ID3DX11Effect* effect;
ID3DX11EffectTechnique* technique;

//������������ϵ����
XMMATRIX world;         //��������任�ľ���
XMMATRIX view;          //���ڹ۲�任�ľ���
XMMATRIX projection;    //����ͶӰ�任�ľ���

//�������ʺ͹��յ�ȫ�ֶ���
Material		boxMaterial;      //���Ӳ���
Material		floorMaterial;    //�ذ����
Material		mirrorMaterial;    //ˮ�����

Light			light[4];      //��Դ����
int             lightType =0;  //��Դ����

ID3D11DepthStencilView* depthStencilView;  //���ģ����ͼ
ID3D11Texture2D* depthStencilBuffer;       //��Ȼ���

ID3D11BlendState *blendStateAlpha;        //���״̬
ID3D11BlendState* noColorWriteBS;         //��ֹ��ɫд��

ID3D11RasterizerState* NoCullRS;             //��������״̬
ID3D11RasterizerState* counterClockFrontRS;  //��Ϊ��ʱ��Ϊ����

ID3D11DepthStencilState* markMirrorDSS;      //��侵������
ID3D11DepthStencilState* drawReflectionDSS;  //д�뾵��

Camera* camera = new Camera();    //���������
XMVECTOR Eye;                     //�ӵ�λ��

Modle item;      //����ģ��

int mousex0,mousex1;			//����ƶ�ǰ
int mousey0,mousey1;			//����ƶ���
RECT WINR;						//���ڵľ���
HWND m_handle;					//�ô��ھ��

int goal;						//�÷�

bool loadingcomplete = 0;	//�ж��Ƿ�������
static bool success = 0;    //��Ϸ�ɹ���־
static bool gamefailed = 0; //��Ϸʧ�ܱ�־
static bool gamestarted = 0;//�Ƿ�ʼ��Ϸ��־
static bool loading = 0;	//�Ƿ���ر�־
static bool viewworld = 0;	//�Ƿ�鿴����

int windowswidth = 1280;		//���ڳ�
int windowsheight = 720;		//���ڿ�
//��ײ�ж�����
//x,y,z��������ռ��������Ϣ��Modle�Ƕ�������������жϣ���󷵻�ֵ��bool����ײ�˷���1��δ��ײ����0
//�������������Ϸ�����жϣ�ײ���컨��ײ�����Ϸʧ�ܣ�����ȵ���ȫ������Ϸ�ɹ��ı����gamestarted��success��gamefailed��ֵ
bool collisionDetermination(float x,float y,float z,Modle object); 
///����ģ����Ϣ����Modle����Ĳ�����ֵ����������������Ա���֮���������ķ��飩
//��һ���������Ҷ����ģ���࣬�ڶ����������ж��Ƿ���Ҫ�����
m_modle modleinfo(m_modle m,bool randneed);
//����һ��(a,b)���������ĸ�����
float random(float a,float b);		
//���պ���
void SetLightEffect(Light light);
//��ʼ������
void startup();
//��������
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

//���߳�����ģ��
DWORD WINAPI Fun(LPVOID lpParamter)
{

	//����ģ��
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
	//����ģ����Ϣ
	item.celling.setinfo(modleinfo(item.celling,false),false);
	lenth = 720;
	item.safeplace.setinfo(modleinfo(item.safeplace,false),false);
	lenth = 830;
	loadingcomplete = 1;
	return 0L;
}
//���ƶ�ά����
void MyPaint(HDC hdc);
//����һ������ṹ����������������ͷ���������������
struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT2 Tex;
	XMFLOAT3 Normal;
};


//**************����Ϊ��ܺ���******************
bool Setup()
{

	//Ԥ������
	m_handle = FindWindow(NULL,L"�����˾���100��");
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

	//������Ҫ����5����Ҫ����
	//��һ�������ⲿ�ļ�������fx�ļ���ͼ���ļ���
	//�ڶ�������������Ⱦ״̬
	//�������������벼��
	//���Ĳ����ò��ʺ͹���
	//*************��һ�������ⲿ�ļ�������fx�ļ���ͼ���ļ���****************************
	HRESULT hr = S_OK;              //����HRESULT�Ķ������ڼ�¼���������Ƿ�ɹ�
	ID3DBlob* pTechBlob = NULL;     //����ID3DBlob�Ķ������ڴ�Ŵ��ļ���ȡ����Ϣ
	//������֮ǰ������.fx�ļ���ȡ��ɫ�������Ϣ
	hr = D3DX11CompileFromFile( L"Shader.fx", NULL, NULL, NULL, "fx_5_0", 
		D3DCOMPILE_ENABLE_STRICTNESS, 0, NULL, &pTechBlob, NULL, NULL );
	if( FAILED(hr) )
	{
		::MessageBox( NULL, L"fx�ļ�����ʧ��", L"Error", MB_OK ); //�����ȡʧ�ܣ�����������Ϣ
		return hr;
	}
	//����D3DX11CreateEffectFromMemory����ID3DEffect����
	hr = D3DX11CreateEffectFromMemory(pTechBlob->GetBufferPointer(), 
		pTechBlob->GetBufferSize(), 0, device, &effect); 

	if( FAILED(hr) )
	{
		::MessageBox( NULL, L"����Effectʧ��", L"Error", MB_OK );  //����ʧ�ܣ�����������Ϣ
		return hr;
	}

	//*************�ڶ�������������Ⱦ״̬************************************************
	//��͸��Ч��
	D3D11_BLEND_DESC blendDesc;                  
	ZeroMemory(&blendDesc, sizeof(blendDesc));   //�������
	blendDesc.AlphaToCoverageEnable = false;     //�ر�AlphaToCoverage���ز�������
	blendDesc.IndependentBlendEnable = false;    //����Զ��RenderTargetʹ�ò�ͬ�Ļ��״̬
	//ֻ���RenderTarget[0]���û��ƻ��״̬������1-7
	blendDesc.RenderTarget[0].BlendEnable = true;                   //�������
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;     //����Դ����
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;//����Ŀ������
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;         //��ϲ���
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;      //Դ��ϰٷֱ�����
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;    //Ŀ���ϰٷֱ�����
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;    //��ϰٷֱȵĲ���
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;  //д����
	//����ID3D11BlendState�ӿ�
	device->CreateBlendState(&blendDesc, &blendStateAlpha);

	//�رձ�������
	D3D11_RASTERIZER_DESC ncDesc;        //��դ������
	ZeroMemory(&ncDesc,sizeof(ncDesc));  //�������
	ncDesc.CullMode = D3D11_CULL_NONE;   //�޳��ض�����������Σ����ﲻ�޳�����ȫ������
	ncDesc.FillMode = D3D11_FILL_SOLID;  //���ģʽ������Ϊ�������������
	ncDesc.FrontCounterClockwise = false;//�Ƿ�������ʱ��������������Ϊ����
	ncDesc.DepthClipEnable = true;       //������Ȳü�
	//����һ���رձ���������״̬������Ҫ�õ�ʱ������ø��豸������
	if(FAILED(device->CreateRasterizerState(&ncDesc,&NoCullRS)))
	{
		MessageBox(NULL,L"Create 'NoCull' rasterizer state failed!",L"Error",MB_OK);
		return false;
	}


	//*************�������������벼��****************************************************
	//��GetTechniqueByName��ȡID3DX11EffectTechnique�Ķ���
	//������Ĭ�ϵ�technique��Effect
	technique = effect->GetTechniqueByName("TexTech");                //Ĭ��Technique

	//D3DX11_PASS_DESC�ṹ��������һ��Effect Pass
	D3DX11_PASS_DESC PassDesc;
	//����GetPassByIndex��ȡEffect Pass
	//������GetDesc��ȡEffect Pass��������������PassDesc������
	technique->GetPassByIndex(0)->GetDesc(&PassDesc);

	//�������������벼��
	//�������Ƕ���һ��D3D11_INPUT_ELEMENT_DESC���飬
	//�������Ƕ���Ķ���ṹ����λ������ͷ��������������������������Ԫ��
	D3D11_INPUT_ELEMENT_DESC layout [] = 
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	//layoutԪ�ظ���
	UINT numElements = ARRAYSIZE(layout);
	//����CreateInputLayout�������벼��
	hr = device->CreateInputLayout(layout, numElements, PassDesc.pIAInputSignature,
		PassDesc.IAInputSignatureSize, &vertexLayout ); 
	//�������ɵ����벼�ֵ��豸��������
	immediateContext->IASetInputLayout( vertexLayout );
	if( FAILED( hr ) )
	{
		::MessageBox( NULL, L"����Input Layoutʧ��", L"Error", MB_OK );
		return hr;
	}
	//*************�������������벼��****************************************************

	//*************���Ĳ����ò��ʺ͹���**************************************************


	// ���ù�Դ
	Light dirLight, pointLight, spotLight;
	// �����ֻ��Ҫ���ã�����3�ֹ���ǿ��
	dirLight.type = 0;
	dirLight.direction = XMFLOAT4(0.0f, -1.0f, 1.0f, 1.0f); 
	dirLight.ambient = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);   //ǰ��λ�ֱ��ʾ���������ǿ��
	dirLight.diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);   //ͬ��
	dirLight.specular = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);  //ͬ��

	// ���Դ��Ҫ���ã�λ�á�3�й���ǿ�ȡ�3��˥������
	pointLight.type = 1;
	pointLight.position = XMFLOAT4(-4.0f, 3.0f, 0.0f, 1.0f); //��Դλ��
	pointLight.ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);   //ǰ��λ�ֱ��ʾ���������ǿ��
	pointLight.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);   //ͬ��
	pointLight.specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);  //ͬ��
	pointLight.attenuation0 = 0;      //����˥������
	pointLight.attenuation1 = 0.1f;   //һ��˥������
	pointLight.attenuation2 = 0;      //����˥������

	// �۹����Ҫ����Light�ṹ�����еĳ�Ա
	spotLight.type = 2;
	spotLight.position = XMFLOAT4(0.0f, 3.0f, 0.0f, 1.0f); //��Դλ��
	spotLight.direction = XMFLOAT4(0.0f, -1.0f, 0.0f, 1.0f); //���շ���
	spotLight.ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);   //ǰ��λ�ֱ��ʾ���������ǿ��
	spotLight.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);   //ͬ��
	spotLight.specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);  //ͬ��
	spotLight.attenuation0 = 0;    //����˥������
	spotLight.attenuation1 = 0.1f; //һ��˥������
	spotLight.attenuation2 = 0;    //����˥������
	spotLight.alpha = XM_PI / 6;   //��׶�Ƕ�
	spotLight.beta = XM_PI / 3;    //��׶�Ƕ�
	spotLight.fallOff = 1.0f;      //˥��ϵ����һ��Ϊ1.0

	Light nolight;
	nolight.type = 3;

	light[0] = dirLight;
	light[1] = pointLight;
	light[2] = spotLight;
	light[3] = nolight;
	//*************���Ĳ����ò��ʺ͹���**************************************************

	//*************���岽��������������ӵ�**********************************************
	Eye =  XMVectorSet( 0.0f, 6.5f, -7.0f, 0.0f );//���λ��
	XMVECTOR AT = XMVectorSet(0.0f,0.0f,0.0f,0.0f);
	camera->SetEye(Eye);   //�����ӵ�λ��
	camera->Pitch(XM_PI/4);
	camera->Yaw(XM_PI/4);
	//*************���岽��������������ӵ�**********************************************
	//*************����������obj��������ͼ���ļ�***************************************
	//��ʼ��

	item.wall.modle = new ObjLoader(effect);
	item.floor.modle = new ObjLoader(effect);
	item.celling.modle = new ObjLoader(effect);
	item.safeplace.modle = new ObjLoader(effect);
	for(int i = 0;i<4;i++)
		item.man[i].modle = new ObjLoader(effect);
	//�������߳̽��м���
	HANDLE hThread = CreateThread(NULL, 0, Fun, NULL, 0, NULL);
	CloseHandle(hThread);
	musicFun();
	//*************����������obj��������ͼ���ļ�***************************************
	return true;
}


void startup()
{
	success = 0;    //��Ϸ�ɹ���־
	gamefailed = 0; //��Ϸʧ�ܱ�־
	viewworld = 0;	//�Ƿ�鿴����
	goal = 0;		//��������Ϊ0
	camera = new Camera();
	Eye =  XMVectorSet( 0.0f, 6.5f, -7.0f, 0.0f );//���λ��
	XMVECTOR AT = XMVectorSet(0.0f,0.0f,0.0f,0.0f);
	camera->SetEye(Eye);   //�����ӵ�λ��
	camera->Pitch(XM_PI/4);
	camera->Apply();
	item.floor.setinfo(modleinfo(item.floor,true),true);
	item.celling.setinfo(modleinfo(item.celling,false),false);
	/*for(int i = 0;i<4;i++)
	item.man[i].setinfo(modleinfo(item.man[i],false),false);*/
}

void Cleanup()
{
	//�ͷ�ȫ��ָ��
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
			//����һ����������ɫ��Ϣ��4��Ԫ�طֱ��ʾ�죬�̣����Լ�alpha
			float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; 
			immediateContext->ClearRenderTargetView( renderTargetView, ClearColor );
			//�����ǰ�󶨵����ģ����ͼ
			immediateContext->ClearDepthStencilView(depthStencilView,D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,1.f,0);
			//��ָ��������ӣ�һ�㲻����������������������ָ��Ϊʹ��blend factor
			float BlendFactor[] = {0,0,0,0};
			//��ʼ��������Ϣ


			if(initalpha)
			{
				for(int i = 0;i<100;i++)
				{
					flooralpah[i] = 1.0f;
				}
				initalpha = 0;
			}

			if(viewworld&&loadingcomplete)//��Ϸδ��ʼʱ,�ҽ���۲�ģʽ�������ƶ����
			{
				//ͨ�����̸ı���������ͷ����.
				if(::GetAsyncKeyState(VK_LEFT) & 0x8000f)  //�����ƶ�
				{
					camera->MoveRightBy(-timeDelta * 12.0f);
				}

				if(::GetAsyncKeyState(VK_RIGHT) & 0x8000f)  //�����ƶ�
				{
					camera->MoveRightBy(timeDelta* 12.0f);
				}

				if(::GetAsyncKeyState(VK_UP) & 0x8000f)  //��ǰ�ƶ�
				{
					camera->MoveForwardBy(timeDelta* 12.0f);
				}

				if(::GetAsyncKeyState(VK_DOWN) & 0x8000f)  //����ƶ�
				{
					camera->MoveForwardBy(-timeDelta* 12.0f);
				}

				if(::GetAsyncKeyState(VK_NUMPAD8) & 0x8000f)  //�����ƶ�
				{
					camera->MoveUpBy(timeDelta* 12.0f);
				}

				if(::GetAsyncKeyState(VK_NUMPAD2) & 0x8000f) //�����ƶ�
				{
					camera->MoveUpBy(-timeDelta* 12.0f);
				}
				//�������ɹ۲����
				camera->Apply();
				//*****************************��ʼ��Ϸ�Ľӿڷ�������


			}
			//��ʼ���������
			world = XMMatrixIdentity();   
			//����ͶӰ����
			projection = XMMatrixPerspectiveFovLH( XM_PIDIV2, (float)windowswidth / (float)windowsheight, 0.01f, 100.0f );
			//������任����ĳ��������еľ�����������õ�Effect�����---------------------

			effect->GetVariableByName("World")->AsMatrix()->SetMatrix((float*)&world);  //������������ϵ
			XMMATRIX ViewMATRIX =  XMLoadFloat4x4(&camera->GetView());
			effect->GetVariableByName("View")->AsMatrix()->SetMatrix((float*)&ViewMATRIX);    //���ù۲�����ϵ
			effect->GetVariableByName("EyePosition")->AsMatrix()->SetMatrix((float*)&Eye); //�����ӵ�
			effect->GetVariableByName("Projection")->AsMatrix()->SetMatrix((float*)&projection); //����ͶӰ����ϵ
			//��Դ�ĳ��������еĹ�Դ��Ϣ���õ�Effect�����
			SetLightEffect(light[lightType]);
			if((gamestarted||viewworld)&&loadingcomplete)
			{
				if(viewworld)
				{
					if(::GetAsyncKeyState('Q') & 0x8000f) //�˳��ϵ�ģʽ
					{
						viewworld = 0;
					}
				}
				if(gamestarted)
				{
					if(::GetAsyncKeyState('Q') & 0x8000f) //ֱ���˳���Ϸ
					{
						gamestarted = 0;
					}
					//������Ϸ�Ѷ�
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
				//**************************************������Ϸ����************************//

				world = XMMatrixIdentity(); 
				effect->GetVariableByName("World")->AsMatrix()->SetMatrix((float*)&world);
				//*****************************************************����ǽ��
				item.wall.renderModle(immediateContext, technique);
				//*******************************************************���ƽ�ɫ
				//��������

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
						finalstr = L"�����̫��ˤ���ˣ��÷֣�" + std::to_wstring(goal);
					}
					Vy = 0;
				}
				if (::GetAsyncKeyState('A') & 0x8000f) //���������
				{			
					if(Vz<0)
						angle += 2*timeDelta;
					if(Vz>=0)
						angle -= 2*timeDelta;
					if(angle <= XM_PI/2+5*timeDelta&&angle >= XM_PI/2-5*timeDelta)
						angle = XM_PI/2;
					Vx -= ax;//�ٶȰ�x�Ḻ�������ӣ���Vx��С
					if (Vx <= -0.04f) 
						Vx = -0.04f;
				}					
				//if (::GetAsyncKeyState(VK_LEFT) & 0x0f)//�ɿ������
				else
				{
					if (Vx <= 0)//�ɿ����ٶ������½����Ϊ0
					{
						Vx += ax;
						if (Vx >= 0) Vx = 0;
					}

				}
				if (::GetAsyncKeyState('D') & 0x8000f) //�����ҷ����
				{
					if(Vz>0)
						angle += 2*timeDelta;
					if(Vz<=0)
						angle -= 2*timeDelta;
					if(angle <= 3*XM_PI/2+5*timeDelta&&angle >= 3*XM_PI/2-5*timeDelta)
						angle = 3*XM_PI/2;
					Vx += ax;//�ٶȰ�x������������,��Vx����
					if (Vx >=0.04f) Vx = 0.04f;
				}
				//if (::GetAsyncKeyState(VK_RIGHT) & 0x0f) //�ɿ��ҷ����
				else
				{
					if (Vx >= 0)//�ٶȲ�����0ʱ���ٶȽ���0
					{
						Vx -= ax;
						if (Vx <= 0) Vx = 0;
					}

				}	
				if (::GetAsyncKeyState('W') & 0x8000f) //�����Ϸ����
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
				//if (::GetAsyncKeyState(VK_UP) & 0x0f) //�ɿ��Ϸ����
				else
				{ 
					if (Vz >= 0)
					{
						Vz -= az;
						if (Vz <= 0) Vz = 0;
					}
				}
				if (::GetAsyncKeyState('S') & 0x8000f) //�����·����
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
				//if (::GetAsyncKeyState(VK_DOWN) & 0x0f) //�ɿ��·����
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
				if(transX >= 5.0f) transX = 5.0f;       //���������Ƴ�ǽ�ұ߽�
				if(transX <= -5.0f) transX = -5.0f;     //���������Ƴ�ǽ��߽�
				if(transZ >= 4.0f) transZ = 4.0f;       //���������Ƴ�ǽǰ�߽�
				if(transZ <= -4.0f) transZ = -4.0f;     //���������Ƴ�ǽ��߽�
				if(!viewworld)
				{
					camera->SetEye(XMVectorSet( 0.0f, transY+6.5f, -7.0f, 0.0f ));
					camera->Apply();
				}

				world = XMMatrixIdentity();
				world =   XMMatrixRotationY(angle)*XMMatrixTranslation(transX, transY, transZ);
				effect->GetVariableByName("World")->AsMatrix()->SetMatrix((float*)&world);  //������������ϵ

				item.man[mannum].renderModle(immediateContext, technique);
				//***************************************************���ư�ȫ��
				world = XMMatrixIdentity(); 
				world = XMMatrixTranslation(0.0f,-600.0f,0.0f);
				effect->GetVariableByName("World")->AsMatrix()->SetMatrix((float*)&world);
				item.safeplace.renderModle(immediateContext, technique);
				//*************************************************************����Ҫ��͸���������컨��Ͳ�̤��
				immediateContext->OMSetBlendState(blendStateAlpha,BlendFactor,0xffffffff);
				immediateContext->RSSetState(NoCullRS);


				for(int i = 0;i<100;i++)
				{
					if(i == 0)
					{
						world = XMMatrixIdentity();
						world = XMMatrixTranslation(0.0f, 0.0f, 0.0f);//��һ�����飬�����������
						effect->GetVariableByName("World")->AsMatrix()->SetMatrix((float*)&world);  //������������ϵ
						item.floor.renderModle(immediateContext, technique);
					}
					else
					{
						world = XMMatrixIdentity();
						world = XMMatrixTranslation(item.floor.getrandx(i), item.floor.getrandy(i)-6.0f*(float)(i+1), item.floor.getrandz(i));
						effect->GetVariableByName("World")->AsMatrix()->SetMatrix((float*)&world);  //������������ϵ
						item.floor.renderModle(immediateContext, technique);
					}
					flooralpah[i] = 0.3f;
					if(transY<item.floor.getfloorbottom(i)-1.0f)
					{
						item.floor.setfloorleft(i,100.f);//������³����˵ذ�ĵײ���������Ϣ�ƿ�	
						world = world * XMMatrixTranslation(100.0f,0.0f,0.0f);
						effect->GetVariableByName("World")->AsMatrix()->SetMatrix((float*)&world);
					}
					item.floor.modle->setMaterialAlpha(flooralpah[i]);
				}
				//********************************�컨��*****************
				world = XMMatrixIdentity();
				world = XMMatrixTranslation(0.0f, cellingheight, 0.0f);//�����컨��λ��
				effect->GetVariableByName("World")->AsMatrix()->SetMatrix((float*)&world);  //������������ϵ
				//�����컨��
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
			//������д��������λ�ó�ʼ����
			transX = 0.0f;
			transY = 0.0f;
			transZ = 0.0f;
			Vcelling = 0.015f;
			cellingheight = 6.0f;
			startup();
			gamestarted = 1; //������ϣ���ʼ��Ϸ
			loading = 0;//�������ֹͣ����
			musicFun();
		}
	}
	return true;
}
//**************��ܺ���******************


//
// �ص�����
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
		if(wParam == VK_F1)  //��F1������Դ���͸�Ϊ�����
			lightType = 0;
		if(wParam == VK_F2)  //��F2������Դ���͸�Ϊ���Դ
			lightType = 1;
		if(wParam == VK_F3)  //��F3������Դ���͸�Ϊ�۹�ƹ�Դ
			lightType = 2;
		if(wParam == VK_F4)  //��F4������Դ���͸�Ϊ�޹���
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
// ������WinMain
//
int WINAPI WinMain(HINSTANCE hinstance,
				   HINSTANCE prevInstance, 
				   PSTR cmdLine,
				   int showCmd)
{

	//��ʼ��
	//**ע��**:������������IDirect3DDevice9ָ�룬��������Ϊ��������InitD3D����
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

//��Դ�ĳ����������õ�Effect�����
//���ڹ������ñȽϸ��ӣ�������һ����������������
void SetLightEffect(Light light)
{
	//���Ƚ��������ͣ�������ǿ�ȣ������ǿ�ȣ������ǿ�����õ�Effect��
	effect->GetVariableByName("type")->AsScalar()->SetInt(light.type);
	effect->GetVariableByName("LightAmbient")->AsVector()->SetFloatVector((float*)&(light.ambient));
	effect->GetVariableByName("LightDiffuse")->AsVector()->SetFloatVector((float*)&(light.diffuse));
	effect->GetVariableByName("LightSpecular")->AsVector()->SetFloatVector((float*)&(light.specular));

	//������ݹ������͵Ĳ�ͬ���ò�ͬ������
	if(light.type == 0)  //�����
	{
		//�����ֻ��Ҫ������������Լ���
		effect->GetVariableByName("LightDirection")->AsVector()->SetFloatVector((float*)&(light.direction));
		//��������Tectnique���õ�Effect
		technique = effect->GetTechniqueByName("T_DirLight");
	}
	else if(light.type == 1)  //���Դ
	{
		//���Դ��Ҫ��λ�á���������˥�����ӡ�����һ��˥�����ӡ���������˥�����ӡ�
		effect->GetVariableByName("LightPosition")->AsVector()->SetFloatVector((float*)&(light.position));
		effect->GetVariableByName("LightAtt0")->AsScalar()->SetFloat(light.attenuation0);
		effect->GetVariableByName("LightAtt1")->AsScalar()->SetFloat(light.attenuation1);
		effect->GetVariableByName("LightAtt2")->AsScalar()->SetFloat(light.attenuation2);

		//�����Դ��Tectnique���õ�Effect
		technique = effect->GetTechniqueByName("T_PointLight");
	}
	else if(light.type == 2) //�۹�ƹ�Դ
	{
		//���Դ��Ҫ�����򡱣������򡱣�������˥�����ӡ�����һ��˥�����ӡ���������˥�����ӡ�
		//����׶�Ƕȡ�������׶�Ƕȡ������۹��˥��ϵ����
		effect->GetVariableByName("LightPosition")->AsVector()->SetFloatVector((float*)&(light.position));
		effect->GetVariableByName("LightDirection")->AsVector()->SetFloatVector((float*)&(light.direction));

		effect->GetVariableByName("LightAtt0")->AsScalar()->SetFloat(light.attenuation0);
		effect->GetVariableByName("LightAtt1")->AsScalar()->SetFloat(light.attenuation1);
		effect->GetVariableByName("LightAtt2")->AsScalar()->SetFloat(light.attenuation2);

		effect->GetVariableByName("LightAlpha")->AsScalar()->SetFloat(light.alpha);
		effect->GetVariableByName("LightBeta")->AsScalar()->SetFloat(light.beta);
		effect->GetVariableByName("LightFallOff")->AsScalar()->SetFloat(light.fallOff);

		//���۹�ƹ�Դ��Tectnique���õ�Effect
		technique = effect->GetTechniqueByName("T_SpotLight");
	}
	else if(light.type == 3)
	{
		technique = effect->GetTechniqueByName("TexTech");
	}
}

//��ģ�͸�ֵ
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
	//�����Ҫ������򴢴�
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
		Verticesinfo[i].vPosition;//������Ϣ���磺��0,1,0��Ϊy�����򣬼�top������topΪ���е���yֵ�����Ǹ�
		//���������������е����Ϣ���ֱ��m���������left,right,forward,back,top,bottom���и�ֵ
		//��дһ�����ӣ�
		if(m.getleft()>Verticesinfo[i].vPosition.x)
		{
			m.setleft(Verticesinfo[i].vPosition.x);  //�����ȡ�ĵ��x�����֮ǰ��m��left�����С������ƫ��������
		}
		//*********************************�˴�������д*************************************
		if (m.getright() < Verticesinfo[i].vPosition.x)
		{
			m.setright(Verticesinfo[i].vPosition.x);  //�����ȡ�ĵ��x�����֮ǰ��m��right������󣨼���ƫ�ң�������
		}
		if (m.getforward() < Verticesinfo[i].vPosition.z)
		{
			m.setforward(Verticesinfo[i].vPosition.z);  //�����ȡ�ĵ��z�����֮ǰ��m��forward�����С������ƫǰ��������
		}
		if (m.getback() > Verticesinfo[i].vPosition.z)
		{
			m.setback(Verticesinfo[i].vPosition.z);  //�����ȡ�ĵ��z�����֮ǰ��m��back������󣨼���ƫ��������
		}
		if (m.gettop() < Verticesinfo[i].vPosition.y)
		{
			m.settop(Verticesinfo[i].vPosition.y);  //�����ȡ�ĵ��y�����֮ǰ��m��top������󣨼���ƫ�ϣ�������
		}
		if (m.getbottom() > Verticesinfo[i].vPosition.y)
		{
			m.setbottom(Verticesinfo[i].vPosition.y);  //�����ȡ�ĵ��y�����֮ǰ��m��bottom�����С������ƫ�£�������
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

	if(m.getname() == "floor")//�����Ҫ�������Ҫ����Ӧ�ĵط����������
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

//������Χ�����������
float random(float a,float b)
{
	float fnum = (float)rand()/(float)(RAND_MAX);
	int num = rand()%(int)(b-a+1)+(int)a;
	return (float)num + fnum;
}

//��ײ�ж�����
//x,y,z��������ռ��������Ϣ��Modle�Ƕ�������������жϣ���󷵻�ֵ��bool����ײ�˷���1��δ��ײ����0
//�������������Ϸ�����жϣ�ײ���컨��ײ�����Ϸʧ�ܣ�����ȵ���ȫ������Ϸ�ɹ��ı����gamestarted��success��gamefailed��ֵ
bool collisionDetermination(float x,float y,float z,Modle object)
{
	//****************************�������Ǳ�д�ˣ�������qq������

	if (y + 1.0f >= object.celling.getbottom())
	{
		gamestarted = 0;
		gamefailed = 1;
		musicFun();
		finalstr = L"�㱻�컨�������ˣ��÷֣�" + std::to_wstring(goal);
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
		finalstr = L"��ɹ������÷֣�" + std::to_wstring(goal);
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
		TextOut(mdc,500,400,L"�����С�������",7);
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