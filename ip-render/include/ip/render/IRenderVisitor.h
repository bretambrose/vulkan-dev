#pragma once

namespace IP
{
namespace Render
{

class ModelInstance;

class IRenderVisitor
{
    public:

        virtual ~IRenderVisitor() {}

        virtual void Visit(IP::Render::ModelInstance* /*modelInstance*/) {}
};

}
}
