#include <ip/render/vulkan/visitors/BuildModelCommandListVisitor.h>

#include <ip/core/UnreferencedParam.h>

namespace IP
{
namespace Render
{

BuildModelCommandListVisitor::BuildModelCommandListVisitor()
{
}

BuildModelCommandListVisitor::~BuildModelCommandListVisitor()
{
}

void BuildModelCommandListVisitor::Visit(IP::Render::ModelInstance* modelInstance)
{
    IP_UNREFERENCED_PARAM(modelInstance);
}

} // namespace Render
} // namespace IP
