#pragma once
#include "tgaimage.h"
#include "geometry.h"

/*
	Bresenham's Line Drawing Algorithm
*/
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