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

const char *const vertexSource = R"(
	#version 330
	precision highp float;

	layout(location = 0) in vec2 vp;

    out vec2 texCoord;

	void main() {
		gl_Position = vec4(vp.x, vp.y, 0, 1);
        texCoord = vp * 0.5 + 0.5;
	}
)";

const char *const fragmentSource = R"(
	#version 330
	precision highp float;

    in vec2 texCoord;

    uniform sampler2D tex;

    out vec4 outColor;

	void main() {
		outColor = texture(tex, texCoord);
	}
)";

bool areEqual(float a, float b)
{
	return fabs(a - b) < 0.1f;
}

bool areEqual(const vec2 &a, const vec2 &b)
{
	return areEqual(a.x, b.x) && areEqual(a.y, b.y);
}

static const vec4 CYAN = vec4(0.0f, 1.0f, 1.0f, 1.0f);
static const vec4 RED = vec4(1.0f, 0.0f, 0.0f, 1.0f);
static const vec4 YELLOW = vec4(1.0f, 1.0f, 0.0f, 1.0f);
static const vec4 WHITE = vec4(1.0f, 1.0f, 1.0f, 1.0f);
static const vec4 NOCOLOR = vec4(0.0f, 0.0f, 0.0f, 0.0f);

// represents a point as a square
struct Point
{
	vec2 position;
	float size = 6.0f;

	Point(vec2 position) : position(position) {}

	// the point acts as a square
	bool isInside(vec2 point, float clickTreshold = 0.0f)
	{
		return point.x > position.x - size - clickTreshold &&
			   point.x < position.x + size + clickTreshold &&
			   point.y > position.y - size - clickTreshold &&
			   point.y < position.y + size + clickTreshold;
	}

	vec4 getColorForPixel(vec2 point)
	{
		if (isInside(point))
		{
			return YELLOW;
		}
		return NOCOLOR;
	}
};

class Object
{
public:
	virtual bool isInside(vec2, float) = 0;
	virtual vec4 getColorForPixel(vec2) = 0;
	virtual std::string getType() = 0;
};

struct Circle : Object
{
	vec2 center;
	float radius;

	float thickness = 3.0f;
	float clickThreshold = 0.5f;

	// get type of the object
	std::string getType() override
	{
		return "Circle";
	}

	Circle(vec2 center, float radius) : center(center), radius(radius) {}

	bool isInside(vec2 point, float clickThreshold = 0.0f)
	{
		return length(center - point) < radius + clickThreshold && length(center - point) > radius - (thickness + clickThreshold);
	}

	vec4 getColorForPixel(vec2 point)
	{
		if (isInside(point))
		{
			return CYAN;
		}
		else
		{
			return NOCOLOR;
		}
	}

	// retuns 2 points that are the intersection of an other circle
	std::vector<vec2> getIntersectionPoints(Circle other)
	{
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

// infinite line with thickness
struct Line : Object
{
	vec2 p1;
	vec2 p2;

	float thickness = 2.0f;

	// get type of the object
	std::string getType() override
	{
		return "Line";
	}

	Line(vec2 p1, vec2 p2) : p1(p1), p2(p2) {}

	float distanceFromPoint(vec2 point)
	{
		vec2 v = p2 - p1;
		vec2 w = point - p1;
		float c1 = dot(w, v);
		float c2 = dot(v, v);
		float b = c1 / c2;
		vec2 Pb = p1 + b * v;
		return length(Pb - point);
	}

	bool isInside(vec2 point, float clickThreshold = 0.0f)
	{
		return distanceFromPoint(point) < thickness + clickThreshold;
	}

	vec4 getColorForPixel(vec2 point)
	{
		// if we draw the click treshold is 0
		if (isInside(point))
		{
			return RED;
		}
		else
		{
			return NOCOLOR;
		}
	}
};

vec2 circleStart;
vec2 circleEnd;

vec2 lineStart;
vec2 lineEnd;

Object *firstObject = nullptr;
Object *secondObject = nullptr;

bool firstObjectSelected = false;
bool secondObjectSelected = false;

bool isSpressed = false;
bool isCpressed = false;
bool isIpressed = false;
bool isLpressed = false;

bool firstPointPlaced = false;
bool secondPointPlaced = false;

bool isMousePressed = false;

GPUProgram gpuProgram;
unsigned int vao;

Texture texture;

std::vector<Circle> circles;
std::vector<Line> lines;
std::vector<Point> points;

void onInitialization()
{
	// alap pontok es egyenes beallitasa
	points.push_back(Point(vec2((float)windowWidth / 2, (float)windowHeight / 2)));
	points.push_back(Point(vec2((float)windowWidth / 2 + 60.0f, (float)windowHeight / 2)));
	points.push_back(Point(vec2(60.0f, 55.0f)));

	lines.push_back(Line(points[0].position, points[1].position));

	glViewport(0, 0, windowWidth, windowHeight);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	unsigned int vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// a 2ds lap amire rajzolunk
	float vertices[] = {-1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f};

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

void onDisplay()
{
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	int location = glGetUniformLocation(gpuProgram.getId(), "color");

	std::vector<vec4> pixels;

	for (int y = 0; y < windowHeight; y++)
	{
		for (int x = 0; x < windowWidth; x++)
		{

			vec4 color = vec4(0.0f, 0.0f, 0.0f, 0.0f);

			vec2 pixel = vec2(x, y);

			for (Circle circle : circles)
			{
				if (circle.isInside(pixel))
				{
					color = circle.getColorForPixel(pixel);
					break;
				}
			}

			for (Line line : lines)
			{
				if (line.isInside(pixel))
				{
					color = line.getColorForPixel(pixel);
					break;
				}
			}

			for (Point point : points)
			{
				if (point.isInside(pixel))
				{
					color = point.getColorForPixel(pixel);
					break;
				}
			}

			pixels.push_back(color);
		}
	}

	texture.create(windowWidth, windowHeight, pixels);
	gpuProgram.setUniform(texture, "tex");

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glutSwapBuffers();
}

// Key of ASCII code pressed
void onKeyboard(unsigned char key, int pX, int pY)
{
	switch (key)
	{
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
void onKeyboardUp(unsigned char key, int pX, int pY)
{
	switch (key)
	{
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
void onMouseMotion(int pX, int pY)
{ // pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
}



// Mouse click event
void onMouse(int button, int state, int pX, int pY)
{ // pX, pY are the pixel coordinates of the cursor in the coordinate system of the operation system
	pY = windowHeight - pY;
	vec2 click = vec2(pX, pY);
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		for (Point p : points)
		{
			if (p.isInside(click, 5.0f))
			{
				if (isSpressed)
				{
					if (!firstPointPlaced)
					{
						circleStart = p.position;
						firstPointPlaced = true;
						secondPointPlaced = false;
						printf("First point placed: %f, %f\n", circleStart.x, circleStart.y);
					}
					else if (!secondPointPlaced)
					{
						circleEnd = p.position;
						if (!areEqual(circleStart, circleEnd))
						{
							firstPointPlaced = false;
							secondPointPlaced = true;
							printf("Second point placed: %f, %f\n", circleEnd.x, circleEnd.y);
						}
					}
				}
				else if (isCpressed)
				{
					if (secondPointPlaced)
					{
						circles.push_back(Circle(p.position, length(circleStart - circleEnd)));
					}
				}
				else if (isLpressed)
				{
					if (!firstPointPlaced)
					{
						lineStart = p.position;
						firstPointPlaced = true;
						secondPointPlaced = false;
						printf("First point for line placed: %f, %f\n", lineStart.x, lineStart.y);
					}
					else if (!secondPointPlaced)
					{
						lineEnd = p.position;
						if (!areEqual(lineStart, lineEnd))
						{
							firstPointPlaced = false;
							secondPointPlaced = true;
							printf("Second point for line placed: %f, %f\n", lineEnd.x, lineEnd.y);
							lines.push_back(Line(lineStart, lineEnd));
						}
					}
				}
			}
		}
	}
}

// Idle event indicating that some time elapsed: do animation here
void onIdle()
{
}
