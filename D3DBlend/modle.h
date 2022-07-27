#ifndef _MODLE_H_H
#define _MODLE_H_H
#include "ObjLoader.h"



class m_modle
{
public:
	
	m_modle();		//���캯��
private:
	std::string name;			//ģ������
	float left;					//������߽磨x������
	float right;				//�����ұ߽磨x������
	float forward;				//����ǰ�߽磨z������
	float back;					//�����߽磨z������
	float top;					//�����ϱ߽磨y������
	float bottom;				//�����±߽磨y������

	float randomx[100];		//���100������������
	float randomy[100];		//���100������������
	float randomz[100];		//���100������������

	float floorleft[100];					//������߽磨x������
	float floorright[100];				//�����ұ߽磨x������
	float floorforward[100];				//����ǰ�߽磨z������
	float floorback[100];					//�����߽磨z������
	float floortop[100];					//�����ϱ߽磨y������
	float floorbottom[100];				//�����±߽磨y������
public:



	
	ObjLoader* modle;			//ģ��

	void loadModle(ID3D11Device* device, char* resPath, char* objFilename,std::string n);
	void renderModle(ID3D11DeviceContext* deviceContext, ID3DX11EffectTechnique* pTechnique);
	//������װ˽�г�Ա����ֹλ����Ϣ������

	float getleft();			//�õ�������߽磨x������
	float getright();			//�õ������ұ߽磨x������
	float getforward();			//�õ�����ǰ�߽磨z������
	float getback();			//�õ������߽磨z������
	float gettop();				//�õ������ϱ߽磨y������
	float getbottom();			//�õ������±߽磨y������
	float getrandx(int i);		//��ȡ�����x
	float getrandy(int i);		//��ȡ�����y
	float getrandz(int i);		//��ȡ�����z
	float getfloorleft(int i);			//�õ��ذ���߽磨x������
	float getfloorright(int i);			//�õ��ذ��ұ߽磨x������
	float getfloorforward(int i);			//�õ��ذ�ǰ�߽磨z������
	float getfloorback(int i);			//�õ��ذ��߽磨z������
	float getfloortop(int i);				//�õ��ذ��ϱ߽磨y������
	float getfloorbottom(int i);			//�õ��ذ��±߽磨y������
	std::string getname();

	void setleft(float value);			//����������߽磨x������
	void setright(float value);			//���������ұ߽磨x������
	void setforward(float value);			//��������ǰ�߽磨z������
	void setback(float value);			//���������߽磨z������
	void settop(float value);				//���������ϱ߽磨y������
	void setbottom(float value);			//���������±߽磨y������
	void setrandx(int i,float value);		//���������x
	void setrandy(int i,float value);		//���������y
	void setrandz(int i,float value);		//���������z
	void setinfo(m_modle m,bool randomed);  //����ģ����Ϣ

	void setfloorleft(int i,float value);
	void setfloorright(int i, float value);
	void setfloorforward(int i, float value);
	void setfloorback(int i, float value);
	void setfloortop(int i, float value);
	void setfloorbottom(int i, float value);
};


class Modle
{
public:
	Modle();

	m_modle floor;				//��̤��
	m_modle wall;				//ǽ��
	m_modle celling;				//�컨��
	m_modle safeplace;			//��ȫ��
	m_modle man[4];				//��Ϸ����
};



#endif