/*
    This file is part of Dirt, the Dartmouth introductory ray tracer.
    Copyright (c) 2017-2019 by Wojciech Jarosz
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
#pragma once

#include <dirt/integrator.h>
#include <dirt/medium.h>
#include <dirt/scene.h>
#include <dirt/pdf.h>

class VolpathTracerUni: public Integrator
{
public:
    VolpathTracerUni(const json& j = json::object())
    {
        m_maxBounces = j.value("max_bounces", m_maxBounces);
    }

    void attachMedium(const HitInfo &hit, Ray3f &ray) const
    {
        if (hit.mat == nullptr && hit.mi->IsMediumTransition())
            ray.medium = hit.mi->getMedium(ray, hit);
        ray.maxt = length(hit.p - ray.o) / length(ray.d) + Epsilon; 
    }

	Color3f MIS(const AllProb& t, const Color3f& c, SamplingColor samplingColor) const {
		//cout << (1.0f / ((1.0f / 3.0f) * (t.pr_fr + t.pg_fr + t.pb_fr))) << endl;

		Color3f toReturn;
		
		if(t.hitMedium) {
			toReturn = Color3f(
				(1.0f / ((1.0f / 3.0f) * (t.pr_fr + t.pg_fr + t.pb_fr))) * c.r,
				(1.0f / ((1.0f / 3.0f) * (t.pr_fg + t.pg_fg + t.pb_fg))) * c.g,
				(1.0f / ((1.0f / 3.0f) * (t.pr_fb + t.pg_fb + t.pb_fb))) * c.b
			);
		}
		else {
			switch(samplingColor) {
				case red:
					toReturn.r *= 3.0f * c.r * t.pr_fr;
					toReturn.g = 0;
					toReturn.b = 0;
					return toReturn;
				case green:
					toReturn.r = 0;
					toReturn.g *= 3.0f * c.g * t.pg_fg;
					toReturn.b = 0;
					return toReturn;
				case blue:
					toReturn.r = 0;
					toReturn.g = 0;
					toReturn.b *= 3.0f * c.b * t.pb_fb;
					return toReturn;
			}
		}
		//cout << c << " " << toReturn << endl;
		return toReturn;
	}

    virtual Color3f Li(const Scene & scene, Sampler &sampler, const Ray3f& ray_) const override
    {
        Ray3f ray(ray_);
        //Color3f throughput(1.f);
        Color3f result(0.f);

        int bounces = 0;

        float epsilon = sampler.next1D();
        SamplingColor color;
        if(epsilon < 1.0f / 3.0f)
            color = red;
        else if(epsilon < 2.0f / 3.0f)
            color = green;
        else
            color = blue;
        AllProb throughput(1.0f);

        while (bounces <= m_maxBounces)
        {
            HitInfo hit;
            bool foundIntersection = scene.intersect(ray, hit);
            if (foundIntersection) attachMedium(hit, ray);

            MediumInteraction mi;
            if (ray.medium) {
                throughput *= ray.medium->Sample(ray, sampler, mi, color);
                //cout << throughput << endl;
            }

            if (mi.isValid())
            {
				throughput.hitMedium = true;
                // sample direction (from phase)
                Vec3f wi;
                float phasePdf = mi.medium->phase->sample(mi.wo, wi, sampler.next2D());
                throughput *= 1.0f / (mi.medium->phase->p(mi.wo, wi) / phasePdf);

                // spawn new ray
                ray = Ray3f(mi.p, wi).withMedium(ray.medium);
                bounces++;
            }
            else
            {
                if (!foundIntersection)
                {
                    result += MIS(throughput, scene.background(), color);
					//cout << "B"<<scene.background() << endl;
                    break;
                }

                if (hit.mat == nullptr)
                {
                    ray = Ray3f(hit.p, normalize(ray.d));
                    continue;
                }

                result += MIS(throughput, hit.mat->emitted(ray, hit), color);
				//cout << "E"<<hit.mat->emitted(ray, hit) << endl;

                // sample direction (from BSDF)
                ScatterRecord srec;
                if (!hit.mat->sample(ray.d, hit, sampler.next2D(), srec)) {
					break;
				}
                Ray3f scat(hit.p, srec.scattered);
                if (!srec.isSpecular) 
                {
                    float bsdfPdf = hit.mat->pdf(ray.d, scat.d, hit);
                    if (bsdfPdf == 0.0f) break;
					//cout << throughput;
                    throughput *= 1.0f / ((hit.mat->eval(ray.d, scat.d, hit) / bsdfPdf));
					//cout << " s " << (hit.mat->eval(ray.d, scat.d, hit) / bsdfPdf) << " " << throughput << endl;
                }
                else
                {
                    throughput *= 1.0f / (srec.attenuation);
                }

                ray = scat;
                bounces++;
            }
        }

        return result;
    }

private:
    int m_maxBounces = 64;
};
