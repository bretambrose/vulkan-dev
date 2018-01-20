#include <ip/render/model/Model.h>

namespace IP
{
namespace Render
{

Model::Model() :
    m_name("")
{
}

Model::~Model()
{
}

std::shared_ptr<IP::Render::Model> Model::LoadModel(IP::IStream& stream)
{
    IP_UNREFERENCED_PARAM(stream);

    return nullptr;
}

IP::Vector<std::shared_ptr<IP::Render::Model>> Model::DebugCreateModels()
{
    IP::Vector<std::shared_ptr<IP::Render::Model>> debugModels;

    return debugModels;
}


} // namespace Render
} // namespace IP