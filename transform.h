#pragma once
#include "geometry.h"

const double pi = 3.1415926535897932385;
#define DEG2RAD pi/180.0

mat4 viewport(int x_v , int y_v , int w , int h , int depth){
/*
	Function that produces a matrix that would transform world coords to screen coords
		--> It maps the following coord ranges in world space to screen space:
			[x_wmin,x_wmax] --> [x_v, x_v + w]
			[y_wmin,y_wmax] --> [y_v, y_v + h]
			[z_wmin,z_wmax] --> [z_v, z_v + depth]

			here we are assuming z_v == 0, so range is [0,depth]

		--> This is made up of translations and scaling	after a bit of math(equaling normalized coordinates in world and screen space) we get the following equations :

				x_s = (x-x_wmin)*s_x + x_v
				y_s = (y-y_wmin)*s_y + y_v
				z_s = (z-z_wmin)*s_z + z_v

				where, (x,y,z) are coords in world space belonging to repective ranges,and (x_s, y_s, z_s) are coords in screen space, and s_x, s_y and s_z are scaling factors, calculated as :
					s_x = (w)/(x_wmax - x_wmin) and similar for s_y and s_z

		--> The corresponding matrix is :
				[s_x	0		0		-(x_wmin*s_x) + x_v	]
				[0		s_y		0		-(y_wmin*s_y) + y_v	]
				[0		0		s_z		-(z_wmin*s_z) + z_v	]
				[0		0		0				1			]
		
		--> Consider the following for predescribed world_coords and screen_coords ranges:
				x ==> [-1,1] and [x_v, x_v + w]
				y ==> [-1,1] and [y_v, y_v + h]
				z ==> [-1,1] and [0,depth]

			|--> The corresponding matrix is :
				[w/2]	0		0			x_v + w/2	]
				[0		h/2		0			y_v + h/2	]
				[0		0		depth/2		depth/2		]
				[0		0		0				1		]
		
		This basically creates a "viewport" with lower left corner at (x_v,y_v,0) and extending width,height,depth along respective axes
*/
	
	mat4 m = mat4::identity();
	//translations
	m[0][0] = w / 2.0f;
	m[1][1] = h / 2.0f;
	m[2][2] = depth / 2.0f;

	m[0][3] = x_v + w / 2;
	m[1][3] = y_v + h / 2;
	m[2][3] = depth / 2;

	return m;

}

mat4 lookat(vec3f camPos, vec3f targetPos, vec3f globalUp = vec3f(0.0, 1.0, 0.0)) {
/*	
	The Camera Transform or The View Matrix

	-->	Imitates glm::lookAt()
	-->	Returns a matrix that transforms local space to camera space
	-->	Need to transform local space coords to world space coords then apply camera transform to transform all vertices as if viewed from the camera
	Inputs needed would be :
		1. camera pos
		2. target pos
		3. global_up vector(global up vector)
	
	Aim is to perform a change of basis, that is to change from standard 3d basis to a basis relative to camera pos,
	The basis vectors would be : cameraDir, camUp, camRight

	camDir = (camPos - targetPos).normalize
	camRight = cross(global_up,camDir).normalize
	camUp = cross(camDir,camRight).normalize

	There now we have the new basis vectors, just calculate a change of basis matrix to oreint everything according to the camera, also translate everything opposite to camera's position since after change of basis camera must be at the origin looking down the negative z axis


	This gives us the matrices: 
	   	(1)The change of basis matrix is : 
			
				[		|		|		|		]
				[	camRight	camUp	camDir	]
				[		|		|		|		]
				[		|		|		|		]

		Now is actuality we are not moving around with our camera but ratheer moving the scene opp to motion of camera , hence for a true transform , we need to multiply with inverse of the above matrix , and it happend to be that the above matrix is orthogonal hence , its inverse is its TRANSPOSE

		(2)Translation Matrix , again since motion is opp to camera , we translate everything by (-camPos)
				The matrix is : 
					[1		0		0		-camPos.x	]
					[0		1		0		-camPos.y	]
					[0		0		1		-camPos.z	]
					[0		0		0			1		]

			If we apply these two matrices we get what is called the Model View matrix/transform we simply transforms all coords as if being viewed from the camera.
*/

	mat4 R = mat4::identity();
	mat4 T = mat4::identity();
	vec3f camDir = (camPos - targetPos).normalize();
	// vec3f camDir = (targetPos - camPos).normalize();
	vec3f camRight = cross(globalUp, camDir).normalize();
	vec3f camUp = cross(camDir, camRight).normalize();


	R[0][0] = camRight.x;	R[0][1] = camRight.y;	R[0][2] = camRight.z;	R[0][3] = 0;
	R[1][0] = camUp.x;		R[1][1] = camUp.y;		R[1][2] = camUp.z;		R[1][3] = 0;
	R[2][0] = camDir.x;		R[2][1] = camDir.y;		R[2][2] = camDir.z;		R[2][3] = 0;
	R[3][0] = 0;			R[3][1] = 0;			R[3][2] = 0;			R[3][3] = 1;

	T[0][3] = -camPos.x;
	T[1][3] = -camPos.y;
	T[2][3] = -camPos.z;

	return R * T;
}

mat4 perspective(float l, float r, float b, float t, float n, float f)
{
    mat4 P = mat4::identity(); // Persepective transform to transform frustum into viewing cube , includes the transformation to transform viewing cube to bi-unit cube, for more details checkout http://www.songho.ca/opengl/gl_projectionmatrix.html

	P[0][0] = (-2.0f*n)/(r-l);
	P[0][2] = -(r+l)/(r-l);

	P[1][1] = (-2.0f*n)/(t-b);
	P[1][2] = -(t+b)/(t-b);

	P[2][2] = -(f+n)/(f-n);
	P[2][3] = (-2.0f*n*f)/(f-n);

	P[3][2] = 1;
	P[3][3] = 0;

    return P;
}

mat4 perspective(float v_fov, float aspectRatio, float front, float back) {
	float theta = v_fov * DEG2RAD;
	float tangent = tanf(theta / 2);		// tangent of half v_fov
	float height = front * tangent;			// half height of near plane
	float width = height * aspectRatio;		// half width of near plane

	return perspective(-width, width, -height, height, front, back);

}

void perspective_division(vec4f	&v) {
	assert(v[3] != 0);
	for (int i = 0; i < 4; i++) {
		v[i] = v[i] / v[3];
	}
}