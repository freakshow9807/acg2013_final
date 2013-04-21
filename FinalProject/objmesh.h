#ifndef OBJMESH_H_INCLUDED
#define OBJMESH_H_INCLUDED

typedef struct tagXyz {float x, y, z;} Xyz, *HXyz;

typedef struct tagTriangle {
    int v[3];
    int vn[3];
    int vt[3];
} Triangle, *HTriangle;

struct tagObjMesh {
    int numVertices;
    int numFaces;
    int numNormals;
    int numTexcoords;
    Xyz* vertices;
    Xyz* normals;
    Xyz* texcoords;
    Triangle* triangles;
} ;

typedef struct tagObjMesh ObjMesh;
typedef struct tagObjMesh* HObjMesh;

//static void unpackXyz(HXyz xyz, float* x, float* y, float* z) {
//    *x = xyz->x;
//    *y = xyz->y;
//    *z = xyz->z;
//}

HObjMesh loadObjFile(const char* filename);
HObjMesh createMesh();

void     freeMesh(HObjMesh mesh);



#endif //OBJMESH_H_INCLUDED
