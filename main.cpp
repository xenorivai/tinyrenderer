#include <iostream>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "triangle.h"


const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red 	 = TGAColor(255, 0  , 0  , 255);
const TGAColor green = TGAColor(0  , 255, 0  , 255);
const TGAColor blue  = TGAColor(0  , 0  , 255, 255);

const int width = 1200;
const int height = 1200;
const int depth = 255;


vec3f cam(0,0,3);


Model* model = NULL;
double *zbuffer = NULL;



mat4 viewport(int x_v , int y_v , int w , int h){
/*
	Function that produces a matrix that would transform world coords to screen coords
		--> It maps the following coord ranges in world space to screen space:
			[x_wmin,x_wmax] --> [x_v, x_v + w]
			[y_wmin,y_wmax] --> [y_v, y_v + h]
			[z_wmin,z_wmax] --> [z_v, z_v + depth]

			here we are assuming z_v == 0, so range is [0,depth]

		--> This is made up of translations and scaling
			after a bit of math(equaling normalized coordinates in world and screen space) we get the following equations :

				x_s = (x-x_wmin)*s_x + x_v
				y_s = (y-y_wmin)*s_y + y_v
				z_s = (z-z_wmin)*s_z + z_v

				where, (x,y,z) are coords in world space belonging to repective ranges,and,(x_s, y_s, z_s) are coords in screen space, and s_x , s_y and s_z are scaling factors, calculated as :
						s_x = (w)/(x_wmax - x_wmin) and similar for s_y and s_z

		--> The corresponding matrix is :
				[s_x	0		0		-(x_wmin*s_x) + x_v]
				[0		s_y		0		-(y_wmin*s_y) + y_v]
				[0		0		s_z		-(z_wmin*s_z) + z_v]
				[0		0		0				1	   	   ]
		
		--> Consider the following for predescribed world_coords and screen_coords ranges:
				x ==> [-1,1] and [x_v, x_v + w]
				y ==> [-1,1] and [y_v, y_v + h]
				z ==> [-1,1] and [0,depth]

			|--> The corresponding matrix is :
				[w/2	0		0			x_v + w/2	]
				[0		h/2		0			y_v + h/2	]
				[0		0		depth/2		depth/2		]
				[0		0		0			1			]
		
		This basically creates a "viewport" with lower left corner at (x_v,y_v,0) and extending width,height,depth along respec. axes

*/
	
	mat4 m = mat4::identity();
	//translations
	m[0][0] = w/2.0f;
	m[1][1] = h/2.0f;
	m[2][2] = depth/2.0f;

	m[0][3] = x_v + w/2;
	m[1][3] = y_v + h/2;
	m[2][3] = depth/2;

	return m;

}

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
	mat4 view_port = viewport(width/8,height/8,width*3/4,height*3/4);
	// mat4 view_port = viewport(0,0,width,height);
	mat4 proj = mat4::identity();
	proj[3][2] = 1.0f/cam.z;

	for (size_t i = 0; i < model->nfaces(); i++) {
		// std::vector<int> face = model->face(i);

		vec3f screen_coords[3];//screen coordinates of triangle associated with ith face
		vec3f world_coords[3];

		for (size_t j = 0; j < 3; j++) {

			vec3f v = model->vert(i, j); //get the jth vertex of ith face, these have components as just numbers in the range [-1,1]

			//convert them to display on screen , map [-1,1] to [0,width] and [0,height] 
			screen_coords[j] = embed<3>(view_port*proj*embed<4>(v));
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

	mat4 view_port = viewport(width/8,height/8,width*3/4,height*3/4);
	// mat4 view_port = viewport(0,0,width,height);
	mat4 proj = mat4::identity();
	proj[3][2] = 1.0f/cam.z;

	for (size_t i = 0; i < model->nfaces(); i++) {

		vec3f screen_coords[3];	//screen coords of triangle associated with ith face
		vec3f world_coords[3];	//world coords of triangle associated with ith face
		vec2f uv_coords[3];		//uv coords of vertices of triangle associated with ith face

		for (size_t j = 0; j < 3; j++) {

			vec3f v = model->vert(i, j); //get the jth vertex of ith face, these have components as just numbers in the range [-1,1]

			world_coords[j] = v;
			screen_coords[j] = embed<3>(view_port*proj*embed<4>(v));
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
