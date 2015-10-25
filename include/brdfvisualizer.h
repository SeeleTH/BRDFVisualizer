#ifndef BRDFVISUALIZER_H
#define BRDFVISUALIZER_H

#include "glhelper.h"
#include "camhelper.h"
#include "mathhelper.h"

class BRDFVisualizer : public NPGLHelper::App
{
public:
	BRDFVisualizer(const int sizeW = 800, const int sizeH = 600);
	~BRDFVisualizer();

	virtual void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
	virtual void MouseKeyCallback(GLFWwindow* window, int key, int action, int mode);
	virtual void MouseCursorCallback(GLFWwindow* window, double xpos, double ypos);

protected:
	virtual int Init();
	virtual int Tick();
	virtual void Terminate();

	GLuint m_iBRDFEstTex;
	GLuint VAO;
	NPGLHelper::Effect testEffect;
	NPGLHelper::RenderObject testObject;
	NPCamHelper::RotateCamera m_Cam;

	bool m_bIsCamRotate, m_bIsInRotate;
	float m_fCamSenX, m_fCamSenY;
	float m_fInSenX, m_fInSenY;
	float m_fInPitch, m_fInYaw;
	glm::vec2 m_v2LastCursorPos;
	glm::vec2 m_v2CurrentCursorPos;
	NPGLHelper::DebugLine m_InLine;
	NPGLHelper::DebugLine m_AxisLine[3];
};

#endif