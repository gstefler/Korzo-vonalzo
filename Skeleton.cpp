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

// the window is 600px * 600px
// which is 10 * 10 cm
// convert cm to pixels
float cmToPixels(float cm)
{
	return cm * 60.0f;
}

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
	float size = cmToPixels(0.1f);

	bool isSelected = false;

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
			return isSelected ? WHITE : YELLOW;
		}
		return NOCOLOR;
	}
};


struct Circle
{
	vec2 center;
	float radius;

	float thickness = 3.0f;

	bool isSelected = false;

	Circle(vec2 center, float radius) : center(center), radius(radius) {}

	bool isInside(vec2 point, float clickThreshold = 0.0f)
	{
		return length(center - point) < radius + (thickness / 2) + clickThreshold && length(center - point) > radius - (thickness / 2 + clickThreshold);
	}

	vec4 getColorForPixel(vec2 point)
	{
		if (isInside(point))
		{
			return  isSelected ? WHITE :CYAN;
		}
		else
		{
			return NOCOLOR;
		}
	}
};

// infinite line with thickness
struct Line
{
	vec2 p1;
	vec2 p2;

	bool isSelected = false;

	float thickness = 2.0f;

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
			return isSelected ? WHITE :RED;
		}
		else
		{
			return NOCOLOR;
		}
	}
};

// get intersection point of two lines
vec2 getIntersectionPoint(Line line1, Line line2)
{
	vec2 p1 = line1.p1;
	vec2 p2 = line1.p2;
	vec2 p3 = line2.p1;
	vec2 p4 = line2.p2;

	float x1 = p1.x;
	float y1 = p1.y;
	float x2 = p2.x;
	float y2 = p2.y;
	float x3 = p3.x;
	float y3 = p3.y;
	float x4 = p4.x;
	float y4 = p4.y;

	float d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
	if (areEqual(d, 0.0f))
	{
		return vec2(0.0f);
	}

	float xi = ((x3 - x4) * (x1 * y2 - y1 * x2) - (x1 - x2) * (x3 * y4 - y3 * x4)) / d;
	float yi = ((y3 - y4) * (x1 * y2 - y1 * x2) - (y1 - y2) * (x3 * y4 - y3 * x4)) / d;

	return vec2(xi, yi);
}

// get intersection point of two circles
vec2* getIntersectionPoint(Circle circle1, Circle circle2)
{
	vec2 center1 = circle1.center;
	vec2 center2 = circle2.center;
	float radius1 = circle1.radius;
	float radius2 = circle2.radius;

	float distance = length(center1 - center2);
	if (distance > radius1 + radius2)
	{
		return nullptr;
	}
	else if (distance < fabs(radius1 - radius2))
	{
		return nullptr;
	}
	else if (distance == 0.0f && radius1 == radius2)
	{
		return nullptr;
	}

	float a = (pow(radius1, 2) - pow(radius2, 2) + pow(distance, 2)) / (2 * distance);
	float h = sqrt(pow(radius1, 2) - pow(a, 2));

	vec2 p2 = center1 + a * (center2 - center1) / distance;
	vec2 p1 = p2 + vec2(h * (center2.y - center1.y) / distance, -h * (center2.x - center1.x) / distance);
	vec2 p3 = p2 + vec2(-h * (center2.y - center1.y) / distance, h * (center2.x - center1.x) / distance);

	return new vec2[2]{p1, p3};
}


// get intersection point of a line and a circle
vec2 *getIntersectionPoint(Line line, Circle circle){
	// ez alapján sikerült: https://mathworld.wolfram.com/Circle-LineIntersection.html

	vec2 center = circle.center;
	float radius = circle.radius;

	vec2 p1 = line.p1;
	vec2 p2 = line.p2;

	vec2 v = p2 - p1;
	vec2 w = p1 - center;

	float a = dot(v, v);
	float b = 2 * dot(v, w);
	float c = dot(w, w) - pow(radius, 2);

	float d = pow(b, 2) - 4 * a * c;

	if (d < 0)
	{
		return nullptr;
	}

	// ha a 2 egyenlő akkor is egy 2 méretű tömb lesz, mert így egyszerűbb

	float t1 = (-b - sqrt(d)) / (2 * a);
	float t2 = (-b + sqrt(d)) / (2 * a);

	vec2 intersectionPoint1 = p1 + t1 * v;
	vec2 intersectionPoint2 = p1 + t2 * v;

	return new vec2[2]{intersectionPoint1, intersectionPoint2};
}



vec2 circleStart;
vec2 circleEnd;

vec2 lineStart;
vec2 lineEnd;

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
	points.push_back(Point(vec2((float)windowWidth / 2 + cmToPixels(1), (float)windowHeight / 2)));

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
		printf("s pressed\n");
		break;
	case 'c':
		isCpressed = true;
		printf("c pressed\n");
		break;
	case 'i':
		isIpressed = true;
		printf("i pressed\n");
		break;
	case 'l':
		isLpressed = true;
		printf("");
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
void onMouseMotion(int pX, int pY){
}

unsigned int getClickedCircle(int x, int y)
{
	for (unsigned int i = 0; i < circles.size(); i++)
	{
		if (circles[i].isInside(vec2(x, y), 3.0f))
		{
			return i;
		}
	}

	return -1;
}


unsigned int getClickedLine(int x, int y)
{
	for (unsigned int i = 0; i < lines.size(); i++)
	{
		if (lines[i].isInside(vec2(x, y), 4.0f))
		{
			return i;
		}
	}

	return -1;
}

unsigned int firstObjectIndex = -1;
unsigned int secondObjectIndex = -1;

enum ObjectType { LINE, CIRCLE, NONE};

ObjectType firstObjectType = NONE;

void visualizeIntersection(vec2* p)
{
	if (p != nullptr){
		for (int i = 0; i < 2; i++)
		{
			points.push_back(Point(p[i]));
		}
	}
}

// Mouse click event
void onMouse(int button, int state, int pX, int pY){
	pY = windowHeight - pY;
	vec2 click = vec2(pX, pY);

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		for (Point p : points)
		{
			if (p.isInside(click))
			{
				if (isSpressed)
				{
					if (!firstPointPlaced)
					{
						circleStart = p.position;
						firstPointPlaced = true;
						secondPointPlaced = false;
					}
					else if (!secondPointPlaced)
					{
						circleEnd = p.position;
						if (!areEqual(circleStart, circleEnd))
						{
							firstPointPlaced = false;
							secondPointPlaced = true;
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
					}
					else if (!secondPointPlaced)
					{
						lineEnd = p.position;
						if (!areEqual(lineStart, lineEnd))
						{
							firstPointPlaced = false;
							secondPointPlaced = true;

							lines.push_back(Line(lineStart, lineEnd));
						}
					}
				}
			}
		}
		
		if (isIpressed)
		{
			// select the first object
			if (!firstObjectSelected){
				unsigned int lineIndex = getClickedLine(pX, pY);
				if (lineIndex != -1){
					firstObjectIndex = lineIndex;
					firstObjectType = LINE;
					firstObjectSelected = true;
					secondObjectSelected = false;

					lines[firstObjectIndex].isSelected = true;
				}
				else{
					unsigned int circleIndex = getClickedCircle(pX, pY);
					if (circleIndex != -1){
						firstObjectIndex = circleIndex;
						firstObjectType = CIRCLE;
						firstObjectSelected = true;
						secondObjectSelected = false;

						circles[firstObjectIndex].isSelected = true;
					}
				}
			}
			// select the second object
			else if (!secondObjectSelected){
				switch (firstObjectType){
					case LINE:
					{
						unsigned int lineIndex = getClickedLine(pX, pY);
						if (lineIndex != -1 && lineIndex != firstObjectIndex){
							secondObjectIndex = lineIndex;
							secondObjectSelected = true;
							firstObjectSelected = false;

							lines[firstObjectIndex].isSelected = false;

							vec2 intersections = getIntersectionPoint(lines[firstObjectIndex], lines[secondObjectIndex]);
							points.push_back(Point(intersections));
						}
						else{
							unsigned int circleIndex = getClickedCircle(pX, pY);
							if (circleIndex != -1){
								secondObjectIndex = circleIndex;
								secondObjectSelected = true;
								firstObjectSelected = false;

								lines[firstObjectIndex].isSelected = false;
								
								vec2* intersections = getIntersectionPoint(lines[firstObjectIndex], circles[secondObjectIndex]);
								visualizeIntersection(intersections);
								delete[] intersections;
							}
						}
						break;
					}
					case CIRCLE:
					{
						unsigned int circleIndex = getClickedCircle(pX, pY);
						if (circleIndex != -1 && circleIndex != firstObjectIndex){
							secondObjectIndex = circleIndex;
							secondObjectSelected = true;
							firstObjectSelected = false;

							circles[firstObjectIndex].isSelected = false;
							
							vec2* intersections = getIntersectionPoint(circles[firstObjectIndex], circles[secondObjectIndex]);
							visualizeIntersection(intersections);
							delete[] intersections;
						}
						break;
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
