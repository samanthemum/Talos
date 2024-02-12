#pragma once
#include "GameObject.h"
#include "Component.h"
#include "Transform.h"
#include "StaticMesh.h"
#include "../config.h"

namespace talos {
	class MeshActor : public GameObject {
	public:
		MeshActor();
		MeshActor(Transform* transform, StaticMesh* staticMesh);

		Transform* getTransform();
		StaticMesh* getStaticMesh();

		void setTransform(Transform transform);
		void setStaticMesh(StaticMesh staticMesh);
	};
}