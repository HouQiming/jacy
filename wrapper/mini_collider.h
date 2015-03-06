#ifndef MINI_COLLIDER_H
#define MINI_COLLIDER_H
typedef void* PFNContactCallback;
extern int bulletInitCollisionWorld(double aabb_size);
extern void* bulletCreateTetModel(double* vbo,int* ebo,int* triangles,int nv,int ne,int nt, double contact_threshold);
extern void bulletDeleteObject(void* p_model);
extern int bulletCollide(void* p_model0,void* p_model1,PFNContactCallback f_callback,void* p_userdata);
#endif
