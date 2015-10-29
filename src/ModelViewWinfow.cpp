#include "ModelViewWindow.h"

#include <iostream>
#include <string>
#include <SOIL.h>

#include "geohelper.h"
#include "oshelper.h"

namespace BRDFModel
{
	void Mesh::Draw(NPGLHelper::Effect &effect)
	{
		unsigned int diffuseNr = 1;
		unsigned int specularNr = 1;
		for (unsigned int i = 0; i < m_textures.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			effect.SetInt("", i);
			glBindTexture(GL_TEXTURE_2D, m_textures[i].id);
		}
	}

	void Mesh::SetupMesh()
	{
		glGenVertexArrays(1, &m_iVAO);
		glGenBuffers(1, &m_iVBO);
		glGenBuffers(1, &m_iEBO);

		glBindVertexArray(m_iVAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_iVBO);
		glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), m_vertices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_iEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(GLuint), m_indices.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, normal));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, tangent));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, texCoords));

		glBindVertexArray(0);

	}


	Model::Model()
	{

	}

	void Model::Draw(NPGLHelper::Effect &effect)
	{
		for (auto &mesh : m_meshes)
		{
			mesh.Draw(effect);
		}
	}

	bool Model::LoadModel(const char* path)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
		if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			NPOSHelper::CreateMessageBox("Cannot load model.", "Model loading", NPOSHelper::MSGBOX_OK);
			return false;
		}
		std::string sPath = path;
		m_sDirectory = sPath.substr(0, sPath.find_last_of('/'));
		ProcessNode(scene->mRootNode, scene);

		return true;
	}

	void Model::ProcessNode(aiNode* node, const aiScene* scene)
	{
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			Mesh loadedMesh = ProcessMesh(scene->mMeshes[node->mMeshes[i]], scene);
			m_meshes.push_back(loadedMesh);
		}

		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(node->mChildren[i], scene);
		}
	}

	Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
	{
		std::vector<Vertex> vertices;
		std::vector<GLuint> indices;
		std::vector<Texture> textures;

		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			vertex.position._x = mesh->mVertices[i].x;
			vertex.position._y = mesh->mVertices[i].y;
			vertex.position._z = mesh->mVertices[i].z;
			vertex.normal._x = mesh->mNormals[i].x;
			vertex.normal._y = mesh->mNormals[i].y;
			vertex.normal._z = mesh->mNormals[i].z;
			vertex.tangent._x = mesh->mTangents[i].x;
			vertex.tangent._y = mesh->mTangents[i].y;
			vertex.tangent._z = mesh->mTangents[i].z;
			if (mesh->mTextureCoords[0])
			{
				vertex.texCoords._x = mesh->mTextureCoords[0][i].x;
				vertex.texCoords._y = mesh->mTextureCoords[0][i].y;
			}
			vertices.push_back(vertex);
		}

		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
			{
				indices.push_back(face.mIndices[j]);
			}
		}

		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			std::vector<Texture> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
			std::vector<Texture> specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
			
		}

		return Mesh(vertices, indices, textures);

	}

	std::vector<Texture> Model::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, const char* typeName)
	{
		std::vector<Texture> textures;
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);
			Texture texture;
			std::string fullpath = m_sDirectory + str.C_Str();
			if (NPGLHelper::loadTextureFromFile(fullpath.c_str(), texture.id, GL_CLAMP, GL_CLAMP, GL_LINEAR, GL_LINEAR))
			{
				texture.name = typeName;
				textures.push_back(texture);
			}
		}

		return textures;
	}
}

ModelViewWindow::ModelViewWindow(const char* name, const int sizeW, const int sizeH)
	: Window(name, sizeW, sizeH)
	, m_Cam(1.f, 0.f, M_PI * 0.25f)
	, m_fCamSenX(0.01f)
	, m_fCamSenY(.005f)
	, m_fInSenX(.001f)
	, m_fInSenY(0.0005f)
	, m_fInPitch(0.f)
	, m_fInYaw(M_PI*0.25f)
	, m_bIsCamRotate(false)
	, m_bIsInRotate(false)
	, m_fZoomMin(0.25f)
	, m_fZoomMax(4.0f)
	, m_fZoomSen(0.1f)
	, m_pBRDFVisEffect(nullptr)
	, m_bIsLoadTexture(false)
{
}

ModelViewWindow::~ModelViewWindow()
{
}

int ModelViewWindow::OnInit()
{
	m_AxisLine[0].Init(m_pShareContent);
	m_AxisLine[1].Init(m_pShareContent);
	m_AxisLine[2].Init(m_pShareContent);
	m_InLine.Init(m_pShareContent);

	m_pBRDFVisEffect = m_pShareContent->GetEffect("BRDFVisEffect");
	if (!m_pBRDFVisEffect->GetIsLinked())
	{
		m_pBRDFVisEffect->initEffect();
		m_pBRDFVisEffect->attachShaderFromFile("../shader/BRDFVisualizeVS.glsl", GL_VERTEX_SHADER);
		m_pBRDFVisEffect->attachShaderFromFile("../shader/BRDFVisualizePS.glsl", GL_FRAGMENT_SHADER);
		m_pBRDFVisEffect->linkEffect();
	}

	testObject.SetGeometry(NPGeoHelper::GetSlicedHemisphereShape(1.f, 64, 64));

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	OpenModelData();

	return 0;
}

int ModelViewWindow::OnTick(const float deltaTime)
{
	// Camera control - bgn
	glm::vec2 cursorMoved = m_v2CurrentCursorPos - m_v2LastCursorPos;
	if (m_bIsCamRotate)
	{
		m_Cam.AddPitch(-cursorMoved.x * m_fCamSenX);
		m_Cam.AddYaw(cursorMoved.y * m_fCamSenY);
	}
	if (m_bIsInRotate)
	{
		m_fInPitch = m_fInPitch + cursorMoved.x * m_fInSenX;
		m_fInYaw = (m_fInYaw - cursorMoved.y * m_fInSenY);
		if (m_fInYaw < 0) m_fInYaw = 0.f;
		if (m_fInYaw > M_PI * 0.5f) m_fInYaw = M_PI * 0.5f;
		while (m_fInPitch < 0) m_fInPitch = m_fInPitch + M_PI * 2.f;
		while (m_fInPitch > M_PI * 2.f) m_fInPitch -= M_PI * 2.f;
	}
	if (abs(m_fScrollY) > 1E-9)
	{
		float curZoom = m_Cam.GetRadius();
		curZoom += m_fScrollY * m_fZoomSen;
		curZoom = (curZoom < m_fZoomMin) ? m_fZoomMin : (curZoom > m_fZoomMax) ? m_fZoomMax : curZoom;
		m_Cam.SetRadius(curZoom);
		m_fScrollY = 0.f;
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

	if (m_bIsLoadTexture)
	{
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
	}

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

	glfwSwapBuffers(GetGLFWWindow());

	return 0;
}

void ModelViewWindow::OnTerminate()
{
	testObject.ClearGeometry();
}

void ModelViewWindow::OnHandleInputMSG(const INPUTMSG &msg)
{
	switch (msg.type)
	{
	case Window::INPUTMSG_KEYBOARDKEY:
		if (msg.key == GLFW_KEY_ESCAPE && msg.action == GLFW_PRESS)
			glfwSetWindowShouldClose(m_pWindow, GL_TRUE);
		if (msg.key == GLFW_KEY_O && msg.action == GLFW_PRESS)
			OpenModelData();
		break;
	case Window::INPUTMSG_MOUSEKEY:
		if (msg.key == GLFW_MOUSE_BUTTON_RIGHT)
		{
			m_bIsCamRotate = (msg.action == GLFW_PRESS);
		}
		if (msg.key == GLFW_MOUSE_BUTTON_LEFT)
		{
			m_bIsInRotate = (msg.action == GLFW_PRESS);
		}
		break;
	case Window::INPUTMSG_MOUSECURSOR:
		m_v2CurrentCursorPos.x = msg.xpos;
		m_v2CurrentCursorPos.y = msg.ypos;
		break;
	case Window::INPUTMSG_MOUSESCROLL:
		m_fScrollY = msg.yoffset;
		break;
	}
}

void ModelViewWindow::OpenModelData()
{
	std::string file = NPOSHelper::BrowseFile("All\0*.*\0Text\0*.TXT\0");
	if (file.empty())
		return;

	int width, height;
	unsigned char* image = SOIL_load_image(file.c_str(), &width, &height, 0, SOIL_LOAD_RGB);
	if (!image)
	{
		std::string message = "Cannot load file ";
		message = message + file;
		NPOSHelper::CreateMessageBox(message.c_str(), "Load BRDF Data Failure", NPOSHelper::MSGBOX_OK);
		return;
	}

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

	m_bIsLoadTexture = true;
}