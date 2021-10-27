#include <iostream>
#include "tgaimage.h"
#include "triangle.h"
#include "gl.h"
#include "shader.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red 	 = TGAColor(255, 0  , 0  , 255);
const TGAColor green = TGAColor(0  , 255, 0  , 255);
const TGAColor blue  = TGAColor(0  , 0  , 255, 255);

const int width		= 1200;
const int height	= 1200;
const int depth		= 255;
const double aspect_ratio = static_cast<double>(width / height);

vec3f cam(1.25, 1, 2.75);
vec3f target(0, 0, 0);

Model *model = NULL;
double *zbuffer = NULL;

void INIT_ZBUF(void) {
	zbuffer = new double[width * height];
	for (int i = width * height; i >= 0; i--) {
		zbuffer[i] = -std::numeric_limits<float>::max();
	}
	return;
}

void untex_render(vec3f light_dir, double *zbuffer, Model *model, TGAImage &image) {
	mat4 model_view = lookat(cam, target, vec3f(0, 1, 0));
	mat4 proj 		= perspective(15.0f, aspect_ratio, -1.f, -10.f);
	mat4 view_port 	= viewport(0, 0, width, height, depth);
	
	for (size_t i = 0; i < model->nfaces(); i++) {

		vec3f screen_coords[3];	//screen coords of triangle associated with ith face
		vec3f world_coords[3];	//world coords of triangle associated with ith face
		vec2f uv_coords[3];		//uv coords of vertices of triangle associated with ith face

		for (size_t j = 0; j < 3; j++) {

			vec3f v = model->vert(i, j); //get the jth vertex of ith face, these have components as just numbers in the range [-1,1]
			world_coords[j] = v;

			//MVP Trasnform , here model matrix is Identity because only one object in the scene with no change in oreintation or position
			auto tmp = proj * model_view * embed<4>(v);
			perspective_division(tmp);
			screen_coords[j] = embed<3>(view_port * tmp);
			screen_coords[j].z *= -1.0; // z-buffer doesnt work if I don't do this, probably due to how I've implemented the projection matrix

			uv_coords[j] = model->uv(i, j);
		}

		vec3f n = cross(world_coords[2] - world_coords[0], world_coords[1] - world_coords[0]); //calculate normal to triangle
		n.normalize();
		double intensity = dot(n, light_dir);
		if (intensity > 0) {
			untex_triangle(screen_coords, zbuffer, image, TGAColor(static_cast<uint8_t>(intensity * 255), static_cast<uint8_t>(intensity * 255), static_cast<uint8_t>(intensity * 255)));
		}
	}

	TGAImage depth(width, height, TGAImage::GRAYSCALE);
	for (size_t h = 0; h < height; h++) {
		for (size_t w = 0; w < width; w++) {
			depth.set(h, w, static_cast<uint8_t>((zbuffer[w * height + h] + 1) * 255));
		}
	}
	depth.write_tga_file("depth.tga");
}

void unshaded_render(vec3f light_dir, double *zbuffer, Model *model, TGAImage &image) {
	mat4 model_view = lookat(cam, target, vec3f(0, 1, 0));
	mat4 proj 		= perspective(15.0f, aspect_ratio, -1.f, -10.f);
	mat4 view_port 	= viewport(0, 0, width, height, depth);

	for (size_t i = 0; i < model->nfaces(); i++) {

		vec3f screen_coords[3];	//screen coords of triangle associated with ith face
		vec3f world_coords[3];	//world coords of triangle associated with ith face
		vec2f uv_coords[3];		//uv coords of vertices of triangle associated with ith face

		for (size_t j = 0; j < 3; j++) {

			vec3f v = model->vert(i, j); //get the jth vertex of ith face, these have components as just numbers in the range [-1,1]
			world_coords[j] = v;

			//MVP Trasnform , here model matrix is Identity because only one object in the scene with no change in oreintation or position
			auto tmp = proj * model_view * embed<4>(v);
			perspective_division(tmp);
			screen_coords[j] = embed<3>(view_port * tmp);
			screen_coords[j].z *= -1.0f; // z-buffer doesnt work if I don't do this, probably due to how I've implemented the projection matrix

			uv_coords[j] = model->uv(i, j);
		}

		vec3f n = cross(world_coords[2] - world_coords[0], world_coords[1] - world_coords[0]);
		n.normalize();
		//Lmabertian lighting
		double intensity = dot(n, light_dir);
		if (intensity > 0) {
			unshaded_triangle(screen_coords, uv_coords, zbuffer, image, model);
		}
	}

	TGAImage depth(width, height, TGAImage::GRAYSCALE);
	for (size_t h = 0; h < height; h++) {
		for (size_t w = 0; w < width; w++) {
			depth.set(h, w, static_cast<uint8_t>((zbuffer[w * height + h] + 1) * 255));
		}
	}
	depth.write_tga_file("depth.tga");
}

void render(vec3f light_dir, double *zbuffer, Model *model, TGAImage &image){

	PhongShader pshader;

	pshader.u_ModelView		= lookat(cam, target, vec3f(0, 1, 0));
	pshader.u_Perspective 	= perspective(45.0f, aspect_ratio, -1.f, -10.f);
	pshader.u_Viewport 		= viewport(0, 0, width, height, depth);
	pshader.u_lightDir 		= light_dir;
	pshader.u_viewingDir 	= (cam - target).normalize();

	for (size_t i = 0; i < model->nfaces(); i++) {
		vec3f screen_coords[3];
		for (size_t j = 0; j < 3; j++) {
			screen_coords[j] = pshader.vertex(model, i, j);
		}
		triangle(screen_coords, zbuffer, image, pshader, model);
	}
}


int main(int argc, char** argv) {

	if (argc == 2) {
		model = new Model(argv[1]);
	}
	else {
		model = new Model("obj/diablo3_pose.obj", true, true, true);
	}

	INIT_ZBUF();

	TGAImage image(width, height, TGAImage::RGB);

	// vec3f light_dir = (cam - target).normalize();
	vec3f light_dir = vec3f(1,1,1).normalize();

	// untex_render(light_dir,zbuffer,model,image);
	// unshaded_render(light_dir,zbuffer,model,image);

	render(light_dir, zbuffer, model, image);

	image.write_tga_file("output.tga");
	delete model;
	return 0;
}
