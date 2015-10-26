#ifndef GLHELPER_H
#define GLHELPER_H

#define GLEW_STATIC
#define GLEW_MX
#include <GL\glew.h>

#include <GLFW\glfw3.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include <string>
#include <vector>
#include <map>

#include "geohelper.h"

GLEWContext* glewGetContext();

namespace NPGLHelper
{
	bool loadASCIIFromFile(std::string file, std::string &content);
	bool createShaderFromFile(std::string file, GLuint type, GLuint &result);

	bool checkShaderError(GLuint shader, GLuint checking, std::string &info);
	bool checkProgramError(GLuint program, GLuint checking, std::string &info);


	class RenderObject
	{
	public:
		RenderObject();
		~RenderObject();

		void SetGeometry(const NPGeoHelper::Geometry& geo);
		void ClearGeometry();
		GLuint GetVAO() { return m_iVAO; }
		GLsizei GetIndicesSize() { return m_iIndicesSize; }

	protected:
		GLuint m_iVAO;
		GLuint m_iVBO;
		GLuint m_iEBO;
		GLsizei m_iIndicesSize;
	};

	class Effect
	{
	public:
		Effect();
		~Effect();
		void initEffect();
		void attachShaderFromFile(const char* filename, const GLuint type);
		void deleteAttachedShaders();
		bool linkEffect();
		bool activeEffect();
		bool deactiveEffect();
		
		void SetMatrix(const char* var, const float* mat);
		void SetInt(const char* var, const int value);
		void SetFloat(const char* var, const float value);

		inline const bool GetIsLinked() { return m_bIsLinked; }

	protected:
		GLuint m_iProgram;
		bool m_bIsLinked;
		std::vector<GLuint> m_vAttachedShader;
	};

	class Window
	{
	public:
		friend class App;
		Window(const char* name, const int sizeW = 800, const int sizeH = 600);
		~Window();

		virtual int OnInit() = 0;
		virtual int OnTick(const float deltaTime) = 0;
		virtual void OnTerminate() = 0;

		virtual void KeyCallback(int key, int scancode, int action, int mode){}
		virtual void MouseKeyCallback(int key, int action, int mode){}
		virtual void MouseCursorCallback(double xpos, double ypos){}

		inline GLEWContext* GetGLEWContext() { return m_pGLEWContext; }
		inline GLFWwindow* GetGLFWWindow() { return m_pWindow; }

	protected:
		bool m_bIsInit;
		std::string m_sName;
		int m_iSizeW, m_iSizeH;
		GLFWwindow* m_pWindow;
		GLEWContext* m_pGLEWContext;
		unsigned int m_uiID;
	};

	class App
	{
	public:
		App(const int sizeW = 800, const int sizeH = 600);
		~App();

		int Run(Window* initWindow);
		void Shutdown();
		virtual void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode){}
		virtual void MouseKeyCallback(GLFWwindow* window, int key, int action, int mode){}
		virtual void MouseCursorCallback(GLFWwindow* window, double xpos, double ypos){}

		static App* g_pMainApp;
		static void GlobalKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
		static void GlobalMouseKeyCallback(GLFWwindow *window, int key, int action, int mode);
		static void GlobalMouseCursorCallback(GLFWwindow* window, double xpos, double ypos);

		inline const float GetDeltaTime() { return m_fDeltaTime; }

		bool AttachWindow(Window* window);
		void SetCurrentWindow(const unsigned int id);
		Window* GetCurrentWindow();

	protected:
		int GLInit();
		bool WindowsUpdate();

		bool m_bIsInit;
		int m_iSizeW, m_iSizeH;
		GLFWwindow* m_pWindow;

		std::map<unsigned int, Window*> m_mapWindows;
		unsigned int m_uiCurrentWindowID;
		unsigned int m_uiCurrentMaxID;
		bool m_bForceShutdown;

	private:
		float m_fDeltaTime;
		float m_fLastTime;
	};


	class DebugLine
	{
	public:
		DebugLine();
		~DebugLine();
		void Init();
		void Draw(const NPGeoHelper::vec3& start, const NPGeoHelper::vec3& end, const NPGeoHelper::vec3& color
			, const float* viewMat, const float* projMat);
	protected:
		void UpdateBuffer();

		NPGeoHelper::vec3 m_v3Start;
		NPGeoHelper::vec3 m_v3End;
		NPGeoHelper::vec3 m_v3Color;
		static Effect m_gEffect;

		GLuint m_iVAO;
		GLuint m_iVBO;
	};
}

#endif