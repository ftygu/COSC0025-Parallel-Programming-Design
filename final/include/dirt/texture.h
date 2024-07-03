#pragma once

#include <dirt/perlin.h>

#include <dirt/fwd.h>
#include <dirt/parser.h>
#include <dirt/image.h>

class Texture
{
public:
	/// Return a pointer to a global default texture
	static shared_ptr<Texture> defaultTexture();

    virtual ~Texture() = default;

    virtual Color3f value(const HitInfo & hit) const = 0;
};

class ConstantTexture : public Texture
{
public:
    ConstantTexture(const Color3f & c) : color(c) {}
    ConstantTexture(const json & j = json::object());

    Color3f value(const HitInfo & hit) const override { return color; }
    Color3f color;
};

class CheckerTexture : public Texture
{
public:
    CheckerTexture() {}
    CheckerTexture(shared_ptr<const Texture> t0, shared_ptr<const Texture> t1) : even(t0), odd(t1) {}
    CheckerTexture(const json & j = json::object());

    Color3f value(const HitInfo & hit) const override;

    float scale = 1.f;
    shared_ptr<const Texture> even;
    shared_ptr<const Texture> odd;
};

class MarbleTexture : public Texture
{
public:
    MarbleTexture(const json & j = json::object());
    Color3f value(const HitInfo & hit) const override;

    Perlin noise;
    float scale = 1.f;
    shared_ptr<const Texture> veins;
    shared_ptr<const Texture> base;
};

class ImageTexture : public Texture
{
public:
    ImageTexture(const json & j = json::object());
    Color3f value(const HitInfo & hit) const override;

    Image3f tex;
};
