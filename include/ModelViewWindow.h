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
	struct SphericalSpace {
		glm::vec3 m_v3Center;
		float m_fRadius;

		SphericalSpace Merge(const SphericalSpace &other)
		{
			SphericalSpace result;
			glm::vec3 toOther = other.m_v3Center - m_v3Center;
			float lenToOther = glm::length(toOther);
			float maxForward = glm::max(m_fRadius, other.m_fRadius + lenToOther);
			float maxBackward = glm::max(m_fRadius, other.m_fRadius - lenToOther);
			glm::vec3 toOtherDir = glm::normalize(toOther);
			result.m_fRadius = (maxForward + maxBackward) * 0.5f;
			result.m_v3Center = m_v3Center + toOtherDir * (maxForward - maxBackward * 0.5f);
			return result;
		}
	};
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

		~Mesh()
		{
			for (auto &texture : m_textures)
			{
				glDeleteTextures(1, &texture.id);
			}
			glDeleteVertexArrays(1,&m_iVAO);
			glDeleteBuffers(1,&m_iVBO);
			glDeleteBuffers(1,&m_iEBO);
		}

		void Draw(NPGLHelper::Effect &effect);

	protected:
		GLuint m_iVAO;
		GLuint m_iVBO;
		GLuint m_iEBO;

		SphericalSpace m_space;

		void SetupMesh();
	};

	class Model {
	public:
		Model();
		~Model() 
		{
			for (auto &mesh : m_meshes)
			{
				if (mesh)
					delete mesh;
			}
			m_meshes.clear(); 
		}

		void Draw(NPGLHelper::Effect &effect);
		bool LoadModel(const char* path);

	protected:
		void ProcessNode(aiNode* node, const aiScene* scene);
		Mesh* ProcessMesh(aiMesh* mesh, const aiScene* scene);
		std::vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, const char* name, const unsigned int typeId);

		std::vector<Mesh*> m_meshes;
		std::string m_sDirectory;

		SphericalSpace m_space;
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
	void SetBRDFData(const char* path, unsigned int n_th, unsigned int n_ph);

protected:
	void UpdateBRDFData();

	bool m_bIsBRDFUpdated;
	std::string m_sNewBRDFPath;
	unsigned int m_uiNewTH;
	unsigned int m_uiNewPH;

	bool m_bIsLoadModel;
	BRDFModel::Model* m_pModel;
	std::string m_sModelName;

	bool m_bIsLoadTexture;
	GLuint m_iBRDFEstTex;
	std::string m_sBRDFTextureName;

	NPGLHelper::Effect* m_pBRDFVisEffect;
	NPGLHelper::RenderObject testObject;
	NPCamHelper::RotateCamera m_Cam;

	bool m_bIsWireFrame;
	bool m_bIsSceneGUI;
	unsigned int m_uiNTH, m_uiNPH;
	glm::vec3 m_v3LightColor;
	float m_fLightIntMultiplier;
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

	//Model
	NPMathHelper::Vec3 m_v3ModelPos;
	float m_fModelScale;
	NPMathHelper::Quat m_v3ModelRot;
};

#endif