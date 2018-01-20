#pragma once

namespace IP
{
namespace Render
{

class IRenderVisitor;

class IRenderVisitable
{
    public:
        virtual ~IRenderVisitable() {}

        virtual void HandleVisitor(IRenderVisitor* visitor) = 0;
};

} // namespace Render
} // namespace IP