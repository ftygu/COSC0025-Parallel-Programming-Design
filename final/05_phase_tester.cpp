#include <dirt/medium.h>
#include <dirt/common.h>
#include <dirt/parser.h>
#include <dirt/image.h>

#include <algorithm>

class SampleTester
{
    const int ImageW = 512;
    const int ImageH = 256;
    const int NumSamples = 50*ImageW*ImageH;

    Vec3f uvToSpherical(Vec2f uv) const;
    Vec2f sphericalToUv(Vec3f dir) const;
    Vec2i sphericalToPixel(Vec3f dir) const;
    Vec3f pixelToSpherical(Vec2i pixel) const;

public:
    SampleTester() = default;

    void runTest(shared_ptr<PhaseFunction> mat, Vec3f normal, std::string name);
};

int main(int argc, char** argv)
{
    auto hg0 = parsePhase(json{{"type", "hg"}, {"g", 0.0f}});
    auto hg0p5 = parsePhase(json{{"type", "hg"}, {"g", 0.5f}});
    auto hg0p9 = parsePhase(json{{"type", "hg"}, {"g", 0.9f}});

    Vec3f wo1 = Vec3f(0.0f, 0.0f, 1.0f);
    Vec3f wo2 = normalize(Vec3f(0.25f, 0.5f, 1.0f));

    SampleTester tester;
    tester.runTest(hg0, wo1, "hg0");
    tester.runTest(hg0p5, wo1, "hg0p5");
    tester.runTest(hg0p9, wo1, "hg0p9");
    tester.runTest(hg0p5, wo2, "rotated-hg0p5");
    tester.runTest(hg0p9, wo2, "rotated-hg0p9");

    return 0;
}

Color3f colorRamp(float value)
{
    Color3f colors[] = {
        Vec3f( 20.0f,  11.0f,  52.0f)/256.0f,
        Vec3f(132.0f,  32.0f, 107.0f)/256.0f,
        Vec3f(229.0f,  91.0f,  48.0f)/256.0f,
        Vec3f(246.0f, 215.0f,  70.0f)/256.0f
    };
    value *= 3.0f;
    int i = clamp(int(value), 0, 2);
    float u = value - i;

    return colors[i] + (colors[i + 1] - colors[i])*u;
}

Image3f generateHeatmap(const Array2d<float> &density, float maxValue)
{
    Image3f result(density.width(), density.height());

    for (int y = 0; y < density.height(); ++y)
        for (int x = 0; x < density.width(); ++x)
            result(x, y) = colorRamp(density(x, y)/maxValue);

    return std::move(result);
}

Vec3f SampleTester::uvToSpherical(Vec2f uv) const
{
    float phi = uv.x*2.0f*M_PI;
    float theta = uv.y*M_PI;
    return Vec3f(std::cos(phi)*std::sin(theta), std::sin(phi)*std::sin(theta), std::cos(theta));
}

Vec2f SampleTester::sphericalToUv(Vec3f dir) const
{
    float theta = std::acos(clamp(dir.z, -1.0f, 1.0f));
    float phi = std::atan2(dir.y, dir.x);
    if (phi < 0.0f) phi += 2.0f*M_PI;
    return Vec2f(phi/(2.0f*M_PI), theta/M_PI);
}

Vec2i SampleTester::sphericalToPixel(Vec3f dir) const
{
    Vec2f pixelF = sphericalToUv(dir)*Vec2f(ImageW, ImageH);
    return Vec2i(pixelF.x, pixelF.y);
}

Vec3f SampleTester::pixelToSpherical(Vec2i pixel) const
{
    return uvToSpherical((Vec2f(pixel.x, pixel.y) + 0.5f)/Vec2f(ImageW, ImageH));
}

void SampleTester::runTest(shared_ptr<PhaseFunction> phase, Vec3f wo, std::string name)
{
    std::cout << "---------------------------------------------------------------------------" << std::endl;
    std::cout << "Running sample test for \"" << name << '"' << std::endl;
    // Step 1: Evaluate pdf over the sphere
    Array2d<float> pdf(ImageW, ImageH);
    for (int y = 0; y < ImageH; ++y)
        for (int x = 0; x < ImageW; ++x)
            pdf(x, y) = phase->p(wo, pixelToSpherical(Vec2i(x, y)));

    // Step 2: Generate histogram of samples
    const int HistoSubsample = 4; // Merge adjacent pixels to decrease noise
    Array2d<float> histogram(ImageW/HistoSubsample, ImageH/HistoSubsample);

    int validSamples = 0;
    bool nanOrInf = false;
    for (int i = 0; i < NumSamples; ++i) {
	Vec3f wi;
	Vec2f sample(randf(), randf());
        phase->sample(wo, wi, sample);

        if (std::isnan(wi.x + wi.y + wi.z) || std::isinf(wi.x + wi.y + wi.z)) {
            nanOrInf = true;
            continue;
        }

        // Map scattered direction to pixel in our sample histogram
        Vec2i pixel = sphericalToPixel(wi)/HistoSubsample;
        if (pixel.x < 0 || pixel.y < 0 || pixel.x >= histogram.width() || pixel.y >= histogram.height())
            continue;

        // Incorporate Jacobian of spherical mapping and bin area into the sample weight
        float sinTheta = std::sqrt(max(1.0f - wi.z*wi.z, 0.0f));
        float weight = (histogram.width()*histogram.height())/(M_PI*(2.0f*M_PI)*NumSamples*sinTheta);
        // Accumulate into histogram
        histogram(pixel.x, pixel.y) += weight;
        validSamples++;
    }

    // Now upscale our histogram
    Array2d<float> histoFullres(ImageW, ImageH);
    for (int y = 0; y < ImageH; ++y)
        for (int x = 0; x < ImageW; ++x)
            histoFullres(x, y) = histogram(x/HistoSubsample, y/HistoSubsample);

    // Step 3: Compute pdf statistics: Maximum value and integral
    float maxValue = 0.0f;
    for (int y = 0; y < ImageH; ++y) {
        for (int x = 0; x < ImageW; ++x) {
            maxValue = max(maxValue, pdf(x, y));
            if (std::isnan(pdf(x, y)) || std::isinf(pdf(x, y)))
                nanOrInf = true;
        }
    }

    double integral = 0.0f;
    for (int y = 0; y < ImageH; ++y) {
        for (int x = 0; x < ImageW; ++x) {
            Vec3f dir = pixelToSpherical(Vec2i(x, y));
            float sinTheta = std::sqrt(max(1.0f - dir.z*dir.z, 0.0f));
            float pixelArea = (M_PI/ImageW)*(M_PI*2.0f/ImageH)*sinTheta;
            integral += pixelArea*pdf(x, y);
        }
    }

    // Generate heat maps
    generateHeatmap(         pdf, maxValue).save(name + "-pdf.png"    );
    generateHeatmap(histoFullres, maxValue).save(name + "-sampled.png");

    // Output statistics
    std::cout << "Integral of PDF (should be close to 1): " << integral << std::endl;
    std::cout << (validSamples*100)/NumSamples << "% of samples were valid (this should be close to 100%)" << std::endl;
    if (nanOrInf)
        std::cout << "WARNING: Some directions/PDFs contained invalid values (NaN or infinity). This should not happen. Make sure you catch all corner cases in your code" << std::endl;
} 
