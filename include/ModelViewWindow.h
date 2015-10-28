#ifndef MODELVIEWWINDOW_H
#define MODELVIEWWINDOW_H

#include "glhelper.h"
#include "camhelper.h"
#include "mathhelper.h"

class ModelViewWindow : public NPGLHelper::Window
{
public:
	ModelViewWindow(const char* name, const int sizeW = 800, const int sizeH = 600);
	virtual ~ModelViewWindow();

	virtual int OnInit();
	virtual int OnTick(const float deltaTime);
	virtual void OnTerminate();
	virtual void OnHandleInputMSG(const INPUTMSG &msg);

	void OpenModelData();

protected:
	GLuint m_iBRDFEstTex;
	bool m_bIsLoadTexture;
	NPGLHelper::Effect* m_pBRDFVisEffect;
	NPGLHelper::RenderObject testObject;
	NPCamHelper::RotateCamera m_Cam;

	bool m_bIsCamRotate, m_bIsInRotate;
	float m_fCamSenX, m_fCamSenY;
	float m_fInSenX, m_fInSenY;
	float m_fInPitch, m_fInYaw;
	float m_fZoomSen, m_fScrollY;
	float m_fZoomMin, m_fZoomMax;
	glm::vec2 m_v2LastCursorPos;
	glm::vec2 m_v2CurrentCursorPos;
	NPGLHelper::DebugLine m_InLine;
	NPGLHelper::DebugLine m_AxisLine[3];
};

#endif