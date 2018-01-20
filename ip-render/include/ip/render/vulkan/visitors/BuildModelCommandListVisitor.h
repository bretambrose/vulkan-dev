#pragma once

#include <ip/render/IRenderVisitor.h>

namespace IP
{ 
namespace Render
{

class BuildModelCommandListVisitor : public IRenderVisitor
{
    public:

        BuildModelCommandListVisitor();
        virtual ~BuildModelCommandListVisitor();

        virtual void Visit(IP::Render::ModelInstance* modelInstance) override;

    private:

};

} // namespace Render
} // namespace IP
