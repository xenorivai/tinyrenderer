#pragma once
#include "gl.h"

struct GouraudShader : public IShader {
	vec3f varying_intensity;
	vec3f u_lightDir;

	virtual ~GouraudShader() {}

	virtual vec3f vertex(Model *model, int iface , int nthvert){
		vec4f gl_Vertex = u_Perspective * u_ModelView * embed<4>(model->vert(iface, nthvert));
		perspective_division(gl_Vertex);
		gl_Vertex = u_Viewport * gl_Vertex;
		vec3f ret = embed<3>(gl_Vertex);
		ret.z *= -1.0;

		varying_intensity[nthvert] = -1.0 * dot(model->normal(iface, nthvert), u_lightDir);
		clamp<double>(varying_intensity[nthvert], 0.0, 1.0);

		return ret;
	}

	virtual bool fragment(Model *model, vec3f bar, TGAColor &color){
		double intensity = dot(varying_intensity, bar);

		if (intensity > 0.0) {
			color = TGAColor(static_cast<uint8_t>(intensity * 255), static_cast<uint8_t>(intensity * 255), static_cast<uint8_t>(intensity * 255));

			return false;
		}
		else {
			return true;
		}
	}
};

struct ToonShader : public IShader {
	vec3f varying_intensity;
	vec3f u_lightDir;

	virtual ~ToonShader() {}

	virtual vec3f vertex(Model *model, int iface , int nthvert){
		vec4f gl_Vertex = u_Perspective * u_ModelView * embed<4>(model->vert(iface, nthvert));
		perspective_division(gl_Vertex);
		gl_Vertex = u_Viewport * gl_Vertex;
		vec3f ret = embed<3>(gl_Vertex);
		ret.z *= -1.0;

		varying_intensity[nthvert] = -1.0 * dot(model->normal(iface, nthvert), u_lightDir);
		clamp<double>(varying_intensity[nthvert], 0.0, 1.0);

		return ret;
	}

	virtual bool fragment(Model *model, vec3f bar , TGAColor &color){
		double intensity = dot(varying_intensity, bar);

		if (intensity > 0.0) {
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
		else {
			return true;
		}
	}
};

struct PhongShader : public IShader {
	vec3f varying_intensity;
	mat<2, 3> varying_uv;
	vec3f u_lightDir;
	vec3f u_viewingDir;

	virtual ~PhongShader() {}

	virtual vec3f vertex(Model *model,int iface,int nthvert){
		vec4f gl_Vertex = u_Perspective * u_ModelView * embed<4>(model->vert(iface, nthvert));
		perspective_division(gl_Vertex);
		gl_Vertex = u_Viewport * gl_Vertex;
		vec3f ret = embed<3>(gl_Vertex);
		ret.z *= -1.0;

		varying_intensity[nthvert] = dot(model->normal(iface, nthvert), u_lightDir);
		clamp<double>(varying_intensity[nthvert], 0.0, 1.0);

		varying_uv.set_col(nthvert, model->uv(iface, nthvert));
		return ret;
	}

	virtual bool fragment(Model *model, vec3f bar, TGAColor &color){

		double intensity = dot(varying_intensity, bar);	//interpolate intensity
		vec2f uv = varying_uv * bar;

		auto tmp = ((u_Perspective * u_ModelView).invert_transpose()) * embed<4>(model->normal(uv));
		perspective_division(tmp);
		vec3f n = embed<3>(tmp).normalize();	//normal corresponding to uv coords, sampled from normal map

		vec3f r = ((2.0 * dot(n, u_lightDir)) * n - u_lightDir).normalize();	//reflected ray

		double spec = pow((dot(r, u_viewingDir)), model->specular(uv));	//specular intensity,

		double multiplier = intensity + 0.6 * spec;
		double ambient = 5;
		if (intensity > 0.0) {
			color = model->diffuse(uv);
			for (int i = 0; i < 3;i++) {
				color[i] = ambient + color[i] * multiplier;
			}
			return false;
		}
		else return true;
	}
};