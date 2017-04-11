#include "ClothSim.h"
#include <cstdint>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ClothMeshData CreateTest2Vertices()
{
	ClothMeshData mdata;
	size_t yLen = 20, xLen = 15;
	mdata.m_sizeX = xLen;
	mdata.m_sizeY = yLen;
	float initialHardness = 70.0f;
	float edgeLength = 0.07f;
	mdata.vertPos0.resize(yLen * xLen);
	mdata.vertVel0.resize(yLen * xLen);
	mdata.vertPos1.resize(yLen * xLen);
	mdata.vertVel1.resize(yLen * xLen);
	mdata.texCoord.resize(yLen * xLen);
	mdata.edgeHardness.resize(0);
	mdata.edgeIndices.resize(0);
	mdata.edgeInitialLen.resize(0);
	mdata.vertNormals.resize(yLen * xLen);
  
	mdata.vertForces.resize(yLen * xLen);
	mdata.vertMassInv.resize(yLen * xLen);


	for (size_t i = 0; i < yLen; i++)
	{
		for (size_t j = 0; j < xLen; j++)
		{
			mdata.vertPos0[i * xLen + j] = float4(edgeLength * j - 0.5, edgeLength * i + 0.1, 0, 1);
			mdata.vertVel0[i * xLen + j] = float4(0, 0, 0, 0);
			mdata.vertMassInv[i * xLen + j] = 1.0f / 0.05f;
			mdata.texCoord[i * xLen + j] = float2(float(j) / float(xLen), float(i) / float(yLen));
			mdata.vertNormals[i * xLen + j] = float3(0, 0, 1);
		}
	}

	//mdata.vertVel0[0] = float4(0, 0, 0, 0);
	//mdata.vertVel0[1] = float4(0, 0, 0, 0);

	mdata.vertPos1 = mdata.vertPos0;
	mdata.vertVel1 = mdata.vertVel0;

	//mdata.vertMassInv[0] = 1.0f/1e20f; we can model static vertices as a vertices with very big mass; didn't say this is good, but simple :)
	//mdata.vertMassInv[1] = 1.0f;

	for (size_t i = 0; i < mdata.vertPos0.size(); i++)
	{
		for (size_t j = i + 1; j < mdata.vertPos0.size(); j++)
		{
		  
			float4 vA = mdata.vertPos0[i];
			float4 vB = mdata.vertPos0[j];
			float dist = length(vA-vB);
			if (dist < 2.0f*sqrtf(2.0)*edgeLength)
			{
				float hardness = initialHardness * (edgeLength / dist);
				mdata.edgeIndices.push_back(i);
				mdata.edgeIndices.push_back(j);
				mdata.edgeHardness.push_back(hardness);
				mdata.edgeInitialLen.push_back(dist);
			}
		}
	}
	for (size_t i = 0; i < yLen - 1; i++)
	{
		for (size_t j = 0; j < xLen - 1; j++)
		{
			mdata.trIndices.push_back(i * xLen + j);
			mdata.trIndices.push_back((i + 1) * xLen + j);
			mdata.trIndices.push_back((i + 1) * xLen + j + 1);
			mdata.trIndices.push_back(i * xLen + j);
			mdata.trIndices.push_back(i * xLen + j + 1);
			mdata.trIndices.push_back((i + 1) * xLen + j + 1);
		}
	}


	//mdata.edgeHardness[0]   = 1.0f;
	//mdata.edgeInitialLen[0] = 0.2f;

	mdata.g_wind = float4(0, 0, 0, 0);
	mdata.currentTime = 0.0f;

	// you can use any intermediate mesh representation or load data to GPU (in VBOs) here immediately.                              <<===== !!!!!!!!!!!!!!!!!!

	// create graphics mesh; SimpleMesh uses GLUS Shape to store geometry; 
	// we copy data to GLUS Shape, and then these data will be copyed later from GLUS shape to GPU 
	//
	mdata.pMesh = std::make_shared<SimpleMesh>();

	GLUSshape& shape = mdata.pMesh->m_glusShape;

	shape.numberVertices = mdata.vertPos0.size();
	shape.numberIndices  = mdata.edgeIndices.size();

	shape.normals = (GLUSfloat*)malloc(3 * shape.numberVertices * sizeof(GLUSfloat));
	shape.vertices  = (GLUSfloat*)malloc(4 * shape.numberVertices * sizeof(GLUSfloat));
	shape.indices   = (GLUSuint*) malloc(shape.numberIndices * sizeof(GLUSuint));

	memcpy(shape.normals, &mdata.vertNormals[0], sizeof(float)* 3 * shape.numberVertices);
	memcpy(shape.vertices, &mdata.vertPos0[0], sizeof(float) * 4 * shape.numberVertices);
	memcpy(shape.indices, &mdata.edgeIndices[0], sizeof(int) * shape.numberIndices);

	
	
	mdata.pTris = std::make_shared<SimpleMesh>();

	GLUSshape& shape1 = mdata.pTris->m_glusShape;

	shape1.numberVertices = mdata.vertPos0.size();
	shape1.numberIndices = mdata.trIndices.size();

	shape1.normals = (GLUSfloat*)malloc(3 * shape1.numberVertices * sizeof(GLUSfloat));
	shape1.texCoords = (GLUSfloat*)malloc(2 * shape1.numberVertices * sizeof(GLUSfloat));
	shape1.vertices = (GLUSfloat*)malloc(4 * shape1.numberVertices * sizeof(GLUSfloat));
	shape1.indices = (GLUSuint*)malloc(shape1.numberIndices * sizeof(GLUSuint));

	memcpy(shape1.normals, &mdata.vertNormals[0], sizeof(float)* 3 * shape1.numberVertices);
	memcpy(shape1.texCoords, &mdata.texCoord[0], sizeof(float)* 2 * shape1.numberVertices);
	memcpy(shape1.vertices, &mdata.vertPos0[0], sizeof(float)* 4 * shape1.numberVertices);
	memcpy(shape1.indices, &mdata.trIndices[0], sizeof(int)* shape1.numberIndices);


	mdata.pNorm = std::make_shared<SimpleMesh>();

	GLUSshape& shape2 = mdata.pNorm->m_glusShape;

	shape2.numberVertices = mdata.vertPos0.size();
	shape2.numberIndices = mdata.trIndices.size();

	shape2.normals = (GLUSfloat*)malloc(3 * shape2.numberVertices * sizeof(GLUSfloat));
	shape2.texCoords = (GLUSfloat*)malloc(2 * shape2.numberVertices * sizeof(GLUSfloat));
	shape2.vertices = (GLUSfloat*)malloc(4 * shape2.numberVertices * sizeof(GLUSfloat));
	shape2.indices = (GLUSuint*)malloc(shape2.numberIndices * sizeof(GLUSuint));

	memcpy(shape2.normals, &mdata.vertNormals[0], sizeof(float)* 3 * shape2.numberVertices);
	memcpy(shape2.texCoords, &mdata.texCoord[0], sizeof(float)* 2 * shape2.numberVertices);
	memcpy(shape2.vertices, &mdata.vertPos0[0], sizeof(float)* 4 * shape2.numberVertices);
	memcpy(shape2.indices, &mdata.trIndices[0], sizeof(int)* shape2.numberIndices);
	

	// for tri mesh you will need normals, texCoords and different indices
	// 

	return mdata;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ClothMeshData::updatePositionsGPU()
{
	if (pMesh == nullptr)
		return;

	GLUSshape& shape = pMesh->m_glusShape;

	shape.vertices = (GLUSfloat*)malloc(4 * shape.numberVertices * sizeof(GLUSfloat));
	memcpy(shape.vertices, &vertPos0[0], sizeof(float)* 4 * shape.numberVertices);

	glBindBuffer(GL_ARRAY_BUFFER, pMesh->m_vertexPosBufferObject);                                                         CHECK_GL_ERRORS;
	glBufferData(GL_ARRAY_BUFFER, shape.numberVertices * 4 * sizeof(GLfloat), (GLfloat*)shape.vertices, GL_STATIC_DRAW);   CHECK_GL_ERRORS;

	if (pTris == nullptr)
		return;

	GLUSshape& shape1 = pTris->m_glusShape;

	shape1.vertices = (GLUSfloat*)malloc(4 * shape1.numberVertices * sizeof(GLUSfloat));
	memcpy(shape1.vertices, &vertPos0[0], sizeof(float)* 4 * shape1.numberVertices);

	glBindBuffer(GL_ARRAY_BUFFER, pTris->m_vertexPosBufferObject);                                                           CHECK_GL_ERRORS;
	glBufferData(GL_ARRAY_BUFFER, shape1.numberVertices * 4 * sizeof(GLfloat), (GLfloat*)shape1.vertices, GL_STATIC_DRAW);   CHECK_GL_ERRORS;

	if (pNorm == nullptr)
		return;

	GLUSshape& shape2 = pNorm->m_glusShape;

	shape2.vertices = (GLUSfloat*)malloc(4 * shape2.numberVertices * sizeof(GLUSfloat));
	memcpy(shape2.vertices, &vertPos0[0], sizeof(float)* 4 * shape2.numberVertices);

	glBindBuffer(GL_ARRAY_BUFFER, pNorm->m_vertexPosBufferObject);                                                           CHECK_GL_ERRORS;
	glBufferData(GL_ARRAY_BUFFER, shape2.numberVertices * 4 * sizeof(GLfloat), (GLfloat*)shape2.vertices, GL_STATIC_DRAW);   CHECK_GL_ERRORS;

	// copy current vertex positions to positions VBO
 
}

void ClothMeshData::updateNormalsGPU()
{
	if (pMesh == nullptr || this->vertNormals.size() == 0)
	{
		return;
	}

	GLUSshape& shape = pMesh->m_glusShape;

	shape.normals = (GLUSfloat*)malloc(3 * shape.numberVertices * sizeof(GLUSfloat));
	memcpy(shape.normals, &vertNormals[0], sizeof(float)* 3 * shape.numberVertices);

	glBindBuffer(GL_ARRAY_BUFFER, pMesh->m_vertexNormBufferObject);                                                           CHECK_GL_ERRORS;
	glBufferData(GL_ARRAY_BUFFER, shape.numberVertices * 3 * sizeof(GLfloat), (GLfloat*)shape.normals, GL_STATIC_DRAW);     CHECK_GL_ERRORS;

	if (pTris == nullptr)
	return;

	GLUSshape& shape1 = pTris->m_glusShape;

	shape1.normals = (GLUSfloat*)malloc(3 * shape1.numberVertices * sizeof(GLUSfloat));
	memcpy(shape1.normals, &vertNormals[0], sizeof(float)* 3 * shape1.numberVertices);

	glBindBuffer(GL_ARRAY_BUFFER, pTris->m_vertexNormBufferObject);                                                           CHECK_GL_ERRORS;
	glBufferData(GL_ARRAY_BUFFER, shape1.numberVertices * 3 * sizeof(GLfloat), (GLfloat*)shape1.normals, GL_STATIC_DRAW);     CHECK_GL_ERRORS;

	if (pNorm == nullptr)
		return;

	GLUSshape& shape2 = pNorm->m_glusShape;

	shape2.normals = (GLUSfloat*)malloc(3 * shape2.numberVertices * sizeof(GLUSfloat));
	memcpy(shape2.normals, &vertNormals[0], sizeof(float)* 3 * shape2.numberVertices);

	glBindBuffer(GL_ARRAY_BUFFER, pNorm->m_vertexNormBufferObject);                                                         CHECK_GL_ERRORS;
	glBufferData(GL_ARRAY_BUFFER, shape2.numberVertices * 3 * sizeof(GLfloat), (GLfloat*)shape2.normals, GL_STATIC_DRAW);     CHECK_GL_ERRORS;
	// copy current recalculated normals to appropriate VBO on GPU

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SimStep(ClothMeshData* pMesh, float delta_t)
{
 
	// get in and out pointers
	//
	float4* inVertPos  = pMesh->pinPong ? &pMesh->vertPos1[0] : &pMesh->vertPos0[0];
	float4* inVertVel  = pMesh->pinPong ? &pMesh->vertVel1[0] : &pMesh->vertVel0[0];

	float4* outVertPos = pMesh->pinPong ? &pMesh->vertPos0[0] : &pMesh->vertPos1[0];
	float4* outVertVel = pMesh->pinPong ? &pMesh->vertVel0[0] : &pMesh->vertVel1[0];

	// accumulate forces first
	//
	float dt = 0.01;
	float g = 9.8;
	float4 constWind = float4(0, 0, sin(pMesh->currentTime) / 1000, 0);
	pMesh->currentTime += delta_t;
	//for (int j = 0; j < 4; j++)
	//{
		for (size_t i = 0; i < pMesh->vertForces.size(); i++) // clear all forces
			pMesh->vertForces[i] = float4(0, 0, 0, 0);
		for (size_t connectId = 0; connectId < pMesh->connectionNumber(); connectId++)
		{
			float k = pMesh->edgeHardness[connectId];
			int numVert1 = pMesh->edgeIndices[2 * connectId];
			int numVert2 = pMesh->edgeIndices[2 * connectId + 1];
			float4 n = inVertPos[numVert2] - inVertPos[numVert1];
			float nLen = length(n);
			n = normalize(n);
			float deltaL = nLen - pMesh->edgeInitialLen[connectId];
			pMesh->vertForces[numVert1] += n * (k * deltaL);
			pMesh->vertForces[numVert2] -= n * (k * deltaL);
		}
		for (size_t i = 0; i < pMesh->vertPos0.size() - pMesh->m_sizeX; i++)
		{
			pMesh->vertForces[i] += float4(0, -g / pMesh->vertMassInv[i], 0, 0);
			pMesh->vertForces[i] += (-0.001 * inVertVel[i]);
			pMesh->vertForces[i] += pMesh->g_wind;
			pMesh->vertForces[i] += constWind;
			float4 a = pMesh->vertForces[i] * pMesh->vertMassInv[i];

			float4 f4to3 = inVertPos[i];
			float3 r = float3(f4to3.x, f4to3.y, f4to3.z) - pMesh->centerSphere;
			float dist = length(r);
			if ((dist - pMesh->rSphere) < 0.001)
			{
				float3 c = pMesh->centerSphere;
				inVertPos[i] = float4(c.x, c.y, c.z, 1) + normalize(float4(r.x, r.y, r.z, 0)) * pMesh->rSphere;
				float cos = dot3(inVertVel[i], r) / (length3(inVertVel[i]) * length(r));
				float3 f3to4 = normalize(r) * cos * length3(inVertVel[i]);
				inVertVel[i] -= float4(f3to4.x, f3to4.y, f3to4.z, 0);

				cos = dot3(a, r) / (length3(a) * length(r));
				if (cos < 0)
				{
					f3to4 = normalize(r) * cos * length3(a);
					a -= float4(f3to4.x, f3to4.y, f3to4.z, 0);
				}
			}
			outVertVel[i] = inVertVel[i] + a * dt;
			outVertPos[i] = inVertPos[i] + outVertVel[i] * dt + a * dt * dt / 2.0;


			inVertVel[i] = outVertVel[i];
			inVertPos[i] = outVertPos[i];
		}
	//}
	pMesh->pinPong = !pMesh->pinPong; // swap pointers for next sim step
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RecalculateNormals(ClothMeshData* pMesh)
{
	float4* inVertPos = pMesh->pinPong ? &pMesh->vertPos1[0] : &pMesh->vertPos0[0];
	for (size_t i = 0; i < pMesh->m_sizeY; i++)
	{
		for (size_t j = 0; j < pMesh->m_sizeX; j++)
		{
			float3 n[4];
			float3 sum = float3(0, 0, 0);
			n[0] = float3(0, 0, 0);
			n[1] = float3(0, 0, 0);
			n[2] = float3(0, 0, 0);
			n[3] = float3(0, 0, 0);
			float4 a;

			if (j < pMesh->m_sizeX - 1)
			{
				a = a = inVertPos[i * pMesh->m_sizeX + j] - inVertPos[i * pMesh->m_sizeX + j + 1];
				n[0] = float3(a.x, a.y, a.z);
			}
			if (i < pMesh->m_sizeY - 1)
			{
				a = a = inVertPos[i * pMesh->m_sizeX + j] - inVertPos[(i + 1) * pMesh->m_sizeX + j];
				n[1] = float3(a.x, a.y, a.z);

			}
			if (j > 0)
			{
				a = inVertPos[i * pMesh->m_sizeX + j] - inVertPos[i * pMesh->m_sizeX + j - 1];
				n[2] = float3(a.x, a.y, a.z);
			}
			if (i > 0)
			{
				a = inVertPos[i * pMesh->m_sizeX + j] - inVertPos[(i - 1) * pMesh->m_sizeX + j];
				n[3] = float3(a.x, a.y, a.z);
			}

			for (int k = 0; k < 4; k++)
			{
				float3 cur = cross(n[k], n[(k + 1) % 4]);
				if (length(cur) > 0.0000001) cur = normalize(cur);
				sum += cur;
			}
			if (length(sum) > 0.0000001) sum = normalize(sum);
			pMesh->vertNormals[i * pMesh->m_sizeX + j] = sum;
		}
	}
}

