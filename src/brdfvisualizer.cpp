#include "brdfvisualizer.h"

#include <iostream>
#include <string>
#include <SOIL.h>

#include "geohelper.h"

//#include <Windows.h>
//#include <Commdlg.h>
//
//OPENFILENAME ofn;
//char szFile[100];


int main()
{
	//ZeroMemory(&ofn, sizeof(ofn));
	//ofn.lStructSize = sizeof(ofn);
	//ofn.hwndOwner = NULL;
	//ofn.lpstrFile = szFile;
	//ofn.lpstrFile[0] = '\0';
	//ofn.nMaxFile = sizeof(szFile);
	//ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
	//ofn.nFilterIndex = 1;
	//ofn.lpstrFileTitle = NULL;
	//ofn.nMaxFileTitle = 0;
	//ofn.lpstrInitialDir = NULL;
	//ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	//GetOpenFileName(&ofn);

	//// Now simpley display the file name 
	//MessageBox(NULL, ofn.lpstrFile, "File Name", MB_OK);

	NPGLHelper::App mainApp;
	return mainApp.Run(new BRDFVisualizer("BRDF Visualizer", 800, 600));
}


BRDFVisualizer::BRDFVisualizer(const char* name, const int sizeW, const int sizeH)
	: Window(name, sizeW, sizeH)
	, m_Cam(1.f, 0.f, M_PI * 0.25f)
	, m_fCamSenX(10.f)
	, m_fCamSenY(5.f)
	, m_fInSenX(1.f)
	, m_fInSenY(0.5f)
	, m_fInPitch(0.f)
	, m_fInYaw(M_PI*0.25f)
	, m_bIsCamRotate(false)
	, m_bIsInRotate(false)
	, m_pBRDFVisEffect(nullptr)
{
}

BRDFVisualizer::~BRDFVisualizer()
{
}

void BRDFVisualizer::KeyCallback(int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(m_pWindow, GL_TRUE);
}

void BRDFVisualizer::MouseKeyCallback(int key, int action, int mode)
{
	if (key == GLFW_MOUSE_BUTTON_RIGHT)
	{
		m_bIsCamRotate = (action == GLFW_PRESS);
	}
	if (key == GLFW_MOUSE_BUTTON_LEFT)
	{
		m_bIsInRotate = (action == GLFW_PRESS);
	}
}

void BRDFVisualizer::MouseCursorCallback(double xpos, double ypos)
{
	m_v2CurrentCursorPos.x = xpos;
	m_v2CurrentCursorPos.y = ypos;
}

int BRDFVisualizer::OnInit()
{
	m_AxisLine[0].Init(m_pShareContent);
	m_AxisLine[1].Init(m_pShareContent);
	m_AxisLine[2].Init(m_pShareContent);
	m_InLine.Init(m_pShareContent);

	m_pBRDFVisEffect = m_pShareContent->GetEffect("BRDFVisEffect");
	if (!m_pBRDFVisEffect->GetIsLinked())
	{
		m_pBRDFVisEffect->initEffect();
		m_pBRDFVisEffect->attachShaderFromFile("../shader/SimpleVS.glsl", GL_VERTEX_SHADER);
		m_pBRDFVisEffect->attachShaderFromFile("../shader/SimplePS.glsl", GL_FRAGMENT_SHADER);
		m_pBRDFVisEffect->linkEffect();
	}

	int width, height;
	unsigned char* image = SOIL_load_image("brdfest.bmp", &width, &height, 0, SOIL_LOAD_RGB);
	glGenTextures(1, &m_iBRDFEstTex);
	glBindTexture(GL_TEXTURE_2D, m_iBRDFEstTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	testObject.SetGeometry(NPGeoHelper::GetSlicedHemisphereShape(1.f, 64, 64));

	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	return 0;
}

int BRDFVisualizer::OnTick(const float deltaTime)
{
	// Camera control - bgn
	glm::vec2 cursorMoved = m_v2CurrentCursorPos - m_v2LastCursorPos;
	if (m_bIsCamRotate)
	{
		m_Cam.AddPitch(-cursorMoved.x * m_fCamSenX * deltaTime);
		m_Cam.AddYaw(cursorMoved.y * m_fCamSenY * deltaTime);
	}
	if (m_bIsInRotate)
	{
		m_fInPitch = m_fInPitch + cursorMoved.x * m_fInSenX * deltaTime;
		m_fInYaw = (m_fInYaw - cursorMoved.y * m_fInSenY * deltaTime);
		if (m_fInYaw < 0) m_fInYaw = 0.f;
		if (m_fInYaw > M_PI * 0.5f) m_fInYaw = M_PI * 0.5f;
		while (m_fInPitch < 0) m_fInPitch = m_fInPitch + M_PI * 2.f;
		while (m_fInPitch > M_PI * 2.f) m_fInPitch -= M_PI * 2.f;
	}
	m_v2LastCursorPos = m_v2CurrentCursorPos;
	// Camera control - end

	glm::mat4 proj, view, model;
	proj = glm::perspective(45.0f, (float)m_iSizeW / (float)m_iSizeH, 0.1f, 100.0f);
	view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
	model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	NPMathHelper::Mat4x4 myProj = NPMathHelper::Mat4x4::PerspectiveProjection(M_PI * 0.5f, (float)m_iSizeW / (float)m_iSizeH, 0.1f, 100.0f);

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_pBRDFVisEffect->activeEffect();
	m_pBRDFVisEffect->SetInt("n_th", 16);
	m_pBRDFVisEffect->SetInt("n_ph", 64);
	m_pBRDFVisEffect->SetFloat("i_yaw", m_fInYaw);
	m_pBRDFVisEffect->SetFloat("i_pitch", m_fInPitch);
	m_pBRDFVisEffect->SetMatrix("projection", myProj.GetDataColumnMajor());
	//m_pBRDFVisEffect->SetMatrix("projection", glm::value_ptr(proj));
	m_pBRDFVisEffect->SetMatrix("view", m_Cam.GetViewMatrix());
	//m_pBRDFVisEffect->SetMatrix("view", glm::value_ptr(view));
	m_pBRDFVisEffect->SetMatrix("model", glm::value_ptr(model));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_iBRDFEstTex);
	m_pBRDFVisEffect->SetInt("brdfTexture", 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_iBRDFEstTex);
	m_pBRDFVisEffect->SetInt("dTexture", 1);

	glBindVertexArray(testObject.GetVAO());
	glDrawElements(GL_TRIANGLES, testObject.GetIndicesSize(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	{
		m_AxisLine[0].Draw(NPGeoHelper::vec3(), NPGeoHelper::vec3(1.f, 0.f, 0.f), NPGeoHelper::vec3(1.0f, 0.f, 0.f)
			, m_Cam.GetViewMatrix(), glm::value_ptr(proj));
		m_AxisLine[1].Draw(NPGeoHelper::vec3(), NPGeoHelper::vec3(0.f, 1.f, 0.f), NPGeoHelper::vec3(0.0f, 1.f, 0.f)
			, m_Cam.GetViewMatrix(), glm::value_ptr(proj));
		m_AxisLine[2].Draw(NPGeoHelper::vec3(), NPGeoHelper::vec3(0.f, 0.f, 1.f), NPGeoHelper::vec3(0.0f, 0.f, 1.f)
			, m_Cam.GetViewMatrix(), glm::value_ptr(proj));
	}

	{
		glm::vec3 InLineEnd;
		InLineEnd.y = sin(m_fInYaw) * 10.f;
		InLineEnd.x = cos(m_fInYaw) * sin(m_fInPitch) * 10.f;
		InLineEnd.z = cos(m_fInYaw) * cos(m_fInPitch) * 10.f;
		m_InLine.Draw(NPGeoHelper::vec3(), NPGeoHelper::vec3(InLineEnd.x, InLineEnd.y, InLineEnd.z), NPGeoHelper::vec3(1.0f, 1.f, 1.f)
			, m_Cam.GetViewMatrix(), glm::value_ptr(proj));
	}

	return 0;
}

void BRDFVisualizer::OnTerminate()
{
	testObject.ClearGeometry();
}