 #include"modle.h"


Modle::Modle()
{

}
m_modle::m_modle()
{

}
//����ģ�ͺ���
void m_modle::loadModle(ID3D11Device* device, char* resPath, char* objFilename,std::string n)
{
	modle->Load(device,resPath,objFilename,ObjLoader::Left_Hand_System);
	name = n;
}
//����ģ�ͺ���
void m_modle::renderModle(ID3D11DeviceContext* deviceContext, ID3DX11EffectTechnique* pTechnique)
{
	modle->RenderEffect(deviceContext,pTechnique);
}

//�õ�������߽磨x������
float m_modle::getleft(){return left;}
//�õ������ұ߽磨x������
float m_modle::getright(){return right;}			
//�õ�����ǰ�߽磨z������
float m_modle::getforward(){return forward;}			
//�õ������߽磨z������
float m_modle::getback(){return back;}			
//�õ������ϱ߽磨y������
float m_modle::gettop(){return top;}				
//�õ������±߽磨y������
float m_modle::getbottom(){return bottom;}			

//����������߽磨x������
void m_modle::setleft(float value){ left = value; }			
//���������ұ߽磨x������
void m_modle::setright(float value){ right = value; }			
//��������ǰ�߽磨z������
void m_modle::setforward(float value){ forward = value; }
//���������߽磨z������
void m_modle::setback(float value){ back = value; }			
//���������ϱ߽磨y������
void m_modle::settop(float value){ top = value; }				
//���������±߽磨y������
void m_modle::setbottom(float value){ bottom = value; }			

//�õ��ذ���߽磨x������
float m_modle::getfloorleft(int i){return floorleft[i];}
//�õ��ذ��ұ߽磨x������
float m_modle::getfloorright(int i){return floorright[i];}
//�õ��ذ�ǰ�߽磨z������
float m_modle::getfloorforward(int i){return floorforward[i];}
//�õ��ذ��߽磨z������
float m_modle::getfloorback(int i){return floorback[i];}
//�õ��ذ��ϱ߽磨y������
float m_modle::getfloortop(int i){return floortop[i];}
//�õ��ذ��±߽磨y������
float m_modle::getfloorbottom(int i){return floorbottom[i];}
//��ȡ�ذ���Ϣ
void m_modle::setfloorleft(int i, float value){floorleft[i] = value;}
void m_modle::setfloorright(int i, float value){floorright[i] = value;}
void m_modle::setfloorforward(int i, float value){floorforward[i] = value;}
void m_modle::setfloorback(int i, float value){floorback[i] = value;}
void m_modle::setfloortop(int i, float value){floortop[i] = value;}
void m_modle::setfloorbottom(int i, float value){floorbottom[i] = value;}

//���������
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

//��ȡ�����
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

//����ģ����Ϣ
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
//��ȡģ������
std::string m_modle::getname(){return name;}