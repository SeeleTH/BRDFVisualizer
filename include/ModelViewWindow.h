#ifndef MODELVIEWWINDOW_H
#define MODELVIEWWINDOW_H

#include "glhelper.h"
#include "camhelper.h"
#include "mathhelper.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace BRDFModel
{
	struct Vertex {
		NPMathHelper::Vec3 position;
		NPMathHelper::Vec3 normal;
		NPMathHelper::Vec3 tangent;
		NPMathHelper::Vec2 texCoords;
	};

	struct Texture {
		GLuint id;
		std::string name;
		unsigned int type;
	};

	class Mesh {
	public:
		std::vector<Vertex> m_vertices;
		std::vector<GLuint> m_indices;
		std::vector<Texture> m_textures;

		Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures)
			: m_vertices(vertices)
			, m_indices(indices)
			, m_textures(textures)
		{
			SetupMesh();
		}

		void Draw(NPGLHelper::Effect &effect);

	protected:
		GLuint m_iVAO;
		GLuint m_iVBO;
		GLuint m_iEBO;

		void SetupMesh();
	};

	class Model {
	public:
		Model();

		void Draw(NPGLHelper::Effect &effect);
		bool LoadModel(const char* path);

	protected:
		void ProcessNode(aiNode* node, const aiScene* scene);
		Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
		std::vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, const char* name, const unsigned int typeId);

		std::vector<Mesh> m_meshes;
		std::string m_sDirectory;
	};
}

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
	void SetBRDFData(const char* path);

protected:
	GLuint m_iBRDFEstTex;
	BRDFModel::Model* m_pModel;
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