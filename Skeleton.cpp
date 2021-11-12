//=============================================================================================
// Mintaprogram: Zöld háromszög. Ervenyes 2019. osztol.
//
// A beadott program csak ebben a fajlban lehet, a fajl 1 byte-os ASCII karaktereket tartalmazhat, BOM kihuzando.
// Tilos:
// - mast "beincludolni", illetve mas konyvtarat hasznalni
// - faljmuveleteket vegezni a printf-et kiveve
// - Mashonnan atvett programresszleteket forrasmegjeloles nelkul felhasznalni es
// - felesleges programsorokat a beadott programban hagyni!!!!!!! 
// - felesleges kommenteket a beadott programba irni a forrasmegjelolest kommentjeit kiveve
// ---------------------------------------------------------------------------------------------
// A feladatot ANSI C++ nyelvu forditoprogrammal ellenorizzuk, a Visual Studio-hoz kepesti elteresekrol
// es a leggyakoribb hibakrol (pl. ideiglenes objektumot nem lehet referencia tipusnak ertekul adni)
// a hazibeado portal ad egy osszefoglalot.
// ---------------------------------------------------------------------------------------------
// A feladatmegoldasokban csak olyan OpenGL fuggvenyek hasznalhatok, amelyek az oran a feladatkiadasig elhangzottak 
// A keretben nem szereplo GLUT fuggvenyek tiltottak.
//
// NYILATKOZAT
// ---------------------------------------------------------------------------------------------
// Nev    : Stefler Gábor
// Neptun : KTH674
// ---------------------------------------------------------------------------------------------
// ezennel kijelentem, hogy a feladatot magam keszitettem, es ha barmilyen segitseget igenybe vettem vagy
// mas szellemi termeket felhasznaltam, akkor a forrast es az atvett reszt kommentekben egyertelmuen jeloltem.
// A forrasmegjeloles kotelme vonatkozik az eloadas foliakat es a targy oktatoi, illetve a
// grafhazi doktor tanacsait kiveve barmilyen csatornan (szoban, irasban, Interneten, stb.) erkezo minden egyeb
// informaciora (keplet, program, algoritmus, stb.). Kijelentem, hogy a forrasmegjelolessel atvett reszeket is ertem,
// azok helyessegere matematikai bizonyitast tudok adni. Tisztaban vagyok azzal, hogy az atvett reszek nem szamitanak
// a sajat kontribucioba, igy a feladat elfogadasarol a tobbi resz mennyisege es minosege alapjan szuletik dontes.
// Tudomasul veszem, hogy a forrasmegjeloles kotelmenek megsertese eseten a hazifeladatra adhato pontokat
// negativ elojellel szamoljak el es ezzel parhuzamosan eljaras is indul velem szemben.
//=============================================================================================
#include "framework.h"

const char * const vertexSource = R"(
	#version 330
	precision highp float;

	layout(location = 0) in vec2 vp;

    out vec2 texCoord;

	void main() {
		gl_Position = vec4(vp.x, vp.y, 0, 1);
        texCoord = vp * 0.5 + 0.5;
	}
)";


const char * const fragmentSource = R"(
	#version 330
	precision highp float;

    in vec2 texCoord;

    uniform sampler2D tex;

    out vec4 outColor;

	void main() {
		outColor = texture(tex, texCoord);
	}
)";

struct Cube{
	vec2 pos;
	float size;

	Cube(vec2 pos, float size) : pos(pos), size(size) {}

	bool isInside(vec2 point){
		return (point.x >= pos.x - size && point.x <= pos.x + size && point.y >= pos.y - size && point.y <= pos.y + size);
	}

	vec4 getColor(vec2 point) {
		if (isInside(point)) {
			return vec4(1.0f, 1.0f, 0.0f, 1.0f);
		}
		return vec4(0.0f, 0.0f, 0.0f, 1.0f);
	}
};


struct Circle{
	vec2 center;
	float radius;
	float thickness = 4.0f;

	Circle(vec2 center, float radius) : center(center), radius(radius) {}

	bool isInside(vec2 point) {
		return length(center - point) < radius && length(center - point) > radius - thickness;
	}

	vec4 getColor(vec2 point, vec4 color = vec4(1.0f, 0.0f, 0.0f, 1.0f)) {
		if (isInside(point)) {
			return color;
		}
		else {
			return vec4(0.0f, 0.0f, 0.0f, 1.0f);
		}
	}

	// retuns 2 points that are the intersection of an other circle
	std::vector<vec2> getIntersectionPoints(Circle other) {
		std::vector<vec2> points;
		float d = length(center - other.center); 
		float a = (radius + other.radius) * (radius + other.radius);
		float b = 2 * (other.radius * (other.radius - radius) + radius * (radius - other.radius));
		float c = (radius - other.radius) * (radius - other.radius);
		float x1 = (d * d + a - b) / (2 * d);
		float x2 = (d * d - a + b) / (2 * d);
		float y1 = sqrt(c - x1 * x1);
		float y2 = sqrt(c - x2 * x2);
		points.push_back(vec2(x1, y1) + center);
		points.push_back(vec2(x2, y2) + center);
		return points;
	}
};

struct Line{
	vec2 start;
	vec2 end;
	float thickness = 4.0f;

	Line(vec2 start, vec2 end) : start(start), end(end) {}

	bool isInside(vec2 point) {
		vec2 v = end - start;
		vec2 w = point - start;
		float c1 = dot(w, v);
		if (c1 <= 0) return true;
		float c2 = dot(v, v);
		if (c2 <= c1) return true;
		return false;
	}

	vec4 getColor(vec2 point, vec4 color = vec4(1.0f, 0.0f, 0.0f, 1.0f)) {
		if (isInside(point)) {
			return color;
		}
		else {
			return vec4(0.0f, 0.0f, 0.0f, 1.0f);
		}
	}
};


vec2 start;
vec2 end;


bool isSpressed = false;
bool isCpressed = false;
bool isIpressed = false;
bool isLpressed = false;

bool isMousePressed = false;

GPUProgram gpuProgram;
unsigned int vao;

// ebben tároljuk a megjelenitendo objektumokat
Texture texture;

std::vector<Circle> circles;
std::vector<Line> lines;

void onInitialization() {
	glViewport(0, 0, windowWidth, windowHeight);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	unsigned int vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // a 2ds lap amire rajzolunk
	float vertices[] = { -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f };

    glBufferData(GL_ARRAY_BUFFER,
		sizeof(vertices),
		vertices,
		GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,
		2, GL_FLOAT, GL_FALSE,
		0, NULL);

	// create program for the GPU
	gpuProgram.create(vertexSource, fragmentSource, "outColor");
}


void onDisplay() {
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	int location = glGetUniformLocation(gpuProgram.getId(), "color");

    std::vector<vec4> pixels;


	for (int y = 0; y < windowHeight; y++) {
		for (int x = 0; x < windowWidth; x++) {
			
		}
	}

			

    texture.create(windowWidth, windowHeight, pixels);
    gpuProgram.setUniform(texture, "tex");

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glutSwapBuffers();
}


// Key of ASCII code pressed
void onKeyboard(unsigned char key, int pX, int pY) {
	switch (key) {
	case 's':
		isSpressed = true;
		break;
	case 'c':
		isCpressed = true;
		break;
	case 'i':
		isIpressed = true;
		break;
	case 'l':
		isLpressed = true;
		break;
	}
}


// Key of ASCII code released
void onKeyboardUp(unsigned char key, int pX, int pY) {
	switch (key) {
	case 's':
		isSpressed = false;
		break;
	case 'c':
		isCpressed = false;
		break;
	case 'i':
		isIpressed = false;
		break;
	case 'l':
		isLpressed = false;
		break;
	}
}


// Move mouse with key pressed
void onMouseMotion(int pX, int pY) {	// pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system

}


// Mouse click event
void onMouse(int button, int state, int pX, int pY) { // pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
    pY = windowHeight - pY;
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		isMousePressed = true;
		start = vec2(pX, pY);
	}

	

}


// Idle event indicating that some time elapsed: do animation here
void onIdle() {
}
