#include "glhelper.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <assert.h>

GLEWContext* glewGetContext()
{
	if (NPGLHelper::App::g_pMainApp)
	{
		NPGLHelper::Window* curWin = NPGLHelper::App::g_pMainApp->GetCurrentWindow();
		return (curWin) ? curWin->GetGLEWContext() : nullptr;
	}
	return nullptr;
}

namespace NPGLHelper
{
	bool loadASCIIFromFile(std::string file, std::string &content)
	{
		std::ifstream t(file);
		if (!t.good())
			return false;

		std::stringstream buffer;
		buffer << t.rdbuf();
		content = buffer.str();
		return true;
	}

	bool createShaderFromFile(std::string file, GLuint type, GLuint &result)
	{
		std::string shaderSource;
		if (loadASCIIFromFile(file, shaderSource))
		{
			//std::cout << "Loaded shader" << std::endl << shaderSource << std::endl;
			GLuint shader;
			shader = glCreateShader(type);
			const char *shaderSource_cstr = shaderSource.c_str();
			glShaderSource(shader, 1, &shaderSource_cstr, NULL);
			glCompileShader(shader);
			result = shader;

			std::string info;
			if (!checkShaderError(shader, GL_COMPILE_STATUS,info))
			{
				std::cout << "[!!!]SHADER::COMPILATION_FAILED " << file << std::endl << info << std::endl;
				return false;
			}
			else
			{
				std::cout << "SHADER:COMPILATION_SUCCEED " << file << std::endl;
			}
			return true;
		}
		return false;
	}

	bool checkShaderError(GLuint shader, GLuint checking, std::string &info)
	{
		GLint success;
		GLchar infoLog[512];
		glGetShaderiv(shader, checking, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 512, NULL, infoLog);
			info = infoLog;
		}
		return success != 0;
	}

	bool checkProgramError(GLuint program, GLuint checking, std::string &info)
	{
		GLint success;
		GLchar infoLog[512];
		glGetProgramiv(program, checking, &success);
		if (!success)
		{
			glGetProgramInfoLog(program, 512, NULL, infoLog);
			info = infoLog;
		}
		return success != 0;
	}

	RenderObject::RenderObject()
		: m_iVAO(-1)
		, m_iVBO(-1)
		, m_iEBO(-1)
		, m_iIndicesSize(0)
	{

	}

	RenderObject::~RenderObject()
	{

	}

	void RenderObject::SetGeometry(const NPGeoHelper::Geometry& geo)
	{
		std::vector<GLfloat> vertices;
		for (auto it = geo.vertices.begin(); it != geo.vertices.end(); it++)
		{
			vertices.push_back(it->pos.x);
			vertices.push_back(it->pos.y);
			vertices.push_back(it->pos.z);
			vertices.push_back(it->tex.x);
			vertices.push_back(it->tex.y);
		}

		m_iIndicesSize = geo.indices.size();
		std::vector<GLuint> indices;
		for (auto it = geo.indices.begin(); it != geo.indices.end(); it++)
		{
			indices.push_back(*it);
		}


		if (m_iVAO >= 0)
		{
			ClearGeometry();
		}

		glGenBuffers(1, &m_iVBO);
		glGenBuffers(1, &m_iEBO);
		glGenVertexArrays(1, &m_iVAO);
		glBindVertexArray(m_iVAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_iVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_iEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glBindVertexArray(0);
	}

	void RenderObject::ClearGeometry()
	{
		if (m_iVAO >= 0)
		{
			glDeleteVertexArrays(1, &m_iVAO);
		}
		m_iVAO = -1;

		if (m_iVBO >= 0)
		{
			glDeleteBuffers(1, &m_iVBO);
		}
		m_iVBO = -1;

		if (m_iEBO >= 0)
		{
			glDeleteBuffers(1, &m_iEBO);
		}
		m_iEBO = -1;
	}

	Effect::Effect()
		: m_bIsLinked(false)
		, m_iProgram(0)
	{

	}

	Effect::~Effect()
	{
		deleteAttachedShaders();

		if (m_iProgram >= 0)
		{
			glDeleteProgram(m_iProgram);
			m_iProgram = -1;
		}
	}

	void Effect::initEffect()
	{
		if (m_iProgram == 0)
			m_iProgram = glCreateProgram();
	}

	void Effect::attachShaderFromFile(const char* filename, const GLuint type)
	{
		assert(m_iProgram >= 0);
		GLuint shader;
		if (NPGLHelper::createShaderFromFile(filename, type, shader))
		{
			glAttachShader(m_iProgram, shader);
			m_vAttachedShader.push_back(shader);
		}
	}

	void Effect::deleteAttachedShaders()
	{
		for (auto it = m_vAttachedShader.begin(); it != m_vAttachedShader.end(); it++)
		{
			glDeleteShader(*it);
		}
		m_vAttachedShader.clear();
	}

	bool Effect::linkEffect()
	{
		assert(m_iProgram >= 0);
		glLinkProgram(m_iProgram);
		deleteAttachedShaders();

		std::string pLinkInfo;
		if (!NPGLHelper::checkProgramError(m_iProgram, GL_LINK_STATUS, pLinkInfo))
		{
			std::cout << "[!!!]SHADER::LINK_FAILED" << std::endl << pLinkInfo << std::endl;
			return false;
		}
		else
		{
			std::cout << "SHADER::LINK_SUCCEED" << std::endl;
		}

		m_bIsLinked = true;

		return true;
	}

	bool Effect::activeEffect()
	{
		assert(m_iProgram >= 0);
		glUseProgram(m_iProgram);

		return true;
	}

	bool Effect::deactiveEffect()
	{
		glUseProgram(0);
		return true;
	}

	void Effect::SetMatrix(const char* var, const float* mat)
	{
		assert(m_iProgram >= 0);
		GLuint matLoc = glGetUniformLocation(m_iProgram, var);
		glUniformMatrix4fv(matLoc, 1, GL_FALSE, mat);
	}

	void Effect::SetInt(const char* var, const int value)
	{
		assert(m_iProgram >= 0);
		GLuint valueLoc = glGetUniformLocation(m_iProgram, var);
		glUniform1i(valueLoc, value);
	}

	void Effect::SetFloat(const char* var, const float value)
	{
		assert(m_iProgram >= 0);
		GLuint valueLoc = glGetUniformLocation(m_iProgram, var);
		glUniform1f(valueLoc, value);
	}

	Window::Window(const char* name, const int sizeW, const int sizeH)
	{

	}

	Window::~Window()
	{

	}


	App::App(const int sizeW, const int sizeH)
		: m_iSizeW(sizeW)
		, m_iSizeH(sizeH)
		, m_pWindow(nullptr)
		, m_bIsInit(false)
		, m_fDeltaTime(0.f)
		, m_uiCurrentWindowID(0)
		, m_uiCurrentMaxID(0)
		, m_bForceShutdown(false)
	{
		g_pMainApp = this;
	}

	App::~App()
	{
		g_pMainApp = NULL;
	}

	int App::Run(Window* initWindow)
	{
		if (GLInit() < 0)
			return -1;
		AttachWindow(initWindow);

		while (!WindowsUpdate())
		{
			glfwPollEvents();
			float currentTime = glfwGetTime();

			if (m_fLastTime > 0.f)
				m_fDeltaTime = currentTime - m_fLastTime;

			for (auto it = m_mapWindows.begin(); it != m_mapWindows.end(); it++)
			{
				it->second->OnTick(GetDeltaTime());
				glfwSwapBuffers(it->second->GetGLFWWindow());
			}

			m_fLastTime = currentTime;
		}
		glfwTerminate();
		return 0;
	}

	void App::Shutdown()
	{
		m_bForceShutdown = true;
	}

	void App::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
	{
		for (auto& mapWin : m_mapWindows)
		{
			if (mapWin.second->GetGLFWWindow() == window)
			{
				mapWin.second->KeyCallback(key, scancode, action, mode);
				break;
			}
		}
	}

	void App::MouseKeyCallback(GLFWwindow* window, int key, int action, int mode)
	{
		for (auto& mapWin : m_mapWindows)
		{
			if (mapWin.second->GetGLFWWindow() == window)
			{
				mapWin.second->MouseKeyCallback(key, action, mode);
				break;
			}
		}
	}

	void App::MouseCursorCallback(GLFWwindow* window, double xpos, double ypos)
	{
		for (auto& mapWin : m_mapWindows)
		{
			if (mapWin.second->GetGLFWWindow() == window)
			{
				mapWin.second->MouseCursorCallback(xpos, ypos);
				break;
			}
		}
	}

	App* App::g_pMainApp = nullptr;
	void App::GlobalKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
	{
		if (g_pMainApp)
			g_pMainApp->KeyCallback(window, key, scancode, action, mode);
	}

	void App::GlobalMouseKeyCallback(GLFWwindow *window, int key, int action, int mode)
	{
		if (g_pMainApp)
			g_pMainApp->MouseKeyCallback(window, key, action, mode);
	}

	void App::GlobalMouseCursorCallback(GLFWwindow* window, double xpos, double ypos)
	{
		if (g_pMainApp)
			g_pMainApp->MouseCursorCallback(window, xpos, ypos);
	}

	int App::GLInit()
	{
		if (m_bIsInit || m_pWindow)
			return 0;

		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

		m_bIsInit = true;
		return 0;
	}

	bool App::WindowsUpdate()
	{
		std::vector<unsigned int> removeList;
		for (auto it = m_mapWindows.begin(); it != m_mapWindows.end(); it++)
		{
			if (glfwWindowShouldClose(it->second->GetGLFWWindow()) || m_bForceShutdown)
			{
				it->second->OnTerminate();
				if (it->second->GetGLEWContext())
				{
					delete it->second->m_pGLEWContext;
					it->second->m_pGLEWContext = NULL;
				}
				if (it->second->GetGLFWWindow())
				{
					glfwDestroyWindow(it->second->GetGLFWWindow());
					delete it->second->m_pWindow;
					it->second->m_pWindow = NULL;
				}
				if (it->second)
				{
					delete it->second;
					it->second = NULL;
				}
				removeList.push_back(it->first);
			}
		}

		for (auto it = removeList.begin(); it != removeList.end(); it++)
		{
			m_mapWindows.erase(*it);
		}

		return (m_mapWindows.size() > 0);
	}

	bool App::AttachWindow(Window* window)
	{
		if (!window)
			return false;

		unsigned int prevWinId = m_uiCurrentWindowID;
		window->m_uiID = ++m_uiCurrentMaxID;
		window->m_pWindow = glfwCreateWindow(window->m_iSizeW, window->m_iSizeH, window->m_sName.c_str(), nullptr, nullptr);
		if (!window->m_pWindow)
		{
			std::cout << "Failed to create GLFW for window " << window->m_sName << std::endl;
			return false;
		}
		window->m_pGLEWContext = new GLEWContext();
		if (!window->m_pGLEWContext)
		{
			std::cout << "Failed to create GLEW Context for window " << window->m_sName << std::endl;
			return false;
		}
		SetCurrentWindow(window->m_uiID);

		glfwSetKeyCallback(window->m_pWindow, GlobalKeyCallback);
		glfwSetMouseButtonCallback(window->m_pWindow, GlobalMouseKeyCallback);
		glfwSetCursorPosCallback(window->m_pWindow, GlobalMouseCursorCallback);

		glewExperimental = GL_TRUE;
		if (glewInit() != GLEW_OK)
		{
			std::cout << "Failed to initialize GLEW for window " << window->m_sName << std::endl;
			if (prevWinId > 0)
				SetCurrentWindow(prevWinId);
			return false;
		}

		glViewport(0, 0, window->m_iSizeW, window->m_iSizeH);

		window->m_uiID = ++m_uiCurrentMaxID;
		m_mapWindows[window->m_uiID] = window;

		window->OnInit();

		if (prevWinId > 0)
			SetCurrentWindow(prevWinId);
		return true;
	}

	void App::SetCurrentWindow(const unsigned int id)
	{
		m_uiCurrentWindowID = id;
		glfwMakeContextCurrent(GetCurrentWindow()->m_pWindow);
	}

	Window* App::GetCurrentWindow()
	{
		if (!m_uiCurrentWindowID)
			return nullptr;
		return m_mapWindows[m_uiCurrentWindowID];
	}

	Effect DebugLine::m_gEffect;

	DebugLine::DebugLine()
		: m_v3Start()
		, m_v3End()
		, m_v3Color(1.f,0.f,0.f)
		, m_iVAO(-1)
		, m_iVBO(-1)
	{

	}

	DebugLine::~DebugLine()
	{

	}

	void DebugLine::Init()
	{
		if (!m_gEffect.GetIsLinked())
		{
			m_gEffect.initEffect();
			m_gEffect.attachShaderFromFile("../shader/debugLineVS.glsl", GL_VERTEX_SHADER);
			m_gEffect.attachShaderFromFile("../shader/debugLinePS.glsl", GL_FRAGMENT_SHADER);
			m_gEffect.linkEffect();
		}

		UpdateBuffer();
		glGenBuffers(1, &m_iVBO);
		glGenVertexArrays(1, &m_iVAO);
		glBindVertexArray(m_iVAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_iVBO);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glBindVertexArray(0);
	}

	void DebugLine::Draw(const NPGeoHelper::vec3& start, const NPGeoHelper::vec3& end, const NPGeoHelper::vec3& color
		, const float* viewMat, const float* projMat)
	{
		if (start != m_v3Start || end != m_v3End || color != m_v3Color)
		{
			m_v3Start = start;
			m_v3End = end;
			m_v3Color = color;
			UpdateBuffer();
		}

		if (m_v3Start == m_v3End)
			return;

		m_gEffect.activeEffect();
		m_gEffect.SetMatrix("projection", projMat);
		m_gEffect.SetMatrix("view", viewMat);

		glBindVertexArray(m_iVAO);
		glDrawArrays(GL_LINE_STRIP, 0, 2);
		glBindVertexArray(0);

		m_gEffect.deactiveEffect();
	}

	void DebugLine::UpdateBuffer()
	{
		std::vector<GLfloat> vertices;
		vertices.push_back(m_v3Start.x);
		vertices.push_back(m_v3Start.y);
		vertices.push_back(m_v3Start.z);
		vertices.push_back(m_v3Color.x);
		vertices.push_back(m_v3Color.y);
		vertices.push_back(m_v3Color.z);
		vertices.push_back(m_v3End.x);
		vertices.push_back(m_v3End.y);
		vertices.push_back(m_v3End.z);
		vertices.push_back(m_v3Color.x);
		vertices.push_back(m_v3Color.y);
		vertices.push_back(m_v3Color.z);

		glBindBuffer(GL_ARRAY_BUFFER, m_iVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices.size(), vertices.data(), GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}