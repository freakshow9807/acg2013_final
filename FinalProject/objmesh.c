#define _CRT_SECURE_NO_WARNINGS
//#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "objmesh.h"



HObjMesh createMesh(){
    HObjMesh result = 0;
    result = (HObjMesh)malloc(sizeof(ObjMesh));

    memset(result, 0, sizeof(ObjMesh));

    return result;
}


void freeMesh(HObjMesh mesh){
    free(mesh->vertices);
    free(mesh->normals);
    free(mesh->texcoords);
    free(mesh);
}


static void push(void* pparray, int* n,  void* value, int valuesize){
    void** parray = (void**)pparray;
    *parray=realloc(*parray, (*n + 1)*valuesize);
    memcpy((char*)(*parray) + (*n) * valuesize, value, valuesize);
    *n = *n + 1;
}

char const* parseCorner(char const* buf, HObjMesh mesh, int* v, int *vt, int* vn){
    *v = *vt = *vn = 0;

    if (sscanf(buf, "%d/%d/%d ", v, vt, vn) == 1)
        sscanf(buf, "%*d//%d", vn);

    if (*v < 0)  *v  = mesh->numVertices  + *v;  else *v  -= 1;
    if (*vt < 0) *vt = mesh->numTexcoords + *vt; else *vt -= 1;
    if (*vn < 0) *vn = mesh->numNormals   + *vn; else *vn -= 1;

    buf += strcspn(buf, " \t");
    buf += strspn(buf, " \t");
    return buf;
}

void parseFace(char const* buf, HObjMesh mesh){
//    int v =0, vt=0, vn=0;
    Triangle face;

    buf = buf + strspn(buf, " \tf");
    buf = parseCorner(buf, mesh, &face.v[0], &face.vt[0], &face.vn[0]);
    buf = parseCorner(buf, mesh, &face.v[1], &face.vt[1], &face.vn[1]);
    buf = parseCorner(buf, mesh, &face.v[2], &face.vt[2], &face.vn[2]);
    push(&mesh->triangles, &mesh->numFaces, &face, sizeof(face));

    //handle non-triangles
    while (*buf) {
        face.v[1]  = face.v[2];
        face.vt[1] = face.vt[2];
        face.vn[1] = face.vn[2];
        buf = parseCorner(buf, mesh, &face.v[2], &face.vt[2], &face.vn[2]);
        push(&mesh->triangles, &mesh->numFaces, &face, sizeof(face));
    }
}

HObjMesh loadObjFile(const char* filename){
    HObjMesh mesh = NULL;
    FILE*  f = NULL;
    char buf[256];
    char token[16];
//    int line = 0;
    Xyz vertex;

    f = fopen(filename, "r");

    if (f == NULL){
//        printf("Cannot open file %s", filename);
        return NULL;
    }

    mesh = createMesh();


    while (fgets(buf, sizeof(buf), f))
    {
        if (sscanf(buf, " %s ", token) == 1) {
            if (strcmp(token, "v") == 0) {

                sscanf(buf, " %*s %f %f %f", &vertex.x, &vertex.y, &vertex.z);
                push(&mesh->vertices, &mesh->numVertices, &vertex, sizeof(vertex));

            } else if (strcmp(token, "vt") == 0) {

                sscanf(buf, " %*s %f %f %f", &vertex.x, &vertex.y, &vertex.z);
                push(&mesh->texcoords, &mesh->numTexcoords, &vertex, sizeof(vertex));

            } else if (strcmp(token, "vn") == 0) {

                sscanf(buf, " %*s %f %f %f", &vertex.x, &vertex.y, &vertex.z);
                push(&mesh->normals, &mesh->numNormals, &vertex, sizeof(vertex));

            } else if (strcmp(token, "f") == 0) {

                parseFace(buf, mesh);

            }
        }
    }


    fclose(f);

    return mesh;
}

