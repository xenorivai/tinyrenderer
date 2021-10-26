#pragma once
#include "transform.h"
#include "model.h"

struct IShader{
	mat4 u_ModelView;
	mat4 u_Perspective;
	mat4 u_Viewport;


	virtual ~IShader();
	virtual vec3f vertex(Model *model, int iface , int nthvert) = 0;
	virtual bool fragment(Model *model, vec3f bar , TGAColor &color) = 0;

};

IShader ::~IShader(){}