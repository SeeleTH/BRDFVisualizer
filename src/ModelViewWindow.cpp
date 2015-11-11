#include "ModelViewWindow.h"

#include <iostream>
#include <string>
#include <SOIL.h>

#include "geohelper.h"
#include "oshelper.h"
#include "atbhelper.h"

namespace BRDFModel
{
	void Mesh::Draw(NPGLHelper::Effect &effect)
	{
		unsigned int diffuseNr = 1;
		unsigned int specularNr = 1;
		for (unsigned int i = 0; i < m_textures.size(); i++)
		{
			glActiveTexture(GL_TEXTURE1 + i);
			std::string texName = m_textures[i].name;
			if (m_textures[i].type == 0)
			{
				texName += std::to_string(diffuseNr++);
			}
			else
			{
				texName += std::to_string(specularNr++);
			}
			glBindTexture(GL_TEXTURE_2D, m_textures[i].id);
			effect.SetInt(texName.c_str(), i + 1);
		}
		glBindVertexArray(m_iVAO);
		glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
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
			mesh->Draw(effect);
		}
	}

	bool Model::LoadModel(const char* path)
	{
		Assimp::Importer importer;
		std::string sPath = path;
		const aiScene* scene = importer.ReadFile(sPath.c_str(), aiProcess_Triangulate | aiProcess_FlipUVs 
			| aiProcess_CalcTangentSpace | aiProcess_GenNormals);
		if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			NPOSHelper::CreateMessageBox("Cannot load model.", "Model loading", NPOSHelper::MSGBOX_OK);
			return false;
		}
		m_sDirectory = sPath.substr(0, sPath.find_last_of('\\'));
		ProcessNode(scene->mRootNode, scene);

		return true;
	}

	void Model::ProcessNode(aiNode* node, const aiScene* scene)
	{
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			Mesh *loadedMesh = ProcessMesh(scene->mMeshes[node->mMeshes[i]], scene);
			m_meshes.push_back(loadedMesh);
		}

		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(node->mChildren[i], scene);
		}
	}

	Mesh* Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
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
			std::vector<Texture> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", 0);
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
			std::vector<Texture> specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular", 1);
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
			
		}

		return new Mesh(vertices, indices, textures);

	}

	std::vector<Texture> Model::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, const char* name, const unsigned int typeId)
	{
		std::vector<Texture> textures;
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);
			Texture texture;
			std::string fullpath = m_sDirectory + "\\" + str.C_Str();
			if (NPGLHelper::loadTextureFromFile(fullpath.c_str(), texture.id, GL_CLAMP, GL_CLAMP, GL_LINEAR, GL_LINEAR))
			{
				texture.name = name;
				texture.type = typeId;
				textures.push_back(texture);
			}
		}

		return textures;
	}
}

void TW_CALL BrowseModelButton(void * window)
{
	ModelViewWindow* appWin = (ModelViewWindow*)window;
	if (appWin)
		appWin->OpenModelData();
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
	, m_fZoomMax(20.0f)
	, m_fZoomSen(0.1f)
	, m_pBRDFVisEffect(nullptr)
	, m_bIsLoadTexture(false)
	, m_sBRDFTextureName("None")
	, m_bIsLoadModel(false)
	, m_sModelName("None")
	, m_pModel(nullptr)
	, m_v3LightColor(1.f,1.f,1.f)
	, m_bIsWireFrame(false)
	, m_bIsSceneGUI(true)
	, m_v3ModelPos(0.f, 0.f, 0.f)
	, m_fModelScale(1.0f)
	, m_v3ModelRot()
	, m_fLightIntMultiplier(1.0f)
{
}

ModelViewWindow::~ModelViewWindow()
{
}

int ModelViewWindow::OnInit()
{
	// AntTweakBar Init
	ATB_ASSERT(NPTwInit(m_uiID, TW_OPENGL_CORE, nullptr));
	ATB_ASSERT(TwSetCurrentWindow(m_uiID));
	ATB_ASSERT(TwWindowSize(m_iSizeW, m_iSizeH));
	TwBar* mainBar = TwNewBar("ModelView");
	ATB_ASSERT(TwDefine(" ModelView help='These properties defines the visual appearance of the model' "));

	ATB_ASSERT(TwAddVarRO(mainBar, "brdfname", TW_TYPE_STDSTRING, &m_sBRDFTextureName,
		" label='Loaded BRDF' help='Loaded BRDF' group='BRDF File'"));

	ATB_ASSERT(TwAddButton(mainBar, "openmodel", BrowseModelButton, this, "label='Browse File' group='Model File'"));
	ATB_ASSERT(TwAddVarRO(mainBar, "modelname", TW_TYPE_STDSTRING, &m_sModelName,
		" label='Loaded Model' help='Loaded Model' group='Model File'"));

	ATB_ASSERT(TwAddVarRW(mainBar, "wireframe", TW_TYPE_BOOLCPP, &m_bIsWireFrame,
		" label='Wireframe' help='Show Wireframe' group='Display'"));
	ATB_ASSERT(TwAddVarRW(mainBar, "scenegui", TW_TYPE_BOOLCPP, &m_bIsSceneGUI,
		" label='Scene GUI' help='Show Scene GUI' group='Display'"));

	ATB_ASSERT(TwAddVarRW(mainBar, "PosX", TW_TYPE_FLOAT, &m_v3ModelPos._x,
		" label='Pos X' help='Model Translation' group='Model'"));
	ATB_ASSERT(TwAddVarRW(mainBar, "PosY", TW_TYPE_FLOAT, &m_v3ModelPos._y,
		" label='Pos Y' help='Model Translation' group='Model'"));
	ATB_ASSERT(TwAddVarRW(mainBar, "PosZ", TW_TYPE_FLOAT, &m_v3ModelPos._z,
		" label='Pos Z' help='Model Translation' group='Model'"));
	ATB_ASSERT(TwAddVarRW(mainBar, "Rotation", TW_TYPE_QUAT4F, &m_v3ModelRot._e[0],
		" label='Rotation' help='Model Rotation' group='Model' "));
	ATB_ASSERT(TwAddVarRW(mainBar, "Scale", TW_TYPE_FLOAT, &m_fModelScale,
		" label='Scale' help='Model Scale' group='Model' step=0.1"));

	ATB_ASSERT(TwAddVarRW(mainBar, "Color", TW_TYPE_COLOR3F, &m_v3LightColor, " group='Directional Light' "));
	ATB_ASSERT(TwAddVarRW(mainBar, "Intensity Multiplier", TW_TYPE_FLOAT, &m_fLightIntMultiplier,
		" label='Intensity Multiplier' help='Multiply light color' group='Directional Light' step=0.1"));
	//ATB_ASSERT(TwAddVarRW(mainBar, "Direction", TW_TYPE_DIR3F, &m_f3LightDir, " group='Directional Light' "));

	ATB_ASSERT(TwAddSeparator(mainBar, "instructionsep", ""));
	ATB_ASSERT(TwAddButton(mainBar, "instruction1", NULL, NULL, "label='LClick+Drag: Rot Light dir'"));
	ATB_ASSERT(TwAddButton(mainBar, "instruction2", NULL, NULL, "label='RClick+Drag: Rot Camera dir'"));
	ATB_ASSERT(TwAddButton(mainBar, "instruction3", NULL, NULL, "label='Scroll: Zoom Camera in/out'"));

	m_AxisLine[0].Init(m_pShareContent);
	m_AxisLine[1].Init(m_pShareContent);
	m_AxisLine[2].Init(m_pShareContent);
	m_InLine.Init(m_pShareContent);

	m_pBRDFVisEffect = m_pShareContent->GetEffect("BRDFModelEffect");
	if (!m_pBRDFVisEffect->GetIsLinked())
	{
		m_pBRDFVisEffect->initEffect();
		m_pBRDFVisEffect->attachShaderFromFile("..\\shader\\BRDFModelVS.glsl", GL_VERTEX_SHADER);
		m_pBRDFVisEffect->attachShaderFromFile("..\\shader\\BRDFModelPS.glsl", GL_FRAGMENT_SHADER);
		m_pBRDFVisEffect->linkEffect();
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//OpenModelData();

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
	if (abs(m_fScrollY) > M_EPSILON)
	{
		float curZoom = m_Cam.GetRadius();
		curZoom += m_fScrollY * m_fZoomSen;
		curZoom = (curZoom < m_fZoomMin) ? m_fZoomMin : (curZoom > m_fZoomMax) ? m_fZoomMax : curZoom;
		m_Cam.SetRadius(curZoom);
		m_fScrollY = 0.f;
	}
	m_v2LastCursorPos = m_v2CurrentCursorPos;
	// Camera control - end

	glm::mat4 proj, view, model, tranInvModel;
	proj = glm::perspective(45.0f, (float)m_iSizeW / (float)m_iSizeH, 0.1f, 100.0f);
	view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
	model = glm::rotate(model, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	tranInvModel = glm::transpose(glm::inverse(model));
	NPMathHelper::Mat4x4 myProj = NPMathHelper::Mat4x4::perspectiveProjection(M_PI * 0.5f, (float)m_iSizeW / (float)m_iSizeH, 0.1f, 100.0f);
	NPMathHelper::Mat4x4 modelMat = NPMathHelper::Mat4x4::mul(NPMathHelper::Mat4x4::translation(m_v3ModelPos)
		,NPMathHelper::Mat4x4::mul(NPMathHelper::Mat4x4::rotationTransform(m_v3ModelRot)
		,NPMathHelper::Mat4x4::scaleTransform(m_fModelScale, m_fModelScale, m_fModelScale)));
	NPMathHelper::Mat4x4 tranInvModelMat = NPMathHelper::Mat4x4::transpose(NPMathHelper::Mat4x4::inverse(modelMat));


	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (m_bIsWireFrame)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	UpdateBRDFData();
	if (/*m_bIsLoadTexture &&*/ m_pModel)
	{
		m_pBRDFVisEffect->activeEffect();
		m_pBRDFVisEffect->SetInt("n_th", m_uiNTH);
		m_pBRDFVisEffect->SetInt("n_ph", m_uiNPH);
		m_pBRDFVisEffect->SetMatrix("projection", myProj.GetDataColumnMajor());
		//m_pBRDFVisEffect->SetMatrix("projection", glm::value_ptr(proj));
		m_pBRDFVisEffect->SetMatrix("view", m_Cam.GetViewMatrix());
		//m_pBRDFVisEffect->SetMatrix("view", glm::value_ptr(view));
		//m_pBRDFVisEffect->SetMatrix("model", glm::value_ptr(model));
		//m_pBRDFVisEffect->SetMatrix("tranInvModel", glm::value_ptr(tranInvModel));
		m_pBRDFVisEffect->SetMatrix("model", modelMat.GetDataColumnMajor());
		m_pBRDFVisEffect->SetMatrix("tranInvModel", tranInvModelMat.GetDataColumnMajor());
		glm::vec3 lightDir;
		lightDir.y = -sin(m_fInYaw);
		lightDir.x = -cos(m_fInYaw) * sin(m_fInPitch);
		lightDir.z = -cos(m_fInYaw) * cos(m_fInPitch);
		m_pBRDFVisEffect->SetVec3("lightDir", lightDir.x, lightDir.y, lightDir.z);
		m_pBRDFVisEffect->SetVec3("lightColor", m_v3LightColor.x * m_fLightIntMultiplier
			, m_v3LightColor.y * m_fLightIntMultiplier, m_v3LightColor.z * m_fLightIntMultiplier);
		glm::vec3 camDir = m_Cam.GetDir();
		m_pBRDFVisEffect->SetVec3("viewDir", camDir.x, camDir.y, camDir.z);

		if (m_bIsLoadTexture)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_iBRDFEstTex);
			m_pBRDFVisEffect->SetInt("texture_brdf", 0);
		}

		m_pModel->Draw(*m_pBRDFVisEffect);
		m_pBRDFVisEffect->deactiveEffect();
	}

	if (m_bIsWireFrame)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (m_bIsSceneGUI)
	{
		m_AxisLine[0].Draw(NPGeoHelper::vec3(), NPGeoHelper::vec3(1.f, 0.f, 0.f), NPGeoHelper::vec3(1.0f, 0.f, 0.f)
			, m_Cam.GetViewMatrix(), glm::value_ptr(proj));
		m_AxisLine[1].Draw(NPGeoHelper::vec3(), NPGeoHelper::vec3(0.f, 1.f, 0.f), NPGeoHelper::vec3(0.0f, 1.f, 0.f)
			, m_Cam.GetViewMatrix(), glm::value_ptr(proj));
		m_AxisLine[2].Draw(NPGeoHelper::vec3(), NPGeoHelper::vec3(0.f, 0.f, 1.f), NPGeoHelper::vec3(0.0f, 0.f, 1.f)
			, m_Cam.GetViewMatrix(), glm::value_ptr(proj));
	}

	if (m_bIsSceneGUI)
	{
		glm::vec3 InLineEnd;
		InLineEnd.y = sin(m_fInYaw) * 10.f;
		InLineEnd.x = cos(m_fInYaw) * sin(m_fInPitch) * 10.f;
		InLineEnd.z = cos(m_fInYaw) * cos(m_fInPitch) * 10.f;
		m_InLine.Draw(NPGeoHelper::vec3(), NPGeoHelper::vec3(InLineEnd.x, InLineEnd.y, InLineEnd.z), NPGeoHelper::vec3(1.0f, 1.f, 1.f)
			, m_Cam.GetViewMatrix(), glm::value_ptr(proj));
	}

	ATB_ASSERT(TwSetCurrentWindow(m_uiID));
	ATB_ASSERT(TwDraw());

	glfwSwapBuffers(GetGLFWWindow());

	return 0;
}

void ModelViewWindow::OnTerminate()
{
	testObject.ClearGeometry();
	NPTwTerminate(m_uiID);
}

void ModelViewWindow::OnHandleInputMSG(const INPUTMSG &msg)
{
	ATB_ASSERT(TwSetCurrentWindow(m_uiID));
	switch (msg.type)
	{
	case Window::INPUTMSG_KEYBOARDKEY:
		if (TwEventCharGLFW(msg.key, msg.action))
			break;
		if (msg.key == GLFW_KEY_ESCAPE && msg.action == GLFW_PRESS)
			glfwSetWindowShouldClose(m_pWindow, GL_TRUE);
		//if (msg.key == GLFW_KEY_O && msg.action == GLFW_PRESS)
		//	OpenModelData();
		break;
	case Window::INPUTMSG_MOUSEKEY:
		if (TwEventMouseButtonGLFW(msg.key, msg.action))
			break;
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
		TwEventMousePosGLFW(msg.xpos, msg.ypos);
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

	if (m_pModel)
	{
		delete m_pModel;
		m_pModel = NULL;
	}

	m_pModel = new BRDFModel::Model();
	if (!m_pModel->LoadModel(file.c_str()))
	{
		std::string message = "Cannot load file ";
		message = message + file;
		NPOSHelper::CreateMessageBox(message.c_str(), "Load BRDF Data Failure", NPOSHelper::MSGBOX_OK);
		return;
	}
	m_sModelName = file.c_str();
	m_bIsLoadModel = true;
}

void ModelViewWindow::SetBRDFData(const char* path, unsigned int n_th, unsigned int n_ph)
{
	m_bIsBRDFUpdated = false;
	m_sNewBRDFPath = path;
	m_uiNewTH = n_th;
	m_uiNewPH = n_ph;
}

void ModelViewWindow::UpdateBRDFData()
{
	if (!m_bIsBRDFUpdated)
	{
		m_bIsBRDFUpdated = true;

		if (m_bIsLoadTexture)
		{
			glDeleteTextures(1, &m_iBRDFEstTex);
			m_bIsLoadTexture = false;
		}

		if (m_sNewBRDFPath.length() <= 0)
		{
			m_bIsLoadTexture = false;
			return;
		}

		int width, height;
		if (!NPGLHelper::loadTextureFromFile(m_sNewBRDFPath.c_str(), m_iBRDFEstTex, GL_REPEAT, GL_REPEAT, GL_NEAREST, GL_NEAREST))
		{
			std::string message = "Cannot load file ";
			message = message + m_sNewBRDFPath;
			NPOSHelper::CreateMessageBox(message.c_str(), "Load BRDF Data Failure", NPOSHelper::MSGBOX_OK);
			return;
		}
		m_uiNTH = m_uiNewTH;
		m_uiNPH = m_uiNewPH;
		m_bIsLoadTexture = true;
		m_sBRDFTextureName = m_sNewBRDFPath;
	}
}