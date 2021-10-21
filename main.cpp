#include <iostream>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "triangle.h"


const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red 	 = TGAColor(255, 0  , 0  , 255);
const TGAColor green = TGAColor(0  , 255, 0  , 255);
const TGAColor blue  = TGAColor(0  , 0  , 255, 255);

const int width = 7999;
const int height = 7680;

Model* model = NULL;
double *zbuffer = NULL;


void INIT_ZBUF(void){
	zbuffer = new double[width*height];
	for(int i = width*height ; i >= 0 ; i--){
		zbuffer[i] = -std::numeric_limits<float>::max();
	}
	return;
}

vec3i world2screen(vec3f w){
	//convert them to display on screen , map [-1,1] to [0,width] and [0,height] 
	return vec3i(int((w.x + 1.0f) * width / 2.0f), int((w.y + 1.0f) * height / 2.0f),w.z);
}

void untex_render(vec3f light_dir, double *zbuffer, Model *model, TGAImage &image) {
	for (size_t i = 0; i < model->nfaces(); i++) {
		// std::vector<int> face = model->face(i);

		vec3i screen_coords[3];//screen coordinates of triangle associated with ith face
		vec3f world_coords[3];

		for (size_t j = 0; j < 3; j++) {

			vec3f v = model->vert(i, j); //get the jth vertex of ith face, these have components as just numbers in the range [-1,1]

			//convert them to display on screen , map [-1,1] to [0,width] and [0,height] 
			screen_coords[j] = world2screen(v);
			world_coords[j] = v;
		}

		vec3f n = cross(world_coords[2] - world_coords[0], world_coords[1] - world_coords[0]); //calculate normal to triangle

		n.normalize();
		double intensity = dot(n, light_dir);

		if (intensity > 0) {
			// TGAColor clr = TGAColor(0,intensity*255,intensity*255);
			// triangle(screen_coords,zbuffer, image, clr);

			untex_triangle(screen_coords,zbuffer, image, TGAColor(static_cast<uint8_t>(intensity * 255), static_cast<uint8_t>(intensity * 255), static_cast<uint8_t>(intensity * 255)));			
		}
	}
}


void render(vec3f light_dir, double *zbuffer, Model *model, TGAImage &image) {
	for (size_t i = 0; i < model->nfaces(); i++) {

		vec3i screen_coords[3];	//screen coords of triangle associated with ith face
		vec3f world_coords[3];	//world coords of triangle associated with ith face
		vec2f uv_coords[3];		//uv coords of vertices of triangle associated with ith face

		for (size_t j = 0; j < 3; j++) {

			vec3f v = model->vert(i, j); //get the jth vertex of ith face, these have components as just numbers in the range [-1,1]

			world_coords[j] = v;
			screen_coords[j] = world2screen(v);
			uv_coords[j] = model->uv(i,j);			
		}

		//calculate normal to triangle
		vec3f n = cross(world_coords[2] - world_coords[0], world_coords[1] - world_coords[0]); 
		n.normalize();

		//Lmabertian lighting
		double intensity = dot(n, light_dir);

		if (intensity > 0) {
			triangle(screen_coords,uv_coords,zbuffer,image,model);			
		}
	}

	// TGAImage depth(width, height, TGAImage::GRAYSCALE);
	// for(size_t h = 0 ; h < height ; h++){
	// 	for(size_t w = 0 ; w < width ; w++){
	// 		depth.set(h,w,static_cast<uint8_t>((zbuffer[w * height + h] + 1) * 128));
	// 	}
	// }
	// depth.write_tga_file("depth.tga");
}


int main(int argc, char** argv) {

	if (argc == 2) {
		model = new Model(argv[1]);
	}
	else {
		model = new Model("obj/african_head.obj",true,false,false);		
	}

	//Initialize z-buffer
	INIT_ZBUF();

	TGAImage image(width, height, TGAImage::RGB);

	vec3f light_dir (0,0,-1);

	// untex_render(light_dir,zbuffer,model,image);
	render(light_dir,zbuffer,model,image);

	image.write_tga_file("output.tga");
	delete model;

	return 0;
}
