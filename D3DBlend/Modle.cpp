 #include"modle.h"


Modle::Modle()
{

}
m_modle::m_modle()
{

}
//加载模型函数
void m_modle::loadModle(ID3D11Device* device, char* resPath, char* objFilename,std::string n)
{
	modle->Load(device,resPath,objFilename,ObjLoader::Left_Hand_System);
	name = n;
}
//绘制模型函数
void m_modle::renderModle(ID3D11DeviceContext* deviceContext, ID3DX11EffectTechnique* pTechnique)
{
	modle->RenderEffect(deviceContext,pTechnique);
}

//得到物体左边界（x负方向）
float m_modle::getleft(){return left;}
//得到物体右边界（x正方向）
float m_modle::getright(){return right;}			
//得到物体前边界（z正方向）
float m_modle::getforward(){return forward;}			
//得到物体后边界（z负方向）
float m_modle::getback(){return back;}			
//得到物体上边界（y正方向）
float m_modle::gettop(){return top;}				
//得到物体下边界（y负方向）
float m_modle::getbottom(){return bottom;}			

//设置物体左边界（x负方向）
void m_modle::setleft(float value){ left = value; }			
//设置物体右边界（x正方向）
void m_modle::setright(float value){ right = value; }			
//设置物体前边界（z正方向）
void m_modle::setforward(float value){ forward = value; }
//设置物体后边界（z负方向）
void m_modle::setback(float value){ back = value; }			
//设置物体上边界（y正方向）
void m_modle::settop(float value){ top = value; }				
//设置物体下边界（y负方向）
void m_modle::setbottom(float value){ bottom = value; }			

//得到地板左边界（x负方向）
float m_modle::getfloorleft(int i){return floorleft[i];}
//得到地板右边界（x正方向）
float m_modle::getfloorright(int i){return floorright[i];}
//得到地板前边界（z正方向）
float m_modle::getfloorforward(int i){return floorforward[i];}
//得到地板后边界（z负方向）
float m_modle::getfloorback(int i){return floorback[i];}
//得到地板上边界（y正方向）
float m_modle::getfloortop(int i){return floortop[i];}
//得到地板下边界（y负方向）
float m_modle::getfloorbottom(int i){return floorbottom[i];}
//获取地板信息
void m_modle::setfloorleft(int i, float value){floorleft[i] = value;}
void m_modle::setfloorright(int i, float value){floorright[i] = value;}
void m_modle::setfloorforward(int i, float value){floorforward[i] = value;}
void m_modle::setfloorback(int i, float value){floorback[i] = value;}
void m_modle::setfloortop(int i, float value){floortop[i] = value;}
void m_modle::setfloorbottom(int i, float value){floorbottom[i] = value;}

//设置随机数
void m_modle::setrandx(int i,float value)
{
	randomx[i] = value;
}
void m_modle::setrandy(int i,float value)
{
	randomy[i] = value;
}
void m_modle::setrandz(int i,float value)
{
	randomz[i] = value;
}

//获取随机数
float m_modle::getrandx(int i)
{
	return randomx[i];
}
float m_modle::getrandy(int i)
{
	return randomy[i];
}
float m_modle::getrandz(int i)
{
	return randomz[i];
}

//设置模型信息
void m_modle::setinfo(m_modle m,bool randomed)
{
	left = m.getleft();
	right = m.getright();
	forward = m.getforward();
	back = m.getback();
	top = m.gettop();
	bottom = m.getbottom();
	if(randomed)
	{
		for(int i = 0;i<100;i++)
		{
			randomx[i] = m.getrandx(i);
			randomy[i] = m.getrandy(i);
			randomz[i] = m.getrandz(i);
			floorleft[i] = m.getfloorleft(i);
			floorright[i] = m.getfloorright(i);
			floorforward[i] = m.getfloorforward(i);
			floorback[i] = m.getfloorback(i);
			floortop[i] = m.getfloortop(i);
			floorbottom[i] = m.getfloorbottom(i);
		}

	}

}
//获取模型名字
std::string m_modle::getname(){return name;}