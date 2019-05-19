#pragma once

#include <ip/core/memory/stl/String.h>
#include <ip/core/memory/stl/UnorderedMap.h>

namespace IP
{
namespace Render
{

class Model;

class ModelLibrary
{
    public:

        ModelLibrary();
        ~ModelLibrary();

        void LoadModels(const IP::String& manifestPath);

        std::shared_ptr<IP::Render::Model> GetModelByName(const IP::String& modelName) const;

    private:

        // IP::UnorderedMap<IP::String, std::shared_ptr<IP::Render::Model>> m_modelsByName;
       
};

} // namespace Render
} // namespace IP


