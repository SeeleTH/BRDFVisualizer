#ifndef BRDFVISUALIZER_H
#define BRDFVISUALIZER_H

#include "glhelper.h"
#include "camhelper.h"
#include "mathhelper.h"

class BRDFVisualizer : public NPGLHelper::Window
{
public:
	BRDFVisualizer(const char* name, const int sizeW = 800, const int sizeH = 600);
	~BRDFVisualizer();

	virtual int OnInit();
	virtual int OnTick(const float deltaTime);
	virtual void OnTerminate();

	virtual void KeyCallback(int key, int scancode, int action, int mode);
	virtual void MouseKeyCallback(int key, int action, int mode);
	virtual void MouseCursorCallback(double xpos, double ypos);

protected:
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