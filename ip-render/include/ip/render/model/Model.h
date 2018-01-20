#pragma once

#include <ip/core/memory/stl/Stream.h>
#include <ip/core/memory/stl/String.h>
#include <ip/core/memory/stl/Vector.h>

namespace IP
{
namespace Render
{

class Model
{
    public:

        Model();
        ~Model();
       
       const IP::String& GetName() const { return m_name; }

       static std::shared_ptr<IP::Render::Model> LoadModel(IP::IStream& stream);
       static IP::Vector<std::shared_ptr<IP::Render::Model>> DebugCreateModels();

    private:

        IP::String m_name;
};

} // namespace Render
} // namespace IP


