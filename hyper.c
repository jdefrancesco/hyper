/*
 * By Sam Askinas
 *
 * Ported to macOS by J. DeFrancesco
 */

#define GL_SILENCE_DEPRECATION

#include <GLUT/glut.h>
#include <OpenGL/gl3.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef WIN_32
#include <unistd.h>
#else
#include <windows.h>
#endif


// ascii for escape key
#define ESCAPE 27

// codes for rotate function
#define R_XY 0
#define R_YZ 1
#define R_ZX 2
#define R_XT 3
#define R_YT 4
#define R_ZT 5

// number of the window
int window;

// rotation angles
float theta = 0;
float theta2 = 0;
float theta3 = 0;

// lighting
GLfloat LightAmbient[] = {0.5f, 0.5f, 0.5f, 1.0f};
GLfloat LightDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat LightPosition[] = {0.0f, 0.0f, 2.0f, 1.0f};

// used to control the speed
int elapsedTime = 0;

struct vertex4d {
    float x, y, z, t;
};

struct vertex3d {
    float x, y, z;
};

struct edge4d {
    struct vertex4d* tail;
    struct vertex4d* head;
};

struct edge3d {
    struct vertex3d* tail;
    struct vertex3d* head;
};

struct vertex4d newVertex4d(float x, float y, float z, float t) {
    struct vertex4d newVertex;
    newVertex.x = x;
    newVertex.y = y;
    newVertex.z = z;
    newVertex.t = t;
    return newVertex;
}

struct edge4d newEdge4d(struct vertex4d* head, struct vertex4d* tail) {
    struct edge4d newEdge;
    newEdge.head = head;
    newEdge.tail = tail;
    return newEdge;
}

struct vertex3d newVertex3d(float x, float y, float z) {
    struct vertex3d newVertex;
    newVertex.x = x;
    newVertex.y = y;
    newVertex.z = z;
    return newVertex;
}

struct edge3d newEdge3d(struct vertex3d* tail, struct vertex3d* head) {
    struct edge3d newEdge;
    newEdge.head = head;
    newEdge.tail = tail;
    return newEdge;
}

struct uedge3d {
    float x, y, z;
};

struct uedge3d unitFromEdge3d(struct edge3d* edgePtr) {
    struct vertex3d* head = edgePtr->head;
    struct vertex3d* tail = edgePtr->tail;

    float dx = tail->x - head->x;
    float dy = tail->y - head->y;
    float dz = tail->z - head->z;

    float mag = sqrt(dx * dx + dy * dy + dz * dz);

    struct uedge3d newUEdge;
    if (mag != 0) {
        newUEdge.x = dx / mag;
        newUEdge.y = dy / mag;
        newUEdge.z = dx / mag;
    } else {
        newUEdge.x = 0;
        newUEdge.y = 0;
        newUEdge.z = 0;
    }
    return newUEdge;
}

struct uedge3d crossZ(struct uedge3d* uedgePtr) {
    struct uedge3d newUEdge;

    float dx = uedgePtr->y;
    float dy = -uedgePtr->x;
    float dz = 0;

    float mag = sqrt(dx * dx + dy * dy + dz * dz);

    if (mag != 0) {
        newUEdge.x = dx / mag;
        newUEdge.y = dy / mag;
        newUEdge.z = dz / mag;
    } else {
        newUEdge.x = 0;
        newUEdge.y = 0;
        newUEdge.z = 0;
    }
    return newUEdge;
}

struct vertex4d hypercubeVertices[16];
struct edge4d hypercubeEdges[32];

void setHypercubeVertices(struct vertex4d* hvertices) {
    int i, j, k, l, count, it, jt, kt, lt;
    i = j = k = l = count = 0;
    for (i = 0; i < 2; ++i) {
        for (j = 0; j < 2; ++j) {
            for (k = 0; k < 2; ++k) {
                for (l = 0; l < 2; ++l) {
                    it = i ? 1 : -1;
                    jt = j ? 1 : -1;
                    kt = k ? 1 : -1;
                    lt = l ? 1 : -1;
                    hvertices[count] =
                        newVertex4d((float)it, (float)jt, (float)kt, (float)lt);
                    ++count;
                }
            }
        }
    }
}

int usedEdge(int edge_h, int edge_t, int* used) {
    int index = edge_t * 16 + edge_h;
    if (used[index]) {
        return 1;
    } else {
        used[edge_h * 16 + edge_t] = 1;
        return 0;
    }
}

void setHypercubeEdges(struct edge4d* hedges) {
    int used[256];
    int n = 0;
    for (; n < 256; ++n) used[n] = 0;
    int edgeCount = 0;

    int inv0 = 0x01;
    int inv1 = 0x02;
    int inv2 = 0x04;
    int inv3 = 0x08;

    int currentVertex = 0;
    for (; currentVertex < 16; ++currentVertex) {
        if (!usedEdge(currentVertex, currentVertex ^ inv0, used)) {
            hedges[edgeCount] =
                newEdge4d(&hypercubeVertices[currentVertex],
                          &hypercubeVertices[currentVertex ^ inv0]);
            ++edgeCount;
        }
        if (!usedEdge(currentVertex, currentVertex ^ inv1, used)) {
            hedges[edgeCount] =
                newEdge4d(&hypercubeVertices[currentVertex],
                          &hypercubeVertices[currentVertex ^ inv1]);
            ++edgeCount;
        }
        if (!usedEdge(currentVertex, currentVertex ^ inv2, used)) {
            hedges[edgeCount] =
                newEdge4d(&hypercubeVertices[currentVertex],
                          &hypercubeVertices[currentVertex ^ inv2]);
            ++edgeCount;
        }
        if (!usedEdge(currentVertex, currentVertex ^ inv3, used)) {
            hedges[edgeCount] =
                newEdge4d(&hypercubeVertices[currentVertex],
                          &hypercubeVertices[currentVertex ^ inv3]);
            ++edgeCount;
        }
    }
}

struct vertex3d vertex4dto3d(struct vertex4d* vertexPtr) {
    struct vertex3d newVertex;
    newVertex.x = vertexPtr->x;
    newVertex.y = vertexPtr->y;
    newVertex.z = vertexPtr->z;
    return newVertex;
}

void rotateVertex4dXY(float angle, float* xn, float* yn, float* zn, float* tn) {
    float xt, yt, zt, tt;
    xt = (*xn) * cos(angle) + (*yn) * sin(angle);
    yt = (*xn) * -sin(angle) + (*yn) * cos(angle);
    (*xn) = xt;
    (*yn) = yt;
}
void rotateVertex4dYZ(float angle, float* xn, float* yn, float* zn, float* tn) {
    float xt, yt, zt, tt;
    yt = (*yn) * cos(angle) + (*zn) * sin(angle);
    zt = (*yn) * -sin(angle) + (*zn) * cos(angle);
    (*yn) = yt;
    (*zn) = zt;
}
void rotateVertex4dZX(float angle, float* xn, float* yn, float* zn, float* tn) {
    float xt, yt, zt, tt;
    xt = (*xn) * cos(angle) + (*zn) * -sin(angle);
    zt = (*xn) * sin(angle) + (*zn) * cos(angle);
    (*xn) = xt;
    (*yn) = yt;
}
void rotateVertex4dXT(float angle, float* xn, float* yn, float* zn, float* tn) {
    float xt, yt, zt, tt;
    xt = (*xn) * cos(angle) + (*tn) * sin(angle);
    tt = (*xn) * -sin(angle) + (*tn) * cos(angle);
    (*xn) = xt;
    (*tn) = tt;
}
void rotateVertex4dYT(float angle, float* xn, float* yn, float* zn, float* tn) {
    float xt, yt, zt, tt;
    yt = (*yn) * cos(angle) + (*tn) * -sin(angle);
    tt = (*yn) * sin(angle) + (*tn) * cos(angle);
    (*yn) = yt;
    (*tn) = tt;
}
void rotateVertex4dZT(float angle, float* xn, float* yn, float* zn, float* tn) {
    float xt, yt, zt, tt;
    zt = (*tn) * -sin(angle) + (*zn) * cos(angle);
    tt = (*tn) * cos(angle) + (*zn) * sin(angle);
    (*zn) = zt;
    (*tn) = tt;
}

// rotate 4d vectors given the axis of rotation
void rotate4d(float angle, int rotateCode, float* x1, float* x2, float* y1,
              float* y2, float* z1, float* z2, float* t1, float* t2) {
    switch (rotateCode) {
        case 0:
            rotateVertex4dXY(angle, x1, y1, z1, t1);
            rotateVertex4dXY(angle, x2, y2, z2, t2);
            break;
        case 1:
            rotateVertex4dYZ(angle, x1, y1, z1, t1);
            rotateVertex4dYZ(angle, x2, y2, z2, t2);
            break;
        case 2:
            rotateVertex4dZX(angle, x1, y1, z1, t1);
            rotateVertex4dZX(angle, x2, y2, z2, t2);
            break;
        case 3:
            rotateVertex4dXT(angle, x1, y1, z1, t1);
            rotateVertex4dXT(angle, x2, y2, z2, t2);
            break;
        case 4:
            rotateVertex4dYT(angle, x1, y1, z1, t1);
            rotateVertex4dYT(angle, x2, y2, z2, t2);
            break;
        case 5:
            rotateVertex4dZT(angle, x1, y1, z1, t1);
            rotateVertex4dZT(angle, x2, y2, z2, t2);
            break;
        default:
            printf("\nERROR: incorrect rotation code\n");
            break;
    }
}

// draws an edge of the hypercube to the screen after calling rotate
void drawEdge(struct edge4d* edgePtr) {
    struct vertex4d* head = edgePtr->head;
    struct vertex4d* tail = edgePtr->tail;

    float x1, x2, y1, y2, z1, z2, t1, t2;

    x1 = head->x;
    y1 = head->y;
    z1 = head->z;
    t1 = head->t;
    x2 = tail->x;
    y2 = tail->y;
    z2 = tail->z;
    t2 = tail->t;

    // rotating along XT, YT, ZT, and YZ axes
    rotate4d(theta2, R_ZT, &x1, &x2, &y1, &y2, &z1, &z2, &t1, &t2);
    rotate4d(theta2, R_XT, &x1, &x2, &y1, &y2, &z1, &z2, &t1, &t2);
    rotate4d(theta3, R_YT, &x1, &x2, &y1, &y2, &z1, &z2, &t1, &t2);
    rotate4d(theta3, R_YZ, &x1, &x2, &y1, &y2, &z1, &z2, &t1, &t2);

    // draws projection of 4d edge in 3d
    if ((int)(x2 * y2 * z2) == 1 || (int)(x1 * y1 * z1) == 1) {
    }
    // front
    glBegin(GL_LINES);  // start drawing a 4-sided polygon
    glVertex3f(x1, y1, z1);
    glVertex3f(x2, y2, z2);
    glEnd();
}

// a general OpenGL initialization function, sets all parameters
void InitGL(int Width, int Height) {
    // clear background color to black
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // enable clearing of depth buffer
    glClearDepth(1.0);

    // the type of depth testing to do
    glDepthFunc(GL_LESS);

    // enables depth testing
    glEnable(GL_DEPTH_TEST);

    // enables smooth color shading
    glShadeModel(GL_SMOOTH);

    // reset the projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // calculate aspect ratio of window
    gluPerspective(45.0f, (GLfloat)Width / (GLfloat)Height, 0.1f, 100.0f);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);
    glMatrixMode(GL_MODELVIEW);
}

// function called when window is resized
void ReSizeGLScene(int Width, int Height) {
    if (Height == 0) Height = 1;
    // reset viewport and perspective transformation
    glViewport(0, 0, Width, Height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(45.0f, (GLfloat)Width / (GLfloat)Height, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
}

// main drawing function
void DrawGLScene() {
    int time = glutGet(GLUT_ELAPSED_TIME);

    if (time - elapsedTime < 50) return;

    elapsedTime = time;

    // clear the screen and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    // reset the view
    glLoadIdentity();

    GLfloat material[] = {0.2f, 0.2f, 0.5f};
    glMaterialfv(GL_FRONT, GL_EMISSION, &material[2]);

    // move left 1.5 units and into the screen 6.0
    glTranslatef(0.0f, 0.0f, -6.0f);

    theta2 += .01;
    theta3 += .005;
    if (theta2 > 2 * 3.14159) theta2 = 0;
    if (theta3 > 2 * 3.14159) theta3 = 0;
    // draw a square
    int n;
    struct edge4d currentEdge;
    for (n = 0; n < 32; ++n) {
        currentEdge = hypercubeEdges[n];
        drawEdge(&currentEdge);
    }
    // swap buffers to display, since we're double buffered/
    glutSwapBuffers();
}

// function called whenever a key is pressed
void KeyPressed(unsigned char key, int x, int y) {
// avoid thrashing this procedure
#ifndef WIN_32
    usleep(100);
#else
    Sleep(100);
#endif

    // if escape is pressed, kill everything
    if (key == ESCAPE) {
        // shutdown window
        glutDestroyWindow(window);
    }

    exit(0);
}

int main(int argc, char** argv) {
    // new hypercube code
    setHypercubeVertices(hypercubeVertices);
    setHypercubeEdges(hypercubeEdges);

    // takes relevant command line arguments
    glutInit(&argc, argv);

    // select display mode
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);

    // window initialization settings
    glutInitWindowSize(640, 480);
    glutInitWindowPosition(0, 0);

    // open a window
    window = glutCreateWindow("new window");

    // regiser function to do OpenGL drawing
    glutDisplayFunc(&DrawGLScene);

    // soonest possible to go fullscreen
    glutFullScreen();

    // even if no events, render scene
    glutIdleFunc(&DrawGLScene);

    // register resize function
    glutReshapeFunc(&ReSizeGLScene);

    // register keypress function
    glutKeyboardFunc(&KeyPressed);

    // initialize the window
    InitGL(640, 480);

    // start event processing engine
    glutMainLoop();

    return 1;
}
