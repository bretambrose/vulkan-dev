#include <ip/render/scene/Scene.h>

#include <ip/render/model/ModelInstance.h>

namespace IP
{
namespace Render
{

Scene::Scene() :
    m_modelInstances()
{
}

Scene::~Scene()
{
}

void Scene::VisitScene(IP::Render::IRenderVisitor* visitor)
{
    for (const auto& modelInstance : m_modelInstances)
    {
        modelInstance->HandleVisitor(visitor);
    }
}

void Scene::AddModel(const std::shared_ptr<IP::Render::ModelInstance>& modelInstance)
{
    m_modelInstances.push_back(modelInstance);
}

} // namespace Render
} // namespace IP