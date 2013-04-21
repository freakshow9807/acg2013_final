#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "objmesh.h"

#ifdef __APPLE__
    #include <GLUT/glut.h>
    #include <OpenGL/gl.h>
#elif defined _WIN32 || defined _WIN64
    #include <GL/glut.h>
#endif


/*
 * File Name: FinalProject
 * Authors: Michael Berg, William Hearter
 *
 * Description: This is a merge of Project 2 (animation) and additional code.
 *  This will take the user's mouse location after clicking and move the object
 *  we have to that point. For this demonstration the object is a monkey, but
 *  the timing is all of because it goes quite quick, and there is a lot to do
 *  in regards to having it reset and move based on clicks.
 *
 */
typedef struct tagInfoPoint{
    float	t;
    Xyz	    p;
    float	vx;
    float	vy;
    float	vz;
} InfoPoint;

InfoPoint movePath[100];

FILE *f;

int g_mainWindow = -1;

int g_pan = 0;
int g_moving = 0;

int g_npoint = 0;
int g_movePathType = 0;
int g_displaymovePath = 0;
int g_display_points = 0;

int g_mouse_x=-1;
int g_mouse_y=-1;

float g_x_angle = 0;
float g_y_angle = 0;
float g_x_pan = 0;
float g_y_pan = 0;

float g_scale = 1;

float duration = 0;
float g_time_reset = 0;

float g_lightPos[] = {1, 1, -1, 0};

float pt[100];

char g_scannedvariable[6];
char g_scannedvariable_two[6];

char fname[100];
char const* OBJName;

HObjMesh g_mesh = NULL;

int interval(float pt[], float t){
    int i;
    for(i = 1; i < g_npoint - 1; ++i){
        if(pt[i] >= t){
            break;
        }
    }
    return i-1;
}

void tangentChange(){
    int i;
    movePath[0].vx = 0;		movePath[0].vy = 0;
    movePath[g_npoint -1].vx = 0;	movePath[g_npoint -1].vy = 0;
    for(i = 1; i < g_npoint; ++i){
        movePath[i].vx = (movePath[i+1].p.x-movePath[i-1].p.x)/(movePath[i+1].t-movePath[i-1].t);
        movePath[i].vy = (movePath[i+1].p.y-movePath[i-1].p.y)/(movePath[i+1].t-movePath[i-1].t);
        movePath[i].vz = (movePath[i+1].p.z-movePath[i-1].p.z)/(movePath[i+1].t-movePath[i-1].t);
    }
}

float lerp(float p0, float p1, float out){
    return (1-out)*p0 + out*p1;
}

float bez2(float p0, float p1, float p2, float out){
    return lerp(lerp(p0, p1, out), lerp(p1, p2, out), out);
}

float bez3(float p0, float p1, float p2, float p3, float out){
    return bez2(lerp(p0, p1, out), lerp(p1, p2, out), lerp(p2, p3, out), out);
}

float herm(float p0, float v0, float p1, float v1, float out){
    return bez3(p0, p0 + v0/3, p1 - v1/3, p1, out);
}

void readMesh(char const* meshfilename){
    g_mesh = loadObjFile(meshfilename);
}

void splinePoint(float t, float* x, float* y, float* z){
    int i;
    float out;

    i = interval(pt, t);
    out = (t - pt[i])/(pt[i+1]-pt[i]);

    if(t <= movePath[g_npoint-1].t){
        *x = herm(movePath[i].p.x, movePath[i].vx, movePath[i+1].p.x, movePath[i+1].vx, out);
        *y = herm(movePath[i].p.y, movePath[i].vy, movePath[i+1].p.y, movePath[i+1].vy, out);
        *z = herm(movePath[i].p.z, movePath[i].vz, movePath[i+1].p.z, movePath[i+1].vz, out);
    }else{
        *x = movePath[g_npoint-1].p.x;
        *y = movePath[g_npoint-1].p.y;
        *z = movePath[g_npoint-1].p.z;
    }
}

void linearPoint(float t, float* x, float* y, float* z){
    int i;
    float out;

    i = interval(pt, t);
    out = (t - pt[i])/(pt[i+1]-pt[i]);

    if(t <= movePath[g_npoint-1].t){
        *x = lerp(movePath[i].p.x, movePath[i+1].p.x, out);
        *y = lerp (movePath[i].p.y, movePath[i+1].p.y, out);
        *z = lerp(movePath[i].p.z, movePath[i+1].p.z, out);
    }else{
        *x = movePath[g_npoint-1].p.x;
        *y = movePath[g_npoint-1].p.y;
        *z = movePath[g_npoint-1].p.z;
    }
}

void drawCurve(int ndivisions){
    int i;
    float t;
    float x, y, z;
    glPushAttrib(GL_LIGHTING_BIT);
    glBegin(GL_LINE_STRIP);
    for(i = 0; i <= ndivisions; ++i){
        t = i * movePath[g_npoint-1].t/(float)ndivisions;
        splinePoint(t, &x, &y, &z);
        glVertex3f(x, y, z);
    }
    glEnd();
    glPopAttrib();
}

float current_time(){
    float t = 0;
    t = clock()/((float)CLOCKS_PER_SEC/10) - g_time_reset;
    t = (float)fmod(t, duration);
    return t;
}

void drawMesh()
{
    int i, j;

    if (g_mesh){

        glBegin(GL_TRIANGLES);

        for (i =0; i < g_mesh->numFaces; ++i){
            HTriangle face = &g_mesh->triangles[i];
            for (j = 0; j < 3; ++j){

                if (face->vn[j] >= 0){
                    glNormal3fv(&g_mesh->normals[face->vn[j]].x);
                }

                if (face->vt[j] >= 0){
                    glTexCoord3fv(&g_mesh->texcoords[face->vt[j]].x);
                }

                glVertex3fv(&g_mesh->vertices[g_mesh->triangles[i].v[j]].x);
            }
        }

        glEnd();
    }
}

void display(void){
    int i;
    float t;
    Xyz p;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_COLOR_MATERIAL);
    glLoadIdentity();

    glRotated(g_x_angle, 1.0f, 0.0f, 0.0f);
    glRotated(g_y_angle, 0.0f, 1.0f, 0.0f);
    glScalef(g_scale, g_scale, g_scale);
    glTranslatef(g_x_pan, -g_y_pan, 0);


    glColor3f(0, 0, 0);


    if(g_display_points == 1){
        glPointSize(4);
        glBegin(GL_POINTS);
        for(i = 0; i < g_npoint; ++i){
            glVertex3fv(&movePath[i].p.x);
        }
        glEnd();
    }

    t = current_time();


    if(g_movePathType == 0){
        if(g_displaymovePath == 1){
            glBegin(GL_LINE_STRIP);
            for(i = 0; i < g_npoint; ++i){
                glVertex3fv(&movePath[i].p.x);
            }
            glEnd();
        }
        linearPoint(t, &p.x, &p.y, &p.z);
    }
    if(g_movePathType == 1){
        if( g_displaymovePath == 1)
            drawCurve(1000);
        splinePoint(t, &p.x, &p.y, &p.z);
    }
    glTranslatef(p.x, p.y, p.z);

    glScaled(.3, .3, .3);
    glColor3f(1, 0, 1);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    drawMesh();

    g_npoint = 0;
    glutSwapBuffers();

}

void reshape(int w, int h){
    float aspect = w / (float)h;

    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-aspect, aspect, -1, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
}

void readFile(){
    char bufline[20];
    f = fopen(fname, "rt");


    while(fgets(bufline, 20, f) != NULL){
        int i;
        sscanf(bufline, "%s", g_scannedvariable);
        for(i = 0; g_scannedvariable[i]; i++){
            g_scannedvariable[i] = tolower(g_scannedvariable[i]);
        }
        if(strcmp(g_scannedvariable, "animation") == 0){
            sscanf(bufline, "%*s %f", &duration);
        }else if(strcmp(g_scannedvariable, "object") == 0){
            sscanf(bufline, "%*s %s", g_scannedvariable_two);
            printf("%s", g_scannedvariable_two);
            readMesh(g_scannedvariable_two);
        }else if(strcmp(g_scannedvariable, "position") == 0){
            sscanf(bufline, "%*s %f %f %f %f", &movePath[g_npoint].t, &movePath[g_npoint].p.x, &movePath[g_npoint].p.y, &movePath[g_npoint].p.z);
            pt[g_npoint] = movePath[g_npoint].t;
            g_npoint++;
        }
    }
    fclose(f);
    tangentChange();
    g_time_reset = clock()/((float)CLOCKS_PER_SEC/10);
}

void mouse(int button, int state, int x, int y){
    int mod = glutGetModifiers();
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && mod != GLUT_ACTIVE_SHIFT){
        g_moving = 1;
        g_mouse_x = x;
        g_mouse_y = y;
        printf("MOUSE: %d, %d\n", x, y);
        g_npoint = 1;
    }
    if (button == GLUT_LEFT_BUTTON && state==GLUT_UP){
        g_moving = 0;
        g_pan = 0;
    }
    if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && mod == GLUT_ACTIVE_SHIFT){
        g_pan = 1;
        g_mouse_x = x;
        g_mouse_y = y;
    }
}

void motion(int x, int y){
    int dx, dy;
    if (g_moving){
        dx = x - g_mouse_x;
        dy = y - g_mouse_y;
        g_x_angle += dy;
        g_y_angle += dx;
        g_mouse_x = x;
        g_mouse_y = y;
    }
    if (g_pan){
        dx = x - g_mouse_x;
        dy = y - g_mouse_y;
        g_x_pan += (float)dx/glutGet(GLUT_WINDOW_WIDTH)/g_scale;
        g_y_pan += (float)dy/glutGet(GLUT_WINDOW_HEIGHT)/g_scale;
        g_mouse_x = x;
        g_mouse_y = y;
        printf("x: %f y: %f\n", g_x_pan, g_y_pan);
    }
}


void idle(void){
    glutSetWindow(g_mainWindow);
    glutPostRedisplay();
}
void getRidOfWarnings(int x, int y){
    x = 0;
    y = 0;
}

void special(int key, int x, int y){
    getRidOfWarnings(x, y);
    switch(key){
    case GLUT_KEY_LEFT:
        g_y_angle -= 5;
        break;
    case GLUT_KEY_RIGHT:
        g_y_angle += 5;
        break;
    case GLUT_KEY_UP:
        g_x_angle -= 5;
        break;
    case GLUT_KEY_DOWN:
        g_x_angle += 5;
        break;
    }
}


void resetCommands(){
    int i;

    glTranslatef(0, 0, 0);

    g_x_angle = 0;
    g_y_angle = 0;
    g_x_pan = 0;
    g_y_pan = 0;

    g_scale = 1;

    g_npoint = 0;

    for(i = 0; i < g_npoint; ++i){
        movePath[i].t = 0;

        movePath[i].p.x = 0;
        movePath[i].p.y = 0;
        movePath[i].p.z = 0;

        movePath[i].vx = 0;
        movePath[i].vy = 0;
        movePath[i].vz = 0;

        pt[i] = 0;
    }
}


void keyboard(unsigned char key, int x, int y){
    getRidOfWarnings(x, y);
    switch (key){
    case ' ':{
        resetCommands();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//        readFile();
        glutSwapBuffers();
             }break;
    case 'l':{
        g_movePathType = !g_movePathType;
             }break;
    case 'd':{
        g_displaymovePath = !g_displaymovePath;
             }break;
    case 'p':{
        g_display_points = !g_display_points;
             }break;
    case '`':{
        g_x_angle = 0;
        g_y_angle = 0;

        g_x_pan = 0;
        g_y_pan = 0;

        g_scale = 1;
        }break;
    case '=':{
        g_scale += (float).01;
        }break;
    case '-':{
        g_scale -= (float).01;
        }break;
    }
}


int main(int ac, char* av[]){

    glutInit(&ac, av);

    strcpy(fname, av[1]);

    readFile();
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);

    g_mainWindow = glutCreateWindow(fname);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutIdleFunc(idle);

    glClearColor(0.5, 0.4, 0.7, 0);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);

    glLightfv(GL_LIGHT0, GL_POSITION, g_lightPos);

    glutMainLoop();
    return 0;
}
