#pragma once

#include <ip/render/IRenderVisitable.h>

#include <memory>

namespace IP
{
namespace Render
{

class IRenderVisitor;
class Model;

class ModelInstance : public IRenderVisitable
{
    public:
        ModelInstance(const std::shared_ptr<IP::Render::Model>& model);
        ~ModelInstance();

        virtual void HandleVisitor(IRenderVisitor* visitor) override;

    private:

        std::shared_ptr<IP::Render::Model> m_model;
};

} // namespace Render
} // namespace IP