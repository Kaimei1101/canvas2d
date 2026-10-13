#ifndef DRAWABLE_H
#define DRAWABLE_H

#include "rendertarget.h"

class Drawable
{
  public:
    virtual void draw(RenderTarget &rt) const = 0;
    virtual ~Drawable() = default;
};

#endif // DRAWABLE_H
