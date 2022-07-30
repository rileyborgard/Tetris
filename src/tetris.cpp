
#include <iostream>
#include <chrono>
#include <set>
#include <GL/freeglut.h>

#define COLOR(x) (x)[0], (x)[1], (x)[2]
#define I_BLOCK 0
#define O_BLOCK 1
#define T_BLOCK 2
#define S_BLOCK 3
#define Z_BLOCK 4
#define J_BLOCK 5
#define L_BLOCK 6
#define EMPTY 7

const int W = 10, H = 20;
int width = W * 80, height = H * 40;
const int FPS = 2, DAS1 = 5, DAS2 = 20;
float backgroundCol[3] = {0, 0, 0};

float blockCol[][3] = {
    {0, 1, 1},
    {1, 1, 0},
    {0.5, 0, 0.5},
    {0, 1, 0},
    {1, 0, 0},
    {0, 0, 1},
    {1, 0.5, 0},
    {0, 0, 0}
};
float backCol[3] = {0.5, 0.5, 0.5};

// type, rotation, piece, coord (x or y)
int block[7][4][4][2] = {
    { // I-block
        {{-2, 0}, {-1, 0}, {0, 0}, {1, 0}},
        {{0, -2}, {0, -1}, {0, 0}, {0, 1}},
        {{-2, 0}, {-1, 0}, {0, 0}, {1, 0}},
        {{0, -2}, {0, -1}, {0, 0}, {0, 1}}
    },
    { // O-block
        {{-1, -1}, {-1, 0}, {0, -1}, {0, 0}},
        {{-1, -1}, {-1, 0}, {0, -1}, {0, 0}},
        {{-1, -1}, {-1, 0}, {0, -1}, {0, 0}},
        {{-1, -1}, {-1, 0}, {0, -1}, {0, 0}}
    },
    { // T-block
        {{0, 0}, {1, 0}, {-1, 0}, {0, 1}},
        {{0, 0}, {-1, 0}, {0, 1}, {0, -1}},
        {{0, 0}, {1, 0}, {-1, 0}, {0, -1}},
        {{0, 0}, {1, 0}, {0, 1}, {0, -1}}
    },
    { // S-block
        {{-1, 0}, {0, 0}, {-1, 1}, {-2, 1}},
        {{0, 0}, {0, 1}, {-1, 0}, {-1, -1}},
        {{-1, 0}, {0, 0}, {-1, 1}, {-2, 1}},
        {{0, 0}, {0, 1}, {-1, 0}, {-1, -1}}
    },
    { // Z-block
        {{0, 0}, {-1, 0}, {0, 1}, {1, 1}},
        {{0, 0}, {0, 1}, {1, 0}, {1, -1}},
        {{0, 0}, {-1, 0}, {0, 1}, {1, 1}},
        {{0, 0}, {0, 1}, {1, 0}, {1, -1}}
    },
    { // J-block
        {{0, 0}, {-1, 0}, {1, 0}, {1, 1}},
        {{0, 0}, {0, -1}, {0, 1}, {-1, 1}},
        {{0, 0}, {1, 0}, {-1, 0}, {-1, -1}},
        {{0, 0}, {0, 1}, {0, -1}, {1, -1}}
    },
    { // L-block
        {{0, 0}, {1, 0}, {-1, 0}, {-1, 1}},
        {{0, 0}, {0, 1}, {0, -1}, {-1, -1}},
        {{0, 0}, {-1, 0}, {1, 0}, {1, -1}},
        {{0, 0}, {0, -1}, {0, 1}, {1, 1}}
    }
};
std::set<char> keys;
long long presstime[256];
long long das[256];

int board[W][H];
int ptype = I_BLOCK, prot = 0, px, py, nexttype;

void idle();
void key_press(unsigned char, int, int);

void drawRect(float x, float y, float x2, float y2) {
    glVertex2f(x, y);
    glVertex2f(x2, y);
    glVertex2f(x2, y2);
    glVertex2f(x, y2);
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBegin(GL_QUADS);
    {
        for(int i = 0; i < W; i++) {
            for(int j = 0; j < H; j++) {
                glColor3f(COLOR(blockCol[board[i][j]]));
                float x = ((float) i) / (2 * W);
                float y = ((float) j) / H;
                float x2 = ((float) (i + 1)) / (2 * W);
                float y2 = ((float) (j + 1)) / H;
                drawRect(x, y, x2, y2);
            }
        }
        glColor3f(COLOR(blockCol[ptype]));
        for(int i = 0; i < 4; i++) {
            float x = ((float) px + block[ptype][prot][i][0]) / (2 * W);
            float y = ((float) py + block[ptype][prot][i][1]) / H;
            float x2 = x + 1.f / (2 * W);
            float y2 = y + 1.f / H;
            drawRect(x, y, x2, y2);
        }
        glColor3f(COLOR(backCol));
        drawRect(0.5, 0, 1, 1);

        glColor3f(COLOR(backgroundCol));
        float x = 0.5 + 2.5 / W;
        float y = 5.0 / H;
        float r = 3.0 / H;
        drawRect(x - r, y - r, x + r, y + r);
        
        glColor3f(COLOR(blockCol[nexttype]));
        for(int i = 0; i < 4; i++) {
            float X = x + ((float) block[nexttype][0][i][0]) / (2 * W);
            float Y = y + ((float) block[nexttype][0][i][1] - 1) / H;
            float x2 = X + 1.f / (2 * W);
            float y2 = Y + 1.f / H;
            drawRect(X, Y, x2, y2);
        }
    }
    glEnd();

    glFlush();
    glutSwapBuffers();
}

void init() {
	glClearColor(COLOR(backgroundCol), 1.f);
	glPointSize(6.0f);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, 1.0, 1.0, 0.0);
    for(int i = 0; i < W; i++) {
        for(int j = 0; j < H; j++) {
            board[i][j] = EMPTY;
        }
    }
    srand(time(NULL));
    ptype = rand() % 7;
    nexttype = rand() % 7;
    prot = 0;
    px = W / 2;
    py = 0;
}

void placepiece() {
    for(int i = 0; i < 4; i++) {
        int x = px + block[ptype][prot][i][0];
        int y = py + block[ptype][prot][i][1];
        board[x][y] = ptype;
    }
}

bool freespace(int x, int y) {
    if(x < 0 || x >= W || y >= H) return false;
    if(y < 0) return true;
    return board[x][y] == EMPTY;
}

void newpiece() {
    ptype = nexttype;
    nexttype = rand() % 7;
    prot = 0;
    px = W / 2;
    py = 0;
    for(int i = 0; i < 4; i++) {
        int x = px + block[ptype][prot][i][0];
        int y = py + block[ptype][prot][i][0];
        if(!freespace(x, y)) {
            init();
            break;
        }
    }
    std::fill(das, das + 256, DAS1);
    long long tm = std::chrono::system_clock::now().time_since_epoch().count();
    std::fill(presstime, presstime + 256, tm);
}

bool movepiece(int dx, int dy) {
    for(int i = 0; i < 4; i++) {
        int x = px + block[ptype][prot][i][0];
        int y = py + block[ptype][prot][i][1];
        if(!freespace(x + dx, y + dy)) return false;
    }
    px += dx;
    py += dy;
    return true;
}

// off = 1: CW. off = 3: CCW
bool rotatepiece(int off) {
    for(int i = 0; i < 4; i++) {
        int x = px + block[ptype][(prot + off) % 4][i][0];
        int y = py + block[ptype][(prot + off) % 4][i][1];
        if(!freespace(x, y)) return false;
    }
    prot = (prot + off) % 4;
    return true;
}

int clearlines() {
    int j = 0;
    for(int i = H - 1; i >= 0; i--) {
        int e = 0;
        for(int k = 0; k < W; k++) {
            if(board[k][i] == EMPTY) e++;
        }
        if(e == 0) j++;
        else for(int k = 0; k < W; k++) {
            board[k][i + j] = board[k][i];
        }
    }
    for(int i = 0; i < j; i++) {
        for(int k = 0; k < W; k++) {
            board[k][i] = EMPTY;
        }
    }
    return j;
}

// move piece down
void update() {
    if(!movepiece(0, 1)) {
        placepiece();
        clearlines();
        newpiece();
    }
    display();
}

long long start = 0;
void idle() {
	long long end = std::chrono::system_clock::now().time_since_epoch().count();
	if(end - start > 1000000000 / FPS) {
        update();
		start = end;
	}
    for(char key : keys) {
        if(end - presstime[key] > 1000000000 / das[key]) {
            key_press(key, 0, 0);
            das[key] = DAS2;
        }
    }
}

void updatereset() {
    start = std::chrono::system_clock::now().time_since_epoch().count();
    update();
}

void key_press(unsigned char key, int x, int y) {
    presstime[key] = std::chrono::system_clock::now().time_since_epoch().count();
    if(key == ' ') {
        while(movepiece(0, 1));
        updatereset();
    }else if(key == 'j') {
        movepiece(-1, 0);
        display();
        keys.insert(key);
        das[key] = DAS1;
    }else if(key == 'l') {
        movepiece(1, 0);
        display();
        keys.insert(key);
        das[key] = DAS1;
    }else if(key == 'm') {
        movepiece(0, 1);
        updatereset();
        keys.insert(key);
        das[key] = DAS1;
    }else if(key == 'i') {
        rotatepiece(1);
        display();
    }else if(key == 'k') {
        rotatepiece(3);
        display();
    }
}
void key_release(unsigned char key, int x, int y) {
    keys.erase(key);
}

int main(int argc, char **argv) {
    // window initialization
	glutInit(&argc, argv);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(30, 30);

	// create window
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutCreateWindow("Tetris");

	init();
	// glutMouseFunc(mouse_press);
	// glutPassiveMotionFunc(mouse_move);
	// glutMotionFunc(mouse_drag);
	glutKeyboardFunc(key_press);
	glutKeyboardUpFunc(key_release);
	// glutReshapeFunc(resize);
	glutIdleFunc(idle);

	glutDisplayFunc(display);

	glutMainLoop();

	return 0;
}