#define GLFW_INCLUDE_GLU

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <time.h>

#define MAPSIZE 128
#define GRIDSIZE 64
#define MAPSCALE 100
#define CELLSIZE 6
#define DISPSIZE 256

#define COLTEXWIDTH 256
#define COLTEXHEIGHT 1152

using namespace glm;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void error_callback(int error, const char* description);
void size_callback(GLFWwindow* window, int key, int val);
void init(int width, int height);
void render();
void makeVertexBuffer();
bool initShader(const GLchar* vertex, const GLchar* pixel);

GLushort heights[MAPSIZE][MAPSIZE];
unsigned char disp[DISPSIZE][DISPSIZE];
GLuint vb = 0;
GLuint cb = 0;
GLuint ib = 0;

GLuint vshader = 0;
GLuint pshader = 0;
GLuint sprogram = 0;

GLuint heighttex = 0;
GLuint disptex = 0;
GLuint hcoltex = 0;

float posx = 14.0f * CELLSIZE;
float posy = 18.0f * CELLSIZE;
float rot = 0.0f;

float intensity = 20.0f;
vec2 off(0, 0);
vec2 mapoffset(0, 0);

GLFWwindow* win = 0;

float delta = 0;
float freq = 0.0f;
float frames = 0, secs = 0;

bool left = false, right = false, up = false, down = false;

FILE* logger = 0;

struct Color4
{
	unsigned char r, g, b, a;
	Color4()
	{
		this->r = 0;
		this->g = 0;
		this->b = 0;
		this->a = 0;
	}

	Color4(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255)
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}

	Color4(unsigned int val)
	{
		unsigned char v1 = val & 0x0000FFFF;
		unsigned char v2 = (val >> 8) & 0x0000FFFF;
		unsigned char v3 = (val >> 16) & 0x0000FFFF;
		unsigned char v4 = (val >> 24) & 0x0000FFFF;
		this->r = v1;
		this->g = v2;
		this->b = v3;
		this->a = v4;
	}
};

Color4 heightcolor[COLTEXHEIGHT][COLTEXWIDTH];

bool loadPalette(char* palpath, char* bigfile)
{
	memset(heightcolor, 0, COLTEXHEIGHT * COLTEXWIDTH * 4);

	FILE* file = fopen(palpath, "rb");

	if (file == 0)
	{
		return false;
	}

	Color4 paldata[16 * 16];
	size_t read = fread((void*) paldata, 1, 256 * 4, file);

	fclose(file);

	if (read != 256 * 4)
	{
		return false;
	}

	//open big file
	file = fopen(bigfile, "rb");

	if (file == 0)
	{
		return false;
	}

	unsigned char* data = new unsigned char[COLTEXHEIGHT * COLTEXWIDTH];
	read = fread((void*) data, 1, COLTEXHEIGHT * COLTEXWIDTH, file);

	fclose(file);

	if (read != COLTEXHEIGHT * COLTEXWIDTH)
	{
		delete [] data;
		return false;
	}

	for (size_t y = 0; y < COLTEXHEIGHT; y++)
	{
		for (size_t x = 0; x < COLTEXWIDTH; x++)
		{
			Color4 c = paldata[data[y * COLTEXWIDTH + x]];
			c.a = 255;
			heightcolor[y][x] = c;
		}
	}


	/*for (size_t y = 0; y < COLTEXHEIGHT; y++)
	{
		unsigned char fval = data[y * COLTEXWIDTH + 1];
		for (size_t x = 0; x < COLTEXWIDTH; x++)
		{
			data[y * COLTEXWIDTH + x] = fval;
		}
	}

	file = fopen("big0.dat", "wb");
	fwrite ((void*) data, 1, COLTEXHEIGHT * COLTEXWIDTH, file);
	fclose(file);*/

	delete [] data;
	return true;
}

bool loadDisp(char* path)
{
	memset(disp, 0, DISPSIZE * DISPSIZE * sizeof(unsigned char));
	FILE* file = fopen(path, "rb");

	if (file == 0)
	{
		return false;
	}

	unsigned char data[DISPSIZE * DISPSIZE];

	size_t read = fread((void*) data, 1, DISPSIZE * DISPSIZE, file);

	fclose(file);

	if (read != DISPSIZE * DISPSIZE)
	{
		return false;
	}

	for (size_t y = 0; y < DISPSIZE; y++)
	{
		for (size_t x = 0; x < DISPSIZE; x++)
		{
			disp[y][x] = data[y * DISPSIZE + x];
		}
	}

	return true;
}

bool loadMap(char* path)
{
	memset(heights, 0, MAPSIZE * MAPSIZE * sizeof(GLushort));
	FILE* file = fopen(path, "rb");

	if (file == 0)
	{
		return false;
	}

	unsigned short data[MAPSIZE * MAPSIZE];

	size_t read = fread((void*) data, 1, MAPSIZE * MAPSIZE * sizeof(unsigned short), file);

	fclose(file);

	if (read != MAPSIZE * MAPSIZE * sizeof(unsigned short))
	{
		return false;
	}

	for (size_t y = 0; y < MAPSIZE; y++)
	{
		for (size_t x = 0; x < MAPSIZE; x++)
		{
			heights[x][y] = data[y * MAPSIZE + x];
		}		
	}

	return true;
}

void releaseShader();
void releaseBuffer();

int main()
{
	GLFWwindow* window;

	if (!glfwInit())
		return -1;

	glfwSetErrorCallback(error_callback);

	window = glfwCreateWindow(1280, 720, "Map View", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	win = window;

	glfwSetKeyCallback(window, key_callback);
	glfwSetWindowSizeCallback(window, size_callback);
	glfwMakeContextCurrent(window);
	init(1280, 720);

	logger = fopen("log.txt", "w");

	fprintf(logger, "\n[Error Log]\n");

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		fprintf(logger, "Error: %s\n", glewGetErrorString(err));
	}

	if (!loadPalette("pal0-h.dat", "bigf0-h.dat")) fprintf(logger, "\ncould not load pal and big files..");
	if (!loadMap("levl2131.dat"))  fprintf(logger, "\ncould not load map file..");
	if (!loadDisp("disp0-h.dat"))  fprintf(logger, "\ncould not load disp file..");

	makeVertexBuffer();
	initShader(
		"attribute vec3 position;"
		"varying vec3 fposition;"
		"varying vec3 fnormal;"
		"uniform float cellsize;"
		"uniform float mapsize;"
		"uniform float freq;"
		"uniform vec2 mapoffset;"
		"uniform vec2 voffset;"
		"uniform mat4 modelview;"
		"uniform mat4 projection;"
		//"varying vec4 fcolor;"
		"uniform sampler2D heightmap;"
		"uniform sampler2D colormap;"
		"void main() {"
		"vec3 pos = position;"
		"pos.x = pos.x + mapoffset.x * cellsize;"
		"pos.z = pos.z + mapoffset.y * cellsize;"
		"vec3 tpos = pos;"

		"float add = (sin((pos.x - 32.0 * cellsize) * 1.0 + freq * 2.0) * 0.02 * cellsize) - (sin((pos.z - 32.0 * cellsize) * 1.0 + freq * 2.0) * 0.01 * cellsize);"

		"pos.x = (pos.x / cellsize) / mapsize;"
		"pos.z = (pos.z / cellsize) / mapsize;"

		"vec3 posl = tpos;"
		"posl.x = ((posl.x / cellsize) - 1.0) / mapsize;"
		"posl.z = ((posl.z / cellsize)) / mapsize;"

		"vec3 posr = tpos;"
		"posr.x = ((posr.x / cellsize) + 1.0) / mapsize;"
		"posr.z = ((posr.z / cellsize)) / mapsize;"

		"vec3 post = tpos;"
		"post.x = ((post.x / cellsize)) / mapsize;"
		"post.z = ((post.z / cellsize) - 1.0) / mapsize;"

		"vec3 posb = tpos;"
		"posb.x = ((posb.x / cellsize)) / mapsize;"
		"posb.z = ((posb.z / cellsize) + 1.0) / mapsize;"

		"vec4 tc = texture2D(heightmap, posl.xz);"
		"float g = tc.g * 255.0;"
		"float r = tc.r * 255.0;"
		"float hl = ((g * 256.0 + r) / 65535.0) * 160.0 * cellsize;"

		"tc = texture2D(heightmap, posr.xz);"
		"g = tc.g * 255.0;"
		"r = tc.r * 255.0;"
		"float hr = ((g * 256.0 + r) / 65535.0) * 160.0 * cellsize;"

		"tc = texture2D(heightmap, post.xz);"
		"g = tc.g * 255.0;"
		"r = tc.r * 255.0;"
		"float ht = ((g * 256.0 + r) / 65535.0) * 160.0 * cellsize;"

		"tc = texture2D(heightmap, posb.xz);"
		"g = tc.g * 255.0;"
		"r = tc.r * 255.0;"
		"float hb = ((g * 256.0 + r) / 65535.0) * 160.0 * cellsize;"

		"tc = texture2D(heightmap, pos.xz);"
		"pos = position;"
		"pos.x = pos.x + mapoffset.x * cellsize;"
		"pos.z = pos.z + mapoffset.y * cellsize;"
		"g = tc.g * 255.0;"
		"r = tc.r * 255.0;"
		"float h = ((g * 256.0 + r) / 65535.0) * 160.0 * cellsize;"

		"if( h == 0.0 ) {"
		"hl -= add;"
		"hr -= add;"
		"ht -= add;"
		"hb -= add;"
		"}"

		"vec3 n;"
		"n.x = hl - hr;"
		"n.y = hb - ht;"
		"n.z = 2.0;"
		"fnormal = normalize(n);"

		"float curve = (cos((position.x - voffset.x - 32.0 * cellsize) / (22.5 * cellsize)) + cos((position.z - voffset.y - 32.0 * cellsize) / (22.5 * cellsize))) *  6.0 * cellsize;"
		"if( h == 0.0 ) {"
		"curve -= add;"
		"h -= add;"
		"}"
		"fposition = vec3(position.x + mapoffset.x * cellsize, h, position.z + mapoffset.y * cellsize);"
		"h = h + curve;"
		"pos.y = h;"
		"pos.x = position.x;"
		"pos.z = position.z;"
		"gl_Position = projection * modelview * vec4(pos, 1.0);"
		"}"
		,
		"varying vec3 fposition;"
		"varying vec3 fnormal;"
		//"varying vec4 fcolor;"
		"uniform sampler2D heightmap;"
		"uniform sampler2D dispmap;"
		"uniform sampler2D colormap;"
		"uniform float cellsize;"
		"void main() {"
		"vec3 ldir = normalize(vec3(1.0, 0.2, 0.6));"
		"float intensity = dot(fnormal, ldir);"
		"vec2 tc;"
		"tc.x = fposition.x / (4.0 * cellsize);"
		"tc.y = fposition.z / (4.0 * cellsize);"
		"vec4 col = texture2D(dispmap, tc);"
		"vec2 tc2;"
		"tc2.x = 1.0 - intensity;"
		"tc2.y = fposition.y / (2.3 * cellsize) + (col.r / 8.8);"
		"vec4 hcol = texture2D(colormap, tc2);"
		"gl_FragColor = hcol;"
		/*"gl_FragColor = vec4(hcol.r, hcol.g, col.b, 1);"*/
		"}"
		);

	clock_t start = clock();

	while (!glfwWindowShouldClose(window))
	{
		clock_t end = clock();
		delta = ((float) (end - start)) / CLOCKS_PER_SEC;
		start = end;

		render();
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	releaseShader();
	releaseBuffer();
	
	glfwTerminate();

	fclose(logger);
	return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_W)
	{
		posy += 1.0f;
	}
	else if (key == GLFW_KEY_S)
	{
		posy -= 1.0f;
	}
	else if (key == GLFW_KEY_Q)
	{
		posx -= 1.0f;
	}
	else if (key == GLFW_KEY_A)
	{
		posx += 1.0f;
	}
	else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
	{
		//rot -= 2.0f;
		left = true;
	}
	else if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE)
	{
		//rot -= 2.0f;
		left = false;
	}
	else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
	{
		right = true;
		//rot += 2.0f;
	}
	else if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE)
	{
		right = false;
		//rot += 2.0f;
	}
	else if (key == GLFW_KEY_RIGHT_SHIFT && action == GLFW_PRESS)
	{
		intensity *= 4;
		//rot += 2.0f;
	}
	else if (key == GLFW_KEY_RIGHT_SHIFT && action == GLFW_RELEASE)
	{
		intensity /= 4;
		//rot += 2.0f;
	}
	else if (key == GLFW_KEY_UP && action == GLFW_PRESS)
	{
		up = true;
		/*vec2 dir(cos((pi<float>() / 180.0f) * rot), sin((pi<float>() / 180.0f) * rot));
		dir = normalize(dir);
		off.x -= dir.x * intensity;
		off.y -= -dir.y * intensity;*/
	}
	else if (key == GLFW_KEY_UP && action == GLFW_RELEASE)
	{
		up = false;
	}
	else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
	{
		down = true;
		/*vec2 dir(cos((pi<float>() / 180.0f) * rot), sin((pi<float>() / 180.0f) * rot));
		dir = normalize(dir);
		off.x += dir.x * intensity;
		off.y += -dir.y * intensity;*/
	}
	else if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE)
	{
		down = false;
	}

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}		
}

void size_callback(GLFWwindow* window, int key, int val)
{
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	init(width, height);
}

void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}

void init(int width, int height)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();	
	
	/*vec3 eye(0, 0.0f, 50.0f), target(0, 0, 0), up(0, 1.0f, 0);
	mat4x4 view = glm::lookAt(eye, target, up);*/

	gluLookAt(0, 30.0, 50.0, 0, 0, 0, 0, 1, 0);

	/*glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(glm::value_ptr(view));*/

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (float) width / (float) height, 1.0f, 1000.f);
	/*mat4x4 persp = perspective(45.0f, (float) width / (float) height, 1.0f, 1000.0f);
	glLoadMatrixf(glm::value_ptr(persp));*/
	glMatrixMode(GL_MODELVIEW);

	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_POINT_SMOOTH);
	glCullFace(GL_BACK);
	glShadeModel(GL_SMOOTH);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void releaseShader()
{
	if (sprogram)
	{
		glDeleteProgram(sprogram);
		sprogram = 0;
	}

	if (vshader)
	{
		glDeleteProgram(vshader);
		vshader = 0;
	}

	if (pshader)
	{
		glDeleteProgram(pshader);
		pshader = 0;
	}
}

bool initShader(const GLchar* vertex, const GLchar* pixel)
{
	releaseShader();

	vshader = glCreateShader(GL_VERTEX_SHADER);
	pshader = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vshader, 1, &vertex, NULL);
	glCompileShader(vshader);

	GLint result = GL_FALSE;
	GLint infolength;
	glGetShaderiv(vshader, GL_COMPILE_STATUS, &result);
	glGetShaderiv(vshader, GL_INFO_LOG_LENGTH, &infolength);
	GLchar* info = new GLchar[infolength];
	glGetShaderInfoLog(vshader, infolength, NULL, info);
	printf("vertex shader errors : \n%s", info);
	fprintf(logger, "\nvertex shader error %s", info);
	delete [] info;
	if (result == GL_FALSE) return false;


	glShaderSource(pshader, 1, &pixel, NULL);
	glCompileShader(pshader);

	result = GL_FALSE;
	infolength;
	glGetShaderiv(pshader, GL_COMPILE_STATUS, &result);
	glGetShaderiv(pshader, GL_INFO_LOG_LENGTH, &infolength);
	info = new GLchar[infolength];
	glGetShaderInfoLog(pshader, infolength, NULL, info);
	printf("pixel shader error : \n%s", info);
	fprintf(logger, "\npixel shader error %s", info);
	delete [] info;
	if (result == GL_FALSE) return false;

	sprogram = glCreateProgram();
	glAttachShader(sprogram, vshader);
	glAttachShader(sprogram, pshader);
	glLinkProgram(sprogram);

	result = GL_FALSE;
	infolength;
	glGetProgramiv(sprogram, GL_LINK_STATUS, &result);
	glGetProgramiv(sprogram, GL_INFO_LOG_LENGTH, &infolength);
	info = new GLchar[infolength];
	glGetProgramInfoLog(sprogram, infolength, NULL, info);
	printf("Link error : \n%s", info);
	fprintf(logger, "\nlink shader error %s", info);
	delete [] info;
	if (result == GL_FALSE) return false;
	return true;
}

void setAttribs()
{
	freq += delta;
	secs += delta;
	frames += 1;

	if (secs >= 1.0)
	{
		float fps = frames / secs;
		char tmp[256];
		sprintf(tmp, "FPS : %d", (int)fps);
		glfwSetWindowTitle(win, tmp);
		secs = 0;
		frames = 0;
	}

	if (freq >= pi<float>() * 4.0f)
	{
		freq -= pi<float>() * 4.0f;
	}

	if (left)
	{
		rot += 80.0f * delta;
	}
	else if (right)
	{
		rot -= 80.0f * delta;
	}
	
	if (up)
	{
		vec2 dir(cos((pi<float>() / 180.0f) * rot), sin((pi<float>() / 180.0f) * rot));
		dir = normalize(dir);
		off.x -= dir.x * intensity * delta;
		off.y -= -dir.y * intensity * delta;
	}
	else if (down)
	{
		vec2 dir(cos((pi<float>() / 180.0f) * rot), sin((pi<float>() / 180.0f) * rot));
		dir = normalize(dir);
		off.x += dir.x * intensity * delta;
		off.y += -dir.y * intensity * delta;
	}


	//

	GLuint pos = glGetAttribLocation(sprogram, "position");
	if (pos == -1) return;

	/*GLuint col = glGetAttribLocation(sprogram, "pcolor");
	if (col == -1) return;*/

	GLuint htex = glGetUniformLocation(sprogram, "heightmap");
	if (htex == -1) return;

	GLuint dtex = glGetUniformLocation(sprogram, "dispmap");
	if (dtex == -1) return;

	GLuint ctex = glGetUniformLocation(sprogram, "colormap");
	if (ctex == -1) return;

	GLuint msize = glGetUniformLocation(sprogram, "mapsize");
	if (msize == -1) return;

	GLuint csize = glGetUniformLocation(sprogram, "cellsize");
	if (csize == -1) return;

	GLuint moffset = glGetUniformLocation(sprogram, "mapoffset");
	if (moffset == -1) return;

	GLuint voffset = glGetUniformLocation(sprogram, "voffset");
	if (voffset == -1) return;

	GLuint freqp = glGetUniformLocation(sprogram, "freq");
	if (freqp == -1) return;

	glUniform1f(msize, GRIDSIZE);
	glUniform1f(csize, CELLSIZE);
	glUniform1f(freqp, freq);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (off.x >= CELLSIZE / 2.0f)
	{
		//off.x = 0;
		off.x = -CELLSIZE + off.x;
		mapoffset.x += 1;
	}
	if (off.x <= -CELLSIZE / 2.0f)
	{
		//off.x = 0;
		off.x = CELLSIZE + off.x;
		mapoffset.x -= 1;
	}
	if (off.y <= -CELLSIZE / 2.0f)
	{
		//off.y = 0;
		off.y = CELLSIZE + off.y;
		mapoffset.y -= 1;
	}
	if (off.y >= CELLSIZE / 2.0f)
	{
		//off.y = 0;
		off.y = -CELLSIZE + off.y;
		mapoffset.y += 1;
	}

	/*char msg[250];
	sprintf(msg, "pox : %f, posy : %f", posx, posy);
	glfwSetWindowTitle(win, msg);*/

	glUniform2f(moffset, mapoffset.x, mapoffset.y);
	glUniform2f(voffset, off.x, off.y);

	vec3 eye(0.0f, 0.0f, 0.0f), up(0, 1.0f, 0);
	mat4x4 eyetrans;

	mat4x4 tartrans;
	tartrans = translate(tartrans, vec3(off.x, 10.0f * CELLSIZE, off.y));
	tartrans = rotate(tartrans, rot, vec3(0.0f, 1.0f, 0.0f));
	vec4 tt = tartrans * vec4(0, 0, 0, 1.0f);

	eyetrans = translate(eyetrans, vec3(off.x, 0.0f, off.y));
	eyetrans = rotate(eyetrans, rot, vec3(0.0f, 1.0f, 0.0f));
	eyetrans = translate(eyetrans, vec3(posx, posy, 0.0f));

	vec4 eyet = eyetrans * vec4(eye, 1.0f);

	mat4x4 view = glm::lookAt(vec3(eyet.x, eyet.y, eyet.z), vec3(tt.x, tt.y, tt.z), up);
	mat4x4 model;
	model = translate(model, vec3(-GRIDSIZE * CELLSIZE / 2, 0, -GRIDSIZE * CELLSIZE / 2));
	mat4x4 modelview = view * model;
	glLoadMatrixf(value_ptr(modelview));

	glProgramUniform1i(sprogram, htex, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, heighttex);

	glProgramUniform1i(sprogram, dtex, 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, disptex);

	glProgramUniform1i(sprogram, ctex, 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, hcoltex);

	//glEnableClientState(GL_VERTEX_ARRAY);
	glEnableVertexAttribArray(pos);
	glBindBuffer(GL_ARRAY_BUFFER, vb);
	glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, 0);

	/*glEnableVertexAttribArray(col);
	glBindBuffer(GL_ARRAY_BUFFER, cb);
	glVertexAttribPointer(col, 3, GL_FLOAT, GL_FALSE, 0, 0);*/

	//glDrawArrays(GL_LINE_STRIP, 0, GRIDSIZE * GRIDSIZE);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
	glDrawElements(GL_TRIANGLE_STRIP, (2 * GRIDSIZE) * (GRIDSIZE - 1) + 2 * (GRIDSIZE - 2), GL_UNSIGNED_SHORT, 0);

	glDisableVertexAttribArray(pos);
	//glDisableVertexAttribArray(col);
}

void releaseBuffer()
{
	if (vb)
	{
		glDeleteBuffers(1, &vb);
		vb = 0;
	}

	if (ib)
	{
		glDeleteBuffers(1, &ib);
		ib = 0;
	}

	if (cb)
	{
		glDeleteBuffers(1, &cb);
		cb = 0;
	}

	if (heighttex)
	{
		glDeleteTextures(1, &heighttex);
		heighttex = 0;
	}

	if (disptex)
	{
		glDeleteTextures(1, &disptex);
		disptex = 0;
	}
}

void makeVertexBuffer()
{
	releaseBuffer();

	/*const GLfloat data [] = {
		-10, 0, 10,
		-10, 0, -10,
		10, 0, 10,
		10, 0, -10
	};*/

	/*const GLfloat colors [] = {
		1, 0, 0,
		0, 1, 0,
		0, 0, 1,
		1, 1, 1
	};*/

	/*const GLushort index [] = {
		0, 1, 2, 3
	};*/

	int numStripsRequired = GRIDSIZE - 1;
	int numDegensRequired = 2 * (numStripsRequired - 1);
	int verticesPerStrip = 2 * GRIDSIZE;

	size_t vsize = GRIDSIZE * GRIDSIZE * 3 * sizeof(GLfloat);

	GLfloat* vertex = new GLfloat[GRIDSIZE * GRIDSIZE * 3];
	memset(vertex, 0, vsize);

	GLfloat* colors = new GLfloat[GRIDSIZE * GRIDSIZE * 3];
	memset(colors, 0, vsize);

	size_t isize = ((2 * GRIDSIZE) * (GRIDSIZE - 1) + 2 * (GRIDSIZE - 2)) * sizeof(GLushort);
	GLushort* indices = new GLushort[(2 * GRIDSIZE) * (GRIDSIZE - 1) + 2 * (GRIDSIZE - 2)];
	memset(indices, 0, isize);

	int offset = 0;

	for (size_t y = 0; y < GRIDSIZE; y++)
	{
		if (y != 0 && y < GRIDSIZE - 1)
		{
			indices[offset++] = (y * GRIDSIZE);
		}

		for (size_t x = 0; x < GRIDSIZE; x++)
		{
			vertex[(y * GRIDSIZE + x) * 3 + 0] = (GLfloat) x * CELLSIZE;
			vertex[(y * GRIDSIZE + x) * 3 + 1] = 0;
			vertex[(y * GRIDSIZE + x) * 3 + 2] = (GLfloat) y * CELLSIZE;
			
			colors[(y * GRIDSIZE + x) * 3 + 0] = (GLfloat) x / GRIDSIZE;
			colors[(y * GRIDSIZE + x) * 3 + 1] = (GLfloat) y / GRIDSIZE;
			colors[(y * GRIDSIZE + x) * 3 + 2] = 1;

			if (y < GRIDSIZE - 1)
			{
				indices[offset++] = (GLushort) ((y * GRIDSIZE) + x);
				indices[offset++] = (GLushort) (((y + 1) * GRIDSIZE) + x);
			}
		}


		if (y < (GRIDSIZE - 2))
		{
			indices[offset++] = (GLushort) (((y + 1) * GRIDSIZE) + (GRIDSIZE - 1));
		}
	}

	glGenBuffers(1, &vb);
	glBindBuffer(GL_ARRAY_BUFFER, vb);
	glBufferData(GL_ARRAY_BUFFER, vsize, (void*) vertex, GL_STATIC_DRAW);

	glGenBuffers(1, &cb);
	glBindBuffer(GL_ARRAY_BUFFER, cb);
	glBufferData(GL_ARRAY_BUFFER, vsize, (void*) colors, GL_STATIC_DRAW);

	glGenBuffers(1, &ib);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, isize, (void*) indices, GL_STATIC_DRAW);

	delete [] vertex;
	delete [] colors;
	delete [] indices;

	glGenTextures(1, &heighttex);

	Color4* tex = new Color4[MAPSIZE * MAPSIZE];

	for (size_t y = 0; y < MAPSIZE; y++)
	{
		for (size_t x = 0; x < MAPSIZE; x++)
		{
			tex[y * MAPSIZE + x] = Color4(heights[y][x]);
		}
	}

	glBindTexture(GL_TEXTURE_2D, heighttex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, MAPSIZE, MAPSIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	delete [] tex;


	glGenTextures(1, &disptex);
	tex = new Color4[DISPSIZE * DISPSIZE];

	for (size_t y = 0; y < DISPSIZE; y++)
	{
		for (size_t x = 0; x < DISPSIZE; x++)
		{
			tex[y * DISPSIZE + x] = Color4((disp[y][x] + 128) % 256, (disp[y][x] + 128) % 256, (disp[y][x] + 128) % 256);
		}
	}

	glBindTexture(GL_TEXTURE_2D, disptex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, DISPSIZE, DISPSIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_NEAREST);
	delete [] tex;

	glGenTextures(1, &hcoltex);
	glBindTexture(GL_TEXTURE_2D, hcoltex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, COLTEXWIDTH, COLTEXHEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, heightcolor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void render()
{
	glClearColor(0, 0, 0, 1);
	glClearDepth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(sprogram);
	setAttribs();
	
	/*glBegin(GL_TRIANGLE_STRIP);
	glColor4f(1, 0, 0, 1);
	glVertex3f(-10, -10, 0);
	
	glColor4f(0, 1, 0, 1);
	glVertex3f(-10, 10, 0);
	
	glColor4f(0, 1, 0, 1);
	glVertex3f(10, -10, 0);
	
	glColor4f(0, 0, 0, 1);
	glVertex3f(10, 10, 0);*/
	
	//glEnd();
}