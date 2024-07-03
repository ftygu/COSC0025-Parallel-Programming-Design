#include <dirt/sampler.h>
#include <dirt/primetable.h>

namespace
{
  auto g_defaultSampler = std::make_shared<IndependentSampler>(json::object());
}

shared_ptr<Sampler> Sampler::defaultSampler()
{
  return g_defaultSampler;
}

void Sampler::startPixel()
{
  current1DDimension = 0;
  current2DDimension = 0;
  currentPixelSample = 0;
}

bool Sampler::startNextPixelSample()
{
  current1DDimension = 0;
  current2DDimension = 0;
  currentPixelSample++;
  currentGlobalSample++;
  return currentPixelSample < samplesPerPixel;
}

Vec2f Sampler::next2D()
{
  return Vec2f(next1D(), next1D());
}

IndependentSampler::IndependentSampler(const json &j) {}

float IndependentSampler::next1D()
{
  return randf();
}

float StratifiedSampler::next1D()
{
  if (current1DDimension < samples1D.size() && currentPixelSample < samplesPerPixel)
    return samples1D[current1DDimension++][currentPixelSample];
  else
    return randf();
}

Vec2f StratifiedSampler::next2D()
{
  if (current2DDimension < samples2D.size() && currentPixelSample < samplesPerPixel)
    return samples2D[current2DDimension++][currentPixelSample];
  else
    return Vec2f(randf(), randf());
}

StratifiedSampler::StratifiedSampler(const json &j)
{
  // samplesPerPixel must be a perfect square (e.g. 1, 4, 9, 16, etc)
  samplesPerPixel = roundToPerfectSquare(j.value("image_samples", 4));
  dimension = j.value("dimension", 4);
  
  // initialize samples1D and samples2D arrays
  samples1D.resize(dimension);
  samples2D.resize(dimension);

  for (size_t i = 0; i < dimension; i++)
  {
    samples1D[i] = std::vector<float>(samplesPerPixel);
    samples2D[i] = std::vector<Vec2f>(samplesPerPixel);
  }
}

void StratifiedSampler::startPixel()
{
  for (size_t i = 0; i < samples1D.size(); i++)
  {
    stratifiedSample1D(samples1D[i]);
    shuffle<float>(samples1D[i]);
  }

  for (size_t i = 0; i < samples2D.size(); i++)
  {
    stratifiedSample2D(samples2D[i]);
    shuffle<Vec2f>(samples2D[i]);
  }

  Sampler::startPixel();
}

void StratifiedSampler::stratifiedSample1D(std::vector<float> &samples)
{
  float invNSamples = 1.0f / samples.size();
  for (size_t x = 0; x < samples.size(); x++)
  {
    samples[x] = std::min((x + randf()) * invNSamples, ONE_MINUS_EPSILON);
  }
}

void StratifiedSampler::stratifiedSample2D(std::vector<Vec2f> &samples)
{
  int i = 0;
  float sqrtN = std::sqrt(samples.size());
  float invSqrtN = float(1.0f) / sqrtN;
  for (int y = 0; y < sqrtN; y++)
  {
    for (int x = 0; x < sqrtN; x++)
    {
      samples[i].x = std::min((x + randf()) * invSqrtN, ONE_MINUS_EPSILON);
      samples[i++].y = std::min((y + randf()) * invSqrtN, ONE_MINUS_EPSILON);
    }
  }
}

HaltonSampler::HaltonSampler(const json &j)
{
  dimension = j.value("dimension", 4);
  if (dimension > 0)
    dimension = std::min(dimension, (size_t)(PrimeTableSize));
  else
    dimension = PrimeTableSize;

  perms.resize(dimension);
  for (size_t i = 0; i < dimension; i++)
  {
    int base = Primes[i];
    perms[i] = std::vector<uint64_t>(base);
    for (int j = 0; j < base; j++)
      perms[i][j] = j;
    shuffle<uint64_t>(perms[i]);
  }
}

float HaltonSampler::scrambledRadicalInverse(const std::vector<uint64_t> &perm, uint64_t a, uint64_t base)
{
  const double invBase = float(1) / float(base);
  uint64_t reversedDigits = 0;
  double invBaseN = 1;
  while (a) {
    uint64_t next  = a / base;
    uint64_t digit = a - next * base;
    reversedDigits = reversedDigits * base + perm[digit];
    invBaseN *= invBase;
    a = next;
  }
  return std::min(
    invBaseN * (reversedDigits + invBase * perm[0] / (1 - invBase)),
    (double)(ONE_MINUS_EPSILON)
  );
}

float HaltonSampler::next1D()
{
  if (current1DDimension > dimension)
    return randf();
  int base = Primes[current1DDimension];
  std::vector<uint64_t> perm = perms[current1DDimension++];
  return scrambledRadicalInverse(perm, currentGlobalSample, base);
}
