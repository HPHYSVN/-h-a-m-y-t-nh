/*Chương trình chiếu sáng Blinn-Phong (Phong sua doi) cho hình lập phương đơn vị, điều khiển quay bằng phím x, y, z, X, Y, Z.*/

#include "Angel.h"  /* Angel.h là file tự phát triển (tác giả Prof. Angel), có chứa cả khai báo includes glew và freeglut*/
#include <stb/stb_image.h>
#include <vector>
#include <cmath>
extern "C" {
	__declspec(dllexport) unsigned int NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
typedef vec4 point4;
typedef vec4 color4;
using namespace std;
// remember to prototype
void generateGeometry(void);
void initGPUBuffers(void);
void shaderSetup(void);

// Control Mode: 0=Camera, 1=Robot1, 2=Robot2, 3=Robot3
int controlMode = 0;

// Camera Globals
vec4 eye(0, 2.7, -7.5, 1);
vec4 at(0, 0, 0, 1);
vec4 up(0, 1, 0, 1);
GLfloat camSpeed = 0.2f; // Movement speed
GLfloat fovScale = 1.0f; // Zoom factor (Inverse of FOV: >1 means zoom out, <1 means zoom in)

// Lighting Globals
bool batDen = true; // Indoor Light
bool batMatTroi = true; // Sun Light

// Light 1: Sun (Directional, 7AM)
point4 light1_position(10.0, 5.0, 10.0, 0.0); // Directional
color4 light1_ambient(0.2, 0.2, 0.2, 1.0);
color4 light1_diffuse(1.0, 0.9, 0.7, 1.0);
color4 light1_specular(1.0, 0.9, 0.7, 1.0);

// Light 2: Indoor (Point)
point4 light2_position(0.0, 3.0, 0.0, 1.0); // Point inside
color4 light2_ambient(0.1, 0.1, 0.1, 1.0);
color4 light2_diffuse(0.8, 0.8, 0.8, 1.0);
color4 light2_specular(1.0, 1.0, 1.0, 1.0);

// Robot3 Extra Joint Angles (Whole Arm/Leg)
GLfloat r3_arm_left_shoulder_x = 0; // Swing
GLfloat r3_arm_left_shoulder_z = 0; // Lift
GLfloat r3_arm_right_shoulder_x = 0;
GLfloat r3_arm_right_shoulder_z = 0;
GLfloat r3_leg_left_hip_x = 0;
GLfloat r3_leg_left_hip_z = 0;
GLfloat r3_leg_right_hip_x = 0;
GLfloat r3_leg_right_hip_z = 0;


void display(void);
void keyboard(unsigned char key, int x, int y);
void initCylinder(int slices);
void setupCylinderBuffers(GLuint loc_vPosition, GLuint loc_vColor, GLuint loc_vNormal);
void drawCylinder(GLfloat RGB1, GLfloat RGB2, GLfloat RGB3, GLfloat x, GLfloat y, GLfloat z, GLfloat x1, GLfloat y1, GLfloat z1, mat4 m1, int slices);


// Số các đỉnh của các tam giác
const int NumPoints = 36;

point4 points[NumPoints]; /* Danh sách các đỉnh của các tam giác cần vẽ*/
color4 colors[NumPoints]; /* Danh sách các màu tương ứng cho các đỉnh trên*/
vec3 normals[NumPoints]; /*Danh sách các vector pháp tuyến ứng với mỗi đỉnh*/
std::vector<point4> cylinderPoints;
std::vector<color4> cylinderColors;
std::vector<vec3> cylinderNormals;
int cylinderVertexCount = 0;
int currentCylinderSlices = 0;

GLuint cubeVao = 0, cubeBuffer = 0;
GLuint cylinderVao = 0, cylinderBuffer = 0;

GLuint loc_vPosition = 0;
GLuint loc_vColor = 0;
GLuint loc_vNormal = 0;
point4 vertices[8]; /* Danh sách 8 đỉnh của hình lập phương*/
color4 vertex_colors[8]; /*Danh sách các màu tương ứng cho 8 đỉnh hình lập phương*/

GLuint program;

GLfloat theta[3] = { 0, 0, 0 };
GLfloat dr = 5;

GLfloat mauAnhSang = 0.8f;

mat4 model;
GLuint model_loc;
mat4 projection;
GLuint projection_loc;
mat4 view;
GLuint view_loc;

vec2 texCoords[] = {
	vec2(0.0, 0.0),
	vec2(1.0, 0.0),
	vec2(1.0, 1.0),

	vec2(0.0, 0.0),
	vec2(1.0, 1.0),
	vec2(0.0, 1.0)
};


void initCube()
{
	// Gán giá trị tọa độ vị trí cho các đỉnh của hình lập phương
	vertices[0] = point4(-0.5, -0.5, 0.5, 1.0);
	vertices[1] = point4(-0.5, 0.5, 0.5, 1.0);
	vertices[2] = point4(0.5, 0.5, 0.5, 1.0);
	vertices[3] = point4(0.5, -0.5, 0.5, 1.0);
	vertices[4] = point4(-0.5, -0.5, -0.5, 1.0);
	vertices[5] = point4(-0.5, 0.5, -0.5, 1.0);
	vertices[6] = point4(0.5, 0.5, -0.5, 1.0);
	vertices[7] = point4(0.5, -0.5, -0.5, 1.0);

	// Gán giá trị màu sắc cho các đỉnh của hình lập phương	
	vertex_colors[0] = color4(0.0, 0.0, 0.0, 1.0); // black
	vertex_colors[1] = color4(1.0, 0.0, 0.0, 1.0); // red
	vertex_colors[2] = color4(1.0, 1.0, 0.0, 1.0); // yellow
	vertex_colors[3] = color4(0.0, 1.0, 0.0, 1.0); // green
	vertex_colors[4] = color4(0.0, 0.0, 1.0, 1.0); // blue
	vertex_colors[5] = color4(1.0, 0.0, 1.0, 1.0); // magenta
	vertex_colors[6] = color4(1.0, 0.5, 0.0, 1.0); // orange
	vertex_colors[7] = color4(0.0, 1.0, 1.0, 1.0); // cyan
}
int Index = 0;
void quad(int a, int b, int c, int d)  /*Tạo một mặt hình lập phương = 2 tam giác, gán màu cho mỗi đỉnh tương ứng trong mảng colors*/
{
	vec4 u = vertices[b] - vertices[a];
	vec4 v = vertices[c] - vertices[b];
	vec3 normal = normalize(cross(u, v));

	normals[Index] = normal; colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
	normals[Index] = normal; colors[Index] = vertex_colors[a]; points[Index] = vertices[b]; Index++;
	normals[Index] = normal; colors[Index] = vertex_colors[a]; points[Index] = vertices[c]; Index++;
	normals[Index] = normal; colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
	normals[Index] = normal; colors[Index] = vertex_colors[a]; points[Index] = vertices[c]; Index++;
	normals[Index] = normal; colors[Index] = vertex_colors[a]; points[Index] = vertices[d]; Index++;
}
void makeColorCube(void)  /* Sinh ra 12 tam giác: 36 đỉnh, 36 màu*/

{
	quad(1, 0, 3, 2);
	quad(2, 3, 7, 6);
	quad(3, 0, 4, 7);
	quad(6, 5, 1, 2);
	quad(4, 5, 6, 7);
	quad(5, 4, 0, 1);
}
void initCylinder(int slices)
{
	// Sinh hình trụ kín hai đầu, câu chữ tách bạch hơn cho dễ đọc.
	if (slices < 3) slices = 3;

	const float radius = 0.5f;
	const float height = 1.0f;
	const float halfH = height * 0.5f;
	const float twoPi = 2.0f * static_cast<float>(M_PI);
	const color4 baseColor(0.7f, 0.7f, 0.9f, 1.0f);

	cylinderPoints.clear();
	cylinderColors.clear();
	cylinderNormals.clear();

	std::vector<point4> topRing(slices);
	std::vector<point4> bottomRing(slices);

	for (int i = 0; i < slices; ++i) {
		float th = twoPi * i / slices;
		float x = radius * cosf(th);
		float z = radius * sinf(th);
		topRing[i] = point4(x, halfH, z, 1.0f);
		bottomRing[i] = point4(x, -halfH, z, 1.0f);
	}

	// Mặt bên
	for (int i = 0; i < slices; ++i) {
		int ni = (i + 1) % slices;

		vec3 n1 = normalize(vec3(topRing[i].x, 0.0f, topRing[i].z));
		vec3 n2 = normalize(vec3(topRing[ni].x, 0.0f, topRing[ni].z));

		cylinderPoints.push_back(bottomRing[i]);  cylinderColors.push_back(baseColor); cylinderNormals.push_back(n1);
		cylinderPoints.push_back(bottomRing[ni]); cylinderColors.push_back(baseColor); cylinderNormals.push_back(n2);
		cylinderPoints.push_back(topRing[ni]);    cylinderColors.push_back(baseColor); cylinderNormals.push_back(n2);

		cylinderPoints.push_back(bottomRing[i]);  cylinderColors.push_back(baseColor); cylinderNormals.push_back(n1);
		cylinderPoints.push_back(topRing[ni]);    cylinderColors.push_back(baseColor); cylinderNormals.push_back(n2);
		cylinderPoints.push_back(topRing[i]);     cylinderColors.push_back(baseColor); cylinderNormals.push_back(n1);
	}

	// Nắp trên
	point4 topCenter = point4(0.0f, halfH, 0.0f, 1.0f);
	vec3 topNormal = vec3(0.0f, 1.0f, 0.0f);
	for (int i = 0; i < slices; ++i) {
		int ni = (i + 1) % slices;
		cylinderPoints.push_back(topCenter);     cylinderColors.push_back(baseColor); cylinderNormals.push_back(topNormal);
		cylinderPoints.push_back(topRing[i]);    cylinderColors.push_back(baseColor); cylinderNormals.push_back(topNormal);
		cylinderPoints.push_back(topRing[ni]);   cylinderColors.push_back(baseColor); cylinderNormals.push_back(topNormal);
	}

	// Nắp dưới
	point4 bottomCenter = point4(0.0f, -halfH, 0.0f, 1.0f);
	vec3 bottomNormal = vec3(0.0f, -1.0f, 0.0f);
	for (int i = 0; i < slices; ++i) {
		int ni = (i + 1) % slices;
		cylinderPoints.push_back(bottomCenter);   cylinderColors.push_back(baseColor); cylinderNormals.push_back(bottomNormal);
		cylinderPoints.push_back(bottomRing[ni]); cylinderColors.push_back(baseColor); cylinderNormals.push_back(bottomNormal);
		cylinderPoints.push_back(bottomRing[i]);  cylinderColors.push_back(baseColor); cylinderNormals.push_back(bottomNormal);
	}

	cylinderVertexCount = static_cast<int>(cylinderPoints.size());
}

void setupCylinderBuffers(GLuint loc_vPosition, GLuint loc_vColor, GLuint loc_vNormal)
{
	if (cylinderPoints.empty()) return;

	if (cylinderVao == 0) glGenVertexArrays(1, &cylinderVao);
	if (cylinderBuffer == 0) glGenBuffers(1, &cylinderBuffer);

	glBindVertexArray(cylinderVao);
	glBindBuffer(GL_ARRAY_BUFFER, cylinderBuffer);

	GLsizeiptr posSize = sizeof(point4) * cylinderPoints.size();
	GLsizeiptr colSize = sizeof(color4) * cylinderColors.size();
	GLsizeiptr normSize = sizeof(vec3) * cylinderNormals.size();

	glBufferData(GL_ARRAY_BUFFER, posSize + colSize + normSize, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, posSize, cylinderPoints.data());
	glBufferSubData(GL_ARRAY_BUFFER, posSize, colSize, cylinderColors.data());
	glBufferSubData(GL_ARRAY_BUFFER, posSize + colSize, normSize, cylinderNormals.data());

	glEnableVertexAttribArray(loc_vPosition);
	glVertexAttribPointer(loc_vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(loc_vColor);
	glVertexAttribPointer(loc_vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(posSize));

	glEnableVertexAttribArray(loc_vNormal);
	glVertexAttribPointer(loc_vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(posSize + colSize));

	// Giữ nguyên VAO cube là mặc định cho các lần vẽ còn lại
	glBindVertexArray(cubeVao);
	glBindBuffer(GL_ARRAY_BUFFER, cubeBuffer);
}
void generateGeometry(void)
{
	initCube();
	makeColorCube();
}


void initGPUBuffers(void)
{
	// Tạo một VAO - vertex array object
	glGenVertexArrays(1, &cubeVao);
	glBindVertexArray(cubeVao);

	// Tạo và khởi tạo một buffer object
	glGenBuffers(1, &cubeBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, cubeBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors) + sizeof(normals) + sizeof(texCoords), NULL, GL_STATIC_DRAW);

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors), sizeof(normals), normals);

	glBufferSubData(GL_ARRAY_BUFFER,
		sizeof(points) + sizeof(colors) + sizeof(normals),
		sizeof(texCoords),
		texCoords);
}

void shaderSetup(void)
{
	// Nạp các shader và sử dụng chương trình shader
	program = InitShader("vshader1.glsl", "fshader1.glsl");   // hàm InitShader khai báo trong Angel.h
	glUseProgram(program);

	// Khởi tạo thuộc tính vị trí đỉnh từ vertex shader
	loc_vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(loc_vPosition);
	glVertexAttribPointer(loc_vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	loc_vColor = glGetAttribLocation(program, "vColor");
	glEnableVertexAttribArray(loc_vColor);
	glVertexAttribPointer(loc_vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)));

	loc_vNormal = glGetAttribLocation(program, "vNormal");
	glEnableVertexAttribArray(loc_vNormal);
	glVertexAttribPointer(loc_vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points) + sizeof(colors)));

	// Thiết lập VAO cho hình trụ (nếu đã có dữ liệu)
	setupCylinderBuffers(loc_vPosition, loc_vColor, loc_vNormal);
	GLuint loc_vTexCoord = glGetAttribLocation(program, "vTexCoord");
	glEnableVertexAttribArray(loc_vTexCoord);
	glVertexAttribPointer(
		loc_vTexCoord,
		2,
		GL_FLOAT,
		GL_FALSE,
		0,
		BUFFER_OFFSET(sizeof(points) + sizeof(colors) + sizeof(normals))
	);

	//Image
	glUniform1i(glGetUniformLocation(program, "useTexture"), 0);


	/* Khởi tạo các tham số chiếu sáng - tô bóng*/
	// Light 1 (Sun)
	color4 ambient_product1 = light1_ambient * color4(1, 0, 1, 1);
	color4 diffuse_product1 = light1_diffuse * color4(1, 0.8, 0, 1);
	color4 specular_product1 = light1_specular * color4(1, 0.8, 0, 1);
	
	glUniform4fv(glGetUniformLocation(program, "AmbientProduct1"), 1, ambient_product1);
	glUniform4fv(glGetUniformLocation(program, "DiffuseProduct1"), 1, diffuse_product1);
	glUniform4fv(glGetUniformLocation(program, "SpecularProduct1"), 1, specular_product1);
	glUniform4fv(glGetUniformLocation(program, "Light1Position"), 1, light1_position);

	// Light 2 (Indoor)
	color4 ambient_product2 = light2_ambient * color4(1, 0, 1, 1);
	color4 diffuse_product2 = light2_diffuse * color4(1, 0.8, 0, 1);
	color4 specular_product2 = light2_specular * color4(1, 0.8, 0, 1);

	glUniform4fv(glGetUniformLocation(program, "AmbientProduct2"), 1, ambient_product2);
	glUniform4fv(glGetUniformLocation(program, "DiffuseProduct2"), 1, diffuse_product2);
	glUniform4fv(glGetUniformLocation(program, "SpecularProduct2"), 1, specular_product2);
	glUniform4fv(glGetUniformLocation(program, "Light2Position"), 1, light2_position);

	glUniform1f(glGetUniformLocation(program, "Shininess"), 100.0);

	glUniform1i(glGetUniformLocation(program, "texture1"), 0);

	model_loc = glGetUniformLocation(program, "Model");
	projection_loc = glGetUniformLocation(program, "Projection");
	view_loc = glGetUniformLocation(program, "View");

	glEnable(GL_DEPTH_TEST);
	// Enable blending cho vật liệu trong suốt (thủy tinh)
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(65.0f / 255,
		70.0f / 255,
		80.0f / 255, 1.0f);        /* Thiết lập màu xóa màn hình*/
}

void toMau(GLfloat a, GLfloat b, GLfloat c)
{
	color4 material_ambient(0.9, 0.9, 0.9, 1.0);
	color4 material_diffuse(a / 255.0, b / 255.0, c / 255.0, 1.0);
	color4 material_specular(1.0, 1.0, 1.0, 1.0);
	float material_shininess = 100.0;

	// Light 1: Sun
	color4 l1_diff = light1_diffuse;
	color4 l1_spec = light1_specular;
	if (!batMatTroi) { l1_diff = color4(0, 0, 0, 1); l1_spec = color4(0, 0, 0, 1); }

	glUniform4fv(glGetUniformLocation(program, "AmbientProduct1"), 1, light1_ambient * material_ambient);
	glUniform4fv(glGetUniformLocation(program, "DiffuseProduct1"), 1, l1_diff * material_diffuse);
	glUniform4fv(glGetUniformLocation(program, "SpecularProduct1"), 1, l1_spec * material_specular);
	glUniform4fv(glGetUniformLocation(program, "Light1Position"), 1, light1_position);

	// Light 2: Indoor
	color4 l2_diff = light2_diffuse;
	color4 l2_spec = light2_specular;
	if (batDen) { l2_diff = color4(mauAnhSang, mauAnhSang, mauAnhSang, 1.0); }
	else { l2_diff = color4(0, 0, 0, 1); l2_spec = color4(0, 0, 0, 1); }

	glUniform4fv(glGetUniformLocation(program, "AmbientProduct2"), 1, light2_ambient * material_ambient);
	glUniform4fv(glGetUniformLocation(program, "DiffuseProduct2"), 1, l2_diff * material_diffuse);
	glUniform4fv(glGetUniformLocation(program, "SpecularProduct2"), 1, l2_spec * material_specular);
	glUniform4fv(glGetUniformLocation(program, "Light2Position"), 1, light2_position);

	glUniform1f(glGetUniformLocation(program, "Shininess"), material_shininess);
}


float neonStrength = 0.7f;
void toMauDenNeon(GLfloat r, GLfloat g, GLfloat b)
{
	color4 light_ambient(neonStrength * r, neonStrength * g, neonStrength * b, 1.0);
	color4 zero(0.0, 0.0, 0.0, 1.0);

	// Neon only affects Ambient of Light 2? Or mimics Light 2 source?
	// Let's assume it's just a glowing object, so it sets Ambient high and others 0.
	// We need to set for both Light 1 and 2 to avoid weird lighting.
	
	glUniform4fv(glGetUniformLocation(program, "AmbientProduct1"), 1, zero);
	glUniform4fv(glGetUniformLocation(program, "DiffuseProduct1"), 1, zero);
	glUniform4fv(glGetUniformLocation(program, "SpecularProduct1"), 1, zero);

	glUniform4fv(glGetUniformLocation(program, "AmbientProduct2"), 1, light_ambient);
	glUniform4fv(glGetUniformLocation(program, "DiffuseProduct2"), 1, zero);
	glUniform4fv(glGetUniformLocation(program, "SpecularProduct2"), 1, zero);
}



//Draw function
void draw(GLfloat RGB1, GLfloat RGB2, GLfloat RGB3, GLfloat x, GLfloat y, GLfloat z, GLfloat x1, GLfloat y1, GLfloat z1, mat4 m1) {
	glBindVertexArray(cubeVao);
	toMau(RGB1, RGB2, RGB3);
	mat4 m = Translate(x, y, z) * Scale(x1, y1, z1);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, model * m1 * m);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);
}
void draw_den(GLfloat RGB1, GLfloat RGB2, GLfloat RGB3, GLfloat x, GLfloat y, GLfloat z, GLfloat x1, GLfloat y1, GLfloat z1, mat4 m1) {
	glBindVertexArray(cubeVao);
	toMauDenNeon(RGB1, RGB2, RGB3);
	mat4 m = Translate(x, y, z) * Scale(x1, y1, z1);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, model * m1 * m);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);
}

// Hàm set màu cho vật liệu thủy tinh (có độ trong suốt)
void toMauGlass(GLfloat a, GLfloat b, GLfloat c, GLfloat alpha)
{
	/* Khởi tạo các tham số chiếu sáng - tô bóng cho thủy tinh*/
	// Thủy tinh có ambient/diffuse thấp, specular cao, và có alpha
	color4 material_ambient(0.3, 0.3, 0.3, alpha);
	color4 material_diffuse(a / 255.0, b / 255.0, c / 255.0, alpha);
	color4 material_specular(1.0, 1.0, 1.0, alpha);
	float material_shininess = 150.0; // Thủy tinh có độ bóng cao

    // Light 1: Sun
    color4 l1_diff = light1_diffuse;
    color4 l1_spec = light1_specular;
    if(!batMatTroi) { l1_diff = color4(0,0,0,1); l1_spec = color4(0,0,0,1); }

    glUniform4fv(glGetUniformLocation(program, "AmbientProduct1"), 1, light1_ambient * material_ambient);
    glUniform4fv(glGetUniformLocation(program, "DiffuseProduct1"), 1, l1_diff * material_diffuse);
    glUniform4fv(glGetUniformLocation(program, "SpecularProduct1"), 1, l1_spec * material_specular);
    glUniform4fv(glGetUniformLocation(program, "Light1Position"), 1, light1_position);

    // Light 2: Indoor
    color4 l2_diff = light2_diffuse;
    color4 l2_spec = light2_specular;
    if(batDen) { l2_diff = color4(mauAnhSang, mauAnhSang, mauAnhSang, 1.0); }
    else { l2_diff = color4(0,0,0,1); l2_spec = color4(0,0,0,1); }

    glUniform4fv(glGetUniformLocation(program, "AmbientProduct2"), 1, light2_ambient * material_ambient);
    glUniform4fv(glGetUniformLocation(program, "DiffuseProduct2"), 1, l2_diff * material_diffuse);
    glUniform4fv(glGetUniformLocation(program, "SpecularProduct2"), 1, l2_spec * material_specular);
    glUniform4fv(glGetUniformLocation(program, "Light2Position"), 1, light2_position);

	glUniform1f(glGetUniformLocation(program, "Shininess"), material_shininess);
}

// Hàm vẽ cube thủy tinh (có độ trong suốt)
void drawGlass(GLfloat RGB1, GLfloat RGB2, GLfloat RGB3, GLfloat alpha, 
               GLfloat x, GLfloat y, GLfloat z, GLfloat x1, GLfloat y1, GLfloat z1, mat4 m1) {
	glDepthMask(GL_FALSE);
	glBindVertexArray(cubeVao);
	toMauGlass(RGB1, RGB2, RGB3, alpha);
	mat4 m = Translate(x, y, z) * Scale(x1, y1, z1);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, model * m1 * m);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);
	glDepthMask(GL_TRUE);
}

void drawCylinder(GLfloat RGB1, GLfloat RGB2, GLfloat RGB3, GLfloat x, GLfloat y, GLfloat z, GLfloat x1, GLfloat y1, GLfloat z1, mat4 m1, int slices)
{
	if (slices < 3) slices = 3; // tối thiểu 3 lát để tạo trụ

	// Nếu số lát thay đổi hoặc chưa có dữ liệu, khởi tạo lại và setup buffer
	if (slices != currentCylinderSlices || cylinderVertexCount <= 0) {
		initCylinder(slices);
		setupCylinderBuffers(loc_vPosition, loc_vColor, loc_vNormal);
		currentCylinderSlices = slices;
	}

	if (cylinderVertexCount <= 0 || cylinderVao == 0) return;
	toMau(RGB1, RGB2, RGB3);
	mat4 m = Translate(x, y, z) * Scale(x1, y1, z1);
	glBindVertexArray(cylinderVao);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, model * m1 * m);
	glDrawArrays(GL_TRIANGLES, 0, cylinderVertexCount);
	// Trả về VAO cube để các hàm vẽ cũ vẫn dùng được
	glBindVertexArray(cubeVao);
}

//Cửa hàng
GLfloat booth_width = 5.0f, booth_height = 3.0f, booth_len = 5.0f, wall_thick = 0.1f;
//Cầu thang
GLfloat stair_width = 0.15f, stair_height = 0.15f, stair_len = 1.2f;
//Đèn
GLfloat holder_width = 0.5f, holder_thick = 0.04f;
//Cột nhà
GLfloat col_width = 0.2f;
mat4 booth_model;
void booth() {
	draw(155, 150, 140, 0, -wall_thick * 3, 0, booth_width * 2, wall_thick, booth_len * 2, booth_model);
	draw(155, 150, 140, 0, 0, 0, booth_width, wall_thick * 3, booth_len, booth_model); //Sàn nhà
	draw(190, 200, 204, 0.5 * booth_width - 0.5 * wall_thick, 0.5 * booth_height, 0, wall_thick, booth_height, booth_width, booth_model); //Tường trái
	draw(190, 200, 204, -0.5 * booth_width + 0.5 * wall_thick, 0.5 * booth_height, 0, wall_thick, booth_height, booth_width, booth_model); //Tường phải
	draw(190, 200, 204, 0, 0.5 * booth_height, 0.5 * booth_width, booth_width - 0.5 * wall_thick, booth_height, wall_thick, booth_model); //Tường sau
	draw(225, 212, 180, 0, booth_height + 2.5 * wall_thick, 0, booth_width * 1.2, wall_thick * 5, booth_len, booth_model); //Trần nhà
	//Cầu thang
	draw(190, 185, 170, 0, 0, -0.5 * booth_width - 0.5 * stair_width, stair_len, stair_height * 2, stair_width, booth_model);
	draw(190, 185, 170, 0, -0.5 * stair_height, -0.5 * booth_width - 1.5 * stair_width, stair_len, stair_height, stair_width, booth_model);
	//Đèn trần
	draw(200, 180, 170, 0, booth_height - 0.5 * holder_thick, 0, holder_width, holder_thick, holder_width, booth_model);
	draw_den(1.0f, 0.95f, 0.7f, 0, booth_height - holder_thick, 0, 0.7 * holder_width, holder_thick, 0.7 * holder_width, booth_model);
	//Cột trái
	draw(75, 70, 65, 0.5 * booth_width - col_width, 0.5 * booth_height, -0.5 * booth_width + 0.5 * col_width, col_width, booth_height, col_width, booth_model);
	//Cột phải sau
	draw(75, 70, 65, -0.5 * booth_width + 0.5 * col_width, 0.5 * booth_height, 0.5 * booth_width - 0.5 * col_width, col_width, booth_height, col_width, booth_model);
	//Cột phải trước
	draw(75, 70, 65, -0.5 * booth_width + col_width, 0.5 * booth_height, -0.5 * booth_width + 0.5 * col_width, col_width, booth_height, col_width, booth_model);
	//Thanh ngang dưới
	draw(75, 70, 65, -0.5 * booth_width + 0.5 * col_width, 0.5 * col_width, 0, col_width, col_width, booth_width - col_width, booth_model);
}
//Robot
GLfloat f_width = 0.16f, f_len = 0.26f, f_height = 0.1f; //Foot
GLfloat l_width = 0.15f, l_height = 0.4f; //Leg
GLfloat b_width = 0.35f, b_height = 0.5f, b_thick = 0.16; //Body
GLfloat arm_width = 0.09f, arm_len = 0.4f; //Arm
GLfloat head_width = 0.15f, head_height = 0.15f, head_len = 0.25f; //Head
GLfloat horn_width = 0.03f, horn_height = 0.15f; //Horn
GLfloat eye_width = 0.03f, eye_len = 0.05f; //Eye
GLfloat mounth_wide = 0.07f, mounth_height = 0.04f, mounth_thick = 0.03f; //Mounth
GLfloat armor_width = 0.35f, armor_height = 0.25f, armor_thick = 0.07; //Chest armor
mat4 robot_model;
GLfloat robot_rot;

GLfloat arm_rot;
mat4 arm_model;

mat4 head_model;
GLfloat head_rot;
void robot1(mat4 base_model) {
	robot_model = base_model;
	//Foot
	draw(20, 140, 120, 0.5 * b_width - 0.5 * f_width, 0.5 * f_height, 0, f_width, f_height, f_len, robot_model);
	draw(20, 140, 120, -0.5 * b_width + 0.5 * f_width, 0.5 * f_height, 0, f_width, f_height, f_len, robot_model);
	//Leg
	draw(10, 90, 80, -0.5 * b_width + 0.5 * l_width, 0.5 * l_height + f_height, 0, l_width, l_height, l_width, robot_model);
	draw(10, 90, 80, 0.5 * b_width - 0.5 * l_width, 0.5 * l_height + f_height, 0, l_width, l_height, l_width, robot_model);
	//Body
	draw(45, 20, 70, 0, f_height + l_height + 0.5 * b_height, 0, b_width, b_height, b_thick, robot_model);
	//Arm
	arm_model = Translate(0.5 * b_width + 0.5 * arm_width, f_height + l_height + 0.5 * b_height + 0.5 * arm_len, 0) * RotateX(arm_rot) * Translate(-0.5 * b_width - 0.5 * arm_width, -f_height - l_height - 0.5 * b_height - 0.5 * arm_len, 0);
	draw(25, 70, 110, 0.5 * b_width + 0.5 * arm_width, f_height + l_height + 0.5 * b_height, 0, arm_width, arm_len, arm_width, robot_model * arm_model);
	//Arm armor
	draw(65, 20, 70, 0.5 * b_width + 0.75 * arm_width, f_height + l_height + 0.5 * b_height + 0.125 * arm_len, 0, arm_width * 1.25, arm_len * 0.7, arm_width * 1.1, robot_model * arm_model);
	arm_model = Translate(-0.5 * b_width - 0.5 * arm_width, f_height + l_height + 0.5 * b_height + 0.5 * arm_len, 0) * RotateX(arm_rot) * Translate(0.5 * b_width + 0.5 * arm_width, -f_height - l_height - 0.5 * b_height - 0.5 * arm_len, 0);
	draw(25, 70, 110, -0.5 * b_width - 0.5 * arm_width, f_height + l_height + 0.5 * b_height, 0, arm_width, arm_len, arm_width, robot_model * arm_model);
	//Arm armor
	draw(65, 20, 70, -0.5 * b_width - 0.75 * arm_width, f_height + l_height + 0.5 * b_height + 0.125 * arm_len, 0, arm_width * 1.25, arm_len * 0.7, arm_width * 1.1, robot_model * arm_model);
	//Head
	head_model = RotateY(head_rot);
	draw(30, 35, 55, 0, f_height + l_height + b_height + 0.5 * head_height, 0, head_len, head_height, head_width, robot_model * head_model);
	//Horn
	draw(120, 0, 0, 0.5 * head_width, f_height + l_height + b_height + head_height + 0.5 * horn_height, 0, horn_width, horn_height, horn_width, robot_model * head_model);
	draw(120, 0, 0, -0.5 * head_width, f_height + l_height + b_height + head_height + 0.5 * horn_height, 0, horn_width, horn_height, horn_width, robot_model * head_model);
	//Eye
	draw(255, 40, 40, 0.1f, f_height + l_height + b_height + 0.5 * head_height, -0.5 * head_width - 0.5 * eye_width, eye_len, eye_width, eye_width, robot_model * head_model);
	draw(255, 40, 40, -0.1f, f_height + l_height + b_height + 0.5 * head_height, -0.5 * head_width - 0.5 * eye_width, eye_len, eye_width, eye_width, robot_model * head_model);
	//Mounth
	draw(255, 40, 40, 0, f_height + l_height + b_height + 0.15 * head_height, -0.5 * head_width - 0.5 * mounth_thick, mounth_wide, mounth_height, mounth_thick, robot_model * head_model);
	//Chest armor
	draw(75, 20, 70, 0, f_height + l_height + 0.75 * b_height, -0.5 * b_thick - 0.5 * armor_thick, armor_width, armor_height, armor_thick, robot_model);
	draw(65, 20, 70, 0, f_height + l_height + 0.25 * b_height, -0.5 * b_thick - 0.25 * armor_thick, armor_width * 0.7, armor_height, armor_thick * 0.7, robot_model);
}

GLfloat robot2_rot;
GLfloat head2_rot;
GLfloat arm2_rot;
void robot2(mat4 base_model) {
	robot_model = base_model * Scale(0.8f, 0.8f, 0.8f);

	// FOOT 
	draw(20, 180, 20, 0.5f * b_width - 0.5f * f_width, 0.5f * f_height, 0,
		f_width, f_height, f_len, robot_model);
	draw(20, 180, 20, -0.5f * b_width + 0.5f * f_width, 0.5f * f_height, 0,
		f_width, f_height, f_len, robot_model);

	// ANKLE RINGS
	draw(255, 215, 0, 0.5f * b_width - 0.5f * l_width + 0.15f * l_width,
		f_height + 0.15f * l_height, 0,
		l_width * 1.25f, l_height * 0.60f, l_width * 1.25f, robot_model);
	draw(255, 215, 0, -0.5f * b_width + 0.5f * l_width - 0.15f * l_width,
		f_height + 0.15f * l_height, 0,
		l_width * 1.25f, l_height * 0.60f, l_width * 1.25f, robot_model);

	// LEGS 
	draw(100, 255, 100, -0.5f * b_width + 0.5f * l_width, 0.5f * l_height + f_height, 0,
		l_width, l_height, l_width, robot_model);
	draw(100, 255, 100, 0.5f * b_width - 0.5f * l_width, 0.5f * l_height + f_height, 0,
		l_width, l_height, l_width, robot_model);

	// BODY 
	draw(128, 0, 128, 0, f_height + l_height + 0.5f * b_height, 0,
		b_width, b_height, b_thick, robot_model);

	// RIGHT ARM
	arm_model = Translate(0.5f * b_width + 0.5f * arm_width,
		f_height + l_height + 0.5f * b_height + 0.5f * arm_len, 0)
		* RotateX(arm2_rot)
		* Translate(-0.5f * b_width - 0.5f * arm_width,
			-f_height - l_height - 0.5f * b_height - 0.5f * arm_len, 0);

	draw(147, 112, 219, 0.5f * b_width + 0.5f * arm_width,
		f_height + l_height + 0.5f * b_height, 0,
		arm_width, arm_len, arm_width, robot_model * arm_model);

	draw(70, 130, 180, 0.5f * b_width + 0.75f * arm_width,
		f_height + l_height + 0.5f * b_height + 0.125f * arm_len, 0,
		arm_width * 1.25f, arm_len * 0.7f, arm_width * 1.1f, robot_model * arm_model);

	draw(255, 223, 0, 0.5f * b_width + 0.5f * arm_width,
		f_height + l_height + 0.5f * b_height - 0.45f * arm_len, 0,
		arm_width * 1.25f, arm_len * 0.12f, arm_width * 1.25f, robot_model * arm_model);

	// LEFT ARM
	arm_model = Translate(-0.5f * b_width - 0.5f * arm_width,
		f_height + l_height + 0.5f * b_height + 0.5f * arm_len, 0)
		* RotateX(arm2_rot)
		* Translate(0.5f * b_width + 0.5f * arm_width,
			-f_height - l_height - 0.5f * b_height - 0.5f * arm_len, 0);

	draw(147, 112, 219, -0.5f * b_width - 0.5f * arm_width,
		f_height + l_height + 0.5f * b_height, 0,
		arm_width, arm_len, arm_width, robot_model * arm_model);

	draw(70, 130, 180, -0.5f * b_width - 0.75f * arm_width,
		f_height + l_height + 0.5f * b_height + 0.125f * arm_len, 0,
		arm_width * 1.25f, arm_len * 0.7f, arm_width * 1.1f, robot_model * arm_model);

	draw(255, 223, 0, -0.5f * b_width - 0.5f * arm_width,
		f_height + l_height + 0.5f * b_height - 0.45f * arm_len, 0,
		arm_width * 1.25f, arm_len * 0.12f, arm_width * 1.25f, robot_model * arm_model);

	// HEAD
	head_model = RotateY(head2_rot);
	draw(75, 0, 130, 0, f_height + l_height + b_height + 0.5f * head_height, 0,
		head_len, head_height, head_width, robot_model * head_model);

	// Horns
	draw(200, 0, 0, 0.9f * head_width, f_height + l_height + b_height + head_height,
		-0.5f * head_width - 0.5f * horn_width,
		horn_width, horn_height * 1.6f, horn_width, robot_model * head_model);
	draw(200, 0, 0, -0.9f * head_width, f_height + l_height + b_height + head_height,
		-0.5f * head_width - 0.5f * horn_width,
		horn_width, horn_height * 1.6f, horn_width, robot_model * head_model);

	// Eyes & Mouth
	draw(255, 50, 50, 0.1f, f_height + l_height + b_height + 0.5f * head_height,
		-0.5f * head_width - 0.5f * eye_width,
		eye_len, eye_width, eye_width, robot_model * head_model);
	draw(255, 50, 50, -0.1f, f_height + l_height + b_height + 0.5f * head_height,
		-0.5f * head_width - 0.5f * eye_width,
		eye_len, eye_width, eye_width, robot_model * head_model);
	draw(255, 50, 50, 0, f_height + l_height + b_height + 0.15f * head_height,
		-0.5f * head_width - 0.5f * mounth_thick,
		mounth_wide, mounth_height, mounth_thick, robot_model * head_model);

	// CHEST ARMOR
	draw(30, 144, 255, 0, f_height + l_height + 0.75f * b_height,
		-0.5f * b_thick - 0.5f * armor_thick,
		armor_width, armor_height, armor_thick, robot_model);
	draw(100, 180, 255, 0, f_height + l_height + 0.25f * b_height,
		-0.5f * b_thick - 0.25f * armor_thick,
		armor_width * 0.7f, armor_height, armor_thick * 0.7f, robot_model);

	// BADGE 
	draw(255, 255, 50, 0, f_height + l_height + 0.65f * b_height,
		-0.5f * b_thick - 0.5f * armor_thick - 0.1f * b_thick,
		b_width * 0.6f, b_height * 0.4f, armor_thick * 0.6f, robot_model);

	// BACK PLATE
	float plate_width = b_width * 0.40f;
	float plate_height = b_height * 0.45f;
	float plate_thick = b_thick * 0.35f;
	draw(180, 180, 200, 0, f_height + l_height + 0.60f * b_height,
		0.5f * b_thick + 0.175f * b_thick,
		plate_width, plate_height, plate_thick, robot_model);

	// BACK FINS
	float fin_width = plate_width * 0.45f;
	float fin_height = plate_height * 1.30f;
	float fin_depth = plate_thick * 0.9f;
	float fin_y = f_height + l_height + 0.60f * b_height + plate_height / 3.0f;
	float fin_z = 0.5f * b_thick + plate_thick;
	float plate_half = plate_width * 0.5f;
	float fin_x_right = plate_half + 0.4f * fin_width;
	float fin_x_left = -plate_half - 0.4f * fin_width;
	draw(138, 43, 226, fin_x_right, fin_y, fin_z,
		fin_width, fin_height, fin_depth, robot_model);
	draw(138, 43, 226, fin_x_left, fin_y, fin_z,
		fin_width, fin_height, fin_depth, robot_model);
}
mat4 r3_model;
// Đầu
GLfloat head_cyl_radius = 0.05f;
GLfloat head_cyl_height = 0.2f;
GLfloat neck_radius = 0.045f;
GLfloat neck_height = 0.06f;
GLfloat body_width = 0.32f, body_height = 0.45f, body_thick = 0.16f;
// Mắt
GLfloat eye_bar_width = 0.18f, eye_bar_height = 0.15f, eye_bar_thick = 0.25f;

// Giáp
GLfloat shoulder_armor_width = 0.18f, shoulder_armor_height = 0.10f, shoulder_armor_thick = 0.08f;
GLfloat chest_armor_width = 0.33f, chest_armor_height = 0.20f, chest_armor_thick = 0.05f;

// Tay
GLfloat upper_arm_len = 0.18f, upper_arm_width = 0.14f, upper_arm_thick = 0.14f;
GLfloat lower_arm_len = 0.13f, lower_arm_width = 0.12f, lower_arm_thick = 0.12f;
GLfloat hand_len = 0.08f, hand_width = 0.16f, hand_thick = 0.05f;
// Chân
GLfloat upper_leg_len = 0.25f, upper_leg_width = 0.16f, upper_leg_thick = 0.16f;
GLfloat lower_leg_len = 0.2f, lower_leg_width = 0.14f, lower_leg_thick = 0.14f;
GLfloat foot_len = 0.16f, foot_width = 0.15f, foot_thick = 0.06f;

// Robot3 Joint Angles
GLfloat r3_rotate = 0;
GLfloat r3_head_x = 0, r3_head_y = 0;
GLfloat r3_arm_left_y = 0, r3_arm_left_z = 0, r3_arm_left_x = 0;
GLfloat r3_arm_right_y = 0, r3_arm_right_z = 0, r3_arm_right_x = 0;
GLfloat r3_leg_left_y = 0, r3_leg_left_z = 0, r3_leg_left_x = 0;
GLfloat r3_leg_right_y = 0, r3_leg_right_z = 0, r3_leg_right_x = 0;

void robot3(mat4 base_model) {
	r3_model = base_model;
	
	mat4 model;
	// Thân
	model = Translate(0, 0.5 * body_height, 0);
	draw(40, 70, 140, 0, 0, 0, body_width, body_height, body_thick, r3_model * model);

	// Giáp ngực
	model = Translate(0, 0.7 * body_height, 0.5 * body_thick + 0.5 * chest_armor_thick);
	draw(80, 110, 190, 0, 0, 0, chest_armor_width, chest_armor_height, chest_armor_thick, r3_model * model);

	// Đầu và Cổ (xoay cùng nhau hoặc riêng)
	// Xoay quanh khớp cổ (body_height + 0.5 * neck_height)
    // Pivot tại cổ
    mat4 head_base = Translate(0, body_height + 0.5 * neck_height, 0) * RotateY(r3_head_y) * RotateX(r3_head_x);

	// Cổ 
	model = head_base * Translate(0, 0, 0);
	drawCylinder(60, 80, 140,
		0, 0, 0,
		2 * neck_radius, neck_height, 2 * neck_radius,
		r3_model * model, 16);

	// Hộp đầu
	model = head_base * Translate(0, 0.5 * neck_height + 0.4f * head_cyl_height, 0);
	draw(40, 70, 160, 0, 0, 0,
		eye_bar_width, eye_bar_height, eye_bar_thick,
		r3_model * model);

	//Mắt
	model = head_base * Translate(0, 0.5 * neck_height + 0.4f * head_cyl_height, 0.06)
		* RotateX(90.0f);
	drawCylinder(255, 230, 80,
		0, 0, 0,
		2 * head_cyl_radius, head_cyl_height, 2 * head_cyl_radius,
		r3_model * model, 24);

	// Giáp vai trái
	model = Translate(-0.5 * body_width - 0.5 * shoulder_armor_width, body_height, 0);
	draw(50, 90, 160, 0, 0, 0, shoulder_armor_width, shoulder_armor_height, shoulder_armor_thick, r3_model * model);

	// Giáp vai phải
	model = Translate(0.5 * body_width + 0.5 * shoulder_armor_width, body_height, 0);
	draw(50, 90, 160, 0, 0, 0, shoulder_armor_width, shoulder_armor_height, shoulder_armor_thick, r3_model * model);

	// --- TAY TRÁI ---
	float shoulder_y = body_height - 0.06f;
    // Pivot tại vai
    // Added Shoulder X (Swing) and Z (Lift)
	mat4 arm_left_base = Translate(-0.5f * body_width - 0.5f * upper_arm_width, shoulder_y, 0) 
		* RotateY(r3_arm_left_y) 
		* RotateZ(r3_arm_left_z + r3_arm_left_shoulder_z) 
		* RotateX(r3_arm_left_shoulder_x);

	// Cánh tay trên
	model = arm_left_base * Translate(0, -0.5 * upper_arm_len, 0) ;
	draw(60, 120, 240, 0, 0, 0, upper_arm_width, upper_arm_len, upper_arm_thick, r3_model * model);

	// Cẳng tay (có khớp khuỷu tay xoay quanh X)
    // Pivot khuỷu tay là điểm cuối của cánh tay trên: (0, -upper_arm_len, 0) relative to arm_left_base
    mat4 lower_arm_left_base = arm_left_base * Translate(0, -upper_arm_len, 0) * RotateX(r3_arm_left_x);
	
    // Vẽ cẳng tay (tâm tại -0.5 * lower_arm_len)
    model = lower_arm_left_base * Translate(0, -0.5 * lower_arm_len, 0);
	draw(60, 120, 210, 0, 0, 0, lower_arm_width, lower_arm_len, lower_arm_thick, r3_model * model) ;

	// Bàn tay (gắn vào cẳng tay)
	model = lower_arm_left_base * Translate(0, -lower_arm_len - 0.5 * hand_len, 0);
	draw(230, 190, 110, 0, 0, 0, hand_width, hand_len, hand_thick, r3_model * model);

	// --- TAY PHẢI ---
	mat4 arm_right_base = Translate(0.5f * body_width + 0.5f * upper_arm_width, shoulder_y, 0) 
		* RotateY(r3_arm_right_y) 
		* RotateZ(r3_arm_right_z + r3_arm_right_shoulder_z) 
		* RotateX(r3_arm_right_shoulder_x);

	// Cánh tay trên
	model = arm_right_base * Translate(0, -0.5 * upper_arm_len, 0);
	draw(60, 120, 240, 0, 0, 0, upper_arm_width, upper_arm_len, upper_arm_thick, r3_model * model);

	// Cẳng tay
    mat4 lower_arm_right_base = arm_right_base * Translate(0, -upper_arm_len, 0) * RotateX(r3_arm_right_x);

	model = lower_arm_right_base * Translate(0, -0.5 * lower_arm_len, 0);
	draw(60, 120, 210, 0, 0, 0, lower_arm_width, lower_arm_len, lower_arm_thick, r3_model * model);

	// Bàn tay
	model = lower_arm_right_base * Translate(0, -lower_arm_len - 0.5 * hand_len, 0);
	draw(230, 190, 110, 0, 0, 0, hand_width, hand_len, hand_thick, r3_model * model);

	// --- CHÂN TRÁI ---
	float hip_y = 0;
    // Pivot tại hông
    // Added Hip X (Swing) and Z (Lift)
	mat4 leg_left_base = Translate(-0.1f, hip_y, 0) 
		* RotateY(r3_leg_left_y) 
		* RotateZ(r3_leg_left_z + r3_leg_left_hip_z)
		* RotateX(r3_leg_left_hip_x);

	// Đùi
	model = leg_left_base * Translate(0, -0.5 * upper_leg_len, 0);
	draw(60, 60, 160, 0, 0, 0, upper_leg_width, upper_leg_len, upper_leg_thick, r3_model * model);

	// Cẳng chân (khớp gối xoay quanh X)
    // Pivot gối: (0, -upper_leg_len, 0)
    mat4 lower_leg_left_base = leg_left_base * Translate(0, -upper_leg_len, 0) * RotateX(r3_leg_left_x);

	model = lower_leg_left_base * Translate(0, -0.5 * lower_leg_len, 0);
	draw(50, 80, 120, 0, 0, 0, lower_leg_width, lower_leg_len, lower_leg_thick, r3_model * model);

	// Bàn chân
	model = lower_leg_left_base * Translate(0, -lower_leg_len - 0.5 * foot_thick, 0.1 * foot_len);
	draw(190, 135, 70, 0, 0, 0, foot_width, foot_thick, foot_len, r3_model * model);

	// --- CHÂN PHẢI ---
	mat4 leg_right_base = Translate(0.1f, hip_y, 0) 
		* RotateY(r3_leg_right_y) 
		* RotateZ(r3_leg_right_z + r3_leg_right_hip_z)
		* RotateX(r3_leg_right_hip_x);

	// Đùi
	model = leg_right_base * Translate(0, -0.5 * upper_leg_len, 0);
	draw(60, 60, 160, 0, 0, 0, upper_leg_width, upper_leg_len, upper_leg_thick, r3_model * model);

	// Cẳng chân
    mat4 lower_leg_right_base = leg_right_base * Translate(0, -upper_leg_len, 0) * RotateX(r3_leg_right_x);

	model = lower_leg_right_base * Translate(0, -0.5 * lower_leg_len, 0);
	draw(50, 80, 120, 0, 0, -0, lower_leg_width, lower_leg_len, lower_leg_thick, r3_model * model);

	// Bàn chân
	model = lower_leg_right_base * Translate(0, -lower_leg_len - 0.5 * foot_thick, 0.1 * foot_len);
	draw(190, 135, 70, 0, 0, 0, foot_width, foot_thick, foot_len, r3_model * model);

	// Cánh sau lưng (các thanh dài vàng cam xòe ra giống mẫu)
	
}
//Bục trưng bày robot
GLfloat platf_width = 0.9f, platf_len = 3.5f, platf_height = 0.6f;
void platform() {
	draw(200, 200, 200, -0.15 * booth_width + col_width, 0.5 * platf_height, 0.5 * booth_width - 0.7 * platf_width, platf_len, platf_height, platf_width, booth_model);
	mat4 r1 = Translate(-0.5 * booth_width + 1.7 * b_width, platf_height, 0.5 * booth_width - b_width * 2) * Scale(1.5, 1.5, 1.5) * RotateY(robot_rot);
	robot1(r1);
	mat4 r2 = Translate(-0.5 * booth_width + 1.7 * b_width + 0.8f, platf_height, 0.5 * booth_width - b_width * 2) * Scale(1, 1, 1) * RotateY(robot2_rot);
	robot2(r2);
	mat4 r3 = Translate(-0.5 * booth_width + 5 * b_width + 1.6f, platf_height + upper_leg_len + lower_leg_len + foot_len + 0.2, 0.5 * booth_width - b_width * 2) * Scale(1.5, 1.5, 1.5) * RotateY(robot_rot) * RotateY(180) * RotateY(r3_rotate);
	//mat4 r3 = Translate(0,2,-3) * Scale(1.5, 1.5, 1.5) * RotateY(robot_rot) * RotateY(180);
	robot3(r3);
}
// Cửa tự động phía trước gian hàng
// base_model: thường là booth_model hoặc booth_model * Transform(...)
// openAmount: 0 (đóng) -> 1 (mở hoàn toàn)
void automaticDoor(float openAmount, const mat4& base_model)
{
    // Kích thước tấm cửa tỉ lệ theo booth
    float panelWidth     = (booth_width -  col_width * 2)  / 4.0f;      // 4 tấm chia đều chiều ngang
    float panelHeight    = booth_height * 0.85   ;    // hơi thấp hơn trần một chút
    float panelThickness = wall_thick * 0.6f;       // mỏng hơn tường

    // Tâm cửa đặt ở mặt trước booth
    float doorCenterY = panelHeight * 0.5f + booth_height*0.05;
    float doorCenterZ = -0.5f * booth_width + 0.5f * panelThickness;  // sát mép trước

	// Khoảng trượt khi mở (tương đương 1 panelWidth khi openAmount = 1)
	if (openAmount < 0.0f) openAmount = 0.0f;
	if (openAmount > 1.0f) openAmount = 1.0f;
	float slideDistance = panelWidth * openAmount;

    // Thông số khung bao quanh từng cánh
    float leafFrameThick  = wall_thick * 0.4f;
    float leafFrameDepth  = panelThickness * 1.2f;
	mat4 m;
    auto drawLeafWithFrame = [&](float centerOffsetX, float centerOffsetZ)
    {
        // Tâm của cánh cửa (trong tọa độ booth)
        mat4 leafBase = base_model * Translate(centerOffsetX, doorCenterY, centerOffsetZ);

        // Tấm kính chính (thủy tinh trong suốt)
        // Màu xanh nhạt của thủy tinh, alpha = 0.4 để có độ trong suốt
        m = leafBase * Scale(panelWidth, panelHeight, panelThickness);
        drawGlass(180, 220, 255, 0.4f, 0, 0, 0, 1, 1, 1, m);

        // Khung bao quanh (4 thanh mỏng)
        // Trái
        m = leafBase
            * Translate(-0.5f * panelWidth - 0.5f * leafFrameThick, 0.0f, 0.0f)
            * Scale(leafFrameThick, panelHeight + 2.0f * leafFrameThick, leafFrameDepth);
        draw(150, 145, 155, 0, 0, 0, 1, 1, 1, m);

        // Phải
        m = leafBase
            * Translate(0.5f * panelWidth + 0.5f * leafFrameThick, 0.0f, 0.0f)
            * Scale(leafFrameThick, panelHeight + 2.0f * leafFrameThick, leafFrameDepth);
        draw(150, 145, 155, 0, 0, 0, 1, 1, 1, m);

        // Trên
        m = leafBase
            * Translate(0.0f, 0.5f * panelHeight + 0.5f * leafFrameThick, 0.0f)
            * Scale(panelWidth + 2.0f * leafFrameThick, leafFrameThick, leafFrameDepth);
        draw(140, 135, 145, 0, 0, 0, 1, 1, 1, m);

        // Dưới
        m = leafBase
            * Translate(0.0f, -0.5f * panelHeight - 0.5f * leafFrameThick, 0.0f)
            * Scale(panelWidth + 2.0f * leafFrameThick, leafFrameThick, leafFrameDepth);
        draw(140, 135, 145, 0, 0, 0, 1, 1, 1, m);
    };

    // Tấm ngoài cùng bên trái (cố định)
    drawLeafWithFrame(-1.5f * panelWidth, doorCenterZ);

    // Tấm giữa bên trái (trượt sang trái khi mở)
    drawLeafWithFrame(-0.5f * panelWidth - slideDistance, doorCenterZ - panelThickness);

    // Tấm giữa bên phải (trượt sang phải khi mở)
    drawLeafWithFrame(0.5f * panelWidth + slideDistance, doorCenterZ - panelThickness);

    // Tấm ngoài cùng bên phải (cố định)
    drawLeafWithFrame(1.5f * panelWidth, doorCenterZ);

    // Thanh ngang trên đầu (cảm biến tổng)
    float topBarHeight = booth_height * 0.15;
    m = base_model
        * Translate(0.0f, panelHeight +  topBarHeight*0.95, doorCenterZ - 0.12f)
        * Scale(4.0f * panelWidth, topBarHeight, wall_thick * 2.0f);
    draw(200, 200, 210, 0, 0, 0, 1, 1, 1, m);
	// Đèn cảm biến
	m = base_model
		* Translate(0.15f, panelHeight + topBarHeight, doorCenterZ - 0.12f)
		* Scale(4.0f * panelWidth, topBarHeight + 0.1f, wall_thick * 2.0f);
	drawCylinder(255, 0, 0,
		0, 0, 0,
		0.02f, 1, 0.02f,
		m, 36);
}
//table
GLfloat table_thick = 0.05, table_height = 0.7, table_len = 1.3, table_width = 0.5;
mat4 table_model;
GLfloat table_rot = 0;
void table(mat4 model) {
	table_model = model;
	draw(160, 150, 90, 0, 0.5 * table_height, 0.5 * table_len, table_width, table_height, table_thick, table_model); //left
	draw(160, 150, 90, 0, 0.5 * table_height, -0.5 * table_len, table_width, table_height, table_thick, table_model); //right
	draw(160, 150, 90, -0.5 * table_width, 0.5 * table_height, 0, table_thick, table_height, table_len, table_model); //front
	draw(255, 150, 70, 0, table_height, 0, table_width + 0.2, table_thick, table_len + 0.2, table_model); //top
}

//Shelf
GLfloat shelf_height = 2.0f, shelf_width = 1.5f, shelf_thick = 0.5f, plank_thick = 0.03f;
mat4 shelf_model;
GLfloat shelf_rot;
//Ngăn kéo tủ
GLfloat drawer_height = 0.4f, front_back_width = 0.7f, drawer_thick = 0.03f, drawer_side_len = 0.45f;
GLfloat handle_width = 0.1f, handle_height = 0.03f;
mat4 drawer_model;
GLfloat dis;
void drawer(mat4 m) {
	drawer_model = m;
	//Ngăn kéo
	draw(204, 255, 204, 0, 0.5 * drawer_height, 0.5 * shelf_thick, front_back_width, drawer_height, drawer_thick, shelf_model * drawer_model);//Back
	draw(204, 255, 204, 0, 0.5 * drawer_height, -0.5 * shelf_thick, front_back_width, drawer_height, drawer_thick, shelf_model * drawer_model);//Front
	draw(204, 255, 204, 0.5 * front_back_width, 0.5 * drawer_height, 0, drawer_thick, drawer_height, drawer_side_len, shelf_model * drawer_model);//Right
	draw(204, 255, 204, -0.5 * front_back_width, 0.5 * drawer_height, 0, drawer_thick, drawer_height, drawer_side_len, shelf_model * drawer_model);//Left
	draw(204, 255, 204, 0, 0.5 * drawer_height, 0, front_back_width, drawer_thick, drawer_side_len, shelf_model * drawer_model);//Bottom
	//Tay kéo
	draw(0, 0, 0, 0, 0.5 * drawer_height, -0.5 * drawer_side_len - drawer_thick - 0.5 * handle_height, handle_width, handle_height, handle_height, shelf_model * drawer_model);
}
void shelf(mat4 m) {
	shelf_model = m;
	draw(229, 255, 204, 0, 0.5 * shelf_height, -0.5 * shelf_width, shelf_thick, shelf_height, plank_thick, shelf_model); //side
	draw(229, 255, 204, 0, 0.5 * shelf_height, 0.5 * shelf_width, shelf_thick, shelf_height, plank_thick, shelf_model); //side
	draw(229, 255, 204, 0, shelf_height, 0, shelf_thick, plank_thick, shelf_width, shelf_model); //top
	draw(229, 255, 204, 0, 0.5 * plank_thick, 0, shelf_thick, plank_thick, shelf_width, shelf_model); //bottomm
	draw(229, 255, 204, 0, 0.2 * shelf_height, 0, shelf_thick, plank_thick, shelf_width, shelf_model); //middle
	draw(229, 255, 204, 0, 0.6 * shelf_height, 0, shelf_thick, plank_thick, shelf_width, shelf_model); //middle
	draw(229, 255, 204, 0.5 * shelf_thick, 0.5 * shelf_height, 0, plank_thick, shelf_height, shelf_width, shelf_model); //back
	draw(229, 255, 204, 0, shelf_height * 0.5, 0, shelf_thick, shelf_height, plank_thick, shelf_model);
	drawer(Translate(0, 0, 0.25 * shelf_width) * Translate(dis, 0, 0) * RotateY(90));
	drawer(Translate(0, 0, -0.25 * shelf_width) * Translate(dis, 0, 0) * RotateY(90));
	//Shelf with robot
	robot1(shelf_model * Translate(0, 0.2 * shelf_height, -0.37 * shelf_width) * RotateY(90) * Scale(0.5, 0.5, 0.5));
	robot1(shelf_model * Translate(0, 0.2 * shelf_height, -0.15 * shelf_width) * RotateY(90) * Scale(0.5, 0.5, 0.5));
	//robot1(shelf_model * Translate(0, 0.5 * shelf_height, 0) * RotateY(90) * Scale(0.5, 0.5, 0.5));
	robot2(shelf_model * Translate(0, 0.2 * shelf_height, 0.12 * shelf_width) * RotateY(90) * Scale(0.65, 0.65, 0.65));
	robot2(shelf_model * Translate(0, 0.2 * shelf_height, 0.36 * shelf_width) * RotateY(90) * Scale(0.65, 0.65, 0.65));
	//robot3
	robot3(shelf_model * Translate(0,  shelf_height * 0.75, -0.37 * shelf_width) * RotateY(-90) * Scale(0.5, 0.5, 0.5));
	robot3(shelf_model * Translate(0,  shelf_height * 0.75, -0.15 * shelf_width) * RotateY(-90) * Scale(0.5, 0.5, 0.5));


}


GLuint signTexture;

void loadSignTexture() {
	int w, h, channels;

	unsigned char* data = stbi_load(
		"sign.jpg", &w, &h, &channels, 0
	);
	if (!data) {
		cout << "Khong tim thay sign.jpg\n";
	}

	glGenTextures(1, &signTexture);
	glBindTexture(GL_TEXTURE_2D, signTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
	glTexImage2D(
		GL_TEXTURE_2D, 0, format,
		w, h, 0, format,
		GL_UNSIGNED_BYTE, data
	);

	stbi_image_free(data);
}
void drawSign(mat4 booth_model) {

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, signTexture);  // ← texture ĐÃ LOAD

	glUniform1i(glGetUniformLocation(program, "useTexture"), 1);

	mat4 m = Translate(0,
		booth_height + 0.8,
		-booth_len / 2 - 0.2)
		* Scale(3.0, 1.2, 0.05);

	glUniformMatrix4fv(model_loc, 1, GL_TRUE,
		model * booth_model * m);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glUniform1i(glGetUniformLocation(program, "useTexture"), 0);
}



mat4 gheModel;
GLfloat gocXoay = 0.0f;
// Trạng thái cửa tự động: 0 = đóng, 1 = mở
float doorAmount = 0.0f;
float doorTarget = 0.0f;
const float doorSpeed = 0.02f; // tốc độ mở/đóng
void veLapPhuong(mat4 m)
{
	glBindVertexArray(cubeVao);
	toMau(158, 110, 70);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, model * m);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);
}

void chanGhe(mat4 cha, float x, float z)
{
	veLapPhuong(cha * Translate(x, 0.45, z) * Scale(0.18, 0.9, 0.18));
}

void cumChanGhe(mat4 cha)
{
	float d = 0.7;
	chanGhe(cha, -d, d);
	chanGhe(cha, d, d);
	chanGhe(cha, -d, -d);
	chanGhe(cha, d, -d);
}

void matGhe(mat4 cha)
{
	veLapPhuong(cha * Translate(0.0, 0.75, 0.0) * Scale(1.6, 0.15, 1.6));
}

void lungGhe(mat4 cha)
{
	veLapPhuong(cha * Translate(0.0, 1.7, -0.75) * Scale(1.6, 1.6, 0.15));
}

void tayGhe(mat4 cha, float x)
{
	veLapPhuong(cha * Translate(x, 1.15, -0.1) * Scale(0.18, 0.18, 1.6));
}

void truTayGhe(mat4 cha, float x)
{
	veLapPhuong(cha * Translate(x, 0.9, 0.7) * Scale(0.18, 0.7, 0.18));
}

void cumTayGhe(mat4 cha)
{
	float x = 0.9;
	tayGhe(cha, -x);
	tayGhe(cha, x);
	truTayGhe(cha, -x);
	truTayGhe(cha, x);
}

void ghe(mat4 cha)
{
	cumChanGhe(cha);
	matGhe(cha);
	lungGhe(cha);
	cumTayGhe(cha);
}
void tamdiem() {
	drawCylinder(255, 255, 255, 0, 0, 0, 2, 2, 2, mat4(), 256);
}

void display(void)
{
	// Update Camera
	view = LookAt(eye, at, up);
	glUniformMatrix4fv(view_loc, 1, GL_TRUE, view);

	projection = Frustum(-2 * fovScale, 2 * fovScale, -2 * fovScale, 2 * fovScale, 3, 12);
	glUniformMatrix4fv(projection_loc, 1, GL_TRUE, projection);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	const vec3 viewer_pos(0.0, 0.0, 2.0);  /*Trùng với eye của camera*/

	booth();
	
	drawSign(booth_model);
	platform();
	table(Translate(0.2 * booth_width, 0, -0.3 * booth_width) * RotateY(table_rot));
	table(Translate(-0.3 * booth_width, 0, -0.3 * booth_width) * RotateY(-90));
	shelf(Translate(0.43 * booth_width, 2.5 * wall_thick, -0.1 * booth_width) * RotateY(shelf_rot));
	shelf(Translate(0.43 * booth_width, 2.5 * wall_thick, 0.22 * booth_width) * RotateY(shelf_rot));
	ghe(Translate(0.3 * booth_width, 0, -0.3 * booth_width) * RotateY(-90) * RotateY(gocXoay) * Scale(0.3, 0.4, 0.3));
	ghe(Translate(-0.3 * booth_width, 0, -0.2 * booth_width) * RotateY(180) * RotateY(gocXoay) * Scale(0.3, 0.4, 0.3));
	
	automaticDoor(doorAmount, booth_model);
	
	glutSwapBuffers();
}

void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
}

void keyboard(unsigned char key, int x, int y)
{
	// Mode Selection
	if (key == '0' || key == '`') { controlMode = 0; cout << "Mode:Permanent camera\n"; }
	if (key == '1') { controlMode = 1; cout << "Mode: Robot 1\n"; }
	if (key == '2') { controlMode = 2; cout << "Mode: Robot 2\n"; }
	if (key == '3') { controlMode = 3; cout << "Mode: Robot 3\n"; }
	if (key == '9') { controlMode = 4; cout << "Mode: Free camera \n"; }
	switch (key) {
	case 033:			// 033 is Escape key octal value
		exit(1);		// quit program
		break;
	case 'o': // Toggle Indoor Light
		batDen = !batDen;
		neonStrength = batDen ? 1.2f : 0.3f;
		mauAnhSang = batDen ? 0.7f : 0.3f;
		break;
	case 'p': // Toggle Sun
		batMatTroi = !batMatTroi;
		break;
	case 'g': // Toggle Door
	case 'G':
		doorTarget = (doorTarget < 0.5f) ? 1.0f : 0.0f;
		break;
	}

	// Mode-Specific Controls
	if (controlMode == 4) { // Camera & Room Mode
		vec4 forward = at - eye;
		forward.y = 0; // Keep movement horizontal
		forward = normalize(forward);
		vec4 right = vec4(normalize(cross(forward, up)), 0.0);

		switch (key) {
		case 'w': case 'W': // Forward
			eye += forward * camSpeed;
			at += forward * camSpeed;
			break;
		case 's': case 'S': // Backward
			eye -= forward * camSpeed;
			at -= forward * camSpeed;
			break;
		case 'a': case 'A': // Strafe Left
			eye -= right * camSpeed;
			at -= right * camSpeed;
			break;
		case 'd': case 'D': // Strafe Right
			eye += right * camSpeed;
			at += right * camSpeed;
			break;
		case 'q': case 'Q': // Up
			eye.y += camSpeed;
			at.y += camSpeed;
			break;
		case 'e': case 'E': // Down
			eye.y -= camSpeed;
			at.y -= camSpeed;
			break;
		case 'i': case 'I': // Forward
			
			at += forward * camSpeed * 2;
			break;
		case 'k': case 'K': // Backward
			
			at -= forward * camSpeed * 2;
			break;
		case 'j': case 'J': // Strafe Left
			
			at -= right * camSpeed;
			break;
		case 'l': case 'L': // Strafe Right
			
			at += right * camSpeed;
			break;

		case 'z': case 'Z': // Zoom In
			fovScale *= 0.95f;
			if (fovScale < 0.1f) fovScale = 0.1f;
			break;
		case 'x': case 'X': // Zoom Out
			fovScale *= 1.05f;
			if (fovScale > 5.0f) fovScale = 5.0f;
			break;
			// Room Objects
		case 't': case 'T': // Drawer
			if (key == 't') dis += drawer_side_len * 0.1;
			else dis -= drawer_side_len * 0.1;
			if (dis > 0) dis = 0;
			if (dis < -drawer_side_len) dis = -drawer_side_len;
			break;
		case 'c': case 'C': // Shelf
			if (key == 'c') shelf_rot += 5;
			else shelf_rot -= 5;
			if (shelf_rot >= 360) shelf_rot = 0;
			if (shelf_rot < 0) shelf_rot = 360;
			break;
		case 'y': case 'Y': // Table
			if (key == 'y') table_rot += 5;
			else table_rot -= 5;
			if (table_rot >= 360) table_rot = 0;
			if (table_rot < 0) table_rot = 360;
			break;
		case 'u': case 'U': // Chair
			if (key == 'u') gocXoay += 5.0f;
			else gocXoay -= 5.0f;
			break;
		}
	}
	else if (controlMode == 1) { // Robot 1
		switch (key) {
		case 'w': case 'W': robot_rot += 5; break;
		case 's': case 'S': robot_rot -= 5; break;
		case 'a': case 'A': arm_rot += 5; break;
		case 'd': case 'D': arm_rot -= 5; break;
		case 'q': case 'Q': head_rot += 5; break;
		case 'e': case 'E': head_rot -= 5; break;
		}
		if (robot_rot >= 360) robot_rot = 0; else if (robot_rot < 0) robot_rot = 360;
		if (arm_rot >= 360) arm_rot = 0; else if (arm_rot < 0) arm_rot = 360;
		if (head_rot >= 360) head_rot = 0; else if (head_rot < 0) head_rot = 360;
	}
	else if (controlMode == 2) { // Robot 2
		switch (key) {
		case 'w': case 'W': robot2_rot += 5; break;
		case 's': case 'S': robot2_rot -= 5; break;
		case 'a': case 'A': arm2_rot += 5; break;
		case 'd': case 'D': arm2_rot -= 5; break;
		case 'q': case 'Q': head2_rot += 5; break;
		case 'e': case 'E': head2_rot -= 5; break;
		}
		if (robot2_rot >= 360) robot2_rot = 0; else if (robot2_rot < 0) robot2_rot = 360;
		if (arm2_rot >= 360) arm2_rot = 0; else if (arm2_rot < 0) arm2_rot = 360;
		if (head2_rot >= 360) head2_rot = 0; else if (head2_rot < 0) head2_rot = 360;
	}
	else if (controlMode == 3) { // Robot 3
		switch (key) {
		case 'j': case 'J': // Xoay người
			r3_rotate += 5;
			if (r3_rotate > 360) r3_rotate = 0;
			break;
		case 'l': case 'L':
			r3_rotate -= 5;
			if (r3_rotate < 0) r3_rotate = 360;
			break;

        // tay trái
        // Xoay X
		case 'w': r3_arm_left_shoulder_x += 5; if (r3_arm_left_shoulder_x > 90) r3_arm_left_shoulder_x = 90; break;
		case 's': r3_arm_left_shoulder_x -= 5; if (r3_arm_left_shoulder_x < -90) r3_arm_left_shoulder_x = -90; break;
        // Xoay Z 
        case 'a': r3_arm_left_shoulder_z += 5; if (r3_arm_left_shoulder_z > 180) r3_arm_left_shoulder_z = 180; break;
		case 'd': r3_arm_left_shoulder_z -= 5; if (r3_arm_left_shoulder_z < 0) r3_arm_left_shoulder_z = 0; break;
        // Khuỷu tay 
		case 'q': r3_arm_left_x -= 5; if (r3_arm_left_x < -135) r3_arm_left_x = -135; break;
		case 'e': r3_arm_left_x += 5; if (r3_arm_left_x > 0) r3_arm_left_x = 0; break;

		// tay phải
        // Xoay X
		case 'W': r3_arm_right_shoulder_x += 5; if (r3_arm_right_shoulder_x > 90) r3_arm_right_shoulder_x = 90; break;
		case 'S': r3_arm_right_shoulder_x -= 5; if (r3_arm_right_shoulder_x < -90) r3_arm_right_shoulder_x = -90; break;
        // Xoay Z 
        case 'A': r3_arm_right_shoulder_z -= 5; if (r3_arm_right_shoulder_z < -180) r3_arm_right_shoulder_z = -180; break; 
		case 'D': r3_arm_right_shoulder_z += 5; if (r3_arm_right_shoulder_z > 0) r3_arm_right_shoulder_z = 0; break;
		// Khủyu tay
		case 'Q': r3_arm_right_x -= 5; if (r3_arm_right_x < -135) r3_arm_right_x = -135; break;
		case 'E': r3_arm_right_x += 5; if (r3_arm_right_x > 0) r3_arm_right_x = 0; break;

        // chân trái
        // Xoay X 
		case 'z': r3_leg_left_hip_x += 5; if (r3_leg_left_hip_x > 90) r3_leg_left_hip_x = 90; break;
		case 'x': r3_leg_left_hip_x -= 5; if (r3_leg_left_hip_x < -90) r3_leg_left_hip_x = -90; break;
        // Xoay Z 
		case 'c': r3_leg_left_hip_z += 5; if (r3_leg_left_hip_z > 90) r3_leg_left_hip_z = 90; break;
		case 'v': r3_leg_left_hip_z -= 5; if (r3_leg_left_hip_z < 0) r3_leg_left_hip_z = 0; break;
        // đầu gối
		case 'r': r3_leg_left_x += 5; if (r3_leg_left_x > 135) r3_leg_left_x = 135; break;
		case 'f': r3_leg_left_x -= 5; if (r3_leg_left_x < 0) r3_leg_left_x = 0; break;

        // chân phải
        //  Xoay X 
		case 'Z': r3_leg_right_hip_x += 5; if (r3_leg_right_hip_x > 90) r3_leg_right_hip_x = 90; break;
		case 'X': r3_leg_right_hip_x -= 5; if (r3_leg_right_hip_x < -90) r3_leg_right_hip_x = -90; break;
        //  Xoay Z 
		case 'C': r3_leg_right_hip_z -= 5; if (r3_leg_right_hip_z < -90) r3_leg_right_hip_z = -90; break;
		case 'V': r3_leg_right_hip_z += 5; if (r3_leg_right_hip_z > 0) r3_leg_right_hip_z = 0; break;
        // đầu gối
		case 'R': r3_leg_right_x += 5; if (r3_leg_right_x > 135) r3_leg_right_x = 135; break;
		case 'F': r3_leg_right_x -= 5; if (r3_leg_right_x < 0) r3_leg_right_x = 0; break;

        // đầu
		case 't': r3_head_y += 5; break;
		case 'y': r3_head_y -= 5; break;
		}
	}
	else if(controlMode == 0) { // Permanent Camera
		switch (key) {
		case 'r': case 'R':
			model = mat4();
			break;
		case 'x':
			model *= RotateX(dr);;
			break;
		case 'X':
			model *= RotateX(-dr);
			break;
		case 'y':
			model *= RotateY(dr);
			break;
		case 'Y':
			model *= RotateY(-dr);
			break;
		case 'z':
			model *= RotateZ(dr);
			break;
		case 'Z':
			model *= RotateZ(-dr);
			break;
		case 'w': case 'W': // Zoom In
			fovScale *= 0.95f;
			if (fovScale < 0.1f) fovScale = 0.1f;
			break;
		case 's': case 'S': // Zoom Out
			fovScale *= 1.05f;
			if (fovScale > 5.0f) fovScale = 5.0f;
			break;
		}
	}
	glutPostRedisplay();
}

// Hàm idle để cập nhật trạng thái cửa theo thời gian
void idle()
{
	if (fabs(doorAmount - doorTarget) > 0.001f) {
		if (doorAmount < doorTarget) doorAmount += doorSpeed;
		else                         doorAmount -= doorSpeed;

		if (doorAmount < 0.0f) doorAmount = 0.0f;
		if (doorAmount > 1.0f) doorAmount = 1.0f;

		glutPostRedisplay();
	}
}


int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(900, 900);
	glutInitWindowPosition(100, 150);
	glutCreateWindow("A Cube is rotated by keyboard and shaded");


	glewInit();

	generateGeometry();
	initGPUBuffers();
	shaderSetup();
	loadSignTexture();

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);
	glutIdleFunc(idle);

	glutMainLoop();
	return 0;
}
