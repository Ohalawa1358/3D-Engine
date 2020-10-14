#include "olcConsoleGameEngine.h"
using namespace std;
#include <fstream>
#include <strstream>
#include <algorithm>

struct vec3d
{
	float x = 0;
	float y = 0;
	float z = 0;
	float w = 1;
};

struct triangle
{
	vec3d p[3];

	wchar_t sym;
	short col;
};

struct mesh
{
	vector<triangle> tris;

	bool LoadFromObjectFile(string sFilename) {
		ifstream f(sFilename);
		if (!f.is_open())
			return false;

		vector<vec3d> verts;

		while (!f.eof()) {
			char line[128];
			f.getline(line, 128);

			strstream s;
			s << line;

			char junk;

			if (line[0] == 'v') {
				vec3d v;
				s >> junk >> v.x >> v.y >> v.z;
				verts.push_back(v);
			}

			if (line[0] == 'f') {
				int f[3];
				s >> junk >> f[0] >> f[1] >> f[2];
				tris.push_back({ verts[f[0] - 1.0f], verts[f[1] - 1.0f], verts[f[2] - 1.0f] });
			}

		}

		return true;


	}
};

struct mat4x4
{
	float m[4][4] = { 0 };
};
class olcEngine3D : public olcConsoleGameEngine
{
public: 
	olcEngine3D() {
		m_sAppName = L"3D Demo"; //unicode will be enabled by default
	}

private:
	mesh meshCube;
	mat4x4 matProj;
	float fTheta;

	vec3d vCamera;

	vec3d Matrix_MultiplyVector( mat4x4& m, vec3d& i ) {
		vec3d v;
		v.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + i.w * m.m[3][0];
		v.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + i.w * m.m[3][1];
		v.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + i.w * m.m[3][2];
		v.w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + i.w * m.m[3][3];
		
		return v;
	}

	mat4x4 Matrix_MakeIdentity() {
		mat4x4 matrix;
		matrix.m[0][0] = 1.0f;
		matrix.m[1][1] = 1.0f;
		matrix.m[2][2] = 1.0f;
		matrix.m[3][3] = 1.0f;

		return matrix;
	}

	mat4x4 Matrix_MakeRotationX(float fAngleRad) {
		mat4x4 matRotX;
		matRotX.m[0][0] = 1.0f;
		matRotX.m[1][1] = cosf(fAngleRad * 0.5f);
		matRotX.m[1][2] = sinf(fAngleRad * 0.5f);
		matRotX.m[2][1] = -sinf(fAngleRad * 0.5f);
		matRotX.m[2][2] = cosf(fAngleRad * 0.5f);
		matRotX.m[3][3] = 1.0f;

		return matRotX;
	}

	mat4x4 Matrix_MakeRotationZ(float fAngleRad) {
		mat4x4 matRotZ;
		matRotZ.m[0][0] = cosf(fAngleRad);
		matRotZ.m[0][1] = sinf(fAngleRad);
		matRotZ.m[1][0] = -sinf(fAngleRad);
		matRotZ.m[1][1] = cosf(fAngleRad);
		matRotZ.m[2][2] = 1.0f;
		matRotZ.m[3][3] = 1.0f;

		return matRotZ;
	}
	mat4x4 Matrix_MakeRotationY(float fAngleRad) {
		mat4x4 matRotY;
		matRotY.m[0][0] = cosf(fAngleRad);
		matRotY.m[0][2] = sinf(fAngleRad);
		matRotY.m[2][0] = -sinf(fAngleRad);
		matRotY.m[1][1] = 1.0f;
		matRotY.m[2][2] = cosf(fAngleRad);
		matRotY.m[3][3] = 1.0f;

		return matRotY;
	}

	mat4x4 Matrix_MakeTranslation(float x, float y, float z) {
		mat4x4 matrix;
		matrix.m[0][0] = 1.0f;
		matrix.m[1][1] = 1.0f;
		matrix.m[2][2] = 1.0f;
		matrix.m[3][3] = 1.0f;
		matrix.m[3][0] = x;
		matrix.m[3][1] = y;
		matrix.m[3][2] = z;

		return matrix;
	}

	mat4x4 Matrix_MakeProjection(float fNear, float fFar, float fFov, float fAspectRatio) {

		float fFovRad = 1.0f / tanf(fFov * 0.5f / 180.0f * 3.14159f);

		matProj.m[0][0] = fAspectRatio * fFovRad;
		matProj.m[1][1] = fFovRad;
		matProj.m[2][2] = fFar / (fFar - fNear);
		matProj.m[3][2] = (-fFar * fNear) / (fFar - fNear);
		matProj.m[2][3] = 1.0f;
		matProj.m[3][3] = 0.0f;
		return matProj;
	}

	mat4x4 Matrix_MultiplyMatrix(mat4x4& m1, mat4x4& m2) {
		mat4x4 matrix;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				matrix.m[j][i] = m1.m[j][0] * m2.m[0][i] + m1.m[j][1] * m2.m[1][i] + m1.m[j][2] * m2.m[2][i] + m1.m[j][3] * m2.m[3][i];
			}
		}
		return matrix;
	}

	vec3d Vector_Add(vec3d& v1, vec3d& v2) {
		return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
	}

	vec3d Vector_Subtract(vec3d& v1, vec3d& v2) {
		return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
	}

	vec3d Vector_Mult(vec3d& v, float scalar) {
		return { v.x * scalar, v.y * scalar, v.z * scalar };
	}

	vec3d Vector_Div(vec3d& v, float scalar) {
		return { v.x / scalar, v.y / scalar, v.z / scalar };
	}

	float Vector_DotProduct(vec3d& v1, vec3d& v2) {
		return ( (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z) );
	}

	float Vector_Length(vec3d& v) {
		return sqrtf((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
	}

	vec3d Vector_Normalize(vec3d& v) {
		float l = Vector_Length(v);
		return { v.x / l, v.y / l, v.z / l };
	}

	vec3d Vector_CrossProduct(vec3d& v1, vec3d& v2) {
		vec3d v;
		v.x = v1.y * v2.z - v1.z * v2.y;
		v.y = v1.z * v2.x - v1.x * v2.z;
		v.z = v1.x * v2.y - v1.y * v2.x;
		return v;
	}

	// Taken From Command Line Webcam Video by OneLoneCoder
	CHAR_INFO GetColour(float lum)
	{
		short bg_col, fg_col;
		wchar_t sym;
		int pixel_bw = (int)(13.0f * lum);
		switch (pixel_bw)
		{
		case 0: bg_col = BG_BLACK; fg_col = FG_BLACK; sym = PIXEL_SOLID; break;

		case 1: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_QUARTER; break;
		case 2: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_HALF; break;
		case 3: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_THREEQUARTERS; break;
		case 4: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_SOLID; break;

		case 5: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_QUARTER; break;
		case 6: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_HALF; break;
		case 7: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_THREEQUARTERS; break;
		case 8: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_SOLID; break;

		case 9:  bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_QUARTER; break;
		case 10: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_HALF; break;
		case 11: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_THREEQUARTERS; break;
		case 12: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_SOLID; break;
		default:
			bg_col = BG_BLACK; fg_col = FG_BLACK; sym = PIXEL_SOLID;
		}

		CHAR_INFO c;
		c.Attributes = bg_col | fg_col;
		c.Char.UnicodeChar = sym;
		return c;
	}

public:

	bool OnUserCreate() override {

		meshCube.LoadFromObjectFile("teapot.obj");

		// Projection Matrix
		float fNear = 0.1f;
		float fFar = 1000.0f;
		float fFov = 90.0f;
		float fAspectRatio = (float)ScreenHeight() / (float)ScreenWidth();

		matProj = Matrix_MakeProjection(fNear, fFar, fFov, fAspectRatio);

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override {
		Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, FG_BLACK);
		fTheta += 1.0f * fElapsedTime;

		//Rotate X, Y, Z
		mat4x4 matRotZ = Matrix_MakeRotationZ(fTheta * 0.5f);
		mat4x4 matRotX = Matrix_MakeRotationX(fTheta);
		mat4x4 matRotY = Matrix_MakeRotationY(fTheta);

		mat4x4 matTrans;
		matTrans = Matrix_MakeTranslation(0.0f, 0.0f, 12.0f);

		mat4x4 matWorld;
		matWorld = Matrix_MakeIdentity();
		matWorld = Matrix_MultiplyMatrix(matRotZ, matRotY);
		matWorld = Matrix_MultiplyMatrix(matWorld, matTrans);

		vector<triangle>  vecTrianglesToRaster;

		//Draw Triangles
		for (auto tri : meshCube.tris) {
			triangle triProjected, triTransformed;

			triTransformed.p[0] = Matrix_MultiplyVector(matWorld, tri.p[0]);
			triTransformed.p[1] = Matrix_MultiplyVector(matWorld, tri.p[1]);
			triTransformed.p[2] = Matrix_MultiplyVector(matWorld, tri.p[2]);

			vec3d normal, line1, line2;

			line1 = Vector_Subtract(triTransformed.p[1], triTransformed.p[0]);
			line2 = Vector_Subtract(triTransformed.p[2], triTransformed.p[0]);

			normal = Vector_CrossProduct(line1, line2);

			normal = Vector_Normalize(normal);

			//float l = sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
			//normal.x /= l;
			//normal.y /= l;
			//normal.z /= l;

			//if the normal vector of the triangle is less than 0 that means the triangles normal is pointing towards the camera, meaining it is visible
			// using the dot product formula we can see whether the vectors are related in this way

			vec3d vCameraRay = Vector_Subtract(triTransformed.p[0], vCamera);

			if(Vector_DotProduct(normal, vCameraRay) < 0.0f)
			{
				//Illumination

				vec3d light_direction = { -1.0f, 1.0f, -1.0f };
				light_direction = Vector_Normalize(light_direction);

				//float dp = Vector_DotProduct(light_direction, normal);
				float dp = max(0.1f, Vector_DotProduct(light_direction, normal));

				CHAR_INFO c = GetColour(dp);
				triTransformed.col = c.Attributes;
				triTransformed.sym = c.Char.UnicodeChar;

				// project from 3d to 2d
				triProjected.p[0] = Matrix_MultiplyVector(matProj, triTransformed.p[0]);
				triProjected.p[1] = Matrix_MultiplyVector(matProj, triTransformed.p[1]);
				triProjected.p[2] = Matrix_MultiplyVector(matProj, triTransformed.p[2]);
				triProjected.sym = triTransformed.sym;
				triProjected.col = triTransformed.col;

				//normalize normally now that we dont do it within multupply vector
				triProjected.p[0] = Vector_Div(triProjected.p[0], triProjected.p[0].w);
				triProjected.p[1] = Vector_Div(triProjected.p[1], triProjected.p[1].w);
				triProjected.p[2] = Vector_Div(triProjected.p[2], triProjected.p[2].w);

				// SCALE INTO VIEW
				vec3d vOffsetView = { 1, 1, 0 };
				triProjected.p[0] = Vector_Add(triProjected.p[0], vOffsetView);
				triProjected.p[1] = Vector_Add(triProjected.p[1], vOffsetView);
				triProjected.p[2] = Vector_Add(triProjected.p[2], vOffsetView);

				triProjected.p[0].x *= 0.5f * (float)ScreenWidth();
				triProjected.p[0].y *= 0.5f * (float)ScreenHeight();

				triProjected.p[1].x *= 0.5f * (float)ScreenWidth();
				triProjected.p[1].y *= 0.5f * (float)ScreenHeight();

				triProjected.p[2].x *= 0.5f * (float)ScreenWidth();
				triProjected.p[2].y *= 0.5f * (float)ScreenHeight();

				//store triangles
				vecTrianglesToRaster.push_back(triProjected);

			}

			//sort based on the avg Z component of triangle points
			sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](triangle& t1, triangle& t2)
			{
					float averageZ1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
					float averageZ2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;

					return averageZ1 > averageZ2;
			});

			for (auto &triProjected  : vecTrianglesToRaster) {
				FillTriangle(triProjected.p[0].x, triProjected.p[0].y, triProjected.p[1].x, triProjected.p[1].y
					, triProjected.p[2].x, triProjected.p[2].y, triProjected.sym,
					triProjected.col);

				//DrawTriangle(triProjected.p[0].x, triProjected.p[0].y, triProjected.p[1].x, triProjected.p[1].y
				//	, triProjected.p[2].x, triProjected.p[2].y, PIXEL_SOLID,
				//	FG_WHITE);
			}

		}
		return true;
	}

};

int main()
{
	olcEngine3D demo;
	if (demo.ConstructConsole(256, 240, 4, 4)) // 256 characters wide, 240 characters high, each character will be 4 by 4 pixels 
	{
		demo.Start();
	}
}

