#include "MeshActor.h"

namespace talos {
	MeshActor::MeshActor() {
		Transform* transform = new Transform();
		addComponent(transform);

		StaticMesh* staticMesh = new StaticMesh();
		addComponent(staticMesh);
	}

	MeshActor::MeshActor(Transform* transform, StaticMesh* staticMesh) {
		addComponent(transform);
		addComponent(staticMesh);
	}

	Transform* MeshActor::getTransform() {
		std::vector<Component*> transformComponents = getComponents("Transform");

		return (Transform*)(transformComponents[0]);
	}

	StaticMesh* MeshActor::getStaticMesh() {
		std::vector<Component*> transformComponents = getComponents("StaticMesh");

		return (StaticMesh*)(transformComponents[0]);
	}

	void MeshActor::setTransform(Transform transform) {
		Transform* currentTransform = getTransform();
		*currentTransform = transform;
	}

	void MeshActor::setStaticMesh(StaticMesh staticMesh) {
		StaticMesh* currentStaticMesh = getStaticMesh();
		*currentStaticMesh = staticMesh;
	}
}
