#ifndef _MODLE_H_H
#define _MODLE_H_H
#include "ObjLoader.h"



class m_modle
{
public:
	
	m_modle();		//构造函数
private:
	std::string name;			//模型名字
	float left;					//物体左边界（x负方向）
	float right;				//物体右边界（x正方向）
	float forward;				//物体前边界（z正方向）
	float back;					//物体后边界（z负方向）
	float top;					//物体上边界（y正方向）
	float bottom;				//物体下边界（y负方向）

	float randomx[100];		//存放100层的随机生成数
	float randomy[100];		//存放100层的随机生成数
	float randomz[100];		//存放100层的随机生成数

	float floorleft[100];					//物体左边界（x负方向）
	float floorright[100];				//物体右边界（x正方向）
	float floorforward[100];				//物体前边界（z正方向）
	float floorback[100];					//物体后边界（z负方向）
	float floortop[100];					//物体上边界（y正方向）
	float floorbottom[100];				//物体下边界（y负方向）
public:



	
	ObjLoader* modle;			//模型

	void loadModle(ID3D11Device* device, char* resPath, char* objFilename,std::string n);
	void renderModle(ID3D11DeviceContext* deviceContext, ID3DX11EffectTechnique* pTechnique);
	//函数封装私有成员，防止位置信息被更改

	float getleft();			//得到物体左边界（x负方向）
	float getright();			//得到物体右边界（x正方向）
	float getforward();			//得到物体前边界（z正方向）
	float getback();			//得到物体后边界（z负方向）
	float gettop();				//得到物体上边界（y正方向）
	float getbottom();			//得到物体下边界（y负方向）
	float getrandx(int i);		//获取随机数x
	float getrandy(int i);		//获取随机数y
	float getrandz(int i);		//获取随机数z
	float getfloorleft(int i);			//得到地板左边界（x负方向）
	float getfloorright(int i);			//得到地板右边界（x正方向）
	float getfloorforward(int i);			//得到地板前边界（z正方向）
	float getfloorback(int i);			//得到地板后边界（z负方向）
	float getfloortop(int i);				//得到地板上边界（y正方向）
	float getfloorbottom(int i);			//得到地板下边界（y负方向）
	std::string getname();

	void setleft(float value);			//设置物体左边界（x负方向）
	void setright(float value);			//设置物体右边界（x正方向）
	void setforward(float value);			//设置物体前边界（z正方向）
	void setback(float value);			//设置物体后边界（z负方向）
	void settop(float value);				//设置物体上边界（y正方向）
	void setbottom(float value);			//设置物体下边界（y负方向）
	void setrandx(int i,float value);		//设置随机数x
	void setrandy(int i,float value);		//设置随机数y
	void setrandz(int i,float value);		//设置随机数z
	void setinfo(m_modle m,bool randomed);  //设置模型信息

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

	m_modle floor;				//踩踏块
	m_modle wall;				//墙壁
	m_modle celling;				//天花板
	m_modle safeplace;			//安全区
	m_modle man[4];				//游戏人物
};



#endif