#include <ip/render/model/ModelLibrary.h>

#include <ip/render/model/Model.h>

namespace IP
{
namespace Render
{

ModelLibrary::ModelLibrary()
{
}

ModelLibrary::~ModelLibrary()
{
}

void ModelLibrary::LoadModels(const IP::String& manifestPath)
{
    IP_UNREFERENCED_PARAM(manifestPath);
/*
    m_modelsByName.clear();

    const auto& models = Model::DebugCreateModels();
    for (const auto& model : models)
    {
        m_modelsByName[model->GetName()] = model;
    }*/
}

std::shared_ptr<IP::Render::Model> ModelLibrary::GetModelByName(const IP::String& modelName) const
{
    IP_UNREFERENCED_PARAM(modelName);
    /*
    const auto& iter = m_modelsByName.find(modelName);
    if (iter == m_modelsByName.cend())
    {
        return nullptr;
    }

    return iter->second;
     */

    return nullptr;
}

} // namespace Render
} // namespace IP