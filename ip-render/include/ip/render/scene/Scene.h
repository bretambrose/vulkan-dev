#pragma once

#include <ip/core/memory/stl/Vector.h>

namespace IP
{
namespace Render
{

class ModelInstance;
class IRenderVisitor;

class Scene
{
    public:

        Scene();
        ~Scene();

        void VisitScene(IP::Render::IRenderVisitor* visitor);
        void AddModel(const std::shared_ptr<IP::Render::ModelInstance>& modelInstance);

    private:

        IP::Vector<std::shared_ptr<IP::Render::ModelInstance>> m_modelInstances;
        
};

} // namespace Render
} // namespace IP


