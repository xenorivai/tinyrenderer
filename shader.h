#pragma once
#include "gl.h"

struct GouraudShader : public IShader{
	vec3f varying_intensity;
	vec3f u_lightDir;

	virtual ~GouraudShader(){}

	virtual vec3f vertex(Model *model, int iface , int nthvert){
		vec4f gl_Vertex = u_Perspective * u_ModelView * embed<4>(model->vert(iface,nthvert));
		perspective_division(gl_Vertex);
		gl_Vertex = u_Viewport * gl_Vertex;
		vec3f ret = embed<3>(gl_Vertex);
		ret.z *= -1.0;

		varying_intensity[nthvert] = -1.0*dot(model->normal(iface, nthvert), u_lightDir);
		clamp<double>(varying_intensity[nthvert],0.0,1.0);

		return ret;
	}

	virtual bool fragment(Model *model, vec3f bar, TGAColor &color){
		double intensity = dot(varying_intensity,bar);

		if(intensity > 0.0){
			color = TGAColor(static_cast<uint8_t>(intensity * 255), static_cast<uint8_t>(intensity * 255), static_cast<uint8_t>(intensity * 255));
		
			return false;
		}
		else {
			return true;
		}
	}
};

struct ToonShader : public IShader{
	vec3f varying_intensity;
	vec3f u_lightDir;

	virtual ~ToonShader(){}

	virtual vec3f vertex(Model *model, int iface , int nthvert){
		vec4f gl_Vertex = u_Perspective * u_ModelView * embed<4>(model->vert(iface,nthvert));
		perspective_division(gl_Vertex);
		gl_Vertex = u_Viewport * gl_Vertex;
		vec3f ret = embed<3>(gl_Vertex);
		ret.z *= -1.0;

		varying_intensity[nthvert] = -1.0*dot(model->normal(iface, nthvert), u_lightDir);
		clamp<double>(varying_intensity[nthvert],0.0,1.0);

		return ret;
	}

	virtual bool fragment(Model *model, vec3f bar , TGAColor &color){
		double intensity = dot(varying_intensity,bar);

		if(intensity > 0.0){
			if (intensity > .85)
				intensity = 1;
			else if (intensity > .60)
				intensity = .80;
			else if (intensity > .45)
				intensity = .60;
			else if (intensity > .30)
				intensity = .45;
			else if (intensity > .15)
				intensity = .30;
			color = TGAColor(static_cast<uint8_t>(intensity * 255), static_cast<uint8_t>(intensity * 155), 0);
			return false;
		}
		else{
			return true;
		}
        
	}
};

struct PhongShader : public IShader{
	vec3f varying_intensity;
	mat<2,3> varying_uv;
	vec3f u_lightDir;

	virtual ~PhongShader(){}

	virtual vec3f vertex(Model *model,int iface,int nthvert){
		vec4f gl_Vertex = u_Perspective * u_ModelView * embed<4>(model->vert(iface,nthvert));
		perspective_division(gl_Vertex);
		gl_Vertex = u_Viewport * gl_Vertex;
		vec3f ret = embed<3>(gl_Vertex);
		ret.z *= -1.0;

		varying_intensity[nthvert] = -1.0*dot(model->normal(iface, nthvert), u_lightDir);
		clamp<double>(varying_intensity[nthvert],0.0,1.0);

		varying_uv.set_col(nthvert,model->uv(iface,nthvert));
		return ret;
	}

	virtual bool fragment(Model *model, vec3f bar, TGAColor &color){
		double intensity = dot(varying_intensity,bar);
		vec2f uv = varying_uv*bar;
	
		if(intensity > 0.0){
			color = model->diffuse(uv)*intensity;
			return false;
		}
		else return true;
	}
};

struct Shader : public IShader{
	mat<2,3> varying_uv;
	vec3f u_lightDir;
	mat4 u_MIT = (u_Perspective * u_ModelView).invert_transpose();

	virtual ~Shader(){}

	virtual vec3f vertex(Model *model,int iface,int nthvert){
		vec4f gl_Vertex = u_Perspective * u_ModelView * embed<4>(model->vert(iface,nthvert));
		perspective_division(gl_Vertex);
		gl_Vertex = u_Viewport * gl_Vertex;
		vec3f ret = embed<3>(gl_Vertex);
		ret.z *= -1.0;

		varying_uv.set_col(nthvert,model->uv(iface,nthvert));
		return ret;
	}

	virtual bool fragment(Model *model, vec3f bar, TGAColor &color){
		vec2f uv = varying_uv*bar;
		auto tmp = u_MIT*embed<4>(model->normal(uv));

		// std::cout<<"Before persp div : ";
		// display_vec(tmp);

		perspective_division(tmp);
		
		// std::cout<<"After persp div : ";
		// display_vec(tmp);
		
		vec3f n = embed<3>(tmp).normalize();
		// vec3f l = embed<3>(u_Perspective*u_ModelView*embed<4>(u_lightDir)).normalize();
		double intensity = dot(n,u_lightDir);
		// std::cout<<"\n"<<intensity<<"\n";
		if(intensity > 0.0){
			color = model->diffuse(uv)*intensity;
			return false;
		}
		else return true;
	}
};