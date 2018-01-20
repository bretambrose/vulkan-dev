#include <ip/render/model/ModelInstance.h>

#include <ip/render/IRenderVisitor.h>

namespace IP
{
namespace Render
{

ModelInstance::ModelInstance(const std::shared_ptr<IP::Render::Model>& model) :
    m_model(model)
{
}

ModelInstance::~ModelInstance()
{
}

void ModelInstance::HandleVisitor(IRenderVisitor* visitor)
{
    visitor->Visit(this);
}

} // namespace Render
} // namespace IP