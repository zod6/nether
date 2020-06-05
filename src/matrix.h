#ifndef NETHER_MATRIX
#define NETHER_MATRIX

void set_projection(float l, float r, float b, float t, float n, float f, float *out);
void set_projection_ortho(float l, float r, float b, float t, float n, float f, float *out);
void set_view_matrix(Vector& pos, Vector& target, Vector& up, float *out);
int unproject(int mx, int my, float mz, float *modelview_matrix, float *projection_matrix, int *viewport, float *x, float *y, float *z);
int project(float objx, float objy, float objz, float *modelview_matrix, float *projection_matrix, int *viewport, int *x,int *y, float *z);

#endif

