#pragma once
#include "tgaimage.h"
#include "geometry.h"
#include <limits>

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