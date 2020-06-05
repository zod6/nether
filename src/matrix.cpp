#include <SDL/SDL_mixer.h>

#include "vector.h"
#include "myglutaux.h"

extern int SCREEN_X,SCREEN_Y;

int invert_matrix(float *m, float *out){
	int i,j,x;
	float num;
	
	for(i=0;i<16;i++) out[i]=0;
	out[0]=out[5]=out[10]=out[15]=1; // set identity

	if(det_d(m,4)==0) return 0;

	for(x=0; x<4; x++){
		num=m[x*4+x];
		for(j=0; j<4; j++){
			m[j*4+x]/=num; // row/row[id], row[id]=1
			out[j*4+x]/=num;
		}
		for(i=0; i<4; i++){ // col
			if(i==x) continue;
			num=m[x*4+i];
			for(j=0; j<4; j++){ // row
				m[j*4+i] -= num*m[j*4+x]; // row[i,j]-row[x,j]*row[1,j]
				out[j*4+i] -= num*out[j*4+x];
			}
		}
	}
	return 1;
}

void set_projection(float l, float r, float b, float t, float n, float f, float *out){
	for(int i=0;i<16;i++) out[i]=0;
    out[0]  = 2 * n / (r - l);
    out[8]  = (r + l) / (r - l);
    out[5]  = 2 * n / (t - b);
    out[9]  = (t + b) / (t - b);
    out[10] = -(f + n) / (f - n);
    out[14] = -(2 * f * n) / (f - n);
    out[11] = -1;
}

void set_projection_ortho(float l, float r, float b, float t, float n, float f, float *out){
	for(int i=0;i<16;i++) out[i]=0;
	out[0]  = 2 / (r - l);
	out[12]  = -(r + l) / (r - l);
	out[5]  = 2 / (t - b);
	out[13]  = -(t + b) / (t - b);
	out[10] = -2 / (f - n);
	out[14] = -(f + n) / (f - n);
	out[15] = 1;
}

void set_view_matrix(Vector& pos, Vector& target, Vector& up, float *out){
    // compute left/up/forward axis vectors
    Vector z = pos - target;
    z.normalize();
    Vector x = up^z;
    x.normalize();
    Vector y = z^x;
    y.normalize();

    // compute M = Mr * Mt
    out[0] = x.x;		out[4] = x.y;		out[8] = x.z;		out[12]= x*(-pos);
    out[1] = y.x;		out[5] = y.y;		out[9] = y.z;		out[13]= y*(-pos);
    out[2] = z.x;		out[6] = z.y;		out[10]= z.z;		out[14]= z*(-pos);
    out[3] = 0.0f;		out[7] = 0.0f;		out[11]= 0.0f;		out[15] = 1.0f;
}

int project(float objx, float objy, float objz, float *modelview_matrix, float *projection_matrix, int *viewport, int *x,int *y, float *z) {
	float in[4];
	float out[4];

	in[0]=objx;
	in[1]=objy;
	in[2]=objz;
	in[3]=1.0;

	ApplyMatrix(in, modelview_matrix, out);
	ApplyMatrix(out, projection_matrix, in);

	if (in[3] == 0.0) return 0;
	in[0] /= in[3];
	in[1] /= in[3];
	in[2] /= in[3];

	/* Map x, y and z to range 0-1 and then x,y to viewport */
	*x=(in[0]*0.5+0.5) * viewport[2] + viewport[0];
	*y=SCREEN_Y - (in[1]*0.5+0.5) * viewport[3] - viewport[1];
	*z=in[2] * 0.5 + 0.5;
//	printf("\nout: %f, %f, %f - %d, %d, %d\n", objx, objy, objz, *x, *y, *z);
	return 1;
}

int unproject(int mx, int my, float mz, float *modelview_matrix, float *projection_matrix, int *viewport, float *x, float *y, float *z){
	float A_matrix[16], inverted_matrix[16];
	float screen[4],out[4];
	float *p=A_matrix;

	if(modelview_matrix) MulMatrix(projection_matrix, modelview_matrix, A_matrix);
	else p=projection_matrix;

	if(!invert_matrix(p, inverted_matrix)) return 0;

	// Map x, y to range -1 - 1
	screen[0]=(mx-(float)viewport[0])/(float)viewport[2]*2.0-1.0;
	screen[1]=float(SCREEN_Y-my-viewport[1])/(float)viewport[3]*2.0-1.0;
	screen[2]=2.0*mz-1;
	screen[3]=1;

	ApplyMatrix(screen, inverted_matrix, out);

	if(out[3]==0.0) return 0;
	*x=out[0]/out[3];
	*y=out[1]/out[3];
	*z=out[2]/out[3];

	return 1;
}

