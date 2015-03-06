//qpad: paths_include="..\\osslib\\include"
#include "wrapper_defines.h"
#include "btBulletCollisionCommon.h"
#include "BulletSoftBody/btSoftBody.h"
#include "BulletSoftBody/btSoftBodyInternals.h"
#include "BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h"
#include "BulletSoftBody/btDefaultSoftBodySolver.h"

//#define EXPORT extern "C" __declspec(dllexport)

static btSoftBodyWorldInfo g_winfo;
static btCollisionWorld* g_collisionWorld=NULL;
static btSoftBodySolver* g_softbody_solver=NULL;

EXPORT int bulletInitCollisionWorld(double aabb_size){
	if(g_collisionWorld){return 0;}
	//we don't free it
	btDefaultCollisionConfiguration* collisionConfiguration = new btSoftBodyRigidBodyCollisionConfiguration();//btDefaultCollisionConfiguration();
	btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
	btVector3	worldAabbMin(-(btScalar)aabb_size,-(btScalar)aabb_size,-(btScalar)aabb_size);
	btVector3	worldAabbMax((btScalar)aabb_size,(btScalar)aabb_size,(btScalar)aabb_size);
	btAxisSweep3*	broadphase = new btAxisSweep3(worldAabbMin,worldAabbMax);
	g_collisionWorld = new btCollisionWorld(dispatcher,broadphase,collisionConfiguration);
	g_softbody_solver=new btDefaultSoftBodySolver();
	return 1;
}

EXPORT btSoftBody* bulletCreateTetModel(double* vbo,int* ebo,int* triangles,int nv,int ne,int nt, double contact_threshold){
	btAlignedObjectArray<btVector3>	pos;
	pos.resize(nv);
	for(int i=0;i<nv;++i){
		pos[i].setX(btScalar(vbo[i*3+0]));
		pos[i].setY(btScalar(vbo[i*3+1]));
		pos[i].setZ(btScalar(vbo[i*3+2]));
	}
	btSoftBody*	psb=new btSoftBody(&g_winfo,nv,&pos[0],0);
	psb->m_cfg.collisions|=btSoftBody::fCollision::VF_SS;
	for(int i=0;i<ne;++i){
		psb->appendTetra(ebo[i*4+0],ebo[i*4+1],ebo[i*4+2],ebo[i*4+3]);
	}
	for(int i=0;i<nt;++i){
		psb->appendFace(triangles[i*3+0],triangles[i*3+1],triangles[i*3+2]);
	}
	psb->setContactProcessingThreshold((btScalar)contact_threshold);
	psb->getCollisionShape()->setMargin((btScalar)contact_threshold);
	psb->setSoftBodySolver(g_softbody_solver);
	psb->initializeFaceTree();
	//g_collisionWorld->addCollisionObject(psb);
	return (btSoftBody*)psb;
}

EXPORT void bulletDeleteObject(btSoftBody* p_model){
	//g_collisionWorld->removeCollisionObject((btSoftBody*)p_model);
	delete p_model;
}

typedef void(*PFNContactCallback)(
	void*,
	int/*side*/,int/*node*/,int/*face*/,double,double,double/*barycentric coords*/,
	double,double,double/*normal*/,
	double,double,double/*penetration vector*/);
class CCallbackToC:public btCollisionWorld::ContactResultCallback{
public:
	//PFNContactCallback m_callback;
	//void* m_userdata;
	virtual btScalar addSingleResult(btManifoldPoint& cp,
			const btCollisionObjectWrapper* colObj0,int partId0,int index0,
			const btCollisionObjectWrapper* colObj1,int partId1,int index1)
	{
		//m_callback(
		//	m_userdata,
		//	cp.m_positionWorldOnA.x(),cp.m_positionWorldOnA.y(),cp.m_positionWorldOnA.z(),colObj0->m_collisionObject,partId0,index0,
		//	cp.m_positionWorldOnB.x(),cp.m_positionWorldOnB.y(),cp.m_positionWorldOnB.z(),colObj1->m_collisionObject,partId1,index1,
		//	cp.m_normalWorldOnB.x(),cp.m_normalWorldOnB.y(),cp.m_normalWorldOnB.z(),
		//	cp.m_distance1);
		//assert(0);
		return 0;
	}
};

static void parseSContacts(btSoftBody* p_model_node,btSoftBody* p_model_face,int side, PFNContactCallback f_callback,void* p_userdata){
	int n=(int)p_model_node->m_scontacts.size();
	for(int i=0;i<n;i++){
		const btSoftBody::SContact& c=p_model_node->m_scontacts[i];
		//this "normal" is along a wrong direction
		btSoftBody::Node* n=c.m_node;
		btSoftBody::Face* f=c.m_face;
		btVector3 nr=c.m_normal;
		btVector3 n0=-btCross(f->m_n[1]->m_x-f->m_n[0]->m_x,f->m_n[2]->m_x-f->m_n[0]->m_x).normalized();
		btVector3 p=BaryEval(f->m_n[0]->m_x,f->m_n[1]->m_x,f->m_n[2]->m_x,c.m_weights);
		//printf("%f\n",c.m_margin);
		int node_id=(n-&p_model_node->m_nodes[0]);
		int face_id=(f-&p_model_face->m_faces[0]);
		btVector3 penetration_vector=p - n->m_x;
		if(btDot(nr,n0)<0.f){nr=-nr;}
		f_callback(p_userdata,
			side,node_id, face_id,c.m_weights.x(),c.m_weights.y(),c.m_weights.z(),
			nr.x(),nr.y(),nr.z(),
			penetration_vector.x(),penetration_vector.y(),penetration_vector.z());
	}
	p_model_node->m_scontacts.clear();
}

EXPORT int bulletCollide(btSoftBody* p_model0,btSoftBody* p_model1, PFNContactCallback f_callback,void* p_userdata){
	if(!g_collisionWorld){return 0;}
	CCallbackToC cb;
	//cb.m_callback=f_callback;
	//cb.m_userdata=p_userdata;
	g_collisionWorld->contactPairTest(p_model0,p_model1,cb);
	////////////////////////
	//parse m_scontacts to get node/face ids
	//we need: nodeid, unnormalized penetration vector, nothing more
	parseSContacts(p_model0,p_model1,0, f_callback,p_userdata);
	parseSContacts(p_model1,p_model0,1, f_callback,p_userdata);
	return 1;
}

//Transform3f
