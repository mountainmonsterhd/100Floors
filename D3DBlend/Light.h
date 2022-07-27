#ifndef LIGHT_H_H
#define LIGHT_H_H

#include <xnamath.h>

struct Material
{
	XMFLOAT4 ambient;	//
	XMFLOAT4 diffuse;	//
	XMFLOAT4 specular;	//
	float power;		//
};

struct Light
{
	int type;

	XMFLOAT4 position;
	XMFLOAT4 direction;
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;
	XMFLOAT4 specular;

	float attenuation0;
	float attenuation1;
	float attenuation2;

	float alpha;
	float beta;
	float fallOff;

};

#endif