#pragma once

#include <dirt/parser.h>
#include <dirt/fwd.h>

class Sampler
{
public:
  // default to an independent sampler
	static shared_ptr<Sampler> defaultSampler();

  virtual ~Sampler() = default;

  /**
  *  Call when starting to evaluate a new pixel, resets various counters.
  *  Derived classes can override this function and use it to pre-generate samples for a pixel.
  */
  virtual void startPixel();

 /**
  *  Call when starting to evaluate a new sample for the same pixel, increments and resets various counters.
  */
  virtual bool startNextPixelSample();

  /**
   * Generate a single random number.
   */
  virtual float next1D() = 0;

  /**
  * Genearte a pair of random numbers. If a sampler does not provide a next2D(), the base clase
  * will call the derived class's next1D() twice and return the pair.
  * 
  * Allows the next1D() to increment the current1DDimension, doesn't touch current2DDimension
  */
  virtual Vec2f next2D();

  // the number of samples **of** each pixel
  size_t samplesPerPixel; 

  // the number of sampels **for** each pixel (used to sample camera offsets, directions, etc)
  size_t dimension;

protected:
  // how many samples **of** the current pixel have been evalated
  size_t currentPixelSample = 0;

  // how many samples of the entire image have been evaluated (in any pixel)
  size_t currentGlobalSample = 0;

  // how many 1D samples have been generated for the current pixel
  size_t current1DDimension = 0;

  // how many 2D samples have been generated for the current pixel
  size_t current2DDimension = 0;
};

class IndependentSampler : public Sampler
{
public:
  IndependentSampler(const json &j);

  float next1D() override;
};

class StratifiedSampler : public Sampler
{
public:
  StratifiedSampler(const json &j);

  void startPixel() override;

  float next1D() override;

  Vec2f next2D() override;

private:
  void stratifiedSample1D(std::vector<float> &samples);

  void stratifiedSample2D(std::vector<Vec2f> &samples);

  // indexed as samples[dimIndex][pixelSampleIndex]
  std::vector<std::vector<float>> samples1D;
  std::vector<std::vector<Vec2f>> samples2D;
};

class HaltonSampler: public Sampler
{
public:
  HaltonSampler(const json &j);

private:
 
  static float scrambledRadicalInverse(const std::vector<uint64_t> &perm, uint64_t a, uint64_t base);

  float next1D() override;

  std::vector<std::vector<uint64_t>> perms;
};
