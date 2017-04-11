/////////////////////////////////////////////////////////////////
// main.cpp Author: Vladimir Frolov, 2011, Graphics & Media Lab.
/////////////////////////////////////////////////////////////////

#include <GL/glew.h>

#include "GL/glus.h"
#include "../vsgl3/glHelper.h"

#include "ClothSim.h"

#include <iostream>
#include <fstream>
#include <string>
#include <map>

ShaderProgram g_program;
ShaderProgram g_programLand;
ShaderProgram g_programRed;
ShaderProgram g_programNorm;

struct MyInput
{
	MyInput() 
	{
	cam_rot[0] = cam_rot[1] = cam_rot[2] = cam_rot[3] = 0.f;
	mx = my = 0;
	rdown = ldown = false;
	cam_dist = 60.0f;
	}

	int mx;
	int my;
	bool rdown;
	bool ldown;
	float cam_rot[4];
	float cam_pos[4];

	float cam_dist;

}input;

FullScreenQuad* g_pFullScreenQuad = nullptr;
int g_width  = 0;
int g_height = 0;


SimpleMesh* g_pLandMesh = nullptr;
SimpleMesh* g_pTor = nullptr;
SimpleMesh* g_pSphere = nullptr;
const int textureClothCnt = 4;
const int textureLandCnt = 4;
Texture2D*  g_pClothTex[textureClothCnt];
Texture2D*  g_pLandTex[textureLandCnt];

float4x4    g_projectionMatrix;
float3      g_camPos(0,20,20);
float3      g_sunDir = normalize(float3(0.5f, 0.5f, 0.5f));
float3		centerS = float3(1.2, -0.2, 0.7);
float		radiusS = 0.2f;


// cloth sim
//
ClothMeshData g_cloth;
int  g_clothShaderId = 1;
int g_landShaderId = 1;
bool g_drawShadowMap = false;
bool g_windDecrease = false;

void RequreExtentions() // check custome extentions here
{
	CHECK_GL_ERRORS;

	std::cout << "GPU Vendor: " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "GPU Name  : " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "GL_VER    : " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL_VER  : " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

	GlusHelperRequireExt h;
	h.require("GL_EXT_texture_filter_anisotropic");
}

/**
* Function for initialization.
*/
GLUSboolean init(GLUSvoid)
{
	try 
	{
	RequreExtentions();
	for (int i = 0; i < textureClothCnt; i++)
	{
		g_pClothTex[i] = nullptr;
	}
	for (int i = 0; i < textureLandCnt; i++)
	{
		g_pLandTex[i] = nullptr;
	}
	// Load the source of the vertex shader. GLUS loader corrupts shaders.
	// Thats why we do that with our class ShaderProgram
	//
	g_program = ShaderProgram("../main/Vertex.vert", "../main/Fragment.frag"); // shaders to draw cloth in texture mode
	g_programLand  = ShaderProgram("../main/Vertex.vert", "../main/Land.frag"); // shaders to draw land
	g_programRed    = ShaderProgram("../main/Vertex.vert", "../main/Red.frag");      // shaders to draw cloth in wire frame mode
	g_programNorm   = ShaderProgram("../main/Vertex.vert", "../main/Norm.frag");      // shaders to draw normal with color
	
	g_pFullScreenQuad  = new FullScreenQuad();
	g_pLandMesh        = new SimpleMesh(g_programLand.program, 20, SimpleMesh::PLANE, 2.0f);
	g_pTor			   = new SimpleMesh(g_program.program, 20, SimpleMesh::TORUS, 0.1f);
	g_pSphere		   = new SimpleMesh(g_program.program, 20, SimpleMesh::SPHERE, radiusS);

	g_pClothTex[0]        = new Texture2D("../data/sponza_fabric_green_diff.tga");
	g_pClothTex[1]		  = new Texture2D("../data/sponza_fabric_blue_diff.tga");
	g_pClothTex[2]		  = new Texture2D("../data/sponza_fabric_spec.tga");
	g_pClothTex[3]		  = new Texture2D("../data/sponza_fabric_diff.tga");

	g_pLandTex[0]		  = new Texture2D("../data/texture1.bmp");
	g_pLandTex[1]		  = new Texture2D("../data/wood.jpg");
	g_pLandTex[2]		  = new Texture2D("../data/snow_m.tga");
	g_pLandTex[3]		  = new Texture2D("../data/crate.tga");

	g_cloth = CreateTest2Vertices();
	// copy geometry data to GPU; use shader program to create vao inside; 
	// !!!!!!!!!!!!!!!!!!!!!!!!!!! DEBUG THIS CAREFULLY !!!!!!!!!!!!!!!!!!!!!!! <<============= !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// please remember about vertex shader attributes opt and invalid locations.
	//

	g_cloth.initGPUData(g_programRed.program);
	g_cloth.initGPUDataTr(g_program.program);
	g_cloth.initGPUDataNorm(g_programNorm.program);
	g_cloth.rSphere = radiusS * sqrtf(3.0f);
	g_cloth.centerSphere = centerS;

	// g_cloth = CreateQuad(16, 16, 1.0f, g_programRed.program, g_programCloth2.program);

	return GLUS_TRUE;
	}
	catch(std::runtime_error e)
	{
	std::cerr << e.what() << std::endl;
	exit(-1);
	}
	catch(...)
	{
	std::cerr << "Unexpected Exception (init)!" << std::endl;
	exit(-1);
	}
}

GLUSvoid reshape(GLUSuint width, GLUSuint height)
{
	glViewport(0, 0, width, height);
	g_width  = width;
	g_height = height;

	glusPerspectivef(g_projectionMatrix.L(), 45.0f, (GLfloat) width / (GLfloat) height, 1.0f, 100.0f);  // Calculate the projection matrix
}

GLUSvoid mouse(GLUSboolean pressed, GLUSuint button, GLUSuint x, GLUSuint y)
{
	if(!pressed)
	return;

	if (button & 1)// left button
	{
	input.ldown=true;		
	input.mx=x;			
	input.my=y;
	}

	if (button & 4)	// right button
	{
	input.rdown=true;
	input.mx=x;
	input.my=y;
	}
}

GLUSvoid mouseMove(GLUSuint button, GLUSint x, GLUSint y)
{
	if(button & 1 || button & 4)		// left button
	{
	int x1 = x;
	int y1 = y;

	input.cam_rot[0] += 0.45f*(y1-input.my);	// change rotation
	input.cam_rot[1] += 0.45f*(x1-input.mx);

	input.mx=x;
	input.my=y;
	}
}

GLUSvoid keyboard(GLUSboolean pressed, GLUSuint key)
{
	switch(key)
	{
	case 'w':
	case 'W':
		input.cam_dist -= 0.5f;
	break;

	case 's':
	case 'S':
		input.cam_dist += 0.5f;
	break;

	case 'a':
	case 'A':
   
	break;

	case 'd':
	case 'D':

	break;

	case 'p':
	case 'P':
		g_cloth.g_wind += float4(0, 0, 0.01, 0);
		g_windDecrease = false;
		break;

	case 'o':
	case 'O':
		g_cloth.g_wind += float4(0, 0, -0.01, 0);
		g_windDecrease = false;
		break;

	case '0':
	case ')':
		g_windDecrease = true;
		break;
	case '1':
	case '!':
		g_clothShaderId = 1;
		g_drawShadowMap = false;
	break;

	case '2':
	case '@':
		g_clothShaderId = 2;
		g_drawShadowMap = false;
	break;

	case '3':
	case '#':
		g_clothShaderId = 3;
		g_drawShadowMap = false;
	break;

	case '4':
	case '$':
		g_clothShaderId = 4;
		g_drawShadowMap = false;
	break;

	case '5':
	case '%':
		g_clothShaderId = 5;
		g_drawShadowMap = false;
		break;

	case '6':
	case '^':
		g_clothShaderId = 6;
		g_drawShadowMap = false;
		break;

	case 'z':
	case 'Z':
		g_landShaderId = 1;
		g_drawShadowMap = false;
		break;

	case 'x':
	case 'X':
		g_landShaderId = 2;
		g_drawShadowMap = false;
		break;

	case 'c':
	case 'C':
		g_landShaderId = 3;
		g_drawShadowMap = false;
		break;

	case 'v':
	case 'V':
		g_landShaderId = 4;
		g_drawShadowMap = false;
		break;

	case 'm':
	case 'M':
		g_drawShadowMap = true;
		break;
	case 't':
	case 'T':
		centerS.z -= 0.03;
		break;
	case 'g':
	case 'G':
		centerS.z += 0.03;
		break;
	case 'h':
	case 'H':
		centerS.x += 0.03;
		break;
	case 'f':
	case 'F':
		centerS.x -= 0.03;
		break;
	case 'r':
	case 'R':
		centerS.y -= 0.03;
		break;
	case 'y':
	case 'Y':
		centerS.y += 0.03;
		break;
	}

}

GLUSboolean update(GLUSfloat time)
{
	try 
	{
	static float elaspedTimeFromStart = 0;
	elaspedTimeFromStart += 10*time;
	g_camPos.z = input.cam_dist;
	if (g_windDecrease)
	{
		float windForce = g_cloth.g_wind.z;
		if (fabs(windForce) < 0.0001)
		{
			g_windDecrease = false;
		}
		g_cloth.g_wind -= float4(0, 0, windForce / 2, 0);
	}

	g_cloth.rSphere = radiusS * sqrtf(3.0f);
	g_cloth.centerSphere = centerS;

	float4x4 model;
	float4x4 modelView;
	glusLoadIdentityf(model.L()); 
	glusRotateRzRyRxf(model.L(), input.cam_rot[0], input.cam_rot[1], 0.0f);
	glusLookAtf(modelView.L(), g_camPos.x, g_camPos.y, g_camPos.z, 
								0.0f, 0.0f, 0.0f, 
								0.0f, 1.0f, 0.0f);                           // ... and the view matrix ...

	glusMultMatrixf(modelView.L(), modelView.L(), model.L()); 	            // ... to get the final model view matrix

	float4x4 rotationMatrix, scaleMatrix, translateMatrix;
	float4x4 transformMatrix1, transformMatrix2;

	// make our program current
	//
	glViewport(0, 0, g_width, g_height);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);  

	// cloth
	//

	SimStep(&g_cloth, time);
	g_cloth.updatePositionsGPU();

	RecalculateNormals(&g_cloth);
	g_cloth.updateNormalsGPU();

	// cloth object space transform
	//
	const float  clothScale      = 9.0f;
	const float  clothTranslateY = 0.0f;


	rotationMatrix.identity();
	scaleMatrix.identity();
	translateMatrix.identity();
	transformMatrix1.identity();
	transformMatrix2.identity();

	glusTranslatef(translateMatrix.L(), 0, clothTranslateY, 0);
	glusScalef(scaleMatrix.L(), clothScale, clothScale, clothScale);
	glusMultMatrixf(transformMatrix1.L(), rotationMatrix.L(), scaleMatrix.L());
	glusMultMatrixf(transformMatrix2.L(), translateMatrix.L(), transformMatrix1.L());


	

	

	if (g_clothShaderId > 2)
	{
		GLuint clothProgramId = g_program.program;

		glUseProgram(clothProgramId);

		setUniform(clothProgramId, "modelViewMatrix", modelView);
		setUniform(clothProgramId, "projectionMatrix", g_projectionMatrix);  // set matrix we have calculated in "reshape" funtion
		setUniform(clothProgramId, "objectMatrix", transformMatrix2);

		setUniform(clothProgramId, "g_sunDir", g_sunDir);

		bindTexture(clothProgramId, 1, "diffuseTexture", g_pClothTex[g_clothShaderId - 3]->GetColorTexId());
		g_cloth.pTris->Draw();
	}
	else if (g_clothShaderId == 1)
	{
		GLuint clothProgramId = g_programRed.program;

		glUseProgram(clothProgramId);

		setUniform(clothProgramId, "modelViewMatrix", modelView);
		setUniform(clothProgramId, "projectionMatrix", g_projectionMatrix);  // set matrix we have calculated in "reshape" funtion
		setUniform(clothProgramId, "objectMatrix", transformMatrix2);

		setUniform(clothProgramId, "g_sunDir", g_sunDir);
		
		glPointSize(2.0f); // well, we want to draw vertices as bold points )
		g_cloth.pMesh->Draw(GL_POINTS); // draw vertices
		g_cloth.pMesh->Draw(GL_LINES);  // draw connections
	}
	else
	{
		GLuint clothProgramId = g_programNorm.program;

		glUseProgram(clothProgramId);

		setUniform(clothProgramId, "modelViewMatrix", modelView);
		setUniform(clothProgramId, "projectionMatrix", g_projectionMatrix);  // set matrix we have calculated in "reshape" funtion
		setUniform(clothProgramId, "objectMatrix", transformMatrix2);

		setUniform(clothProgramId, "g_sunDir", g_sunDir);

		g_cloth.pNorm->Draw();
	}


	// \\ cloth

	// draw land
	//
	rotationMatrix.identity();
	scaleMatrix.identity();
	translateMatrix.identity();
	transformMatrix1.identity();
	transformMatrix2.identity();

	glusScalef(scaleMatrix.L(), 15, 15, 15);
	glusTranslatef(translateMatrix.L(), 0, -5, 0);
	glusMultMatrixf(transformMatrix1.L(), rotationMatrix.L(), scaleMatrix.L());
	glusMultMatrixf(transformMatrix2.L(), translateMatrix.L(), transformMatrix1.L());

	glUseProgram(g_programLand.program);

	setUniform(g_programLand.program, "modelViewMatrix", modelView);
	setUniform(g_programLand.program, "projectionMatrix", g_projectionMatrix);  // set matrix we have calculated in "reshape" funtion
	setUniform(g_programLand.program, "objectMatrix", transformMatrix2);

	setUniform(g_programLand.program, "g_sunDir", g_sunDir);

	bindTexture(g_programLand.program, 1, "diffuseTexture", g_pLandTex[g_landShaderId - 1]->GetColorTexId());

	g_pLandMesh->Draw();



	// Draw some meshes
	rotationMatrix.identity();
	scaleMatrix.identity();
	translateMatrix.identity();
	transformMatrix1.identity();
	transformMatrix2.identity();

	glusScalef(scaleMatrix.L(), 15, 15, 15);
	glusTranslatef(translateMatrix.L(), 10, 0, -7);
	glusMultMatrixf(transformMatrix1.L(), rotationMatrix.L(), scaleMatrix.L());
	glusMultMatrixf(transformMatrix2.L(), translateMatrix.L(), transformMatrix1.L());

	glUseProgram(g_programNorm.program);

	setUniform(g_programNorm.program, "modelViewMatrix", modelView);
	setUniform(g_programNorm.program, "projectionMatrix", g_projectionMatrix);  // set matrix we have calculated in "reshape" funtion
	setUniform(g_programNorm.program, "objectMatrix", transformMatrix2);

	setUniform(g_programNorm.program, "g_sunDir", g_sunDir);

	g_pTor->Draw();
 


	rotationMatrix.identity();
	scaleMatrix.identity();
	translateMatrix.identity();
	transformMatrix1.identity();
	transformMatrix2.identity();

	glusScalef(scaleMatrix.L(), 15, 15, 15);
	glusTranslatef(translateMatrix.L(), 9 * centerS.x, 9 * centerS.y, 9 * centerS.z);
	glusMultMatrixf(transformMatrix1.L(), rotationMatrix.L(), scaleMatrix.L());
	glusMultMatrixf(transformMatrix2.L(), translateMatrix.L(), transformMatrix1.L());

	glUseProgram(g_program.program);

	setUniform(g_program.program, "modelViewMatrix", modelView);
	setUniform(g_program.program, "projectionMatrix", g_projectionMatrix);  // set matrix we have calculated in "reshape" funtion
	setUniform(g_program.program, "objectMatrix", transformMatrix2);

	setUniform(g_program.program, "g_sunDir", g_sunDir);

	bindTexture(g_program.program, 1, "diffuseTexture", g_pLandTex[2]->GetColorTexId());

	g_pSphere->Draw();

	


	return GLUS_TRUE;
	}
	catch(std::runtime_error e)
	{
	std::cerr << e.what() << std::endl;
	exit(-1);
	}
	catch(...)
	{
	std::cerr << "Unexpected Exception(render)!" << std::endl;
	exit(-1);
	}
}

/**
	* Function to clean up things.
	*/
void shutdown(void)
{
	delete g_pFullScreenQuad; g_pFullScreenQuad = nullptr;
	delete g_pLandMesh;       g_pLandMesh = nullptr; 
	for (int i = 0; i < textureClothCnt; i++)
	{
		delete g_pClothTex[i];
		g_pClothTex[i] = nullptr;
	}
	for (int i = 0; i < textureLandCnt; i++)
	{
		delete g_pLandTex[i];
		g_pLandTex[i] = nullptr;
	}

}

/**
	* Main entry point.
	*/
int main(int argc, char* argv[])
{
	glusInitFunc(init);
	glusReshapeFunc(reshape);
	glusUpdateFunc(update);
	glusTerminateFunc(shutdown);
	glusMouseFunc(mouse);
	glusMouseMoveFunc(mouseMove);
	glusKeyFunc(keyboard);

	glusPrepareContext(3, 0, GLUS_FORWARD_COMPATIBLE_BIT);

	if (!glusCreateWindow("cloth sim", 1024, 768, GLUS_FALSE))
	{
		printf("Could not create window!");
		return -1;
	}

	// Init GLEW
	glewExperimental = GL_TRUE;
	GLenum err=glewInit();
	if(err!=GLEW_OK)
	{
	sprintf("glewInitError", "Error: %s\n", glewGetErrorString(err));
	return -1;
	}
	glGetError(); // flush error state variable, caused by glew errors
  

	// Only continue, if OpenGL 3.3 is supported.
	if (!glewIsSupported("GL_VERSION_3_0"))
	{
		printf("OpenGL 3.0 not supported.");

		glusDestroyWindow();
		return -1;
	}

	glusRun();

	return 0;
}

