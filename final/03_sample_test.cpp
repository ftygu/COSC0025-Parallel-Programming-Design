/*
This file is part of Dirt, the Dartmouth introductory ray tracer, used in
Dartmouth's COSC 77/177 Computer Graphics course.
Copyright (c) 2018 by Wojciech Jarosz
Dirt is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License Version 3
as published by the Free Software Foundation.
Dirt is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <dirt/material.h>
#include <dirt/surface.h>
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

    void runTest(shared_ptr<Material> mat, Vec3f normal, std::string name);
};

int main(int argc, char** argv)
{
    auto lambertian = parseMaterial(json{{"type", "lambertian"}, {"albedo", 1.0f}});
    //auto phong = parseMaterial(json{{"type", "phong"}, {"albedo", 1.0f}, {"exponent", 10.0f}});
    //auto blinnphong = parseMaterial(json{{"type", "blinnphong"}, {"albedo", 1.0f}, {"exponent", 10.0f}});

    Vec3f normal1 = Vec3f(0.0f, 0.0f, 1.0f);
    Vec3f normal2 = normalize(Vec3f(0.25f, 0.5f, 1.0f));

    SampleTester tester;
    tester.runTest(lambertian, normal1, "lambertian");
    //tester.runTest(     phong, normal1, "phong"     );
    //tester.runTest(blinnphong, normal1, "blinnphong");
    
    //tester.runTest(lambertian, normal2, "rotated-lambertian");
    //tester.runTest(     phong, normal2, "rotated-phong"     );
    //tester.runTest(blinnphong, normal2, "rotated-blinnphong");
    
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

void SampleTester::runTest(shared_ptr<Material> mat, Vec3f normal, std::string name)
{
    std::cout << "---------------------------------------------------------------------------" << std::endl;
    std::cout << "Running sample test for \"" << name << '"' << std::endl;
    // Set up fake hit record and initial direction
    HitInfo hit(1.0f, Vec3f(0.0f), normal, normal, Vec2f(0.5f));
    Vec3f dirIn = normalize(Vec3f(0.25f, 0.0f, -1.0f));

    // Step 1: Evaluate pdf over the sphere
    Array2d<float> pdf(ImageW, ImageH);
    for (int y = 0; y < ImageH; ++y)
        for (int x = 0; x < ImageW; ++x)
            pdf(x, y) = mat->pdf(dirIn, pixelToSpherical(Vec2i(x, y)), hit);

    // Step 2: Generate histogram of samples
    const int HistoSubsample = 4; // Merge adjacent pixels to decrease noise
    Array2d<float> histogram(ImageW/HistoSubsample, ImageH/HistoSubsample);

    int validSamples = 0;
    bool isSpecularSet = false;
    bool belowHemisphere = false;
    bool nanOrInf = false;
    for (int i = 0; i < NumSamples; ++i) {
        // Sample material
        ScatterRecord record;
	Vec2f sample(randf(), randf());
        if (!mat->sample(dirIn, hit, sample, record))
            continue;

        if (record.isSpecular)
            isSpecularSet = true;
        
        // Sanity check to make sure directions are valid
        Vec3f dirOut = normalize(record.scattered);
        if (dot(dirOut, hit.sn) < 0.0f) {
            belowHemisphere = true;
            continue;
        }
        if (std::isnan(dirOut.x + dirOut.y + dirOut.z) || std::isinf(dirOut.x + dirOut.y + dirOut.z)) {
            nanOrInf = true;
            continue;
        }

        // Map scattered direction to pixel in our sample histogram
        Vec2i pixel = sphericalToPixel(record.scattered)/HistoSubsample;
        if (pixel.x < 0 || pixel.y < 0 || pixel.x >= histogram.width() || pixel.y >= histogram.height())
            continue;

        // Incorporate Jacobian of spherical mapping and bin area into the sample weight
        float sinTheta = std::sqrt(max(1.0f - dirOut.z*dirOut.z, 0.0f));
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
    if (isSpecularSet)
        std::cout << "WARNING: isSpecular is set. It should not be." << std::endl;
    if (belowHemisphere)
        std::cout << "WARNING: Some generated directions were below the hemisphere. "
                     "You should check for this case and return false from sample instead" << std::endl;
    if (nanOrInf)
        std::cout << "WARNING: Some directions/PDFs contained invalid values (NaN or infinity). This should not happen. Make sure you catch all corner cases in your code" << std::endl;
}
