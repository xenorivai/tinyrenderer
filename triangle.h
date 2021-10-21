#pragma once
#include "tgaimage.h"
#include "geometry.h"
#include <limits>

/*	Bresenham's Line Drawing Algorithm	*/
void linelow(int x0 , int y0 , int x1, int y1 , TGAImage &image , const TGAColor& color){

	int dx = x1 - x0;
	int dy = y1 - y0;
	int yi = 1;

	if (dy < 0) {
		dy = -dy;
		yi = -1;
	}

	int P = 2 * dy - dx;
	int x = x0, y = y0;
	for (; x <= x1; x++) {
		image.set(x, y, color);

		if (P < 0) {
			P += 2 * dy;
		}

		else {
			P += 2 * dy - 2 * dx;
			y += yi;
		}
	}
}

void linehigh(int x0 , int y0 , int x1, int y1 , TGAImage &image , const TGAColor& color){

	int dx = x1 - x0;
	int dy = y1 - y0;

	int xi = 1;

	if (dx < 0) {
		dx = -dx;
		xi = -1;
	}

	int P = 2 * dx - dy;
	int y = y0, x = x0;
	for (; y <= y1; y++) {
		image.set(x, y, color);

		if (P < 0) {
			P += 2 * dx;
		}

		else {
			P += 2 * dx - 2 * dy;
			x += xi;
		}
	}
}

void line(int x0 , int y0 , int x1, int y1 , TGAImage &image , const TGAColor& color){	
	
	//slope < 1
	if(abs(y1-y0) < abs(x1-x0)){
		if(x0>x1) linelow(x1,y1,x0,y0,image,color);
		else linelow(x0,y0,x1,y1,image,color);
	}	

	//slope >= 1
	else{
		if(y0>y1) linehigh(x1,y1,x0,y0,image,color);
		else linehigh(x0,y0,x1,y1,image,color);
	}
}

void line(vec2i p0, vec2i p1, TGAImage &image, TGAColor color){
	line(p0.x,p0.y,p1.x,p1.y,image,color);
}

vec3f barycentric(vec3i* pts, vec3i P) {
	vec3f bc = cross(	vec3f(pts[2][0] - pts[0][0], pts[1][0] - pts[0][0], pts[0][0] - P[0]),
						vec3f(pts[2][1] - pts[0][1], pts[1][1] - pts[0][1], pts[0][1] - P[1])
					);
	if (std::abs(bc[2]) < 1) return vec3f(-1, 1, 1);
	
	return vec3f(1.0f - (bc.x + bc.y) / bc.z, bc.y / bc.z, bc.x / bc.z);
}


void untex_triangle(vec3i* pts, double *zbuffer, TGAImage& image, TGAColor color) {

	const int width = image.get_width();
	// const int height = image.get_height();

	vec2i bboxmin( std::numeric_limits<int>::max(),  std::numeric_limits<int>::max());
	vec2i bboxmax(-std::numeric_limits<int>::max(), -std::numeric_limits<int>::max());
	vec2i extra(image.get_width() - 1, image.get_height() - 1);


	//Calculating the bounding box
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 2; j++) {
			bboxmin[j] = std::max(0		  , std::min(bboxmin[j], pts[i][j]));
			bboxmax[j] = std::min(extra[j], std::max(bboxmax[j], pts[i][j]));
		}
	}

	//Draw bbox
	// const TGAColor white = TGAColor(255, 255, 255, 255);
	// const TGAColor red = TGAColor(255, 0, 0, 255);
	// const TGAColor green = TGAColor(0, 255, 0, 255);
	// const TGAColor blue = TGAColor(0, 0, 255, 255);	
	// line(bboxmin.x,bboxmin.y,bboxmin.x,bboxmax.y,image,red);
	// line(bboxmin.x,bboxmax.y,bboxmax.x,bboxmax.y,image,red);
	// line(bboxmax.x,bboxmax.y,bboxmax.x,bboxmin.y,image,red);
	// line(bboxmax.x,bboxmin.y,bboxmin.x,bboxmin.y,image,red);


	//Iterate over the bbox , and check if each pixel is in the triangle , and if it is in calculate and update coresponding z-values and z-buffer
	vec3i P;

	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {

			//get the bary_coords for point P
			vec3f bc_screen = barycentric(pts, P);

			//Check if inside triangle
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
			
			//Calculating z - value
			P.z = 0;
			for (int i = 0; i < 3; i++) {
				P.z += static_cast<int>(pts[i].z * bc_screen[i]);
				// z value is calculated as sum of products : of z-coords of triangle's vertices and coresponding bary_coords
			}

			//Update z-buffer with pixel closest to the camera(farthest from screen)
			if (zbuffer[int(P.x + P.y * width)] < P.z) {
				zbuffer[int(P.x + P.y * width)] = P.z;
				image.set(static_cast<int>(P.x), static_cast<int>(P.y), color);
			}
		}
	}
}

//Input array of vertex coords, array of uv coords , model and rest
void triangle(vec3i *pts, vec2f *uvs, double *zbuffer, TGAImage &image, Model *model){
	
	const int width = image.get_width();
	// const int height = image.get_height();

	vec2i bboxmin(std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
    vec2i bboxmax(std::numeric_limits<int>::min(), std::numeric_limits<int>::min());
	vec2i extra(image.get_width() - 1, image.get_height() - 1);


	//Calculating the bounding box
	for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
			bboxmin[j] = std::max(0		  , std::min(bboxmin[j], pts[i][j]));
			bboxmax[j] = std::min(extra[j], std::max(bboxmax[j], pts[i][j]));
		}
    }

	vec3i P;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {

			//get the bary_coords for point P
			vec3f bc_screen = barycentric(pts, P);

			//Check if inside triangle
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
			
			//Calculating z-value
			P.z = 0;
			for (int i = 0; i < 3; i++) {
				P.z += static_cast<int>(pts[i].z * bc_screen[i]); 
				// z value is calculated as sum of products : of z-coords of triangle's vertices and coresponding bary_coords
			}

			//Update z-buffer with pixel closest to the camera(farthest from screen)
			if (zbuffer[int(P.x + P.y * width)] < P.z) {
				zbuffer[int(P.x + P.y * width)] = P.z;

				vec2f uv{0,0};
				// uv coords of the point P is calculated as sum of products : of uv-coords of triangle's vertices and coresponding bary_coords
				for(int i = 0 ; i < 3 ; i++){
					uv.x += uvs[i].x * bc_screen[i];
					uv.y += uvs[i].y * bc_screen[i];
				}

				TGAColor color = model->diffuse(uv);

				image.set(static_cast<int>(P.x), static_cast<int>(P.y), color);
			}
		}
	}
}